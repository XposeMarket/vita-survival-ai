#include "ui.h"
#include "survival_ai.h"
#include <sstream>
#include <ctime>
#include <iomanip>

UI::UI() : currentScreen(SCREEN_MAIN_MENU), previousScreen(SCREEN_MAIN_MENU),
           selectedIndex(0), scrollOffset(0), currentAnswer(nullptr),
           answerScrollPos(0), notificationTimer(0.0f), isLoading(false),
           loadingSpinner(0.0f) {
    keyboard.active = false;
    keyboard.submitted = false;
    keyboard.cursorPos = 0;
}

UI::~UI() {
    if (currentAnswer) {
        delete currentAnswer;
    }
}

bool UI::Initialize() {
    font = g_app.font;
    fontSmall = g_app.fontSmall;
    fontLarge = vita2d_load_default_pgf();
    
    return true;
}

void UI::Shutdown() {
    if (fontLarge) {
        vita2d_free_pgf(fontLarge);
    }
}

void UI::Update(float deltaTime) {
    // Update notification timer
    if (notificationTimer > 0.0f) {
        notificationTimer -= deltaTime;
    }
    
    // Update loading spinner
    if (isLoading) {
        loadingSpinner += deltaTime * 360.0f;
        if (loadingSpinner >= 360.0f) {
            loadingSpinner -= 360.0f;
        }
    }
}

void UI::Render() {
    // Render current screen
    switch (currentScreen) {
        case SCREEN_MAIN_MENU:
            RenderMainMenu();
            break;
        case SCREEN_ASK:
            RenderAsk();
            break;
        case SCREEN_ASK_RESULTS:
            RenderAskResults();
            break;
        case SCREEN_LIBRARY:
            RenderLibrary();
            break;
        case SCREEN_WIKIPEDIA:
            RenderWikipedia();
            break;
        case SCREEN_VAULT:
            RenderVault();
            break;
        case SCREEN_MANUALS:
            RenderManuals();
            break;
        case SCREEN_SCENARIOS:
            RenderScenarios();
            break;
        case SCREEN_TOOLKIT:
            RenderToolkit();
            break;
        case SCREEN_SYNC:
            RenderSync();
            break;
        case SCREEN_SETTINGS:
            RenderSettings();
            break;
        default:
            break;
    }
    
    // Render keyboard if active
    if (keyboard.active) {
        RenderKeyboard();
    }
    
    // Render notification
    if (notificationTimer > 0.0f) {
        RenderNotification();
    }
    
    // Render loading overlay
    if (isLoading) {
        RenderLoading();
    }
}

