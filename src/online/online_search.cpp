#include "online_search.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

#include <psp2/kernel/threadmgr.h>

OnlineSearch::OnlineSearch() : netFetcher(nullptr), rssParser(nullptr),
                               extractor(nullptr), database(nullptr) {
    // Default settings
    settings.enabled = true;
    settings.maxResults = 10;
    settings.timeoutSeconds = 30;
    settings.saveAutomatically = true;
    settings.cacheSizeLimitMB = 100;
}

OnlineSearch::~OnlineSearch() {
}

void OnlineSearch::Initialize(NetFetcher* net, RSSParser* rss,
                              ContentExtractor* ext, Database* db) {
    netFetcher = net;
    rssParser = rss;
    extractor = ext;
    database = db;
}

bool OnlineSearch::SearchAndSave(const std::string& query, std::vector<VaultItem>& outItems) {
    if (!settings.enabled || !IsOnline()) {
        return false;
    }
    
    // Step 1: Search RSS feeds for relevant items
    auto results = SearchRSSFeeds(query, settings.maxResults);
    if (results.empty()) {
        return false;
    }
    
    // Step 2: Fetch and extract content from top results
    int fetchedCount = 0;
    for (const auto& result : results) {
        if (fetchedCount >= settings.maxResults) break;
        
        VaultItem item;
        if (FetchAndExtract(result.url, item)) {
            // Check if duplicate
            if (!IsDuplicate(item)) {
                if (SaveToVault(item)) {
                    outItems.push_back(item);
                    fetchedCount++;
                }
            }
        }
        
        // Rate limiting - small delay between fetches
        sceKernelDelayThread(2000000);  // 2 seconds
    }
    
    // Step 3: Check cache size limit
    if (settings.cacheSizeLimitMB > 0) {
        CheckCacheSizeLimit();
    }
    
    return !outItems.empty();
}

std::vector<OnlineResult> OnlineSearch::SearchRSSFeeds(const std::string& query, int limit) {
    std::vector<OnlineResult> allResults;
    
    if (!netFetcher || !rssParser) return allResults;
    
    // Get configured feeds
    auto feeds = rssParser->GetConfiguredFeeds();
    
    // Sort by priority
    std::sort(feeds.begin(), feeds.end(),
        [](const FeedConfig& a, const FeedConfig& b) {
            return a.priority > b.priority;
        });
    
    // Search each enabled feed
    for (const auto& feed : feeds) {
        if (!feed.enabled) continue;
        
        auto feedResults = SearchFeed(feed, query);
        allResults.insert(allResults.end(), feedResults.begin(), feedResults.end());
        
        if (allResults.size() >= limit * 2) break;  // Get more than needed for filtering
    }
    
    // Filter and rank
    auto filtered = FilterAndRank(allResults, query);
    
    // Limit to max results
    if (filtered.size() > limit) {
        filtered.resize(limit);
    }
    
    return filtered;
}

std::vector<OnlineResult> OnlineSearch::SearchFeed(const FeedConfig& feed, const std::string& query) {
    std::vector<OnlineResult> results;
    
    // Fetch feed
    auto fetchResult = netFetcher->FetchURL(feed.url, settings.timeoutSeconds);
    if (!fetchResult.success) {
        return results;
    }
    
    // Parse feed
    RSSFeed parsedFeed;
    if (!rssParser->ParseFeed(fetchResult.html, parsedFeed)) {
        return results;
    }
    
    // Filter items by query
    for (const auto& item : parsedFeed.items) {
        if (FeedItemMatches(item, query)) {
            OnlineResult result;
            result.url = item.link;
            result.title = item.title;
            result.snippet = item.description;
            result.source = feed.name;
            result.published = item.pubDate;
            result.relevance = CalculateRelevance(result, query);
            
            results.push_back(result);
        }
    }
    
    return results;
}

bool OnlineSearch::FeedItemMatches(const RSSItem& item, const std::string& query) {
    // Convert to lowercase for matching
    std::string lowerTitle = item.title;
    std::string lowerDesc = item.description;
    std::string lowerQuery = query;
    
    std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
    std::transform(lowerDesc.begin(), lowerDesc.end(), lowerDesc.begin(), ::tolower);
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    // Simple keyword matching
    std::istringstream iss(lowerQuery);
    std::string keyword;
    int matches = 0;
    int keywords = 0;
    
    while (iss >> keyword) {
        keywords++;
        if (lowerTitle.find(keyword) != std::string::npos ||
            lowerDesc.find(keyword) != std::string::npos) {
            matches++;
        }
    }
    
    // Require at least 50% keyword match
    return keywords > 0 && (float)matches / keywords >= 0.5f;
}

