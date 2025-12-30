#!/usr/bin/env python3
"""
PC Collector for Vita Survival AI
Collects, processes, and packages web references for offline access
"""

import argparse
import hashlib
import json
import os
import sqlite3
import time
from datetime import datetime
from typing import List, Dict, Optional
from urllib.parse import urlparse

# Optional imports (install with: pip install requests beautifulsoup4 readability-lxml feedparser)
try:
    import requests
    from bs4 import BeautifulSoup
    from readability import Document
    import feedparser
    HAS_DEPS = True
except ImportError:
    HAS_DEPS = False
    print("Warning: Required dependencies not installed")
    print("Install with: pip install requests beautifulsoup4 readability-lxml feedparser")


class VaultCollector:
    def __init__(self, output_dir: str):
        self.output_dir = output_dir
        self.db_path = os.path.join(output_dir, "vault.sqlite")
        self.items_dir = os.path.join(output_dir, "items")
        self.conn = None
        
        # Create directories
        os.makedirs(self.items_dir, exist_ok=True)
        
    def init_database(self):
        """Initialize SQLite database with schema"""
        self.conn = sqlite3.connect(self.db_path)
        cursor = self.conn.cursor()
        
        # Create tables
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS items (
                id TEXT PRIMARY KEY,
                title TEXT NOT NULL,
                url TEXT,
                source_domain TEXT,
                author TEXT,
                published_at INTEGER,
                retrieved_at INTEGER NOT NULL,
                topic_tags TEXT,
                text_snippet TEXT,
                text_clean TEXT,
                quotes_json TEXT,
                language TEXT DEFAULT 'en',
                content_type TEXT,
                license_note TEXT
            )
        """)
        
        # Create FTS5 index
        cursor.execute("""
            CREATE VIRTUAL TABLE IF NOT EXISTS items_fts USING fts5(
                title,
                text_snippet,
                text_clean,
                quotes_json,
                topic_tags,
                content='items',
                content_rowid='rowid'
            )
        """)
        
        # Create triggers for FTS
        cursor.execute("""
            CREATE TRIGGER IF NOT EXISTS items_ai AFTER INSERT ON items BEGIN
                INSERT INTO items_fts(rowid, title, text_snippet, text_clean, quotes_json, topic_tags)
                VALUES (new.rowid, new.title, new.text_snippet, new.text_clean, new.quotes_json, new.topic_tags);
            END
        """)
        
        self.conn.commit()
        
    def fetch_url(self, url: str) -> Optional[Dict]:
        """Fetch and parse a URL"""
        if not HAS_DEPS:
            print("Cannot fetch - dependencies not installed")
            return None
            
        try:
            headers = {
                'User-Agent': 'VitaSurvivalAI-Collector/1.0 (Educational/Research)'
            }
            
            response = requests.get(url, headers=headers, timeout=30)
            response.raise_for_status()
            
            # Extract main content using readability
            doc = Document(response.text)
            
            # Parse with BeautifulSoup for additional extraction
            soup = BeautifulSoup(response.text, 'html.parser')
            
            # Extract metadata
            title = doc.title() or soup.find('title').text if soup.find('title') else url
            
            # Get author
            author = None
            author_meta = soup.find('meta', attrs={'name': 'author'}) or \
                         soup.find('meta', property='article:author')
            if author_meta:
                author = author_meta.get('content')
            
            # Get publish date
            published_at = None
            date_meta = soup.find('meta', property='article:published_time') or \
                       soup.find('meta', attrs={'name': 'date'})
            if date_meta:
                date_str = date_meta.get('content')
                try:
                    published_at = int(datetime.fromisoformat(date_str.replace('Z', '+00:00')).timestamp())
                except:
                    pass
            
            # Clean text
            text_clean = doc.summary()
            soup_clean = BeautifulSoup(text_clean, 'html.parser')
            text_clean = soup_clean.get_text(separator='\n', strip=True)
            
            # Create snippet (first 500 chars)
            text_snippet = text_clean[:500] + "..." if len(text_clean) > 500 else text_clean
            
            # Extract quotes (simplified - look for quotation marks)
            quotes = []
            for paragraph in soup_clean.find_all('p'):
                text = paragraph.get_text()
                if '"' in text or '"' in text:
                    quotes.append(text.strip())
            
            return {
                'url': url,
                'title': title.strip(),
                'author': author,
                'published_at': published_at,
                'text_snippet': text_snippet,
                'text_clean': text_clean,
                'quotes': quotes,
                'domain': urlparse(url).netloc
            }
            
        except Exception as e:
            print(f"Error fetching {url}: {e}")
            return None
    
    def add_item(self, data: Dict, tags: List[str] = None, content_type: str = "article"):
        """Add item to vault"""
        if not self.conn:
            self.init_database()
        
        # Generate ID from URL
        item_id = hashlib.sha256(data['url'].encode()).hexdigest()[:16]
        
        # Prepare data
        retrieved_at = int(time.time())
        topic_tags = ','.join(tags) if tags else ''
        quotes_json = json.dumps(data.get('quotes', [])) if data.get('quotes') else ''
        
        # Insert into database
        cursor = self.conn.cursor()
        cursor.execute("""
            INSERT OR REPLACE INTO items 
            (id, title, url, source_domain, author, published_at, retrieved_at,
             topic_tags, text_snippet, text_clean, quotes_json, content_type)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            item_id,
            data['title'],
            data['url'],
            data['domain'],
            data.get('author'),
            data.get('published_at'),
            retrieved_at,
            topic_tags,
            data['text_snippet'],
            data['text_clean'],
            quotes_json,
            content_type
        ))
        
        self.conn.commit()
        
        # Save text file
        text_file = os.path.join(self.items_dir, f"{item_id}.txt")
        with open(text_file, 'w', encoding='utf-8') as f:
            f.write(f"Title: {data['title']}\n")
            f.write(f"URL: {data['url']}\n")
            f.write(f"Retrieved: {datetime.fromtimestamp(retrieved_at).isoformat()}\n")
            f.write("\n" + data['text_clean'])
        
        # Save JSON metadata
        json_file = os.path.join(self.items_dir, f"{item_id}.json")
        with open(json_file, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2)
        
        print(f"Added: {data['title']}")
        
    def collect_from_rss(self, feed_url: str, tags: List[str] = None, limit: int = 10):
        """Collect items from RSS feed"""
        if not HAS_DEPS:
            print("Cannot collect from RSS - dependencies not installed")
            return
            
        print(f"Fetching RSS feed: {feed_url}")
        feed = feedparser.parse(feed_url)
        
        count = 0
        for entry in feed.entries[:limit]:
            if count >= limit:
                break
                
            url = entry.link
            print(f"\nProcessing: {url}")
            
            data = self.fetch_url(url)
            if data:
                self.add_item(data, tags=tags)
                count += 1
                time.sleep(2)  # Rate limiting
        
        print(f"\nCollected {count} items from RSS feed")
    
    def collect_from_urls(self, urls: List[str], tags: List[str] = None):
        """Collect items from list of URLs"""
        for url in urls:
            print(f"\nProcessing: {url}")
            data = self.fetch_url(url)
            if data:
                self.add_item(data, tags=tags)
                time.sleep(2)  # Rate limiting
    
    def optimize_database(self):
        """Optimize database for better performance"""
        if not self.conn:
            return
            
        print("\nOptimizing database...")
        cursor = self.conn.cursor()
        cursor.execute("VACUUM")
        cursor.execute("INSERT INTO items_fts(items_fts) VALUES('optimize')")
        self.conn.commit()
        print("Database optimized")
    
    def get_stats(self):
        """Get vault statistics"""
        if not self.conn:
            return
            
        cursor = self.conn.cursor()
        cursor.execute("SELECT COUNT(*) FROM items")
        total = cursor.fetchone()[0]
        
        cursor.execute("SELECT COUNT(DISTINCT source_domain) FROM items")
        domains = cursor.fetchone()[0]
        
        print(f"\nVault Statistics:")
        print(f"Total items: {total}")
        print(f"Unique domains: {domains}")
        print(f"Database: {self.db_path}")
        print(f"Items directory: {self.items_dir}")
    
    def close(self):
        """Close database connection"""
        if self.conn:
            self.conn.close()


def main():
    parser = argparse.ArgumentParser(description='PC Collector for Vita Survival AI')
    parser.add_argument('--output', '-o', default='vault_pack', help='Output directory')
    parser.add_argument('--urls', '-u', nargs='+', help='URLs to collect')
    parser.add_argument('--rss', '-r', help='RSS feed URL')
    parser.add_argument('--tags', '-t', nargs='+', help='Tags to apply')
    parser.add_argument('--limit', '-l', type=int, default=10, help='Limit for RSS items')
    parser.add_argument('--stats', '-s', action='store_true', help='Show stats only')
    
    args = parser.parse_args()
    
    collector = VaultCollector(args.output)
    collector.init_database()
    
    if args.stats:
        collector.get_stats()
    elif args.urls:
        collector.collect_from_urls(args.urls, tags=args.tags)
        collector.optimize_database()
        collector.get_stats()
    elif args.rss:
        collector.collect_from_rss(args.rss, tags=args.tags, limit=args.limit)
        collector.optimize_database()
        collector.get_stats()
    else:
        parser.print_help()
    
    collector.close()


if __name__ == '__main__':
    main()
