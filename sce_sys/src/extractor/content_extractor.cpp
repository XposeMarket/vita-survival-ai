#include "content_extractor.h"
#include <algorithm>
#include <sstream>
#include <cctype>

ContentExtractor::ContentExtractor() : maxTextLength(2000), maxQuoteLength(200) {
}

ContentExtractor::~ContentExtractor() {
}

ExtractedContent ContentExtractor::Extract(const std::string& html, const std::string& url) {
    ExtractedContent content;
    
    // Extract domain from URL
    size_t protoEnd = url.find("://");
    if (protoEnd != std::string::npos) {
        size_t domainStart = protoEnd + 3;
        size_t domainEnd = url.find("/", domainStart);
        if (domainEnd == std::string::npos) domainEnd = url.length();
        content.domain = url.substr(domainStart, domainEnd - domainStart);
    }
    
    // Extract metadata
    content.title = ExtractTitle(html);
    content.author = ExtractAuthor(html);
    content.publishDate = ExtractPublishDate(html);
    
    // Extract main content
    std::string cleanHtml = RemoveScriptsAndStyles(html);
    content.mainText = ExtractMainContent(cleanHtml);
    content.mainText = CleanText(content.mainText);
    
    // Limit text length
    std::istringstream iss(content.mainText);
    std::string word;
    std::string limitedText;
    int wordCount = 0;
    
    while (iss >> word && wordCount < maxTextLength) {
        limitedText += word + " ";
        wordCount++;
    }
    content.mainText = limitedText;
    content.wordCount = wordCount;
    
    // Create snippet (first 500 chars)
    if (content.mainText.length() > 500) {
        content.snippet = content.mainText.substr(0, 500) + "...";
    } else {
        content.snippet = content.mainText;
    }
    
    // Extract quotes
    auto quotes = ExtractQuotes(html, maxQuoteLength);
    for (const auto& quote : quotes) {
        if (quote.text.length() > 20) {  // Skip very short quotes
            content.quotes.push_back(quote.text);
        }
    }
    
    // Detect language (simplified)
    content.language = DetectLanguage(content.mainText);
    
    // Check for paywall
    content.hasPaywall = DetectPaywall(html);
    
    return content;
}

std::string ContentExtractor::ExtractTitle(const std::string& html) {
    // Try Open Graph title first
    std::string ogTitle = GetMetaProperty(html, "og:title");
    if (!ogTitle.empty()) return CleanText(ogTitle);
    
    // Try regular title tag
    std::string title = FindBetween(html, "<title>", "</title>");
    if (!title.empty()) return CleanText(title);
    
    // Try h1
    std::string h1 = FindBetween(html, "<h1", "</h1>");
    if (!h1.empty()) {
        size_t contentStart = h1.find(">");
        if (contentStart != std::string::npos) {
            return CleanText(h1.substr(contentStart + 1));
        }
    }
    
    return "Untitled";
}

std::string ContentExtractor::ExtractAuthor(const std::string& html) {
    // Try meta author tag
    std::string author = GetMetaTag(html, "author");
    if (!author.empty()) return author;
    
    // Try article:author
    author = GetMetaProperty(html, "article:author");
    if (!author.empty()) return author;
    
    return "";
}

time_t ContentExtractor::ExtractPublishDate(const std::string& html) {
    // Try article:published_time
    std::string dateStr = GetMetaProperty(html, "article:published_time");
    if (dateStr.empty()) {
        dateStr = GetMetaProperty(html, "datePublished");
    }
    
    // Would need proper date parsing here
    // For now, return 0 (unknown)
    return 0;
}

std::string ContentExtractor::ExtractMainContent(const std::string& html) {
    // Try <article> tag first
    std::string article = FindBetween(html, "<article", "</article>");
    if (!article.empty() && article.length() > 200) {
        return StripHTML(article);
    }
    
    // Try <main> tag
    std::string main = FindBetween(html, "<main", "</main>");
    if (!main.empty() && main.length() > 200) {
        return StripHTML(main);
    }
    
    // Find content blocks and select best one
    auto blocks = FindContentBlocks(html);
    if (!blocks.empty()) {
        auto best = SelectBestBlock(blocks);
        return StripHTML(best.content);
    }
    
    // Fallback: extract all paragraphs
    std::string result;
    auto paragraphs = FindAllBetween(html, "<p", "</p>");
    for (const auto& p : paragraphs) {
        size_t contentStart = p.find(">");
        if (contentStart != std::string::npos) {
            result += StripHTML(p.substr(contentStart + 1)) + "\n\n";
        }
    }
    
    return result;
}

