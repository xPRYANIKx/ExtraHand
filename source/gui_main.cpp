#include "gui_main.hpp"
#include "app_utils.hpp"
#include "localization.hpp"

MainGui* g_mainGui = nullptr;

MainGui::MainGui() {
    g_mainGui = this;
}

MainGui::~MainGui() {
    if (g_mainGui == this)
        g_mainGui = nullptr;
}

void MainGui::refreshTexts() {
    if (m_primaryMenuHeader != nullptr) m_primaryMenuHeader->setText(tr(currentLang, TextId::Menu));
    if (m_perfItem != nullptr)          m_perfItem->setText(tr(currentLang, TextId::Performance));
    if (m_cheatsItem != nullptr)        m_cheatsItem->setText(tr(currentLang, TextId::CheatCodesMenu));
    if (m_systemInfoItem != nullptr)    m_systemInfoItem->setText(tr(currentLang, TextId::SystemInformation));
    if (m_settingsItem != nullptr)      m_settingsItem->setText(tr(currentLang, TextId::OverlaySettings));

    if (m_powerItem != nullptr) {
        const std::string glyph = "\uE040";
        m_powerItem->setText(glyph + "  " + tr(currentLang, TextId::Power));
    }
}

tsl::elm::Element* MainGui::createUI() {
    m_frame = new tsl::elm::OverlayFrame(tr(currentLang, TextId::AppName), "v1.0");
    auto* list = new tsl::elm::List();

    m_primaryMenuHeader = new tsl::elm::CategoryHeader(tr(currentLang, TextId::Menu));
    list->addItem(m_primaryMenuHeader);

    m_perfItem = new tsl::elm::ListItem(tr(currentLang, TextId::Performance));
    m_perfItem->setClickListener([](u64 keys) {
        if (!(keys & HidNpadButton_A))
            return false;

        std::string err;
        if (!sysclk::ensureReady(err))
            return true;

        tsl::changeTo<sysclk::PerformanceGui>();
        return true;
        });
    list->addItem(m_perfItem);

    m_cheatsItem = new tsl::elm::ListItem(tr(currentLang, TextId::CheatCodesMenu));
    m_cheatsItem->setClickListener([](u64 keys) {
        if (!(keys & HidNpadButton_A))
            return false;

        tsl::changeTo<CheatsGui>();
        return true;
        });
    list->addItem(m_cheatsItem);

    m_systemInfoItem = new tsl::elm::ListItem(tr(currentLang, TextId::SystemInformation));
    m_systemInfoItem->setClickListener([](u64 keys) {
        if (keys & HidNpadButton_A) {
            tsl::changeTo<SystemInfoGui>();
            return true;
        }
        return false;
        });
    list->addItem(m_systemInfoItem);

    m_settingsItem = new tsl::elm::ListItem(tr(currentLang, TextId::OverlaySettings));
    m_settingsItem->setClickListener([](u64 keys) {
        if (keys & HidNpadButton_A) {
            tsl::changeTo<SettingsGui>();
            return true;
        }
        return false;
        });
    list->addItem(m_settingsItem);

    const std::string glyph = "\uE040";

    m_powerItem = new tsl::elm::ListItem(
        glyph + "  " + tr(currentLang, TextId::Power)
    );

    m_powerItem->setClickListener([](u64 keys) {
        if (!(keys & HidNpadButton_A))
            return false;

        std::string err;
        if (!sysclk::ensureReady(err))
            return true;

        tsl::changeTo<PowerGui>();
        return true;
        });
    list->addItem(m_powerItem);

    m_frame->setContent(list);
    return m_frame;
}