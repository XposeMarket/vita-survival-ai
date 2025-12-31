#ifndef RSS_PARSER_H
#define RSS_PARSER_H

#include <string>
#include <vector>
#include <ctime>

// RSS feed item
struct RSSItem {
    std::string title;
    std::string link;
    std::string description;
    std::string author;
    time_t pubDate;
    std::string guid;
};

// RSS feed metadata
struct RSSFeed {
    std::string title;
    std::string link;
    std::string description;
    time_t lastBuildDate;
    std::vector<RSSItem> items;
};

// Feed configuration
struct FeedConfig {
    std::string name;
    std::string url;
    std::string category;
    bool enabled;
    int priority;  // Higher = fetch first
};

class RSSParser {
public:
    RSSParser();
    ~RSSParser();
    
    // Parse RSS/Atom feeds
    bool ParseRSS(const std::string& xmlContent, RSSFeed& outFeed);
    bool ParseAtom(const std::string& xmlContent, RSSFeed& outFeed);
    
    // Auto-detect format
    bool ParseFeed(const std::string& xmlContent, RSSFeed& outFeed);
    
    // Feed configuration
    bool LoadFeedConfig(const std::string& configPath);
    bool SaveFeedConfig(const std::string& configPath);
    std::vector<FeedConfig> GetConfiguredFeeds() const { return feeds; }
    
    // Add/remove feeds
    void AddFeed(const FeedConfig& feed);
    void RemoveFeed(const std::string& url);
    void EnableFeed(const std::string& url, bool enable);
    
    // Get feeds by category
    std::vector<FeedConfig> GetFeedsByCategory(const std::string& category);
    
private:
    std::vector<FeedConfig> feeds;
    
    // XML parsing helpers
    std::string ExtractTag(const std::string& xml, const std::string& tag);
    std::vector<std::string> ExtractAllTags(const std::string& xml, const std::string& tag);
    std::string StripTags(const std::string& html);
    time_t ParseRFC822Date(const std::string& dateStr);
    time_t ParseISO8601Date(const std::string& dateStr);
};

#endif // RSS_PARSER_H
