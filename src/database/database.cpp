#include "database.h"
#include <cstring>
#include <sstream>
#include <iomanip>

Database::Database() : db(nullptr), isOpen(false), 
    stmt_insert(nullptr), stmt_search(nullptr), stmt_get_by_id(nullptr) {
}

Database::~Database() {
    Close();
}

bool Database::Initialize(const std::string& dbPath) {
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        return false;
    }
    
    isOpen = true;
    
    // Enable FTS5
    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr);
    
    return PrepareStatements();
}

void Database::Close() {
    if (isOpen) {
        FinalizeStatements();
        sqlite3_close(db);
        isOpen = false;
    }
}

bool Database::CreateTables() {
    const char* sql = R"(
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
        );
        
        CREATE TABLE IF NOT EXISTS topics (
            topic_id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT UNIQUE NOT NULL,
            query_rules TEXT,
            whitelist_sources TEXT,
            last_updated INTEGER
        );
        
        CREATE INDEX IF NOT EXISTS idx_items_domain ON items(source_domain);
        CREATE INDEX IF NOT EXISTS idx_items_retrieved ON items(retrieved_at);
        CREATE INDEX IF NOT EXISTS idx_items_published ON items(published_at);
    )";
    
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }
    
    return true;
}

bool Database::CreateFTSIndex() {
    const char* sql = R"(
        CREATE VIRTUAL TABLE IF NOT EXISTS items_fts USING fts5(
            title,
            text_snippet,
            text_clean,
            quotes_json,
            topic_tags,
            content='items',
            content_rowid='rowid'
        );
        
        CREATE TRIGGER IF NOT EXISTS items_ai AFTER INSERT ON items BEGIN
            INSERT INTO items_fts(rowid, title, text_snippet, text_clean, quotes_json, topic_tags)
            VALUES (new.rowid, new.title, new.text_snippet, new.text_clean, new.quotes_json, new.topic_tags);
        END;
        
        CREATE TRIGGER IF NOT EXISTS items_ad AFTER DELETE ON items BEGIN
            INSERT INTO items_fts(items_fts, rowid, title, text_snippet, text_clean, quotes_json, topic_tags)
            VALUES('delete', old.rowid, old.title, old.text_snippet, old.text_clean, old.quotes_json, old.topic_tags);
        END;
        
        CREATE TRIGGER IF NOT EXISTS items_au AFTER UPDATE ON items BEGIN
            INSERT INTO items_fts(items_fts, rowid, title, text_snippet, text_clean, quotes_json, topic_tags)
            VALUES('delete', old.rowid, old.title, old.text_snippet, old.text_clean, old.quotes_json, old.topic_tags);
            INSERT INTO items_fts(rowid, title, text_snippet, text_clean, quotes_json, topic_tags)
            VALUES (new.rowid, new.title, new.text_snippet, new.text_clean, new.quotes_json, new.topic_tags);
        END;
    )";
    
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }
    
    return true;
}

bool Database::InsertItem(const VaultItem& item) {
    if (!stmt_insert) return false;
    
    sqlite3_reset(stmt_insert);
    sqlite3_bind_text(stmt_insert, 1, item.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt_insert, 2, item.title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt_insert, 3, item.url.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt_insert, 4, item.source_domain.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt_insert, 5, item.author.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt_insert, 6, item.published_at);
    sqlite3_bind_int64(stmt_insert, 7, item.retrieved_at);
    sqlite3_bind_text(stmt_insert, 8, item.topic_tags.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt_insert, 9, item.text_snippet.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt_insert, 10, item.text_clean.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt_insert, 11, item.quotes_json.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt_insert, 12, item.language.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt_insert, 13, item.content_type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt_insert, 14, item.license_note.c_str(), -1, SQLITE_TRANSIENT);
    
    int rc = sqlite3_step(stmt_insert);
    return (rc == SQLITE_DONE);
}

