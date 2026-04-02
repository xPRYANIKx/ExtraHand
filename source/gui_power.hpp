#pragma once
#include <string>
#include <tesla.hpp>
#include "sysclk_api.hpp"

class PowerGui : public tsl::Gui {
public:
    PowerGui();
    ~PowerGui() override;

    void refreshTexts();
    void update() override;
    tsl::elm::Element* createUI() override;

private:
    tsl::elm::OverlayFrame* m_frame = nullptr;

    tsl::elm::CategoryHeader* m_infoHeader = nullptr;
    tsl::elm::CategoryHeader* m_actionsHeader = nullptr;

    std::string m_powerTitle;
    std::string m_powerValue;
    std::string m_tempTitle;
    std::string m_tempValue;

    tsl::elm::CustomDrawer* m_powerRow = nullptr;
    tsl::elm::CustomDrawer* m_tempRow = nullptr;

    tsl::elm::ListItem* m_rebootItem = nullptr;
    tsl::elm::ListItem* m_shutdownItem = nullptr;

    sysclk::SysClkContext m_ctx{};
    bool m_ctxValid = false;
    uint64_t m_lastCtxTick = 0;

    void refreshContext();
};

extern PowerGui* g_powerGui;