bool OnlineSearch::FetchAndExtract(const std::string& url, VaultItem& outItem) {
    if (!netFetcher || !extractor) return false;
    
    // Fetch URL
    auto fetchResult = netFetcher->FetchURL(url, settings.timeoutSeconds);
    if (!fetchResult.success) {
        return false;
    }
    
    // Extract content
    auto content = extractor->Extract(fetchResult.html, url);
    
    // Skip if paywall detected
    if (content.hasPaywall) {
        return false;
    }
    
    // Generate ID
    outItem.id = GenerateItemHash(url, content.title, content.publishDate);
    outItem.title = content.title;
    outItem.url = url;
    outItem.source_domain = content.domain;
    outItem.author = content.author;
    outItem.published_at = content.publishDate;
    outItem.retrieved_at = time(nullptr);
    outItem.text_snippet = content.snippet;
    outItem.text_clean = content.mainText;
    outItem.language = content.language;
    outItem.content_type = "article";
    
    // Convert quotes to JSON (simplified)
    std::string quotesJson = "[";
    for (size_t i = 0; i < content.quotes.size(); i++) {
        if (i > 0) quotesJson += ",";
        quotesJson += "\"" + content.quotes[i] + "\"";
    }
    quotesJson += "]";
    outItem.quotes_json = quotesJson;
    
    return true;
}

bool OnlineSearch::SaveToVault(const VaultItem& item) {
    if (!database) return false;
    
    return database->InsertItem(item);
}

bool OnlineSearch::FetchMultipleAndSave(const std::vector<std::string>& urls,
                                       std::vector<VaultItem>& outItems) {
    for (const auto& url : urls) {
        VaultItem item;
        if (FetchAndExtract(url, item)) {
            if (!IsDuplicate(item)) {
                if (SaveToVault(item)) {
                    outItems.push_back(item);
                }
            }
        }
        
        // Rate limiting
        sceKernelDelayThread(2000000);  // 2 seconds between fetches
    }
    
    return !outItems.empty();
}

bool OnlineSearch::IsDuplicate(const VaultItem& item) {
    if (!database) return false;
    
    // Check if item with same ID already exists
    VaultItem existing;
    return database->GetItemById(item.id, existing);
}

std::string OnlineSearch::GenerateItemHash(const std::string& url,
                                          const std::string& title,
                                          time_t published) {
    // Simple hash generation (should use proper hash function)
    std::stringstream ss;
    ss << url << "_" << title << "_" << published;
    
    // Take first 16 chars of combined string as hash
    std::string combined = ss.str();
    if (combined.length() > 16) {
        combined = combined.substr(0, 16);
    }
    
    // Replace non-alphanumeric with underscores
    for (char& c : combined) {
        if (!isalnum(c)) c = '_';
    }
    
    return combined;
}

std::vector<OnlineResult> OnlineSearch::FilterAndRank(const std::vector<OnlineResult>& results,
                                                      const std::string& query) {
    auto filtered = results;
    
    // Sort by relevance
    std::sort(filtered.begin(), filtered.end(),
        [](const OnlineResult& a, const OnlineResult& b) {
            return a.relevance > b.relevance;
        });
    
    // Remove very low relevance items
    filtered.erase(
        std::remove_if(filtered.begin(), filtered.end(),
            [](const OnlineResult& r) { return r.relevance < 0.3f; }),
        filtered.end()
    );
    
    return filtered;
}

float OnlineSearch::CalculateRelevance(const OnlineResult& result, const std::string& query) {
    float score = 0.0f;
    
    std::string lowerTitle = result.title;
    std::string lowerSnippet = result.snippet;
    std::string lowerQuery = query;
    
    std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
    std::transform(lowerSnippet.begin(), lowerSnippet.end(), lowerSnippet.begin(), ::tolower);
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    // Count keyword matches
    std::istringstream iss(lowerQuery);
    std::string keyword;
    int keywords = 0;
    int titleMatches = 0;
    int snippetMatches = 0;
    
    while (iss >> keyword) {
        keywords++;
        if (lowerTitle.find(keyword) != std::string::npos) titleMatches++;
        if (lowerSnippet.find(keyword) != std::string::npos) snippetMatches++;
    }
    
    if (keywords > 0) {
        score = (titleMatches * 0.6f + snippetMatches * 0.4f) / keywords;
    }
    
    // Boost recent items
    time_t now = time(nullptr);
    int ageInDays = (now - result.published) / (24 * 3600);
    if (ageInDays < 7) score *= 1.2f;      // Boost last week
    else if (ageInDays < 30) score *= 1.1f; // Boost last month
    
    return std::min(score, 1.0f);
}

bool OnlineSearch::IsOnline() {
    return netFetcher && netFetcher->IsOnline();
}

int OnlineSearch::GetCachedItemsCount() {
    return database ? database->GetTotalItems() : 0;
}

int OnlineSearch::GetCacheSizeMB() {
    // Would need to calculate actual storage size
    // For now, estimate based on item count
    return GetCachedItemsCount() / 10;  // Rough estimate: ~10 items per MB
}

void OnlineSearch::PruneOldCache(int daysOld) {
    // Would implement pruning logic here
    // Delete items older than N days
}

void OnlineSearch::ClearCache() {
    // Would implement cache clearing
}

bool OnlineSearch::CheckCacheSizeLimit() {
    if (GetCacheSizeMB() > settings.cacheSizeLimitMB) {
        PruneOldCache(30);  // Prune items older than 30 days
        return true;
    }
    return false;
}

void OnlineSearch::LoadSettings(const std::string& configPath) {
    // Would load settings from file
}

void OnlineSearch::SaveSettings(const std::string& configPath) {
    // Would save settings to file
}