void UI::HandleInput(const SceCtrlData& pad, const SceCtrlData& oldPad) {
    if (keyboard.active) {
        HandleKeyboardInput(pad, oldPad);
        return;
    }
    
    // Handle input based on current screen
    switch (currentScreen) {
        case SCREEN_MAIN_MENU:
            HandleListInput(pad, oldPad, 8); // 8 menu items
            if (IsButtonPressed(SCE_CTRL_CROSS)) {
                switch (selectedIndex) {
                    case 0: SetScreen(SCREEN_ASK); break;
                    case 1: SetScreen(SCREEN_LIBRARY); break;
                    case 2: SetScreen(SCREEN_WIKIPEDIA); break;
                    case 3: SetScreen(SCREEN_VAULT); break;
                    case 4: SetScreen(SCREEN_MANUALS); break;
                    case 5: SetScreen(SCREEN_SCENARIOS); break;
                    case 6: SetScreen(SCREEN_TOOLKIT); break;
                    case 7: SetScreen(SCREEN_SYNC); break;
                }
            }
            break;
            
        case SCREEN_ASK:
            if (IsButtonPressed(SCE_CTRL_CROSS)) {
                ShowKeyboard("Enter your question:", "");
            }
            if (IsButtonPressed(SCE_CTRL_TRIANGLE)) {
                // Toggle online mode
                g_app.onlineModeEnabled = !g_app.onlineModeEnabled;
                std::string msg = g_app.onlineModeEnabled ? "Online mode enabled" : "Offline mode enabled";
                ShowNotification(msg, 2.0f);
            }
            if (IsButtonPressed(SCE_CTRL_CIRCLE)) {
                SetScreen(SCREEN_MAIN_MENU);
            }
            break;
            
        case SCREEN_ASK_RESULTS:
            // Handle scrolling through answer
            if (IsButtonHeld(SCE_CTRL_UP)) {
                answerScrollPos = std::max(0, answerScrollPos - 10);
            }
            if (IsButtonHeld(SCE_CTRL_DOWN)) {
                answerScrollPos += 10;
            }
            if (IsButtonPressed(SCE_CTRL_TRIANGLE)) {
                // Speak answer
                if (g_app.voice && currentAnswer) {
                    g_app.voice->SpeakAnswer(*currentAnswer, VOICE_SUMMARY);
                }
            }
            if (IsButtonPressed(SCE_CTRL_CIRCLE)) {
                SetScreen(SCREEN_ASK);
                ClearAnswer();
            }
            break;
            
        case SCREEN_WIKIPEDIA:
            HandleListInput(pad, oldPad, listItems.size());
            if (IsButtonPressed(SCE_CTRL_CROSS)) {
                // Load article
                ShowNotification("Loading article...");
            }
            if (IsButtonPressed(SCE_CTRL_CIRCLE)) {
                SetScreen(SCREEN_MAIN_MENU);
            }
            break;
            
        default:
            if (IsButtonPressed(SCE_CTRL_CIRCLE)) {
                SetScreen(SCREEN_MAIN_MENU);
            }
            break;
    }
}

void UI::SetScreen(UIScreen screen) {
    previousScreen = currentScreen;
    currentScreen = screen;
    selectedIndex = 0;
    scrollOffset = 0;
}

void UI::ShowKeyboard(const std::string& title, const std::string& initialText) {
    keyboard.text = initialText;
    keyboard.active = true;
    keyboard.submitted = false;
    keyboard.cursorPos = initialText.length();
}

void UI::HideKeyboard() {
    keyboard.active = false;
}

void UI::DisplayAnswer(const Answer& answer) {
    if (currentAnswer) {
        delete currentAnswer;
    }
    currentAnswer = new Answer(answer);
    answerScrollPos = 0;
    SetScreen(SCREEN_ASK_RESULTS);
}

void UI::ClearAnswer() {
    if (currentAnswer) {
        delete currentAnswer;
        currentAnswer = nullptr;
    }
    answerScrollPos = 0;
}

void UI::ShowNotification(const std::string& message, float duration) {
    notification = message;
    notificationTimer = duration;
}

void UI::SetLoading(bool loading, const std::string& message) {
    isLoading = loading;
    loadingMessage = message;
    loadingSpinner = 0.0f;
}

// Screen renderers
void UI::RenderMainMenu() {
    RenderHeader("Survival AI");
    
    std::vector<std::string> menuItems = {
        "Ask",
        "Library",
        "Wikipedia",
        "Vault",
        "Manuals",
        "Scenarios",
        "Toolkit",
        "Sync"
    };
    
    RenderList(menuItems, selectedIndex, scrollOffset);
    
    // Status bar
    DrawText("Press X to select | Circle to exit", 20, SCREEN_HEIGHT - 40, COLOR_GRAY, fontSmall);
    
    // Online status and mode
    std::string status;
    uint32_t statusColor;
    
    if (g_app.online && g_app.onlineModeEnabled) {
        status = "ONLINE MODE";
        statusColor = COLOR_GREEN;
    } else if (g_app.online && !g_app.onlineModeEnabled) {
        status = "OFFLINE MODE (WiFi ON)";
        statusColor = COLOR_YELLOW;
    } else {
        status = "OFFLINE";
        statusColor = COLOR_RED;
    }
    
    DrawText(status, SCREEN_WIDTH - 250, SCREEN_HEIGHT - 40, statusColor, fontSmall);
    
    // Vault item count
    if (g_app.db) {
        int itemCount = g_app.db->GetTotalItems();
        std::string vaultInfo = "Vault: " + std::to_string(itemCount) + " items";
        DrawText(vaultInfo, SCREEN_WIDTH - 250, SCREEN_HEIGHT - 70, COLOR_GRAY, fontSmall);
    }
}

