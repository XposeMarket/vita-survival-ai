#include "rss_parser.h"
#include <algorithm>
#include <sstream>
#include <fstream>

RSSParser::RSSParser() {
}

RSSParser::~RSSParser() {
}

bool RSSParser::ParseFeed(const std::string& xmlContent, RSSFeed& outFeed) {
    // Auto-detect RSS vs Atom
    if (xmlContent.find("<rss") != std::string::npos) {
        return ParseRSS(xmlContent, outFeed);
    } else if (xmlContent.find("<feed") != std::string::npos) {
        return ParseAtom(xmlContent, outFeed);
    }
    return false;
}

bool RSSParser::ParseRSS(const std::string& xmlContent, RSSFeed& outFeed) {
    // Extract channel metadata
    outFeed.title = ExtractTag(xmlContent, "title");
    outFeed.link = ExtractTag(xmlContent, "link");
    outFeed.description = ExtractTag(xmlContent, "description");
    
    // Extract items
    std::vector<std::string> itemXmls = ExtractAllTags(xmlContent, "item");
    
    for (const auto& itemXml : itemXmls) {
        RSSItem item;
        item.title = StripTags(ExtractTag(itemXml, "title"));
        item.link = ExtractTag(itemXml, "link");
        item.description = StripTags(ExtractTag(itemXml, "description"));
        item.author = ExtractTag(itemXml, "author");
        if (item.author.empty()) {
            item.author = ExtractTag(itemXml, "dc:creator");
        }
        item.guid = ExtractTag(itemXml, "guid");
        
        // Parse date
        std::string pubDateStr = ExtractTag(itemXml, "pubDate");
        if (pubDateStr.empty()) {
            pubDateStr = ExtractTag(itemXml, "dc:date");
        }
        item.pubDate = ParseRFC822Date(pubDateStr);
        
        outFeed.items.push_back(item);
    }
    
    return !outFeed.items.empty();
}

bool RSSParser::ParseAtom(const std::string& xmlContent, RSSFeed& outFeed) {
    // Extract feed metadata
    outFeed.title = ExtractTag(xmlContent, "title");
    
    std::string linkTag = ExtractTag(xmlContent, "link");
    size_t hrefPos = linkTag.find("href=\"");
    if (hrefPos != std::string::npos) {
        size_t start = hrefPos + 6;
        size_t end = linkTag.find("\"", start);
        outFeed.link = linkTag.substr(start, end - start);
    }
    
    outFeed.description = ExtractTag(xmlContent, "subtitle");
    
    // Extract entries
    std::vector<std::string> entryXmls = ExtractAllTags(xmlContent, "entry");
    
    for (const auto& entryXml : entryXmls) {
        RSSItem item;
        item.title = StripTags(ExtractTag(entryXml, "title"));
        
        // Extract link
        std::string linkTag = ExtractTag(entryXml, "link");
        size_t hrefPos = linkTag.find("href=\"");
        if (hrefPos != std::string::npos) {
            size_t start = hrefPos + 6;
            size_t end = linkTag.find("\"", start);
            item.link = linkTag.substr(start, end - start);
        }
        
        item.description = StripTags(ExtractTag(entryXml, "summary"));
        if (item.description.empty()) {
            item.description = StripTags(ExtractTag(entryXml, "content"));
        }
        
        item.author = ExtractTag(entryXml, "author");
        item.guid = ExtractTag(entryXml, "id");
        
        // Parse date
        std::string pubDateStr = ExtractTag(entryXml, "published");
        if (pubDateStr.empty()) {
            pubDateStr = ExtractTag(entryXml, "updated");
        }
        item.pubDate = ParseISO8601Date(pubDateStr);
        
        outFeed.items.push_back(item);
    }
    
    return !outFeed.items.empty();
}

std::string RSSParser::ExtractTag(const std::string& xml, const std::string& tag) {
    std::string openTag = "<" + tag;
    std::string closeTag = "</" + tag + ">";
    
    size_t start = xml.find(openTag);
    if (start == std::string::npos) return "";
    
    // Find end of opening tag
    start = xml.find(">", start);
    if (start == std::string::npos) return "";
    start++;
    
    size_t end = xml.find(closeTag, start);
    if (end == std::string::npos) return "";
    
    return xml.substr(start, end - start);
}

std::vector<std::string> RSSParser::ExtractAllTags(const std::string& xml, const std::string& tag) {
    std::vector<std::string> results;
    std::string openTag = "<" + tag;
    std::string closeTag = "</" + tag + ">";
    
    size_t pos = 0;
    while (true) {
        size_t start = xml.find(openTag, pos);
        if (start == std::string::npos) break;
        
        size_t end = xml.find(closeTag, start);
        if (end == std::string::npos) break;
        
        end += closeTag.length();
        results.push_back(xml.substr(start, end - start));
        pos = end;
    }
    
    return results;
}

std::string RSSParser::StripTags(const std::string& html) {
    std::string result;
    bool inTag = false;
    
    for (char c : html) {
        if (c == '<') {
            inTag = true;
        } else if (c == '>') {
            inTag = false;
        } else if (!inTag) {
            result += c;
        }
    }
    
    return result;
}

time_t RSSParser::ParseRFC822Date(const std::string& dateStr) {
    // Simplified RFC 822 parser
    // Format: "Mon, 15 Jun 2024 10:30:00 GMT"
    
    if (dateStr.empty()) return 0;
    
    struct tm tm = {0};
    // This would need proper date parsing implementation
    // For now, return current time as placeholder
    return time(nullptr);
}

time_t RSSParser::ParseISO8601Date(const std::string& dateStr) {
    // Simplified ISO 8601 parser
    // Format: "2024-06-15T10:30:00Z"
    
    if (dateStr.empty()) return 0;
    
    struct tm tm = {0};
    // This would need proper date parsing implementation
    // For now, return current time as placeholder
    return time(nullptr);
}

bool RSSParser::LoadFeedConfig(const std::string& configPath) {
    // Load feed configuration from JSON file
    // Simplified implementation - would need proper JSON parser
    feeds.clear();
    
    // Add some default feeds for testing
    FeedConfig defaultFeed;
    defaultFeed.name = "Wikipedia Featured";
    defaultFeed.url = "https://en.wikipedia.org/w/api.php?action=featuredfeed&feed=featured&feedformat=rss";
    defaultFeed.category = "reference";
    defaultFeed.enabled = true;
    defaultFeed.priority = 5;
    feeds.push_back(defaultFeed);
    
    return true;
}

bool RSSParser::SaveFeedConfig(const std::string& configPath) {
    // Save feed configuration to JSON file
    // Would need proper JSON serialization
    return true;
}

void RSSParser::AddFeed(const FeedConfig& feed) {
    feeds.push_back(feed);
}

void RSSParser::RemoveFeed(const std::string& url) {
    feeds.erase(
        std::remove_if(feeds.begin(), feeds.end(),
            [&url](const FeedConfig& f) { return f.url == url; }),
        feeds.end()
    );
}

void RSSParser::EnableFeed(const std::string& url, bool enable) {
    for (auto& feed : feeds) {
        if (feed.url == url) {
            feed.enabled = enable;
            break;
        }
    }
}

std::vector<FeedConfig> RSSParser::GetFeedsByCategory(const std::string& category) {
    std::vector<FeedConfig> result;
    for (const auto& feed : feeds) {
        if (feed.category == category && feed.enabled) {
            result.push_back(feed);
        }
    }
    return result;
}
