#include "gui_settings.hpp"
#include "gui_hotkeys.hpp"
#include "localization.hpp"
#include "sysclk_api.hpp"
#include "edizon_api.hpp"

SettingsGui* g_settingsGui = nullptr;
SystemInfoGui* g_systemInfoGui = nullptr;

SettingsGui::SettingsGui() {
    g_settingsGui = this;
}

SettingsGui::~SettingsGui() {
    if (g_settingsGui == this)
        g_settingsGui = nullptr;
}

void SettingsGui::rebuildComponentStrings() {
    m_sysclkTitle = "sys-clk";
    m_edizonTitle = tr(currentLang, TextId::AtmosphereCheatsComponent);

    std::string err;
    m_sysclkValue = tr(currentLang, sysclk::ensureReady(err) ? TextId::Working : TextId::NotWorking);
    m_edizonValue = tr(currentLang, edizon::hasInstalledComponent() ? TextId::Working : TextId::NotWorking);

    m_sysclkDescription = tr(currentLang, TextId::SysClkInlineDescription);
    m_edizonDescription = tr(currentLang, TextId::EdizonInlineDescription);
}

void SettingsGui::syncDynamicTextsIfNeeded(bool force) {
    const uint64_t now = armGetSystemTick();
    if (!force && m_lastUiSyncTick != 0 && armTicksToNs(now - m_lastUiSyncTick) < 5000000000ULL)
        return;

    m_lastUiSyncTick = now;
    rebuildComponentStrings();
}

void SettingsGui::refreshTexts() {
    if (m_frame != nullptr)
        m_frame->setSubtitle(tr(currentLang, TextId::OverlaySettings));
    if (m_componentsHeader != nullptr)
        m_componentsHeader->setText(tr(currentLang, TextId::ActiveComponents));
    if (m_overlaySettingsHeader != nullptr)
        m_overlaySettingsHeader->setText(tr(currentLang, TextId::OverlaySettings));

    if (m_hotkeyItem != nullptr) {
        m_hotkeyItem->setText(tr(currentLang, TextId::Hotkeys));
        m_hotkeyItem->setValue(getHotkeyGlyphTitle(currentHotkey), false);
    }

    if (m_langItem != nullptr) {
        m_langItem->setText(tr(currentLang, TextId::Language));
        m_langItem->setValue(getLangDisplay(currentLang), false);
    }

    rebuildComponentStrings();
}

void SettingsGui::update() {
    syncDynamicTextsIfNeeded(false);
}

tsl::elm::CustomDrawer* SettingsGui::createCompactRow(std::string* left, std::string* right) {
    return new tsl::elm::CustomDrawer(
        [left, right](tsl::gfx::Renderer* renderer, u16 x, u16 y, u16 w, u16 h) {
            const s32 fontSize = 14;
            const s32 textY = y + (h / 2) + (fontSize / 2) - 2;

            renderer->drawRect(x, y + h - 1, w, 1, tsl::gfx::Renderer::a({ 0x3, 0x3, 0x3, 0xF }));

            renderer->drawString(
                left->c_str(),
                false,
                x + 8,
                textY,
                fontSize,
                tsl::gfx::Renderer::a({ 0x9, 0x9, 0x9, 0xF })
            );

            auto size = renderer->drawString(
                right->c_str(),
                false,
                0,
                0,
                fontSize,
                tsl::style::color::ColorTransparent
            );

            renderer->drawString(
                right->c_str(),
                false,
                x + w - static_cast<u16>(size.first) - 8,
                textY,
                fontSize,
                tsl::gfx::Renderer::a({ 0x7, 0xD, 0xF, 0xF })
            );
        }
    );
}

tsl::elm::CustomDrawer* SettingsGui::createDescriptionRow(std::string* text) {
    return new tsl::elm::CustomDrawer(
        [text](tsl::gfx::Renderer* renderer, u16 x, u16 y, u16 w, u16 h) {
            renderer->drawRect(x, y + h - 1, w, 1, tsl::gfx::Renderer::a({ 0x3, 0x3, 0x3, 0xF }));

            renderer->drawString(
                text->c_str(),
                false,
                x + 8,
                y + 14,
                12,
                tsl::gfx::Renderer::a({ 0x7, 0x7, 0x7, 0xF })
            );
        }
    );
}

