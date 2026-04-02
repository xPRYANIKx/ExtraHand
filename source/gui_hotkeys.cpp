#include "gui_hotkeys.hpp"
#include "gui_settings.hpp"

tsl::elm::ListItem* HotkeysGui::createHotkeyItem(HotkeyPreset preset) {
    auto* item = new tsl::elm::ListItem(getHotkeyGlyphTitle(preset));
    item->setClickListener([preset](u64 keys) {
        if (!(keys & HidNpadButton_A))
            return false;

        saveHotkey(preset);
        refreshSettingsTexts();
        tsl::goBack();
        return true;
    });
    return item;
}

tsl::elm::Element* HotkeysGui::createUI() {
    auto* frame = new tsl::elm::OverlayFrame(
        tr(currentLang, TextId::AppName),
        tr(currentLang, TextId::Hotkeys)
    );

    auto* list = new tsl::elm::List();
    list->addItem(createHotkeyItem(HotkeyPreset::ZL_ZR_DDOWN));
    list->addItem(createHotkeyItem(HotkeyPreset::L_R_DDOWN));
    list->addItem(createHotkeyItem(HotkeyPreset::L_R_RS));
    list->addItem(createHotkeyItem(HotkeyPreset::L_DDOWN_RS));
    list->addItem(createHotkeyItem(HotkeyPreset::R_DDOWN_RS));
    frame->setContent(list);
    return frame;
}