void UI::RenderAsk() {
    RenderHeader("Ask");
    
    std::string mode;
    uint32_t modeColor;
    if (g_app.online && g_app.onlineModeEnabled) {
        mode = "Online Mode: Will search web + save results";
        modeColor = COLOR_GREEN;
    } else {
        mode = "Offline Mode: Searching local vault only";
        modeColor = COLOR_YELLOW;
    }
    
    DrawText(mode, 40, 80, modeColor, fontSmall);
    
    DrawText("Press X to enter a question", 40, 140, COLOR_WHITE, font);
    DrawText("Press Triangle to toggle online/offline mode", 40, 180, COLOR_GRAY, fontSmall);
    DrawText("Press Square to view recent questions", 40, 210, COLOR_GRAY, fontSmall);
    
    RenderButton("Ask Question", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2, SCE_CTRL_CROSS);
}

void UI::RenderAskResults() {
    if (!currentAnswer) return;
    
    RenderHeader("Answer");
    
    int y = 80;
    
    // Type indicator
    std::string typeStr;
    switch (currentAnswer->type) {
        case ANSWER_DIRECT: typeStr = "Direct Answer"; break;
        case ANSWER_STEPS: typeStr = "Step-by-Step"; break;
        case ANSWER_QUOTES: typeStr = "Quotes"; break;
        case ANSWER_SUMMARY: typeStr = "Summary"; break;
        default: typeStr = "No Answer"; break;
    }
    DrawText(typeStr, 40, y, COLOR_BLUE, fontSmall);
    y += 30;
    
    // Summary
    if (!currentAnswer->summary.empty()) {
        DrawTextWrapped(currentAnswer->summary, 40, y - answerScrollPos, SCREEN_WIDTH - 80, COLOR_WHITE);
        y += 100;
    }
    
    // Steps
    if (!currentAnswer->steps.empty()) {
        DrawText("Steps:", 40, y - answerScrollPos, COLOR_YELLOW, font);
        y += 30;
        for (size_t i = 0; i < currentAnswer->steps.size(); i++) {
            std::string step = std::to_string(i+1) + ". " + currentAnswer->steps[i];
            DrawTextWrapped(step, 60, y - answerScrollPos, SCREEN_WIDTH - 100, COLOR_WHITE);
            y += 40;
        }
    }
    
    // Quotes
    if (!currentAnswer->quotes.empty()) {
        DrawText("Quotes:", 40, y - answerScrollPos, COLOR_YELLOW, font);
        y += 30;
        for (const auto& quote : currentAnswer->quotes) {
            DrawTextWrapped("\"" + quote + "\"", 60, y - answerScrollPos, SCREEN_WIDTH - 100, COLOR_WHITE);
            y += 50;
        }
    }
    
    // Sources
    if (!currentAnswer->sources.empty()) {
        y += 20;
        DrawText("Sources:", 40, y - answerScrollPos, COLOR_YELLOW, font);
        y += 30;
        RenderSourcesList(currentAnswer->sources);
    }
    
    // Controls
    DrawText("Up/Down: Scroll | Triangle: Speak | Circle: Back", 20, SCREEN_HEIGHT - 40, COLOR_GRAY, fontSmall);
}

void UI::RenderLibrary() {
    RenderHeader("Library");
    DrawText("Coming soon...", 40, 120, COLOR_GRAY, font);
}

void UI::RenderWikipedia() {
    RenderHeader("Wikipedia");
    
    if (g_app.zimReader && g_app.zimReader->IsLoaded()) {
        DrawText("Wikipedia loaded: " + g_app.zimReader->GetTitle(), 40, 80, COLOR_GREEN, fontSmall);
        DrawText("Press X to search", 40, 120, COLOR_WHITE, font);
    } else {
        DrawText("No Wikipedia file found", 40, 80, COLOR_RED, font);
        DrawText("Place wikipedia_en.zim in:", 40, 120, COLOR_GRAY, fontSmall);
        DrawText(ZIM_PATH, 40, 150, COLOR_GRAY, fontSmall);
    }
}

