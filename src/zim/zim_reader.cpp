#include "zim_reader.h"
#include <ctime>
#include <algorithm>

// NOTE: This is a stub implementation
// Full ZIM reading requires libzim library integration
// See: https://github.com/openzim/libzim

ZIMReader::ZIMReader() : zimFile(nullptr), isLoaded(false) {
}

ZIMReader::~ZIMReader() {
    Close();
}

bool ZIMReader::LoadZIM(const std::string& zimPath) {
    // TODO: Implement actual ZIM loading with libzim
    // For now, return false to indicate ZIM not loaded
    currentZimPath = zimPath;
    isLoaded = false;
    return false;
}

void ZIMReader::Close() {
    if (zimFile) {
        // TODO: Close ZIM file
        zimFile = nullptr;
    }
    isLoaded = false;
    articleCache.clear();
}

bool ZIMReader::GetArticleByUrl(const std::string& url, ZIMArticle& article) {
    if (!isLoaded) return false;
    
    // Check cache first
    if (GetFromCache(url, article)) {
        return true;
    }
    
    // TODO: Implement actual article retrieval from ZIM
    return false;
}

bool ZIMReader::GetMainPage(ZIMArticle& article) {
    if (!isLoaded) return false;
    
    // TODO: Get main page from ZIM
    return false;
}

std::vector<ZIMSearchResult> ZIMReader::SearchArticles(const std::string& query, int limit) {
    std::vector<ZIMSearchResult> results;
    
    if (!isLoaded) return results;
    
    // TODO: Implement ZIM search using libzim's search functionality
    // This would use the ZIM's full-text search index
    
    return results;
}

std::vector<std::string> ZIMReader::GetSuggestions(const std::string& prefix, int limit) {
    std::vector<std::string> suggestions;
    
    if (!isLoaded) return suggestions;
    
    // TODO: Implement prefix-based suggestions from ZIM title index
    
    return suggestions;
}

std::string ZIMReader::GetTitle() {
    if (!isLoaded) return "";
    
    // TODO: Return ZIM metadata title
    return "Wikipedia";
}

std::string ZIMReader::GetDescription() {
    if (!isLoaded) return "";
    
    // TODO: Return ZIM metadata description
    return "";
}

int ZIMReader::GetArticleCount() {
    if (!isLoaded) return 0;
    
    // TODO: Return ZIM article count
    return 0;
}

void ZIMReader::AddToCache(const std::string& url, const ZIMArticle& article) {
    // Remove oldest if cache is full
    if (articleCache.size() >= MAX_CACHE_SIZE) {
        articleCache.erase(articleCache.begin());
    }
    
    CacheEntry entry;
    entry.url = url;
    entry.article = article;
    entry.timestamp = time(nullptr);
    
    articleCache.push_back(entry);
}

bool ZIMReader::GetFromCache(const std::string& url, ZIMArticle& article) {
    for (const auto& entry : articleCache) {
        if (entry.url == url) {
            article = entry.article;
            return true;
        }
    }
    return false;
}

void ZIMReader::ClearOldCache() {
    time_t now = time(nullptr);
    time_t maxAge = 3600; // 1 hour
    
    articleCache.erase(
        std::remove_if(articleCache.begin(), articleCache.end(),
            [now, maxAge](const CacheEntry& entry) {
                return (now - entry.timestamp) > maxAge;
            }),
        articleCache.end()
    );
}