tsl::elm::Element* SettingsGui::createUI() {
    rebuildComponentStrings();

    m_frame = new tsl::elm::OverlayFrame(
        tr(currentLang, TextId::AppName),
        tr(currentLang, TextId::OverlaySettings)
    );

    auto* list = new tsl::elm::List();

    m_componentsHeader = new tsl::elm::CategoryHeader(tr(currentLang, TextId::ActiveComponents));
    list->addItem(m_componentsHeader);

    m_sysclkRow = createCompactRow(&m_sysclkTitle, &m_sysclkValue);
    list->addItem(m_sysclkRow, 28);

    m_sysclkDescriptionRow = createDescriptionRow(&m_sysclkDescription);
    list->addItem(m_sysclkDescriptionRow, 20);

    m_edizonRow = createCompactRow(&m_edizonTitle, &m_edizonValue);
    list->addItem(m_edizonRow, 28);

    m_edizonDescriptionRow = createDescriptionRow(&m_edizonDescription);
    list->addItem(m_edizonDescriptionRow, 20);

    m_overlaySettingsHeader = new tsl::elm::CategoryHeader(tr(currentLang, TextId::OverlaySettings));
    list->addItem(m_overlaySettingsHeader);

    m_hotkeyItem = new tsl::elm::ListItem(
        tr(currentLang, TextId::Hotkeys),
        getHotkeyGlyphTitle(currentHotkey)
    );
    m_hotkeyItem->setClickListener([](u64 keys) {
        if (keys & HidNpadButton_A) {
            tsl::changeTo<HotkeysGui>();
            return true;
        }
        return false;
        });
    list->addItem(m_hotkeyItem);

    m_langItem = new tsl::elm::ListItem(
        tr(currentLang, TextId::Language),
        getLangDisplay(currentLang)
    );
    m_langItem->setClickListener([](u64 keys) {
        if (!(keys & HidNpadButton_A))
            return false;

        currentLang = getNextLang(currentLang);
        saveConfig();
        applyFooterLanguage(currentLang);
        refreshVisibleTexts();
        return true;
        });
    list->addItem(m_langItem);

    m_frame->setContent(list);
    return m_frame;
}

SystemInfoGui::SystemInfoGui() {
    g_systemInfoGui = this;
}

SystemInfoGui::~SystemInfoGui() {
    if (g_systemInfoGui == this)
        g_systemInfoGui = nullptr;
}

void SystemInfoGui::buildControllerStringsFromSnapshot() {
    m_controllerTitles.clear();
    m_controllerValues.clear();

    for (int i = 0; i < 8; ++i) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), tr(currentLang, TextId::ControllerNumberFormat), i + 1);
        m_controllerTitles.push_back(buf);

        if (i < static_cast<int>(m_snapshot.controllers.size()))
            m_controllerValues.push_back(m_snapshot.controllers[i].value);
        else
            m_controllerValues.push_back(tr(currentLang, TextId::NoConnected));
    }
}

void SystemInfoGui::rebuildSnapshot(bool forceControllerRefresh) {
    m_snapshot = buildDeviceInfoSnapshot(forceControllerRefresh);
    m_modelValue = m_snapshot.modelLine;
    m_versionValue = m_snapshot.systemVersion;
    m_sdStorageValue = m_snapshot.sdStorageLine;
    m_internalStorageValue = m_snapshot.internalStorageLine;
    buildControllerStringsFromSnapshot();
}

void SystemInfoGui::syncDynamicTextsIfNeeded(bool force) {
    const uint64_t now = armGetSystemTick();
    if (!force && m_lastUiSyncTick != 0 && armTicksToNs(now - m_lastUiSyncTick) < 5000000000ULL)
        return;

    m_lastUiSyncTick = now;
    rebuildSnapshot(force);
}

void SystemInfoGui::refreshTexts() {
    if (m_frame != nullptr)
        m_frame->setSubtitle(tr(currentLang, TextId::SystemInformation));
    if (m_infoHeader != nullptr)
        m_infoHeader->setText(tr(currentLang, TextId::SystemInformation));

    m_modelTitle = tr(currentLang, TextId::ConsoleModel);
    m_versionTitle = tr(currentLang, TextId::SystemVersion);
    m_sdStorageTitle = tr(currentLang, TextId::FreeSdStorage);
    m_internalStorageTitle = tr(currentLang, TextId::FreeInternalStorage);

    rebuildSnapshot(true);
}

void SystemInfoGui::update() {
    syncDynamicTextsIfNeeded(false);
}





tsl::elm::Element* SystemInfoGui::createUI() {
    rebuildSnapshot(true);

    m_modelTitle = tr(currentLang, TextId::ConsoleModel);
    m_versionTitle = tr(currentLang, TextId::SystemVersion);
    m_sdStorageTitle = tr(currentLang, TextId::FreeSdStorage);
    m_internalStorageTitle = tr(currentLang, TextId::FreeInternalStorage);

    m_frame = new tsl::elm::OverlayFrame(
        tr(currentLang, TextId::AppName),
        tr(currentLang, TextId::SystemInformation)
    );

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

    m_infoHeader = new tsl::elm::CategoryHeader(tr(currentLang, TextId::SystemInformation));
    list->addItem(m_infoHeader);

    m_modelRow = createCompactInfoRow(&m_modelTitle, &m_modelValue);
    list->addItem(m_modelRow, 28);

    m_versionRow = blueInfoRow(&m_versionTitle, &m_versionValue);
    list->addItem(m_versionRow, 28);

    m_sdStorageRow = blueInfoRow(&m_sdStorageTitle, &m_sdStorageValue);
    list->addItem(m_sdStorageRow, 28);

    m_internalStorageRow = blueInfoRow(&m_internalStorageTitle, &m_internalStorageValue);
    list->addItem(m_internalStorageRow, 28);

    m_controllerRows.clear();
    for (std::size_t i = 0; i < m_controllerTitles.size(); ++i) {
        m_controllerRows.push_back(blueInfoRow(&m_controllerTitles[i], &m_controllerValues[i]));
        list->addItem(m_controllerRows.back(), 28);
    }

    m_frame->setContent(list);
    return m_frame;
}