std::vector<ContentExtractor::ContentBlock> ContentExtractor::FindContentBlocks(const std::string& html) {
    std::vector<ContentBlock> blocks;
    
    // Find all divs and other container elements
    std::vector<std::string> tags = {"<div", "<section", "<article"};
    
    for (const auto& tag : tags) {
        size_t pos = 0;
        while (true) {
            size_t start = html.find(tag, pos);
            if (start == std::string::npos) break;
            
            size_t end = html.find("</", start);
            if (end == std::string::npos) break;
            
            std::string blockContent = html.substr(start, end - start);
            
            ContentBlock block;
            block.tag = tag;
            block.content = blockContent;
            block.textLength = CountWords(StripHTML(blockContent));
            block.linkDensity = CalculateLinkDensity(blockContent);
            
            // Score block (more text, less links = better)
            block.score = block.textLength * (1.0f - block.linkDensity);
            
            if (block.textLength > 50) {  // Minimum length threshold
                blocks.push_back(block);
            }
            
            pos = end;
        }
    }
    
    return blocks;
}

ContentExtractor::ContentBlock ContentExtractor::SelectBestBlock(const std::vector<ContentBlock>& blocks) {
    ContentBlock best;
    best.score = 0.0f;
    
    for (const auto& block : blocks) {
        if (block.score > best.score) {
            best = block;
        }
    }
    
    return best;
}

std::vector<Quote> ContentExtractor::ExtractQuotes(const std::string& html, int maxLength) {
    std::vector<Quote> quotes;
    
    std::string text = StripHTML(html);
    
    // Find text in regular quotes
    auto regularQuotes = FindQuotedText(text, "\"", "\"");
    quotes.insert(quotes.end(), regularQuotes.begin(), regularQuotes.end());
    
    // Find text in curly quotes
    auto curlyQuotes = FindQuotedText(text, """, """);
    quotes.insert(quotes.end(), curlyQuotes.begin(), curlyQuotes.end());
    
    // Filter by length
    quotes.erase(
        std::remove_if(quotes.begin(), quotes.end(),
            [maxLength](const Quote& q) { return q.text.length() > maxLength; }),
        quotes.end()
    );
    
    return quotes;
}

std::vector<Quote> ContentExtractor::FindQuotedText(const std::string& text, 
                                                    const std::string& openQuote, 
                                                    const std::string& closeQuote) {
    std::vector<Quote> quotes;
    size_t pos = 0;
    
    while (true) {
        size_t start = text.find(openQuote, pos);
        if (start == std::string::npos) break;
        
        size_t end = text.find(closeQuote, start + openQuote.length());
        if (end == std::string::npos) break;
        
        Quote quote;
        quote.text = text.substr(start + openQuote.length(), end - start - openQuote.length());
        quote.position = start;
        quote.context = GetQuoteContext(text, start, 10);
        
        quotes.push_back(quote);
        pos = end + closeQuote.length();
    }
    
    return quotes;
}

std::string ContentExtractor::GetQuoteContext(const std::string& text, int position, int contextWords) {
    // Get words before and after the quote
    std::string context;
    
    // Find start of context
    int wordCount = 0;
    int start = position;
    while (start > 0 && wordCount < contextWords) {
        if (isspace(text[start])) wordCount++;
        start--;
    }
    
    // Find end of context
    wordCount = 0;
    size_t end = position;
    while (end < text.length() && wordCount < contextWords) {
        if (isspace(text[end])) wordCount++;
        end++;
    }
    
    return text.substr(start, end - start);
}

std::string ContentExtractor::StripHTML(const std::string& html) {
    std::string result;
    bool inTag = false;
    bool inScript = false;
    bool inStyle = false;
    
    for (size_t i = 0; i < html.length(); i++) {
        if (html[i] == '<') {
            inTag = true;
            
            // Check for script/style tags
            if (html.substr(i, 7) == "<script") inScript = true;
            if (html.substr(i, 6) == "<style") inStyle = true;
            if (html.substr(i, 9) == "</script>") inScript = false;
            if (html.substr(i, 8) == "</style>") inStyle = false;
        } else if (html[i] == '>') {
            inTag = false;
        } else if (!inTag && !inScript && !inStyle) {
            result += html[i];
        }
    }
    
    return result;
}