void UI::RenderVault() {
    RenderHeader("Vault");
    
    if (g_app.db) {
        int totalItems = g_app.db->GetTotalItems();
        std::string itemStr = std::to_string(totalItems) + " items in vault";
        DrawText(itemStr, 40, 80, COLOR_GREEN, fontSmall);
    }
    
    DrawText("Press X to search vault", 40, 120, COLOR_WHITE, font);
}

void UI::RenderManuals() {
    RenderHeader("Manuals");
    DrawText("Coming soon...", 40, 120, COLOR_GRAY, font);
}

void UI::RenderScenarios() {
    RenderHeader("Scenarios");
    
    std::vector<std::string> scenarios = {
        "Bleeding",
        "Burns",
        "Lost/Navigation",
        "No Water",
        "Cold Weather",
        "Hot Weather",
        "Shelter",
        "Food/Hunting"
    };
    
    RenderList(scenarios, selectedIndex, scrollOffset);
}

void UI::RenderToolkit() {
    RenderHeader("Toolkit");
    
    std::vector<std::string> tools = {
        "SOS Signal Patterns",
        "Morse Code Helper",
        "Flashlight/Strobe",
        "Unit Converter",
        "Checklist Generator"
    };
    
    RenderList(tools, selectedIndex, scrollOffset);
}

void UI::RenderSync() {
    RenderHeader("Sync");
    DrawText("Coming soon...", 40, 120, COLOR_GRAY, font);
}

void UI::RenderSettings() {
    RenderHeader("Settings");
    DrawText("Coming soon...", 40, 120, COLOR_GRAY, font);
}

// UI Components
void UI::RenderHeader(const std::string& title) {
    vita2d_draw_rectangle(0, 0, SCREEN_WIDTH, 60, COLOR_DARK_GRAY);
    DrawText(title, 40, 20, COLOR_WHITE, fontLarge);
    vita2d_draw_line(0, 60, SCREEN_WIDTH, 60, COLOR_BLUE);
}

void UI::RenderList(const std::vector<std::string>& items, int selected, int scroll) {
    int y = 100;
    int visibleItems = 8;
    
    for (int i = scroll; i < std::min((int)items.size(), scroll + visibleItems); i++) {
        uint32_t color = (i == selected) ? COLOR_BLUE : COLOR_WHITE;
        uint32_t bgColor = (i == selected) ? RGBA8(0, 120, 215, 50) : COLOR_BLACK;
        
        if (i == selected) {
            vita2d_draw_rectangle(20, y - 5, SCREEN_WIDTH - 40, 35, bgColor);
        }
        
        DrawText(items[i], 40, y, color, font);
        y += 40;
    }
}

void UI::RenderSourcesList(const std::vector<SourceInfo>& sources) {
    int y = answerScrollPos + 350;
    
    for (size_t i = 0; i < sources.size(); i++) {
        const auto& source = sources[i];
        
        // Title
        DrawText(std::to_string(i+1) + ". " + source.title, 60, y, COLOR_BLUE, fontSmall);
        y += 25;
        
        // Domain and date
        std::string info = source.domain;
        if (source.published > 0) {
            char dateStr[32];
            struct tm* tm = localtime(&source.published);
            strftime(dateStr, sizeof(dateStr), " | %Y-%m-%d", tm);
            info += dateStr;
        }
        DrawText(info, 80, y, COLOR_GRAY, fontSmall);
        y += 30;
    }
}

void UI::RenderKeyboard() {
    // Simplified keyboard - in real implementation, use SCE OSK
    int kbHeight = 200;
    vita2d_draw_rectangle(0, SCREEN_HEIGHT - kbHeight, SCREEN_WIDTH, kbHeight, RGBA8(40, 40, 40, 240));
    
    DrawText("Type: " + keyboard.text, 40, SCREEN_HEIGHT - kbHeight + 20, COLOR_WHITE, font);
    DrawText("Press START to submit | SELECT to cancel", 40, SCREEN_HEIGHT - 40, COLOR_GRAY, fontSmall);
}

