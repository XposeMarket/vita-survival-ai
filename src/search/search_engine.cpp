#include "search_engine.h"
#include "llm_engine.h"
#include <algorithm>
#include <cctype>
#include <sstream>

#include "survival_ai.h"
#include "online_search.h"

SearchEngine::SearchEngine() : database(nullptr), zimReader(nullptr), 
                               onlineSearch(nullptr), llmEngine(nullptr) {
}

SearchEngine::~SearchEngine() {
}

void SearchEngine::Initialize(Database* db, ZIMReader* zim, OnlineSearch* online, LLMEngine* llm) {
    database = db;
    zimReader = zim;
    onlineSearch = online;
    llmEngine = llm;
}

Answer SearchEngine::Ask(const std::string& query) {
    // Auto-detect online/offline and route accordingly
    if (onlineSearch && onlineSearch->IsOnline() && g_app.onlineModeEnabled) {
        return AskOnline(query);
    } else {
        return AskOffline(query);
    }
}

Answer SearchEngine::AskOnline(const std::string& query) {
    Answer answer;
    answer.type = ANSWER_NONE;
    answer.confidence = 0.0f;
    
    if (query.empty()) {
        return answer;
    }
    
    // Step 1: Search online and save results
    std::vector<VaultItem> onlineItems;
    if (onlineSearch) {
        onlineSearch->SearchAndSave(query, onlineItems);
    }
    
    // Step 2: Search vault (includes newly saved items)
    std::vector<SearchResult> vaultResults;
    if (database) {
        QueryAnalysis analysis = AnalyzeQuery(query);
        if (analysis.intent == INTENT_QUOTE) {
            vaultResults = database->SearchQuotes(analysis.person, analysis.secondaryTopic, 10);
        } else {
            vaultResults = database->SearchFTS(query, 10);
        }
    }
    
    // Step 3: Fallback to Wikipedia if needed
    std::vector<ZIMSearchResult> zimResults;
    if (vaultResults.empty() && zimReader && zimReader->IsLoaded()) {
        zimResults = zimReader->SearchArticles(query, 5);
    }
    
    // Step 4: Generate answer
    return GenerateAnswer(query, vaultResults, zimResults);
}

Answer SearchEngine::AskOffline(const std::string& query) {
    Answer answer;
    answer.type = ANSWER_NONE;
    answer.confidence = 0.0f;
    
    if (query.empty()) {
        return answer;
    }
    
    // Analyze query intent
    QueryAnalysis analysis = AnalyzeQuery(query);
    
    // Search vault
    std::vector<SearchResult> vaultResults;
    if (database) {
        if (analysis.intent == INTENT_QUOTE) {
            vaultResults = database->SearchQuotes(analysis.person, analysis.secondaryTopic, 10);
        } else {
            vaultResults = database->SearchFTS(query, 10);
        }
    }
    
    // Search Wikipedia
    std::vector<ZIMSearchResult> zimResults;
    if (zimReader && zimReader->IsLoaded()) {
        zimResults = zimReader->SearchArticles(query, 5);
    }
    
    // Generate answer based on intent
    switch (analysis.intent) {
        case INTENT_QUOTE:
            answer = BuildQuotesAnswer(analysis, vaultResults);
            break;
        case INTENT_HOWTO:
            answer = BuildStepsAnswer(analysis, vaultResults);
            break;
        case INTENT_WHAT:
        case INTENT_WHEN:
        case INTENT_WHERE:
            answer = BuildDirectAnswer(analysis, vaultResults);
            break;
        default:
            answer = BuildSummaryAnswer(analysis, vaultResults);
            break;
    }
    
    // Add Wikipedia results as additional sources if answer is weak
    if (answer.confidence < 0.7f && !zimResults.empty()) {
        for (const auto& zimResult : zimResults) {
            SourceInfo source;
            source.title = zimResult.title;
            source.url = "wikipedia://" + zimResult.url;
            source.domain = "Wikipedia";
            source.content_type = "encyclopedia";
            source.confidence = 0.8f;
            answer.sources.push_back(source);
        }
    }
    
    return answer;
}

