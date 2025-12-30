#include "llm_engine.h"
#include "search_engine.h"
#include <ctime>
#include <sstream>
#include <algorithm>

// NOTE: This is a wrapper around llama.cpp
// llama.cpp headers will be included when we add the library
// For now, we provide the interface

LLMEngine::LLMEngine() : model(nullptr), ctx(nullptr), samplingCtx(nullptr),
                         modelLoaded(false), isGenerating(false), shouldStop(false),
                         tokensGenerated(0), tokensPerSecond(0.0f) {
}

LLMEngine::~LLMEngine() {
    UnloadModel();
}

bool LLMEngine::LoadModel(const std::string& modelPath) {
    if (modelLoaded) {
        UnloadModel();
    }
    
    this->modelPath = modelPath;
    
    // TODO: Actual llama.cpp model loading
    // This will be implemented with llama.cpp integration
    // For now, return false to indicate not yet implemented
    
    /*
    // Example of what this will look like:
    
    llama_backend_init(false);
    
    llama_model_params model_params = llama_model_default_params();
    model = llama_load_model_from_file(modelPath.c_str(), model_params);
    
    if (!model) {
        return false;
    }
    
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = settings.contextSize;
    ctx_params.n_threads = settings.nThreads;
    
    ctx = llama_new_context_with_model(model, ctx_params);
    
    if (!ctx) {
        llama_free_model(model);
        model = nullptr;
        return false;
    }
    
    modelLoaded = true;
    return true;
    */
    
    return false; // Not implemented yet
}

void LLMEngine::UnloadModel() {
    if (!modelLoaded) return;
    
    // TODO: Actual cleanup
    /*
    if (ctx) {
        llama_free(ctx);
        ctx = nullptr;
    }
    
    if (model) {
        llama_free_model(model);
        model = nullptr;
    }
    
    llama_backend_free();
    */
    
    modelLoaded = false;
}

std::string LLMEngine::Generate(const std::string& prompt, int maxTokens) {
    std::string result;
    
    GenerateStreaming(prompt, [&result](const std::string& token) {
        result += token;
    }, maxTokens);
    
    return result;
}

void LLMEngine::GenerateStreaming(const std::string& prompt, StreamCallback callback, int maxTokens) {
    if (!modelLoaded) {
        callback("[Error: No model loaded]");
        return;
    }
    
    if (isGenerating) {
        callback("[Error: Already generating]");
        return;
    }
    
    isGenerating = true;
    shouldStop = false;
    tokensGenerated = 0;
    
    time_t startTime = time(nullptr);
    
    // Generate answer
    std::string answer = GenerateInternal(prompt, callback, maxTokens);
    
    time_t endTime = time(nullptr);
    float elapsed = (float)(endTime - startTime);
    UpdateStats(tokensGenerated, elapsed);
    
    isGenerating = false;
}

std::string LLMEngine::GenerateInternal(const std::string& prompt,
                                        StreamCallback callback,
                                        int maxTokens) {
    // TODO: Actual generation with llama.cpp
    // This is a placeholder showing the structure
    
    /*
    // Tokenize prompt
    auto tokens = Tokenize(prompt);
    
    // Evaluate prompt
    if (llama_decode(ctx, llama_batch_get_one(tokens.data(), tokens.size(), 0, 0))) {
        return "[Error: Failed to process prompt]";
    }
    
    // Generate tokens
    std::string result;
    for (int i = 0; i < maxTokens && !shouldStop; i++) {
        // Sample next token
        int nextToken = SampleNextToken();
        
        // Check for end of sequence
        if (nextToken == llama_token_eos(model)) {
            break;
        }
        
        // Decode token to string
        char buf[256];
        int n = llama_token_to_piece(model, nextToken, buf, sizeof(buf));
        std::string tokenStr(buf, n);
        
        result += tokenStr;
        tokensGenerated++;
        
        // Callback for streaming
        if (callback) {
            callback(tokenStr);
        }
        
        // Evaluate next token
        if (llama_decode(ctx, llama_batch_get_one(&nextToken, 1, tokens.size() + i, 0))) {
            break;
        }
    }
    
    return result;
    */
    
    // Placeholder response
    if (callback) {
        callback("[LLM not yet integrated - placeholder response]");
    }
    return "[LLM not yet integrated - placeholder response]";
}

void LLMEngine::StopGeneration() {
    shouldStop = true;
}

void LLMEngine::SetSettings(const LLMSettings& newSettings) {
    settings = newSettings;
}

std::string LLMEngine::GetModelName() const {
    if (!modelLoaded) return "No model loaded";
    return modelPath;
}

int LLMEngine::GetModelSize() const {
    // TODO: Return actual model size
    return 0;
}

int LLMEngine::GetContextSize() const {
    return settings.contextSize;
}

void LLMEngine::ResetContext() {
    // TODO: Reset llama context
}

void LLMEngine::UpdateStats(int tokens, float timeSeconds) {
    tokensGenerated = tokens;
    if (timeSeconds > 0) {
        tokensPerSecond = (float)tokens / timeSeconds;
    }
}

std::vector<int> LLMEngine::Tokenize(const std::string& text) {
    // TODO: Actual tokenization
    return std::vector<int>();
}

std::string LLMEngine::Detokenize(const std::vector<int>& tokens) {
    // TODO: Actual detokenization
    return "";
}

int LLMEngine::SampleNextToken() {
    // TODO: Actual sampling
    return 0;
}

// Helper functions

std::string BuildLLMContext(const std::vector<SearchResult>& results, int maxWords) {
    std::string context;
    int wordCount = 0;
    
    for (size_t i = 0; i < results.size() && wordCount < maxWords; i++) {
        const auto& result = results[i];
        
        // Add source header
        context += "\n--- Source " + std::to_string(i+1) + ": " + 
                   result.item.source_domain + " ---\n";
        context += "Title: " + result.item.title + "\n";
        
        // Add content
        std::string text = result.item.text_clean.empty() ? 
                          result.item.text_snippet : result.item.text_clean;
        
        // Count words
        std::istringstream iss(text);
        std::string word;
        std::string limitedText;
        
        while (iss >> word && wordCount < maxWords) {
            limitedText += word + " ";
            wordCount++;
        }
        
        context += limitedText + "\n";
        
        // Add quotes if available
        if (!result.item.quotes_json.empty() && result.item.quotes_json != "[]") {
            context += "Key Quotes: " + result.item.quotes_json + "\n";
        }
    }
    
    return context;
}

std::string BuildSourcedPrompt(const std::string& query, const std::string& context) {
    std::string prompt = 
        "You are a helpful survival and knowledge assistant. "
        "Answer questions using ONLY the information provided in the sources below. "
        "Do not use your training knowledge. "
        "If the sources don't contain enough information, say so. "
        "Cite sources by number when making claims.\n\n";
    
    prompt += "SOURCES:\n" + context + "\n";
    prompt += "---\n\n";
    prompt += "QUESTION: " + query + "\n\n";
    prompt += "ANSWER (be clear, concise, and cite sources by number):\n";
    
    return prompt;
}
