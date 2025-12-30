#ifndef CONTENT_EXTRACTOR_H
#define CONTENT_EXTRACTOR_H

#include <string>
#include <vector>
#include <ctime>

// Extracted content structure
struct ExtractedContent {
    std::string title;
    std::string author;
    time_t publishDate;
    std::string domain;
    std::string mainText;
    std::string snippet;  // First ~500 chars
    std::vector<std::string> quotes;  // Text in quotation marks
    std::string language;
    int wordCount;
    bool hasPaywall;
};

// Quote with context
struct Quote {
    std::string text;
    std::string context;  // Surrounding text
    int position;  // Position in document
};

class ContentExtractor {
public:
    ContentExtractor();
    ~ContentExtractor();
    
    // Main extraction
    ExtractedContent Extract(const std::string& html, const std::string& url);
    
    // Component extraction
    std::string ExtractTitle(const std::string& html);
    std::string ExtractAuthor(const std::string& html);
    time_t ExtractPublishDate(const std::string& html);
    std::string ExtractMainContent(const std::string& html);
    std::vector<Quote> ExtractQuotes(const std::string& html, int maxLength = 200);
    
    // Cleaning
    std::string StripHTML(const std::string& html);
    std::string RemoveScriptsAndStyles(const std::string& html);
    std::string CleanText(const std::string& text);
    
    // Detection
    bool DetectPaywall(const std::string& html);
    std::string DetectLanguage(const std::string& text);
    
    // Settings
    void SetMaxTextLength(int length) { maxTextLength = length; }
    void SetMaxQuoteLength(int length) { maxQuoteLength = length; }
    
private:
    int maxTextLength;  // Max words to extract (default 2000)
    int maxQuoteLength;  // Max quote length (default 200 chars)
    
    // Readability-like extraction
    struct ContentBlock {
        std::string tag;
        std::string content;
        int textLength;
        int linkDensity;
        float score;
    };
    
    std::vector<ContentBlock> FindContentBlocks(const std::string& html);
    ContentBlock SelectBestBlock(const std::vector<ContentBlock>& blocks);
    
    // Helpers
    std::string FindBetween(const std::string& str, const std::string& start, const std::string& end);
    std::vector<std::string> FindAllBetween(const std::string& str, const std::string& start, const std::string& end);
    int CountWords(const std::string& text);
    float CalculateLinkDensity(const std::string& html);
    
    // Meta tag extraction
    std::string GetMetaTag(const std::string& html, const std::string& name);
    std::string GetMetaProperty(const std::string& html, const std::string& property);
    
    // Quote extraction helpers
    std::vector<Quote> FindQuotedText(const std::string& text, const std::string& openQuote, const std::string& closeQuote);
    std::string GetQuoteContext(const std::string& text, int position, int contextWords = 10);
};

#endif // CONTENT_EXTRACTOR_H