QueryAnalysis SearchEngine::AnalyzeQuery(const std::string& query) {
    QueryAnalysis analysis;
    analysis.intent = INTENT_GENERAL;
    analysis.needsRecent = false;
    analysis.needsOfficial = false;
    
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    // Check for quote intent
    std::string person, topic;
    if (MatchesQuotePattern(lowerQuery, person, topic)) {
        analysis.intent = INTENT_QUOTE;
        analysis.person = person;
        analysis.secondaryTopic = topic;
        analysis.needsOfficial = true;
        return analysis;
    }
    
    // Check for how-to
    if (MatchesHowToPattern(lowerQuery)) {
        analysis.intent = INTENT_HOWTO;
    }
    // Check for what
    else if (MatchesWhatPattern(lowerQuery)) {
        analysis.intent = INTENT_WHAT;
    }
    // Check for when
    else if (lowerQuery.find("when ") == 0 || lowerQuery.find("when did") != std::string::npos) {
        analysis.intent = INTENT_WHEN;
    }
    // Check for where
    else if (lowerQuery.find("where ") == 0 || lowerQuery.find("where is") != std::string::npos) {
        analysis.intent = INTENT_WHERE;
    }
    // Check for why
    else if (lowerQuery.find("why ") == 0) {
        analysis.intent = INTENT_WHY;
    }
    
    // Check for recency needs
    if (lowerQuery.find("recent") != std::string::npos ||
        lowerQuery.find("latest") != std::string::npos ||
        lowerQuery.find("current") != std::string::npos ||
        lowerQuery.find("today") != std::string::npos ||
        lowerQuery.find("now") != std::string::npos) {
        analysis.needsRecent = true;
    }
    
    return analysis;
}

bool SearchEngine::MatchesQuotePattern(const std::string& query, std::string& person, std::string& topic) {
    // Patterns: "what did X say", "X said about Y", "quote from X"
    
    size_t pos;
    
    // "what did X say"
    if ((pos = query.find("what did ")) != std::string::npos) {
        size_t start = pos + 9;
        size_t end = query.find(" say", start);
        if (end != std::string::npos) {
            person = query.substr(start, end - start);
            
            // Check for "about Y"
            size_t aboutPos = query.find(" about ", end);
            if (aboutPos != std::string::npos) {
                topic = query.substr(aboutPos + 7);
            }
            return true;
        }
    }
    
    // "X said"
    if ((pos = query.find(" said")) != std::string::npos) {
        person = query.substr(0, pos);
        
        size_t aboutPos = query.find(" about ", pos);
        if (aboutPos != std::string::npos) {
            topic = query.substr(aboutPos + 7);
        }
        return true;
    }
    
    // "quote from X"
    if ((pos = query.find("quote from ")) != std::string::npos) {
        size_t start = pos + 11;
        person = query.substr(start);
        
        size_t aboutPos = person.find(" about ");
        if (aboutPos != std::string::npos) {
            topic = person.substr(aboutPos + 7);
            person = person.substr(0, aboutPos);
        }
        return true;
    }
    
    return false;
}

bool SearchEngine::MatchesHowToPattern(const std::string& query) {
    return (query.find("how to ") == 0 || 
            query.find("how do i ") == 0 ||
            query.find("how can i ") == 0);
}

bool SearchEngine::MatchesWhatPattern(const std::string& query) {
    return (query.find("what is ") == 0 || 
            query.find("what are ") == 0 ||
            query.find("what's ") == 0);
}

