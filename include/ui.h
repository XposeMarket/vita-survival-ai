#ifndef UI_H
#define UI_H

#include <string>
#include <vector>
#include <vita2d.h>
#include <psp2/ime_dialog.h>
#include "survival_ai.h"
#include "search_engine.h"

// UI Screen types
enum UIScreen {
    SCREEN_MAIN_MENU,
    SCREEN_ASK,
    SCREEN_ASK_RESULTS,
    SCREEN_LIBRARY,
    SCREEN_WIKIPEDIA,
    SCREEN_WIKIPEDIA_ARTICLE,
    SCREEN_VAULT,
    SCREEN_VAULT_ITEM,
    SCREEN_MANUALS,
    SCREEN_SCENARIOS,
    SCREEN_TOOLKIT,
    SCREEN_SYNC,
    SCREEN_SETTINGS
};

// Keyboard input for queries
struct KeyboardInput {
    std::string text;
    bool active;
    bool submitted;
    SceImeDialogResult result;
    uint16_t inputTextBuffer[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1];
    uint8_t param[SCE_IME_DIALOG_MAX_OPTION_SIZE];
};

class UI {
public:
    UI();
    ~UI();
    
    bool Initialize();
    void Shutdown();
    
    void Update(float deltaTime);
    void Render();
    void HandleInput(const SceCtrlData& pad, const SceCtrlData& oldPad);
    
    // Screen management
    void SetScreen(UIScreen screen);
    UIScreen GetCurrentScreen() const { return currentScreen; }
    
    // Keyboard
    void ShowKeyboard(const std::string& title, const std::string& initialText = "");
    void HideKeyboard();
    bool IsKeyboardActive() const { return keyboard.active; }
    std::string GetKeyboardText() const { return keyboard.text; }
    
    // Answer display
    void DisplayAnswer(const Answer& answer);
    void ClearAnswer();
    
    // Lists
    void SetListItems(const std::vector<std::string>& items);
    int GetSelectedIndex() const { return selectedIndex; }
    
    // Notifications
    void ShowNotification(const std::string& message, float duration = 3.0f);
    
    // Loading
    void SetLoading(bool loading, const std::string& message = "");
    
private:
    UIScreen currentScreen;
    UIScreen previousScreen;
    
    // Fonts
    vita2d_pgf* font;
    vita2d_pgf* fontSmall;
    vita2d_pgf* fontLarge;
    
    // Input
    KeyboardInput keyboard;
    
    // State
    int selectedIndex;
    int scrollOffset;
    std::vector<std::string> listItems;
    
    // Current answer
    Answer* currentAnswer;
    int answerScrollPos;
    
    // Notifications
    std::string notification;
    float notificationTimer;
    
    // Loading
    bool isLoading;
    std::string loadingMessage;
    float loadingSpinner;
    
    // Screen renderers
    void RenderMainMenu();
    void RenderAsk();
    void RenderAskResults();
    void RenderLibrary();
    void RenderWikipedia();
    void RenderWikipediaArticle();
    void RenderVault();
    void RenderVaultItem();
    void RenderManuals();
    void RenderScenarios();
    void RenderToolkit();
    void RenderSync();
    void RenderSettings();
    
    // UI Components
    void RenderHeader(const std::string& title);
    void RenderList(const std::vector<std::string>& items, int selected, int scroll);
    void RenderAnswer(const Answer& answer, int scrollPos);
    void RenderKeyboard();
    void RenderNotification();
    void RenderLoading();
    void RenderButton(const std::string& label, int x, int y, uint32_t button);
    void RenderSourcesList(const std::vector<SourceInfo>& sources);
    
    // Helpers
    void DrawText(const std::string& text, int x, int y, uint32_t color, vita2d_pgf* font);
    void DrawTextWrapped(const std::string& text, int x, int y, int maxWidth, uint32_t color);
    int GetTextWidth(const std::string& text, vita2d_pgf* font);
    std::vector<std::string> WrapText(const std::string& text, int maxWidth, vita2d_pgf* font);
    
    // Input helpers
    void HandleListInput(const SceCtrlData& pad, const SceCtrlData& oldPad, int itemCount);
    void HandleKeyboardInput(const SceCtrlData& pad, const SceCtrlData& oldPad);
};

#endif // UI_H
