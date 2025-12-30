#ifndef SURVIVAL_AI_H
#define SURVIVAL_AI_H
#include <psp2/ctrl.h>
#include <psp2/kernel/threadmgr.h>
#include <algorithm>
#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/io/fcntl.h>
#include <vita2d.h>
#include <string>
#include <vector>

// App configuration
#define APP_VERSION "1.0.0"
#define DATA_PATH "ux0:data/survivalkit/"
#define ZIM_PATH DATA_PATH "zim/"
#define VAULT_PATH DATA_PATH "vault/"
#define DB_PATH DATA_PATH "db/"
#define CACHE_PATH DATA_PATH "cache/"
#define VOICE_PATH DATA_PATH "voice/"

// Screen dimensions
#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544

// Colors
#define COLOR_WHITE RGBA8(255, 255, 255, 255)
#define COLOR_BLACK RGBA8(0, 0, 0, 255)
#define COLOR_GRAY RGBA8(128, 128, 128, 255)
#define COLOR_DARK_GRAY RGBA8(64, 64, 64, 255)
#define COLOR_BLUE RGBA8(0, 120, 215, 255)
#define COLOR_GREEN RGBA8(16, 124, 16, 255)
#define COLOR_RED RGBA8(232, 17, 35, 255)
#define COLOR_YELLOW RGBA8(255, 185, 0, 255)

// App states
enum AppState {
    STATE_ASK,
    STATE_LIBRARY,
    STATE_WIKIPEDIA,
    STATE_VAULT,
    STATE_MANUALS,
    STATE_SCENARIOS,
    STATE_TOOLKIT,
    STATE_SYNC
};

// Forward declarations
class UI;
class Database;
class ZIMReader;
class SearchEngine;
class VoiceSystem;
class NetFetcher;
class RSSParser;
class ContentExtractor;
class OnlineSearch;
class LLMEngine;

// Main app structure
struct AppContext {
    AppState currentState;
    UI* ui;
    Database* db;
    ZIMReader* zimReader;
    SearchEngine* search;
    VoiceSystem* voice;
    
    // Online components
    NetFetcher* netFetcher;
    RSSParser* rssParser;
    ContentExtractor* extractor;
    OnlineSearch* onlineSearch;
    
    // LLM component
    LLMEngine* llm;
    bool llmEnabled;
    
    vita2d_pgf* font;
    vita2d_pgf* fontSmall;
    
    bool running;
    bool online;
    bool onlineModeEnabled;  // User setting
    
    SceCtrlData pad;
    SceCtrlData oldPad;
};

// Global app context
extern AppContext g_app;

// Utility functions
void InitApp();
void ShutdownApp();
void CreateDirectories();
bool IsButtonPressed(uint32_t button);
bool IsButtonHeld(uint32_t button);

#endif // SURVIVAL_AI_H
