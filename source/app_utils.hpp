#pragma once
#include <string>
#include <vector>
#include <tesla.hpp>
#include <switch.h>
#include "app_types.hpp"
#include "localization.hpp"

extern Lang currentLang;
extern HotkeyPreset currentHotkey;
extern std::string g_selfOverlayFileName;

std::string resultToHex(Result rc);

std::string getFileNameFromPath(const std::string& fullPath);

void trimInPlace(char* str);

bool ensureDirectory(const char* path);
bool ensureConfigDirectory();
bool ensureSysclkDirectory();

void applyFooterLanguage(Lang lang);

const char* getLangDisplay(Lang lang);
const char* langToConfigString(Lang lang);

Lang langFromConfigString(const char* value);

u64 getHotkeyKeys(HotkeyPreset preset);

const char* getHotkeyComboString(HotkeyPreset preset);

HotkeyPreset presetFromComboString(const char* combo);

HotkeyPreset presetFromKeys(u64 keys);

std::string getHotkeyGlyphTitle(HotkeyPreset preset);
std::string getPowerStandbyGlyphLabel(const char* text);

bool saveConfig();
void loadConfig();

void saveHotkey(HotkeyPreset preset);

Result doReboot();
Result doShutdown();

void refreshVisibleTexts();
void refreshSettingsTexts();

struct ControllerEntry {
    HidNpadIdType id;
    u32 style;
    std::string value;
};

struct DeviceInfoSnapshot {
    std::string modelLine;
    std::string systemVersion;
    std::string sdStorageLine;
    std::string internalStorageLine;
    std::string consoleMode;
    std::string resolutionLine;
    std::vector<ControllerEntry> controllers;
};

std::string formatStorageHuman(s64 freeBytes, s64 totalBytes);
std::string getSystemVersionString();
std::string getSdStorageLine();
std::string getInternalStorageLine();
std::string getConsoleModelString();
std::string getConsoleModeString();
std::string getResolutionString();
std::vector<ControllerEntry> readCurrentlyConnectedControllers();

void updateControllerOrderCache(bool force = false);

DeviceInfoSnapshot buildDeviceInfoSnapshot(bool forceControllerRefresh = false);

tsl::elm::CustomDrawer* createCompactInfoRow(std::string* left, std::string* right);