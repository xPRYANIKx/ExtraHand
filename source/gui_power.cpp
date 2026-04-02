#include "gui_power.hpp"
#include "app_utils.hpp"
#include "gui_message.hpp"
#include "localization.hpp"
#include "sysclk_api.hpp"
#include "sysclk_common.hpp"

PowerGui* g_powerGui = nullptr;

PowerGui::PowerGui() {
    g_powerGui = this;
    m_powerTitle = tr(currentLang, TextId::PowerShort);
    m_tempTitle = tr(currentLang, TextId::TempShort);
}

PowerGui::~PowerGui() {
    if (g_powerGui == this)
        g_powerGui = nullptr;
}

void PowerGui::refreshContext() {
    std::string err;
    if (!sysclk::ensureReady(err)) {
        m_ctxValid = false;
        m_powerValue = "-";
        m_tempValue = "-";
        return;
    }

    const uint64_t ticks = armGetSystemTick();
    if (!m_ctxValid || armTicksToNs(ticks - m_lastCtxTick) > 250000000ULL) {
        m_lastCtxTick = ticks;
        Result rc = sysclk::getCurrentContext(&m_ctx);
        m_ctxValid = R_SUCCEEDED(rc);
    }

    if (!m_ctxValid) {
        m_powerValue = "-";
        m_tempValue = "-";
        return;
    }

    m_powerValue =
        std::string(tr(currentLang, TextId::PowerNowPrefix)) + " " +
        sysclk::formatPowerMilliwatts((int)m_ctx.power[sysclk::SysClkPowerSensor_Now]) +
        " | " +
        tr(currentLang, TextId::PowerAvgPrefix) + " " +
        sysclk::formatPowerMilliwatts((int)m_ctx.power[sysclk::SysClkPowerSensor_Avg]);

    m_tempValue = sysclk::formatTemperatureSummary(m_ctx);
}

void PowerGui::refreshTexts() {
    if (m_frame != nullptr)
        m_frame->setSubtitle(tr(currentLang, TextId::Power));
    if (m_infoHeader != nullptr)
        m_infoHeader->setText(tr(currentLang, TextId::Info));
    if (m_actionsHeader != nullptr)
        m_actionsHeader->setText(tr(currentLang, TextId::Actions));
    if (m_rebootItem != nullptr)
        m_rebootItem->setText(tr(currentLang, TextId::Reboot));
    if (m_shutdownItem != nullptr)
        m_shutdownItem->setText(tr(currentLang, TextId::Shutdown));

    m_powerTitle = tr(currentLang, TextId::PowerShort);
    m_tempTitle = tr(currentLang, TextId::TempShort);

    refreshContext();
}

void PowerGui::update() {
    refreshContext();
}

tsl::elm::Element* PowerGui::createUI() {
    m_frame = new tsl::elm::OverlayFrame(
        tr(currentLang, TextId::AppName),
        tr(currentLang, TextId::Power)
    );

    refreshContext();

    auto* list = new tsl::elm::List();

    auto blueInfoRow = [](std::string* left, std::string* right) -> tsl::elm::CustomDrawer* {
        return new tsl::elm::CustomDrawer(
            [left, right](tsl::gfx::Renderer* renderer, u16 x, u16 y, u16 w, u16 h) {
                renderer->drawRect(x, y + h - 1, w, 1, tsl::gfx::Renderer::a({ 0x3, 0x3, 0x3, 0xF }));

                renderer->drawString(
                    left->c_str(),
                    false,
                    x + 8,
                    y + 18,
                    14,
                    tsl::gfx::Renderer::a({ 0x9, 0x9, 0x9, 0xF })
                );

                auto size = renderer->drawString(
                    right->c_str(),
                    false,
                    0,
                    0,
                    14,
                    tsl::style::color::ColorTransparent
                );

                renderer->drawString(
                    right->c_str(),
                    false,
                    x + w - static_cast<u16>(size.first) - 8,
                    y + 18,
                    14,
                    tsl::gfx::Renderer::a({ 0x7, 0xD, 0xF, 0xF })
                );
            }
        );
        };

    m_infoHeader = new tsl::elm::CategoryHeader(tr(currentLang, TextId::Info));
    list->addItem(m_infoHeader);

    m_powerRow = blueInfoRow(&m_powerTitle, &m_powerValue);
    list->addItem(m_powerRow, 28);

    m_tempRow = blueInfoRow(&m_tempTitle, &m_tempValue);
    list->addItem(m_tempRow, 28);

    m_actionsHeader = new tsl::elm::CategoryHeader(tr(currentLang, TextId::Actions));
    list->addItem(m_actionsHeader);

    m_rebootItem = new tsl::elm::ListItem(tr(currentLang, TextId::Reboot));
    m_rebootItem->setClickListener([](u64 keys) {
        if (!(keys & HidNpadButton_A))
            return false;

        Result rc = doReboot();
        if (R_FAILED(rc)) {
            tsl::changeTo<MessageGui>(
                tr(currentLang, TextId::Error),
                std::string(tr(currentLang, TextId::CommandFailedPrefix)) + resultToHex(rc)
            );
        }
        return true;
        });
    list->addItem(m_rebootItem);

    m_shutdownItem = new tsl::elm::ListItem(tr(currentLang, TextId::Shutdown));
    m_shutdownItem->setClickListener([](u64 keys) {
        if (!(keys & HidNpadButton_A))
            return false;

        Result rc = doShutdown();
        if (R_FAILED(rc)) {
            tsl::changeTo<MessageGui>(
                tr(currentLang, TextId::Error),
                std::string(tr(currentLang, TextId::CommandFailedPrefix)) + resultToHex(rc)
            );
        }
        return true;
        });
    list->addItem(m_shutdownItem);

    m_frame->setContent(list);
    return m_frame;
}