std::string ContentExtractor::RemoveScriptsAndStyles(const std::string& html) {
    std::string result = html;
    
    // Remove scripts
    size_t pos = 0;
    while ((pos = result.find("<script", pos)) != std::string::npos) {
        size_t end = result.find("</script>", pos);
        if (end == std::string::npos) break;
        result.erase(pos, end - pos + 9);
    }
    
    // Remove styles
    pos = 0;
    while ((pos = result.find("<style", pos)) != std::string::npos) {
        size_t end = result.find("</style>", pos);
        if (end == std::string::npos) break;
        result.erase(pos, end - pos + 8);
    }
    
    return result;
}

std::string ContentExtractor::CleanText(const std::string& text) {
    std::string result;
    bool lastWasSpace = false;
    
    for (char c : text) {
        if (isspace(c)) {
            if (!lastWasSpace) {
                result += ' ';
                lastWasSpace = true;
            }
        } else {
            result += c;
            lastWasSpace = false;
        }
    }
    
    // Trim
    size_t start = result.find_first_not_of(" \t\n\r");
    size_t end = result.find_last_not_of(" \t\n\r");
    if (start != std::string::npos && end != std::string::npos) {
        return result.substr(start, end - start + 1);
    }
    
    return result;
}

bool ContentExtractor::DetectPaywall(const std::string& html) {
    // Simple paywall detection
    std::string lower = html;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    return (lower.find("paywall") != std::string::npos ||
            lower.find("subscriber only") != std::string::npos ||
            lower.find("subscribe to read") != std::string::npos);
}

std::string ContentExtractor::DetectLanguage(const std::string& text) {
    // Very simple language detection (just return "en" for now)
    return "en";
}

int ContentExtractor::CountWords(const std::string& text) {
    std::istringstream iss(text);
    std::string word;
    int count = 0;
    while (iss >> word) count++;
    return count;
}

float ContentExtractor::CalculateLinkDensity(const std::string& html) {
    int totalChars = html.length();
    if (totalChars == 0) return 0.0f;
    
    int linkChars = 0;
    size_t pos = 0;
    while ((pos = html.find("<a ", pos)) != std::string::npos) {
        size_t end = html.find("</a>", pos);
        if (end == std::string::npos) break;
        linkChars += (end - pos);
        pos = end;
    }
    
    return (float)linkChars / (float)totalChars;
}

std::string ContentExtractor::FindBetween(const std::string& str, 
                                         const std::string& start, 
                                         const std::string& end) {
    size_t startPos = str.find(start);
    if (startPos == std::string::npos) return "";
    
    startPos += start.length();
    size_t endPos = str.find(end, startPos);
    if (endPos == std::string::npos) return "";
    
    return str.substr(startPos, endPos - startPos);
}

std::vector<std::string> ContentExtractor::FindAllBetween(const std::string& str,
                                                          const std::string& start,
                                                          const std::string& end) {
    std::vector<std::string> results;
    size_t pos = 0;
    
    while (true) {
        size_t startPos = str.find(start, pos);
        if (startPos == std::string::npos) break;
        
        size_t endPos = str.find(end, startPos);
        if (endPos == std::string::npos) break;
        
        results.push_back(str.substr(startPos, endPos - startPos + end.length()));
        pos = endPos + end.length();
    }
    
    return results;
}

std::string ContentExtractor::GetMetaTag(const std::string& html, const std::string& name) {
    std::string search = "name=\"" + name + "\"";
    size_t pos = html.find(search);
    if (pos == std::string::npos) return "";
    
    size_t contentPos = html.find("content=\"", pos);
    if (contentPos == std::string::npos) return "";
    
    contentPos += 9;
    size_t endPos = html.find("\"", contentPos);
    if (endPos == std::string::npos) return "";
    
    return html.substr(contentPos, endPos - contentPos);
}

std::string ContentExtractor::GetMetaProperty(const std::string& html, const std::string& property) {
    std::string search = "property=\"" + property + "\"";
    size_t pos = html.find(search);
    if (pos == std::string::npos) return "";
    
    size_t contentPos = html.find("content=\"", pos);
    if (contentPos == std::string::npos) return "";
    
    contentPos += 9;
    size_t endPos = html.find("\"", contentPos);
    if (endPos == std::string::npos) return "";
    
    return html.substr(contentPos, endPos - contentPos);
}
