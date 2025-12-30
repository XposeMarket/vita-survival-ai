#ifndef ONLINE_SEARCH_H
#define ONLINE_SEARCH_H

#include <string>
#include <vector>
#include "net_fetcher.h"
#include "rss_parser.h"
#include "content_extractor.h"
#include "database.h"

// Online search result
struct OnlineResult {
    std::string url;
    std::string title;
    std::string snippet;
    std::string source;
    time_t published;
    float relevance;
};

// Search settings
struct OnlineSearchSettings {
    bool enabled;
    int maxResults;
    int timeoutSeconds;
    bool saveAutomatically;
    int cacheSizeLimitMB;
    std::vector<std::string> enabledFeeds;
};

class OnlineSearch {
public:
    OnlineSearch();
    ~OnlineSearch();
    
    void Initialize(NetFetcher* net, RSSParser* rss, 
                   ContentExtractor* extractor, Database* db);
    
    // Main online search flow
    bool SearchAndSave(const std::string& query, std::vector<VaultItem>& outItems);
    
    // Component operations
    std::vector<OnlineResult> SearchRSSFeeds(const std::string& query, int limit = 10);
    bool FetchAndExtract(const std::string& url, VaultItem& outItem);
    bool SaveToVault(const VaultItem& item);
    
    // Batch operations
    bool FetchMultipleAndSave(const std::vector<std::string>& urls, 
                             std::vector<VaultItem>& outItems);
    
    // Settings
    void LoadSettings(const std::string& configPath);
    void SaveSettings(const std::string& configPath);
    OnlineSearchSettings GetSettings() const { return settings; }
    void SetSettings(const OnlineSearchSettings& s) { settings = s; }
    
    // Status
    bool IsOnline();
    int GetCachedItemsCount();
    int GetCacheSizeMB();
    
    // Cache management
    void PruneOldCache(int daysOld = 30);
    void ClearCache();
    bool CheckCacheSizeLimit();
    
private:
    NetFetcher* netFetcher;
    RSSParser* rssParser;
    ContentExtractor* extractor;
    Database* database;
    
    OnlineSearchSettings settings;
    
    // Deduplication
    bool IsDuplicate(const VaultItem& item);
    std::string GenerateItemHash(const std::string& url, const std::string& title, time_t published);
    
    // Filtering
    std::vector<OnlineResult> FilterAndRank(const std::vector<OnlineResult>& results,
                                           const std::string& query);
    float CalculateRelevance(const OnlineResult& result, const std::string& query);
    
    // RSS search
    std::vector<OnlineResult> SearchFeed(const FeedConfig& feed, const std::string& query);
    bool FeedItemMatches(const RSSItem& item, const std::string& query);
};

#endif // ONLINE_SEARCH_H
