#include <draw.h>
#include <configMenu.h>
#include <cstring>
#include <input.h>
#include <language.h>
#include <savemng.h>
#include <state.h>

#define M_OFF 1

static int cursorPos = 0;
bool redraw = true;

static std::string language = "";

static void drawConfigMenuFrame() {
    OSScreenClearBufferEx(SCREEN_DRC, 0x00006F00);
    OSScreenClearBufferEx(SCREEN_TV, 0x00006F00);
    consolePrintPos(0, 0, "SaveMii v%u.%u.%u", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
    consolePrintPos(0, 1, "----------------------------------------------------------------------------");
    consolePrintPos(M_OFF, 2, gettext("   Language: %s"), language.c_str());
    consolePrintPos(M_OFF, 2 + cursorPos, "\u2192");
    consolePrintPos(0, 16, "----------------------------------------------------------------------------");
    consolePrintPos(0, 17, gettext("Press \ue044 to exit."));
    flipBuffers();
    WHBLogFreetypeDraw();
}

void configMenu() {
    Input input;
    language = getLoadedLanguage();
    while (AppRunning()) {
        input.read();
        if (input.get(TRIGGER, PAD_BUTTON_ANY))
            redraw = true;
        if (input.get(TRIGGER, PAD_BUTTON_B))
            break;
        if (input.get(TRIGGER, PAD_BUTTON_RIGHT)) {
            if (language == "Japanese")
                loadLanguage(Swkbd_LanguageType__Italian);
            else if (language == "Italian")
                loadLanguage(Swkbd_LanguageType__Spanish);
            else if (language == "Spanish")
                loadLanguage(Swkbd_LanguageType__Chinese1);
            else if (language == "Traditional Chinese")
                loadLanguage(Swkbd_LanguageType__Korean);
            else if (language == "Korean")
                loadLanguage(Swkbd_LanguageType__Russian);
            else if (language == "Russian")
                loadLanguage(Swkbd_LanguageType__Chinese2);
            else if (language == "Simplified Chinese")
                loadLanguage(Swkbd_LanguageType__English);
            else if (language == "English")
                loadLanguage(Swkbd_LanguageType__Japanese);
        }
        if (redraw) {
            language = getLoadedLanguage();
            drawConfigMenuFrame();
            redraw = false;
        }
    }
    return;
}