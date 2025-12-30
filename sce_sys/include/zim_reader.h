#ifndef ZIM_READER_H
#define ZIM_READER_H

#include <string>
#include <vector>

// Simplified ZIM article structure
struct ZIMArticle {
    std::string url;
    std::string title;
    std::string content;
    std::string mimeType;
    bool isRedirect;
};

// ZIM search result
struct ZIMSearchResult {
    std::string title;
    std::string url;
    std::string snippet;
    int relevance;
};

class ZIMReader {
public:
    ZIMReader();
    ~ZIMReader();
    
    bool LoadZIM(const std::string& zimPath);
    void Close();
    
    // Article access
    bool GetArticleByUrl(const std::string& url, ZIMArticle& article);
    bool GetMainPage(ZIMArticle& article);
    
    // Search
    std::vector<ZIMSearchResult> SearchArticles(const std::string& query, int limit = 20);
    std::vector<std::string> GetSuggestions(const std::string& prefix, int limit = 10);
    
    // Info
    std::string GetTitle();
    std::string GetDescription();
    int GetArticleCount();
    
    bool IsLoaded() const { return isLoaded; }
    
private:
    void* zimFile; // Opaque pointer to ZIM file handle
    bool isLoaded;
    std::string currentZimPath;
    
    // Cache for frequently accessed articles
    struct CacheEntry {
        std::string url;
        ZIMArticle article;
        time_t timestamp;
    };
    std::vector<CacheEntry> articleCache;
    static const int MAX_CACHE_SIZE = 50;
    
    void AddToCache(const std::string& url, const ZIMArticle& article);
    bool GetFromCache(const std::string& url, ZIMArticle& article);
    void ClearOldCache();
};

#endif // ZIM_READER_H
