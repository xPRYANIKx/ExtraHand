#pragma once
#include <tesla.hpp>
#include "app_utils.hpp"

class HotkeysGui : public tsl::Gui {
public:
    tsl::elm::Element* createUI() override;

private:
    tsl::elm::ListItem* createHotkeyItem(HotkeyPreset preset);
};
