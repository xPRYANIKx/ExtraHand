#pragma once
#include <string>
#include <tesla.hpp>
#include "sysclk_api.hpp"

namespace sysclk {

class PerformanceGui : public tsl::Gui {
public:
    PerformanceGui();
    ~PerformanceGui() override;

    void refreshTexts();
    void update() override;
    tsl::elm::Element* createUI() override;

private:
    tsl::elm::OverlayFrame* m_frame = nullptr;
    tsl::elm::CategoryHeader* m_infoHeader = nullptr;
    tsl::elm::CategoryHeader* m_profilesHeader = nullptr;
    std::string m_consoleModelTitle;
    std::string m_consoleModelValue;
    std::string m_currentTypeTitle;
    std::string m_currentTypeValue;
    std::string m_usbConnectedTitle;
    std::string m_usbConnectedValue;
    std::string m_resolutionTitle;
    std::string m_resolutionValue;
    std::string m_profileHandheldTitle;
    std::string m_profileHandheldValue;
    std::string m_profileDockTitle;
    std::string m_profileDockValue;
    std::string m_profileChargingTitle;
    std::string m_profileChargingValue;
    std::string m_cpuTitle;
    std::string m_cpuValue;
    std::string m_gpuTitle;
    std::string m_gpuValue;
    std::string m_memTitle;
    std::string m_memValue;
    std::string m_powerTitle;
    std::string m_powerValue;
    std::string m_tempTitle;
    std::string m_tempValue;
    std::string m_infoHeaderText;
    std::string m_profilesHeaderText;

    tsl::elm::CustomDrawer* m_consoleModelRow = nullptr;
    tsl::elm::CustomDrawer* m_currentTypeRow = nullptr;
    tsl::elm::CustomDrawer* m_usbConnectedRow = nullptr;
    tsl::elm::CustomDrawer* m_resolutionRow = nullptr;

    tsl::elm::ListItem* m_handheldProfileItem = nullptr;
    tsl::elm::ListItem* m_dockProfileItem = nullptr;
    tsl::elm::ListItem* m_chargingProfileItem = nullptr;

    tsl::elm::CustomDrawer* m_cpuRow = nullptr;
    tsl::elm::CustomDrawer* m_gpuRow = nullptr;
    tsl::elm::CustomDrawer* m_memRow = nullptr;
    tsl::elm::CustomDrawer* m_powerRow = nullptr;
    tsl::elm::CustomDrawer* m_tempRow = nullptr;

    SysClkContext m_ctx{};
    bool m_ctxValid = false;

    SysClkAppModePresetFile m_modePresets{};
    bool m_modePresetsValid = false;

    uint64_t m_lastCtxTick = 0;
    uint64_t m_lastPresetTick = 0;

    void refreshModePresets();
    void refreshContext();
    void rebuildStrings();
    tsl::elm::CustomDrawer* createCompactRow(std::string* left, std::string* right);
};

extern PerformanceGui* g_sysclkPerformanceGui;

} 