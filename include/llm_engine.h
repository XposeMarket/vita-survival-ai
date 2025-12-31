#ifndef LLM_ENGINE_H
#define LLM_ENGINE_H

#include <string>
#include <vector>
#include <functional>

// Forward declaration of llama.cpp types (to avoid including heavy headers)
struct llama_model;
struct llama_context;
struct llama_batch;
struct llama_sampling_context;

// LLM generation settings
struct LLMSettings {
    int maxTokens;          // Max tokens to generate (default: 200)
    float temperature;      // Randomness 0.0-2.0 (default: 0.7)
    float topP;            // Nucleus sampling (default: 0.9)
    int topK;              // Top-K sampling (default: 40)
    float repeatPenalty;   // Penalty for repetition (default: 1.1)
    int nThreads;          // CPU threads (default: 2 for Vita)
    int contextSize;       // Context window (default: 2048)
    
    LLMSettings() : 
        maxTokens(200), 
        temperature(0.7f),
        topP(0.9f),
        topK(40),
        repeatPenalty(1.1f),
        nThreads(2),
        contextSize(2048) {}
};

// Streaming callback type
typedef std::function<void(const std::string& token)> StreamCallback;

class LLMEngine {
public:
    LLMEngine();
    ~LLMEngine();
    
    // Model management
    bool LoadModel(const std::string& modelPath);
    void UnloadModel();
    bool IsModelLoaded() const { return modelLoaded; }
    
    // Generation
    std::string Generate(const std::string& prompt, int maxTokens = 200);
    void GenerateStreaming(const std::string& prompt, StreamCallback callback, int maxTokens = 200);
    
    // Stop generation early
    void StopGeneration();
    
    // Settings
    void SetSettings(const LLMSettings& settings);
    LLMSettings GetSettings() const { return settings; }
    
    // Model info
    std::string GetModelName() const;
    int GetModelSize() const;
    int GetContextSize() const;
    
    // Status
    bool IsGenerating() const { return isGenerating; }
    int GetTokensGenerated() const { return tokensGenerated; }
    float GetTokensPerSecond() const { return tokensPerSecond; }
    
private:
    llama_model* model;
    llama_context* ctx;
    llama_sampling_context* samplingCtx;
    
    bool modelLoaded;
    bool isGenerating;
    bool shouldStop;
    
    LLMSettings settings;
    std::string modelPath;
    
    int tokensGenerated;
    float tokensPerSecond;
    
    // Internal generation
    std::string GenerateInternal(const std::string& prompt, 
                                 StreamCallback callback,
                                 int maxTokens);
    
    // Token processing
    std::vector<int> Tokenize(const std::string& text);
    std::string Detokenize(const std::vector<int>& tokens);
    int SampleNextToken();
    
    // Helpers
    void ResetContext();
    void UpdateStats(int tokens, float timeSeconds);
};

// Helper function to build context from search results
std::string BuildLLMContext(const std::vector<struct SearchResult>& results, int maxWords = 1000);

// Helper to build prompt with sources
std::string BuildSourcedPrompt(const std::string& query, const std::string& context);

#endif // LLM_ENGINE_H
