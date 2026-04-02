#pragma once
#include <string>
#include <tesla.hpp>
#include "sysclk_api.hpp"

namespace sysclk {

class PresetPickerGui : public tsl::Gui {
public:
    PresetPickerGui(uint64_t appId, SysClkUiMode mode);

    void notifyCustomApplied(const SysClkPresetValues& values);
    void update() override;
    tsl::elm::Element* createUI() override;

private:
    uint64_t m_appId;
    SysClkUiMode m_mode;

    tsl::elm::OverlayFrame* m_frame = nullptr;
    tsl::elm::CategoryHeader* m_profilesHeader = nullptr;
    tsl::elm::CustomDrawer* m_currentProfileRow = nullptr;
    tsl::elm::ListItem* m_customItem = nullptr;

    std::string m_currentProfileTitle;
    std::string m_currentProfileValue;

    SysClkPresetValues m_current;
    bool m_currentOk = false;
    uint64_t m_lastRefreshTick = 0;

    tsl::elm::CustomDrawer* createCompactRow(std::string* left, std::string* right);
    bool loadCurrentFromSystemOrFile();
    std::string buildHeaderText() const;
    void refreshHeader();
    void rebuildCurrentProfileStrings();
    void refreshCustomItem();
    Result applyCurrentSelection(const SysClkPresetValues& values);
};
} 