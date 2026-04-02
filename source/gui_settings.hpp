#pragma once
#include <string>
#include <vector>
#include <tesla.hpp>
#include "app_utils.hpp"

class SettingsGui : public tsl::Gui {
public:
    SettingsGui();
    ~SettingsGui() override;
    void refreshTexts();
    void update() override;
    tsl::elm::Element* createUI() override;

private:
    tsl::elm::OverlayFrame* m_frame = nullptr;
    tsl::elm::CategoryHeader* m_componentsHeader = nullptr;
    tsl::elm::CategoryHeader* m_overlaySettingsHeader = nullptr;

    std::string m_sysclkTitle;
    std::string m_sysclkValue;
    std::string m_sysclkDescription;

    std::string m_edizonTitle;
    std::string m_edizonValue;
    std::string m_edizonDescription;

    tsl::elm::CustomDrawer* m_sysclkRow = nullptr;
    tsl::elm::CustomDrawer* m_sysclkDescriptionRow = nullptr;

    tsl::elm::CustomDrawer* m_edizonRow = nullptr;
    tsl::elm::CustomDrawer* m_edizonDescriptionRow = nullptr;

    tsl::elm::ListItem* m_hotkeyItem = nullptr;
    tsl::elm::ListItem* m_langItem = nullptr;

    uint64_t m_lastUiSyncTick = 0;

    void rebuildComponentStrings();
    void syncDynamicTextsIfNeeded(bool force = false);

    tsl::elm::CustomDrawer* createCompactRow(std::string* left, std::string* right);
    tsl::elm::CustomDrawer* createDescriptionRow(std::string* text);
};

class SystemInfoGui : public tsl::Gui {
public:
    SystemInfoGui();
    ~SystemInfoGui() override;
    void refreshTexts();
    void update() override;
    tsl::elm::Element* createUI() override;

private:
    tsl::elm::OverlayFrame* m_frame = nullptr;
    tsl::elm::CategoryHeader* m_infoHeader = nullptr;

    std::string m_modelTitle;
    std::string m_modelValue;
    std::string m_versionTitle;
    std::string m_versionValue;
    std::string m_sdStorageTitle;
    std::string m_sdStorageValue;
    std::string m_internalStorageTitle;
    std::string m_internalStorageValue;

    tsl::elm::CustomDrawer* m_modelRow = nullptr;
    tsl::elm::CustomDrawer* m_versionRow = nullptr;
    tsl::elm::CustomDrawer* m_sdStorageRow = nullptr;
    tsl::elm::CustomDrawer* m_internalStorageRow = nullptr;

    std::vector<std::string> m_controllerTitles;
    std::vector<std::string> m_controllerValues;
    std::vector<tsl::elm::CustomDrawer*> m_controllerRows;

    DeviceInfoSnapshot m_snapshot;
    uint64_t m_lastUiSyncTick = 0;

    void buildControllerStringsFromSnapshot();
    void rebuildSnapshot(bool forceControllerRefresh);
    void syncDynamicTextsIfNeeded(bool force = false);
};

extern SettingsGui* g_settingsGui;
extern SystemInfoGui* g_systemInfoGui;