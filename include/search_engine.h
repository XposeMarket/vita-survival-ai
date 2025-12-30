#ifndef SEARCH_ENGINE_H
#define SEARCH_ENGINE_H

#include <string>
#include <vector>
#include "database.h"
#include "zim_reader.h"

// Answer types
enum AnswerType {
    ANSWER_DIRECT,      // Direct factual answer
    ANSWER_STEPS,       // Step-by-step instructions
    ANSWER_QUOTES,      // Quote collection
    ANSWER_SUMMARY,     // General summary
    ANSWER_NONE         // No answer found
};

// Source info for citations
struct SourceInfo {
    std::string title;
    std::string url;
    std::string domain;
    std::string author;
    time_t published;
    time_t retrieved;
    std::string content_type;
    float confidence;
};

// Structured answer
struct Answer {
    AnswerType type;
    std::string summary;
    std::vector<std::string> steps;
    std::vector<std::string> bullets;
    std::vector<std::string> warnings;
    std::vector<std::string> quotes;
    std::vector<SourceInfo> sources;
    std::string raw_text;
    float confidence;
};

// Intent detection
enum QueryIntent {
    INTENT_QUOTE,       // "What did X say about Y"
    INTENT_HOWTO,       // "How to do X"
    INTENT_WHAT,        // "What is X"
    INTENT_WHEN,        // "When did X"
    INTENT_WHERE,       // "Where is X"
    INTENT_WHY,         // "Why X"
    INTENT_GENERAL      // General query
};

struct QueryAnalysis {
    QueryIntent intent;
    std::string mainTopic;
    std::string secondaryTopic;
    std::string person;         // For quote queries
    bool needsRecent;           // Current events
    bool needsOfficial;         // Official sources preferred
};

class OnlineSearch; // Forward declaration
class LLMEngine; // Forward declaration

class SearchEngine {
public:
    SearchEngine();
    ~SearchEngine();
    
    void Initialize(Database* db, ZIMReader* zim, OnlineSearch* online = nullptr, LLMEngine* llm = nullptr);
    
    // Main search interface - auto-detects online/offline
    Answer Ask(const std::string& query);
    
    // Explicit mode control
    Answer AskOffline(const std::string& query);
    Answer AskOnline(const std::string& query);
    
    // Component searches
    std::vector<SearchResult> SearchVault(const std::string& query, int limit = 10);
    std::vector<ZIMSearchResult> SearchWikipedia(const std::string& query, int limit = 10);
    
    // Answer generation
    Answer GenerateAnswer(const std::string& query, 
                         const std::vector<SearchResult>& vaultResults,
                         const std::vector<ZIMSearchResult>& zimResults);
    
    // LLM-enhanced answer generation
    Answer GenerateAnswerWithLLM(const std::string& query,
                                const QueryAnalysis& analysis,
                                const std::vector<SearchResult>& results);
    
    // Intent detection
    QueryAnalysis AnalyzeQuery(const std::string& query);
    
private:
    Database* database;
    ZIMReader* zimReader;
    OnlineSearch* onlineSearch;
    LLMEngine* llmEngine;
    
    // Answer builders for different types
    Answer BuildDirectAnswer(const QueryAnalysis& analysis, 
                            const std::vector<SearchResult>& results);
    Answer BuildStepsAnswer(const QueryAnalysis& analysis,
                           const std::vector<SearchResult>& results);
    Answer BuildQuotesAnswer(const QueryAnalysis& analysis,
                            const std::vector<SearchResult>& results);
    Answer BuildSummaryAnswer(const QueryAnalysis& analysis,
                             const std::vector<SearchResult>& results);
    
    // Helpers
    std::vector<std::string> ExtractKeywords(const std::string& query);
    float CalculateRelevance(const std::string& query, const std::string& text);
    std::string ExtractSnippet(const std::string& text, const std::string& query, int contextWords = 20);
    
    // Pattern matching for intent
    bool MatchesQuotePattern(const std::string& query, std::string& person, std::string& topic);
    bool MatchesHowToPattern(const std::string& query);
    bool MatchesWhatPattern(const std::string& query);
};

#endif // SEARCH_ENGINE_H
