#include "survival_ai.h"
#include "ui.h"
#include "database.h"
#include "zim_reader.h"
#include "search_engine.h"
#include "voice_system.h"
#include "net_fetcher.h"
#include "rss_parser.h"
#include "content_extractor.h"
#include "online_search.h"
#include "llm_engine.h"
#include <psp2/kernel/threadmgr.h>
#include <psp2/io/dirent.h>
#include <psp2/sysmodule.h>
#include <cstring>

// Global app context
AppContext g_app;

void CreateDirectories() {
    // Create all required directories
    sceIoMkdir(DATA_PATH, 0777);
    sceIoMkdir(ZIM_PATH, 0777);
    sceIoMkdir(VAULT_PATH, 0777);
    sceIoMkdir(VAULT_PATH "items", 0777);
    sceIoMkdir(VAULT_PATH "media", 0777);
    sceIoMkdir(DB_PATH, 0777);
    sceIoMkdir(CACHE_PATH, 0777);
    sceIoMkdir(VOICE_PATH, 0777);
    sceIoMkdir(VOICE_PATH "pack", 0777);
    sceIoMkdir(DATA_PATH "models", 0777);
}

void InitApp() {
    // Load system modules
    sceSysmoduleLoadModule(SCE_SYSMODULE_IME);
    
    // Initialize vita2d
    vita2d_init();
    vita2d_set_clear_color(COLOR_BLACK);
    
    // Create directories
    CreateDirectories();
    
    // Load fonts
    g_app.font = vita2d_load_default_pgf();
    g_app.fontSmall = vita2d_load_default_pgf();
    
    // Initialize subsystems
    g_app.db = new Database();
    g_app.zimReader = new ZIMReader();
    g_app.voice = new VoiceSystem();
    
    // Initialize online components
    g_app.netFetcher = new NetFetcher();
    g_app.rssParser = new RSSParser();
    g_app.extractor = new ContentExtractor();
    g_app.onlineSearch = new OnlineSearch();
    
    // Initialize network
    if (g_app.netFetcher->Initialize()) {
        g_app.online = g_app.netFetcher->IsOnline();
    } else {
        g_app.online = false;
    }
    
    // Load RSS feed configuration
    std::string feedConfigPath = std::string(DATA_PATH) + "feeds.json";
    g_app.rssParser->LoadFeedConfig(feedConfigPath);
    
    // Initialize online search coordinator
    g_app.onlineSearch->Initialize(g_app.netFetcher, g_app.rssParser,
                                   g_app.extractor, g_app.db);
    
    // Initialize LLM engine
    g_app.llm = new LLMEngine();
    g_app.llmEnabled = true; // LLM enabled by default
    
    // Try to load LLM model (if exists)
    std::string modelPath = std::string(DATA_PATH) + "models/model.gguf";
    if (g_app.llm->LoadModel(modelPath)) {
        printf("LLM model loaded successfully!\n");
    } else {
        printf("No LLM model found at %s - using template answers\n", modelPath.c_str());
        g_app.llmEnabled = false;
    }
    
    // Initialize search engine (with online + LLM support)
    g_app.search = new SearchEngine();
    g_app.search->Initialize(g_app.db, g_app.zimReader, g_app.onlineSearch, g_app.llm);
    
    // Initialize database
    std::string dbPath = std::string(DB_PATH) + "vault.sqlite";
    if (g_app.db->Initialize(dbPath)) {
        g_app.db->CreateTables();
        g_app.db->CreateFTSIndex();
    }
    
    // Try to load Wikipedia ZIM (if exists)
    std::string zimPath = std::string(ZIM_PATH) + "wikipedia_en.zim";
    g_app.zimReader->LoadZIM(zimPath);
    
    // Initialize voice system
    std::string voicePath = std::string(VOICE_PATH) + "pack/";
    g_app.voice->Initialize(voicePath);
    
    // Initialize UI
    g_app.ui = new UI();
    g_app.ui->Initialize();
    
    // Set initial state
    g_app.currentState = STATE_ASK;
    g_app.running = true;
    g_app.onlineModeEnabled = true;  // Online mode ON by default
    
    // Clear input
    memset(&g_app.pad, 0, sizeof(SceCtrlData));
    memset(&g_app.oldPad, 0, sizeof(SceCtrlData));
}

void ShutdownApp() {
    // Shutdown subsystems
    if (g_app.ui) {
        g_app.ui->Shutdown();
        delete g_app.ui;
    }
    
    if (g_app.voice) {
        g_app.voice->Shutdown();
        delete g_app.voice;
    }
    
    if (g_app.search) {
        delete g_app.search;
    }
    
    if (g_app.llm) {
        g_app.llm->UnloadModel();
        delete g_app.llm;
    }
    
    if (g_app.onlineSearch) {
        delete g_app.onlineSearch;
    }
    
    if (g_app.extractor) {
        delete g_app.extractor;
    }
    
    if (g_app.rssParser) {
        delete g_app.rssParser;
    }
    
    if (g_app.netFetcher) {
        g_app.netFetcher->Shutdown();
        delete g_app.netFetcher;
    }
    
    if (g_app.zimReader) {
        g_app.zimReader->Close();
        delete g_app.zimReader;
    }
    
    if (g_app.db) {
        g_app.db->Close();
        delete g_app.db;
    }
    
    // Shutdown vita2d
    vita2d_fini();
    vita2d_free_pgf(g_app.font);
    vita2d_free_pgf(g_app.fontSmall);
    
    // Unload system modules
    sceSysmoduleUnloadModule(SCE_SYSMODULE_IME);
}

bool IsButtonPressed(uint32_t button) {
    return (g_app.pad.buttons & button) && !(g_app.oldPad.buttons & button);
}

bool IsButtonHeld(uint32_t button) {
    return (g_app.pad.buttons & button);
}

int main() {
    // Initialize app
    InitApp();
    
    // Main loop
    while (g_app.running) {
        // Store old pad state
        g_app.oldPad = g_app.pad;
        
        // Read input
        sceCtrlPeekBufferPositive(0, &g_app.pad, 1);
        
        // Handle input
        g_app.ui->HandleInput(g_app.pad, g_app.oldPad);
        
        // Check for exit (Start + Select)
        if ((g_app.pad.buttons & SCE_CTRL_START) && 
            (g_app.pad.buttons & SCE_CTRL_SELECT)) {
            g_app.running = false;
        }
        
        // Update
        static uint64_t lastTime = sceKernelGetProcessTimeWide();
        uint64_t currentTime = sceKernelGetProcessTimeWide();
        float deltaTime = (currentTime - lastTime) / 1000000.0f;
        lastTime = currentTime;
        
        g_app.ui->Update(deltaTime);
        
        // Render
        vita2d_start_drawing();
        vita2d_clear_screen();
        
        g_app.ui->Render();
        
        vita2d_end_drawing();
        vita2d_swap_buffers();
        
        // Cap at 60 FPS
        sceKernelDelayThread(16666);
    }
    
    // Cleanup
    ShutdownApp();
    
    sceKernelExitProcess(0);
    return 0;
}