Answer SearchEngine::BuildQuotesAnswer(const QueryAnalysis& analysis, 
                                        const std::vector<SearchResult>& results) {
    Answer answer;
    answer.type = ANSWER_QUOTES;
    answer.confidence = 0.0f;
    
    if (results.empty()) {
        answer.summary = "No quotes found for " + analysis.person;
        if (!analysis.secondaryTopic.empty()) {
            answer.summary += " about " + analysis.secondaryTopic;
        }
        answer.summary += ".";
        return answer;
    }
    
    answer.summary = "Quotes from " + analysis.person;
    if (!analysis.secondaryTopic.empty()) {
        answer.summary += " about " + analysis.secondaryTopic;
    }
    answer.summary += ":";
    
    // Extract quotes from results
    for (const auto& result : results) {
        // Parse quotes_json (simplified - should use proper JSON parser)
        if (!result.item.quotes_json.empty()) {
            // For now, just add the raw quote data
            answer.quotes.push_back(result.item.quotes_json);
        } else if (!result.item.text_snippet.empty()) {
            answer.quotes.push_back(result.item.text_snippet);
        }
        
        // Add source
        SourceInfo source;
        source.title = result.item.title;
        source.url = result.item.url;
        source.domain = result.item.source_domain;
        source.author = result.item.author;
        source.published = result.item.published_at;
        source.retrieved = result.item.retrieved_at;
        source.content_type = result.item.content_type;
        source.confidence = result.score;
        answer.sources.push_back(source);
        
        if (answer.quotes.size() >= 3) break; // Limit to top 3 quotes
    }
    
    answer.confidence = results.empty() ? 0.0f : 0.8f;
    return answer;
}

Answer SearchEngine::BuildStepsAnswer(const QueryAnalysis& analysis,
                                      const std::vector<SearchResult>& results) {
    Answer answer;
    answer.type = ANSWER_STEPS;
    answer.confidence = 0.0f;
    
    if (results.empty()) {
        answer.summary = "No instructions found.";
        return answer;
    }
    
    // Use first result for main content
    const auto& topResult = results[0];
    answer.summary = "Instructions:";
    
    // Extract steps from text (simplified)
    std::string text = topResult.item.text_clean.empty() ? 
                      topResult.item.text_snippet : topResult.item.text_clean;
    
    // Look for numbered steps or bullet points
    std::istringstream iss(text);
    std::string line;
    int stepNum = 1;
    
    while (std::getline(iss, line) && answer.steps.size() < 10) {
        // Check if line looks like a step
        if (!line.empty() && (
            line.find(std::to_string(stepNum)) == 0 ||
            line[0] == '-' || line[0] == '*' ||
            line.find("step") != std::string::npos)) {
            
            answer.steps.push_back(line);
            stepNum++;
        }
    }
    
    // If no steps found, create generic summary
    if (answer.steps.empty()) {
        answer.steps.push_back(text.substr(0, 200) + "...");
    }
    
    // Add sources
    for (const auto& result : results) {
        SourceInfo source;
        source.title = result.item.title;
        source.url = result.item.url;
        source.domain = result.item.source_domain;
        source.published = result.item.published_at;
        source.retrieved = result.item.retrieved_at;
        source.confidence = result.score;
        answer.sources.push_back(source);
    }
    
    answer.confidence = 0.7f;
    return answer;
}

Answer SearchEngine::BuildDirectAnswer(const QueryAnalysis& analysis,
                                       const std::vector<SearchResult>& results) {
    Answer answer;
    answer.type = ANSWER_DIRECT;
    answer.confidence = 0.0f;
    
    if (results.empty()) {
        answer.summary = "No information found.";
        return answer;
    }
    
    // Use top result
    const auto& topResult = results[0];
    answer.summary = topResult.item.text_snippet;
    
    if (!topResult.item.text_clean.empty()) {
        answer.raw_text = topResult.item.text_clean;
    }
    
    // Add sources
    for (size_t i = 0; i < std::min(results.size(), size_t(5)); i++) {
        const auto& result = results[i];
        SourceInfo source;
        source.title = result.item.title;
        source.url = result.item.url;
        source.domain = result.item.source_domain;
        source.published = result.item.published_at;
        source.retrieved = result.item.retrieved_at;
        source.confidence = result.score;
        answer.sources.push_back(source);
    }
    
    answer.confidence = 0.75f;
    return answer;
}