void UI::RenderNotification() {
    int width = 400;
    int height = 60;
    int x = (SCREEN_WIDTH - width) / 2;
    int y = SCREEN_HEIGHT - 100;
    
    vita2d_draw_rectangle(x, y, width, height, RGBA8(0, 0, 0, 200));
    vita2d_draw_line(x, y, x + width, y, COLOR_BLUE);
    vita2d_draw_line(x, y + height, x + width, y + height, COLOR_BLUE);
    
    DrawText(notification, x + 20, y + 20, COLOR_WHITE, font);
}

void UI::RenderLoading() {
    vita2d_draw_rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, RGBA8(0, 0, 0, 150));
    
    DrawText(loadingMessage.empty() ? "Loading..." : loadingMessage, 
             SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2, COLOR_WHITE, font);
    
    // Simple spinner (just text for now)
    const char* spinChars = "|/-\\";
    int spinIdx = ((int)loadingSpinner / 90) % 4;
    char spinChar[2] = {spinChars[spinIdx], 0};
    DrawText(spinChar, SCREEN_WIDTH/2 + 150, SCREEN_HEIGHT/2, COLOR_BLUE, fontLarge);
}

void UI::RenderButton(const std::string& label, int x, int y, uint32_t button) {
    vita2d_draw_rectangle(x, y, 200, 40, COLOR_BLUE);
    DrawText(label, x + 20, y + 10, COLOR_WHITE, font);
}

// Helpers
void UI::DrawText(const std::string& text, int x, int y, uint32_t color, vita2d_pgf* pgf) {
    vita2d_pgf_draw_text(pgf, x, y + 20, color, 1.0f, text.c_str());
}

void UI::DrawTextWrapped(const std::string& text, int x, int y, int maxWidth, uint32_t color) {
    auto lines = WrapText(text, maxWidth, font);
    int lineY = y;
    for (const auto& line : lines) {
        DrawText(line, x, lineY, color, font);
        lineY += 25;
    }
}

std::vector<std::string> UI::WrapText(const std::string& text, int maxWidth, vita2d_pgf* pgf) {
    std::vector<std::string> lines;
    std::istringstream words(text);
    std::string word, line;
    
    while (words >> word) {
        std::string testLine = line.empty() ? word : line + " " + word;
        if (GetTextWidth(testLine, pgf) > maxWidth) {
            if (!line.empty()) {
                lines.push_back(line);
            }
            line = word;
        } else {
            line = testLine;
        }
    }
    
    if (!line.empty()) {
        lines.push_back(line);
    }
    
    return lines;
}

int UI::GetTextWidth(const std::string& text, vita2d_pgf* pgf) {
    return vita2d_pgf_text_width(pgf, 1.0f, text.c_str());
}

void UI::HandleListInput(const SceCtrlData& pad, const SceCtrlData& oldPad, int itemCount) {
    if (IsButtonPressed(SCE_CTRL_UP)) {
        selectedIndex = (selectedIndex - 1 + itemCount) % itemCount;
    }
    if (IsButtonPressed(SCE_CTRL_DOWN)) {
        selectedIndex = (selectedIndex + 1) % itemCount;
    }
}

void UI::HandleKeyboardInput(const SceCtrlData& pad, const SceCtrlData& oldPad) {
    // Simplified - real implementation would use SCE IME/OSK
    if (IsButtonPressed(SCE_CTRL_START)) {
        keyboard.submitted = true;
        keyboard.active = false;
        
        // Process query
        if (!keyboard.text.empty() && g_app.search) {
            SetLoading(true, "Searching...");
            Answer answer = g_app.search->Ask(keyboard.text, g_app.online);
            SetLoading(false);
            DisplayAnswer(answer);
        }
    }
    if (IsButtonPressed(SCE_CTRL_SELECT)) {
        HideKeyboard();
    }
}
