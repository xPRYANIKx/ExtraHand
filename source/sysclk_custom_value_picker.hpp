#pragma once
#include <tesla.hpp>
#include "sysclk_api.hpp"

namespace sysclk {

class CustomPresetGui;

class CustomValuePickerGui : public tsl::Gui {
public:
    CustomValuePickerGui(CustomPresetGui* parent, SysClkModule module);

    tsl::elm::Element* createUI() override;

private:
    CustomPresetGui* m_parent;
    SysClkModule m_module;
};

} 