Answer SearchEngine::BuildSummaryAnswer(const QueryAnalysis& analysis,
                                       const std::vector<SearchResult>& results) {
    Answer answer;
    answer.type = ANSWER_SUMMARY;
    answer.confidence = 0.0f;
    
    if (results.empty()) {
        answer.summary = "No relevant information found.";
        return answer;
    }
    
    // Combine top results
    std::string combined;
    for (size_t i = 0; i < std::min(results.size(), size_t(3)); i++) {
        combined += results[i].item.text_snippet + "\n\n";
        
        SourceInfo source;
        source.title = results[i].item.title;
        source.url = results[i].item.url;
        source.domain = results[i].item.source_domain;
        source.published = results[i].item.published_at;
        source.retrieved = results[i].item.retrieved_at;
        source.confidence = results[i].score;
        answer.sources.push_back(source);
    }
    
    answer.summary = combined;
    answer.confidence = 0.65f;
    
    return answer;
}

Answer SearchEngine::GenerateAnswer(const std::string& query,
                                   const std::vector<SearchResult>& vaultResults,
                                   const std::vector<ZIMSearchResult>& zimResults) {
    Answer answer;
    answer.type = ANSWER_SUMMARY;
    answer.confidence = 0.0f;

    if (!vaultResults.empty()) {
        // Use the combined snippets from vault results
        std::string combined;
        for (size_t i = 0; i < std::min(vaultResults.size(), size_t(3)); i++) {
            combined += vaultResults[i].item.text_snippet + "\n\n";
            SourceInfo source;
            source.title = vaultResults[i].item.title;
            source.url = vaultResults[i].item.url;
            source.domain = vaultResults[i].item.source_domain;
            source.author = vaultResults[i].item.author;
            source.published = vaultResults[i].item.published_at;
            source.retrieved = vaultResults[i].item.retrieved_at;
            source.content_type = vaultResults[i].item.content_type;
            source.confidence = vaultResults[i].score;
            answer.sources.push_back(source);
        }
        answer.summary = combined;
        answer.confidence = 0.8f;
        return answer;
    }

    if (!zimResults.empty()) {
        // Use first ZIM result as a fallback
        answer.summary = zimResults[0].snippet.empty() ? zimResults[0].title : zimResults[0].snippet;
        SourceInfo source;
        source.title = zimResults[0].title;
        source.url = "wikipedia://" + zimResults[0].url;
        source.domain = "Wikipedia";
        source.confidence = 0.7f;
        answer.sources.push_back(source);
        answer.confidence = 0.7f;
        return answer;
    }

    answer.summary = "No relevant results found.";
    answer.confidence = 0.0f;
    return answer;
}

Answer SearchEngine::GenerateAnswerWithLLM(const std::string& query,
                                          const QueryAnalysis& analysis,
                                          const std::vector<SearchResult>& results) {
    Answer answer;
    answer.type = ANSWER_SUMMARY;
    
    // Build context from search results (max 1000 words)
    std::string context = BuildLLMContext(results, 1000);
    
    // Build prompt
    std::string prompt = BuildSourcedPrompt(query, context);
    
    // Generate answer with LLM (streaming to show progress)
    std::string llmAnswer;
    llmEngine->GenerateStreaming(prompt, 
        [&llmAnswer](const std::string& token) {
            llmAnswer += token;
            // Could update UI here for streaming effect
        }, 
        200 // Max tokens
    );
    
    // Package answer
    answer.summary = llmAnswer;
    answer.confidence = 0.90f; // Higher confidence with LLM
    answer.type = ANSWER_SUMMARY;
    
    // Add sources
    for (const auto& result : results) {
        SourceInfo source;
        source.title = result.item.title;
        source.url = result.item.url;
        source.domain = result.item.source_domain;
        source.author = result.item.author;
        source.published = result.item.published_at;
        source.retrieved = result.item.retrieved_at;
        source.content_type = result.item.content_type;
        source.confidence = result.score;
        answer.sources.push_back(source);
    }
    
    return answer;
}
