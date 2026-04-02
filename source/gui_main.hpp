#pragma once
#include <tesla.hpp>
#include "sysclk_performance.hpp"
#include "gui_settings.hpp"
#include "gui_power.hpp"
#include "gui_message.hpp"
#include "gui_cheats.hpp"

class MainGui : public tsl::Gui {
public:
    MainGui();
    ~MainGui() override;

    void refreshTexts();
    tsl::elm::Element* createUI() override;

private:
    tsl::elm::OverlayFrame* m_frame = nullptr;
    tsl::elm::CategoryHeader* m_primaryMenuHeader = nullptr;
    tsl::elm::ListItem* m_perfItem = nullptr;
    tsl::elm::ListItem* m_cheatsItem = nullptr;
    tsl::elm::ListItem* m_systemInfoItem = nullptr;
    tsl::elm::ListItem* m_settingsItem = nullptr;
    tsl::elm::ListItem* m_powerItem = nullptr;
};

extern MainGui* g_mainGui;