std::vector<SearchResult> Database::SearchFTS(const std::string& query, int limit) {
    std::vector<SearchResult> results;
    
    std::string sql = "SELECT items.*, rank FROM items_fts "
                     "JOIN items ON items.rowid = items_fts.rowid "
                     "WHERE items_fts MATCH ? "
                     "ORDER BY rank LIMIT ?";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return results;
    }
    
    sqlite3_bind_text(stmt, 1, query.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, limit);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        SearchResult result;
        
        result.item.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        result.item.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        result.item.url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        result.item.source_domain = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        result.item.author = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        result.item.published_at = sqlite3_column_int64(stmt, 5);
        result.item.retrieved_at = sqlite3_column_int64(stmt, 6);
        result.item.topic_tags = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        result.item.text_snippet = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        result.item.text_clean = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        result.item.quotes_json = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        result.item.language = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        result.item.content_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
        result.item.license_note = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
        
        result.score = sqlite3_column_double(stmt, 14);
        
        results.push_back(result);
    }
    
    sqlite3_finalize(stmt);
    return results;
}

std::vector<SearchResult> Database::SearchQuotes(const std::string& person, 
                                                  const std::string& topic, 
                                                  int limit) {
    std::vector<SearchResult> results;
    
    // Build query for quotes about person
    std::string query = person;
    if (!topic.empty()) {
        query += " " + topic;
    }
    
    // Search with priority for transcripts and direct quotes
    std::string sql = "SELECT items.*, rank FROM items_fts "
                     "JOIN items ON items.rowid = items_fts.rowid "
                     "WHERE items_fts MATCH ? "
                     "AND (content_type = 'transcript' OR content_type = 'statement' "
                     "     OR quotes_json LIKE ?) "
                     "ORDER BY rank, "
                     "CASE content_type "
                     "  WHEN 'transcript' THEN 1 "
                     "  WHEN 'statement' THEN 2 "
                     "  ELSE 3 "
                     "END "
                     "LIMIT ?";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return results;
    }
    
    sqlite3_bind_text(stmt, 1, query.c_str(), -1, SQLITE_TRANSIENT);
    std::string quoteLike = "%" + person + "%";
    sqlite3_bind_text(stmt, 2, quoteLike.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, limit);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        SearchResult result;
        
        result.item.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        result.item.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        result.item.url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        result.item.source_domain = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        result.item.author = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        result.item.published_at = sqlite3_column_int64(stmt, 5);
        result.item.retrieved_at = sqlite3_column_int64(stmt, 6);
        result.item.topic_tags = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        result.item.text_snippet = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        result.item.text_clean = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        result.item.quotes_json = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        result.item.language = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        result.item.content_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
        result.item.license_note = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
        
        result.score = sqlite3_column_double(stmt, 14);
        
        results.push_back(result);
    }
    
    sqlite3_finalize(stmt);
    return results;
}

int Database::GetTotalItems() {
    const char* sql = "SELECT COUNT(*) FROM items";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return 0;
    }
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

bool Database::GetItemById(const std::string& id, VaultItem& item) {
    if (!db) return false;

    sqlite3_stmt* stmt;
    const char* sql = "SELECT id, title, url, source_domain, author, published_at, retrieved_at, topic_tags, text_snippet, text_clean, quotes_json, language, content_type, license_note FROM items WHERE id = ?";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        item.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        item.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        item.url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        item.source_domain = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        item.author = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        item.published_at = sqlite3_column_int64(stmt, 5);
        item.retrieved_at = sqlite3_column_int64(stmt, 6);
        item.topic_tags = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        item.text_snippet = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        item.text_clean = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        item.quotes_json = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        item.language = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        item.content_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
        item.license_note = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
        found = true;
    }

    sqlite3_finalize(stmt);
    return found;
}

bool Database::PrepareStatements() {
    const char* insertSql = R"(
        INSERT OR REPLACE INTO items 
        (id, title, url, source_domain, author, published_at, retrieved_at, 
         topic_tags, text_snippet, text_clean, quotes_json, language, 
         content_type, license_note)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    if (sqlite3_prepare_v2(db, insertSql, -1, &stmt_insert, nullptr) != SQLITE_OK) {
        return false;
    }
    
    return true;
}

void Database::FinalizeStatements() {
    if (stmt_insert) sqlite3_finalize(stmt_insert);
    if (stmt_search) sqlite3_finalize(stmt_search);
    if (stmt_get_by_id) sqlite3_finalize(stmt_get_by_id);
}

bool Database::Vacuum() {
    return (sqlite3_exec(db, "VACUUM;", nullptr, nullptr, nullptr) == SQLITE_OK);
}

bool Database::OptimizeFTS() {
    return (sqlite3_exec(db, "INSERT INTO items_fts(items_fts) VALUES('optimize');", 
                        nullptr, nullptr, nullptr) == SQLITE_OK);
}
