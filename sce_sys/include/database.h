#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <ctime>
#include "sqlite3.h"

struct VaultItem {
    std::string id;
    std::string title;
    std::string url;
    std::string source_domain;
    std::string author;
    time_t published_at;
    time_t retrieved_at;
    std::string topic_tags;
    std::string text_snippet;
    std::string text_clean;
    std::string quotes_json;
    std::string language;
    std::string content_type;
    std::string license_note;
    float relevance_score;
};

struct SearchResult {
    VaultItem item;
    float score;
    std::vector<std::string> matched_snippets;
};

class Database {
public:
    Database();
    ~Database();
    
    bool Initialize(const std::string& dbPath);
    void Close();
    
    // Schema creation
    bool CreateTables();
    bool CreateFTSIndex();
    
    // Item operations
    bool InsertItem(const VaultItem& item);
    bool GetItemById(const std::string& id, VaultItem& item);
    bool DeleteItem(const std::string& id);
    bool UpdateItem(const VaultItem& item);
    
    // Search operations
    std::vector<SearchResult> SearchFTS(const std::string& query, int limit = 10);
    std::vector<SearchResult> SearchByTag(const std::string& tag, int limit = 10);
    std::vector<SearchResult> SearchByAuthor(const std::string& author, int limit = 10);
    std::vector<SearchResult> SearchQuotes(const std::string& person, const std::string& topic = "", int limit = 10);
    
    // Stats
    int GetTotalItems();
    std::vector<std::string> GetAllTags();
    time_t GetLastUpdated();
    
    // Maintenance
    bool Vacuum();
    bool OptimizeFTS();
    
private:
    sqlite3* db;
    bool isOpen;
    
    // Prepared statements (for performance)
    sqlite3_stmt* stmt_insert;
    sqlite3_stmt* stmt_search;
    sqlite3_stmt* stmt_get_by_id;
    
    bool PrepareStatements();
    void FinalizeStatements();
    
    std::string EscapeString(const std::string& str);
};

#endif // DATABASE_H
