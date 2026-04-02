#include "app_overlay.hpp"
#include "app_utils.hpp"
#include "sysclk_api.hpp"

ExtraHandOverlay::ExtraHandOverlay() {
    ensureConfigDirectory();
    sysclk::initialize();
    loadConfig();
    applyFooterLanguage(currentLang);

    const HidNpadIdType ids[] = {
        HidNpadIdType_No1,
        HidNpadIdType_No2,
        HidNpadIdType_No3,
        HidNpadIdType_No4,
        HidNpadIdType_No5,
        HidNpadIdType_No6,
        HidNpadIdType_No7,
        HidNpadIdType_No8,
        HidNpadIdType_Handheld
    };

    hidInitializeNpad();
    hidSetSupportedNpadStyleSet(
        HidNpadStyleSet_NpadStandard |
        HidNpadStyleTag_NpadGc |
        HidNpadStyleTag_NpadPalma |
        HidNpadStyleTag_NpadLark |
        HidNpadStyleTag_NpadHandheldLark |
        HidNpadStyleTag_NpadLucia |
        HidNpadStyleTag_NpadLagon |
        HidNpadStyleTag_NpadLager |
        HidNpadStyleTag_NpadSystemExt |
        HidNpadStyleTag_NpadSystem
    );
    hidSetSupportedNpadIdType(ids, sizeof(ids) / sizeof(ids[0]));

    updateControllerOrderCache(true);
}

std::unique_ptr<tsl::Gui> ExtraHandOverlay::loadInitialGui() {
    return std::make_unique<MainGui>();
}