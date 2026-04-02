#pragma once
#include <tesla.hpp>
#include "sysclk_api.hpp"

namespace sysclk {

class PresetPickerGui;

class CustomPresetGui : public tsl::Gui {
public:
    CustomPresetGui(
        PresetPickerGui* parentPicker,
        uint64_t appId,
        SysClkUiMode mode,
        const SysClkPresetValues& initialValues
    );

    void refreshValues();
    void setValue(SysClkModule module, uint32_t value);
    const SysClkPresetValues& getValues() const;

    virtual tsl::elm::Element* createUI() override;

private:
    uint64_t m_appId = 0;
    SysClkUiMode m_mode = SysClkUiMode::Handheld;
    SysClkPresetValues m_values{};
    PresetPickerGui* m_parentPicker = nullptr;

    tsl::elm::OverlayFrame* m_frame = nullptr;
    tsl::elm::ListItem* m_cpuItem = nullptr;
    tsl::elm::ListItem* m_gpuItem = nullptr;
    tsl::elm::ListItem* m_memItem = nullptr;
};

}