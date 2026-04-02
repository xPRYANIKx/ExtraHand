#include "app_utils.hpp"
#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include "gui_main.hpp"
#include "gui_power.hpp"
#include "gui_settings.hpp"
#include "sysclk_performance.hpp"
#include "sysclk_api.hpp"


Lang currentLang = Lang::English;
HotkeyPreset currentHotkey = HotkeyPreset::L_DDOWN_RS;
std::string g_selfOverlayFileName;

static std::vector<ControllerEntry> g_controllerOrderCache;
static uint64_t g_lastControllerCacheTick = 0;

static const char* EXTRAHAND_CONFIG_PATH = "/config/ExtraHand/config.ini";
static const char* SYSCLK_OVERLAY_CONFIG_PATH = "/config/ExtraHand/overlay.ini";
static const char* SYSCLK_OVERLAY_CONFIG_PATH_LEGACY = "/config/ExtraHand/sysclk/overlay.ini";

std::string resultToHex(Result rc) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "0x%08X", (unsigned int)rc);
    return std::string(buf);
}

std::string getFileNameFromPath(const std::string& fullPath) {
    const std::size_t pos = fullPath.find_last_of("/\\");
    return (pos == std::string::npos) ? fullPath : fullPath.substr(pos + 1);
}

void trimInPlace(char* str) {
    if (str == nullptr || *str == '\0')
        return;

    char* start = str;
    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n')
        ++start;

    if (start != str)
        std::memmove(str, start, std::strlen(start) + 1);

    std::size_t len = std::strlen(str);
    while (len > 0 &&
        (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\r' || str[len - 1] == '\n')) {
        str[len - 1] = '\0';
        --len;
    }
}

bool ensureDirectory(const char* path) {
    bool ok = false;
    tsl::hlp::doWithSDCardHandle([&] {
        errno = 0;
        if (mkdir(path, 0777) == 0 || errno == EEXIST)
            ok = true;
        });
    return ok;
}

bool ensureConfigDirectory() {
    return ensureDirectory("/config") &&
        ensureDirectory("/config/ExtraHand");
}

bool ensureSysclkDirectory() {
    return ensureConfigDirectory() &&
        ensureDirectory("/config/ExtraHand/sysclk");
}

void applyFooterLanguage(Lang lang) {
    switch (lang) {
    case Lang::Russian:
        tsl::setFooterLanguage(tsl::cfg::FooterLang::Russian);
        break;
    default:
        tsl::setFooterLanguage(tsl::cfg::FooterLang::English);
        break;
    }
}

static const char* getLangDisplayLocalized(Lang /*uiLang*/, Lang lang) {
    switch (lang) {
    case Lang::English: return "English";
    case Lang::German:  return "Deutsch";
    case Lang::French:  return "Français";
    case Lang::Spanish: return "Español";
    case Lang::Italian: return "Italiano";
    case Lang::Russian: return "Русский";
    default:            return "English";
    }
}

static const char* getControllerTypeDisplay(Lang /*uiLang*/, u32 style) {
    if (style & HidNpadStyleTag_NpadFullKey)   return "Pro Controller";
    if (style & HidNpadStyleTag_NpadHandheld)  return "Handheld";
    if (style & HidNpadStyleTag_NpadJoyDual)   return "Joy-Con Pair";
    if (style & HidNpadStyleTag_NpadJoyLeft)   return "Joy-Con (L)";
    if (style & HidNpadStyleTag_NpadJoyRight)  return "Joy-Con (R)";
    if (style & HidNpadStyleTag_NpadGc)        return "GameCube";
    if (style & HidNpadStyleTag_NpadPalma)     return "Poké Ball Plus";
    if (style & HidNpadStyleTag_NpadSystemExt) return "System External";
    if (style & HidNpadStyleTag_NpadSystem)    return "System";
    return tr(currentLang, TextId::UnknownController);
}

static std::string getConsoleModelDisplay(SetSysProductModel model) {
    switch (model) {
    case SetSysProductModel_Iowa:
        return "Nintendo Switch";
    case SetSysProductModel_Hoag:
        return "Nintendo Switch Lite";
    case SetSysProductModel_Calcio:
        return "Nintendo Switch OLED";
    default:
        return "Unknown";
    }
}

const char* getLangDisplay(Lang lang) {
    return getLangDisplayLocalized(currentLang, lang);
}

const char* langToConfigString(Lang lang) {
    switch (lang) {
    case Lang::English: return "en";
    case Lang::German:  return "de";
    case Lang::French:  return "fr";
    case Lang::Spanish: return "es";
    case Lang::Italian: return "it";
    case Lang::Russian: return "ru";
    default:            return "en";
    }
}

Lang langFromConfigString(const char* value) {
    if (value == nullptr) return Lang::English;
    if (std::strcmp(value, "en") == 0) return Lang::English;
    if (std::strcmp(value, "de") == 0) return Lang::German;
    if (std::strcmp(value, "fr") == 0) return Lang::French;
    if (std::strcmp(value, "es") == 0) return Lang::Spanish;
    if (std::strcmp(value, "it") == 0) return Lang::Italian;
    if (std::strcmp(value, "ru") == 0) return Lang::Russian;
    return Lang::English;
}

u64 getHotkeyKeys(HotkeyPreset preset) {
    switch (preset) {
    case HotkeyPreset::ZL_ZR_DDOWN:
        return HidNpadButton_ZL | HidNpadButton_ZR | HidNpadButton_Down;
    case HotkeyPreset::L_R_DDOWN:
        return HidNpadButton_L | HidNpadButton_R | HidNpadButton_Down;
    case HotkeyPreset::L_R_RS:
        return HidNpadButton_L | HidNpadButton_R | HidNpadButton_StickR;
    case HotkeyPreset::L_DDOWN_RS:
        return HidNpadButton_L | HidNpadButton_Down | HidNpadButton_StickR;
    case HotkeyPreset::R_DDOWN_RS:
        return HidNpadButton_R | HidNpadButton_Down | HidNpadButton_StickR;
    default:
        return HidNpadButton_L | HidNpadButton_Down | HidNpadButton_StickR;
    }
}

const char* getHotkeyComboString(HotkeyPreset preset) {
    switch (preset) {
    case HotkeyPreset::ZL_ZR_DDOWN: return "ZL+ZR+DDOWN";
    case HotkeyPreset::L_R_DDOWN:   return "L+R+DDOWN";
    case HotkeyPreset::L_R_RS:      return "L+R+RS";
    case HotkeyPreset::L_DDOWN_RS:  return "L+DDOWN+RS";
    case HotkeyPreset::R_DDOWN_RS:  return "R+DDOWN+RS";
    default:                        return "L+DDOWN+RS";
    }
}

HotkeyPreset presetFromComboString(const char* combo) {
    if (combo == nullptr) return HotkeyPreset::L_DDOWN_RS;
    if (std::strcmp(combo, "ZL+ZR+DDOWN") == 0) return HotkeyPreset::ZL_ZR_DDOWN;
    if (std::strcmp(combo, "L+R+DDOWN") == 0)   return HotkeyPreset::L_R_DDOWN;
    if (std::strcmp(combo, "L+R+RS") == 0)      return HotkeyPreset::L_R_RS;
    if (std::strcmp(combo, "L+DDOWN+RS") == 0)  return HotkeyPreset::L_DDOWN_RS;
    if (std::strcmp(combo, "R+DDOWN+RS") == 0)  return HotkeyPreset::R_DDOWN_RS;
    return HotkeyPreset::L_DDOWN_RS;
}

HotkeyPreset presetFromKeys(u64 keys) {
    if (keys == (HidNpadButton_ZL | HidNpadButton_ZR | HidNpadButton_Down))
        return HotkeyPreset::ZL_ZR_DDOWN;
    if (keys == (HidNpadButton_L | HidNpadButton_R | HidNpadButton_Down))
        return HotkeyPreset::L_R_DDOWN;
    if (keys == (HidNpadButton_L | HidNpadButton_R | HidNpadButton_StickR))
        return HotkeyPreset::L_R_RS;
    if (keys == (HidNpadButton_L | HidNpadButton_Down | HidNpadButton_StickR))
        return HotkeyPreset::L_DDOWN_RS;
    if (keys == (HidNpadButton_R | HidNpadButton_Down | HidNpadButton_StickR))
        return HotkeyPreset::R_DDOWN_RS;
    return HotkeyPreset::L_DDOWN_RS;
}

static const char* getGlyphForKey(u64 key) {
    for (const auto& keyInfo : tsl::impl::KEYS_INFO) {
        if (keyInfo.key == key)
            return keyInfo.glyph;
    }
    return "?";
}

std::string getHotkeyGlyphTitle(HotkeyPreset preset) {
    switch (preset) {
    case HotkeyPreset::ZL_ZR_DDOWN:
        return std::string(getGlyphForKey(HidNpadButton_ZL)) + " + " +
            getGlyphForKey(HidNpadButton_ZR) + " + " +
            getGlyphForKey(HidNpadButton_Down);
    case HotkeyPreset::L_R_DDOWN:
        return std::string(getGlyphForKey(HidNpadButton_L)) + " + " +
            getGlyphForKey(HidNpadButton_R) + " + " +
            getGlyphForKey(HidNpadButton_Down);
    case HotkeyPreset::L_R_RS:
        return std::string(getGlyphForKey(HidNpadButton_L)) + " + " +
            getGlyphForKey(HidNpadButton_R) + " + " +
            getGlyphForKey(HidNpadButton_StickR);
    case HotkeyPreset::L_DDOWN_RS:
        return std::string(getGlyphForKey(HidNpadButton_L)) + " + " +
            getGlyphForKey(HidNpadButton_Down) + " + " +
            getGlyphForKey(HidNpadButton_StickR);
    case HotkeyPreset::R_DDOWN_RS:
        return std::string(getGlyphForKey(HidNpadButton_R)) + " + " +
            getGlyphForKey(HidNpadButton_Down) + " + " +
            getGlyphForKey(HidNpadButton_StickR);
    default:
        return std::string(getGlyphForKey(HidNpadButton_L)) + " + " +
            getGlyphForKey(HidNpadButton_Down) + " + " +
            getGlyphForKey(HidNpadButton_StickR);
    }
}

std::string getPowerStandbyGlyphLabel(const char* text) {
    return std::string("\xE2\x8F\xBB ") + (text != nullptr ? text : "");
}

static bool writeOverlayConfigToPath(const char* finalPath) {
    if (!ensureSysclkDirectory())
        return false;

    bool ok = false;
    tsl::hlp::doWithSDCardHandle([&] {
        std::string tempPath = std::string(finalPath) + ".tmp";
        FILE* f = fopen(tempPath.c_str(), "wb");
        if (!f)
            return;

        const int written = std::fprintf(
            f,
            "[ExtraHand]\n"
            "key_combo=%s\n"
            "language=%s\n",
            getHotkeyComboString(currentHotkey),
            langToConfigString(currentLang)
        );

        if (written <= 0) {
            fclose(f);
            remove(tempPath.c_str());
            return;
        }

        std::fflush(f);
        fclose(f);

        remove(finalPath);
        if (rename(tempPath.c_str(), finalPath) != 0) {
            remove(tempPath.c_str());
            return;
        }

        ok = true;
        });

    return ok;
}

static void parseOverlayConfigFile(
    const char* path,
    bool& languageLoaded,
    bool& hotkeyLoaded
) {
    tsl::hlp::doWithSDCardHandle([&] {
        FILE* f = fopen(path, "rb");
        if (!f)
            return;

        char line[256] = {};
        bool inExtraHandSection = false;

        while (fgets(line, sizeof(line), f)) {
            trimInPlace(line);

            if (line[0] == '\0' || line[0] == ';' || line[0] == '#')
                continue;

            if (line[0] == '[') {
                inExtraHandSection = (std::strcmp(line, "[ExtraHand]") == 0);
                continue;
            }

            if (!inExtraHandSection)
                continue;

            if (std::strncmp(line, "language=", 9) == 0) {
                char* value = line + 9;
                trimInPlace(value);
                currentLang = langFromConfigString(value);
                languageLoaded = true;
                continue;
            }

            if (std::strncmp(line, "key_combo=", 10) == 0) {
                char* value = line + 10;
                trimInPlace(value);
                currentHotkey = presetFromComboString(value);
                hotkeyLoaded = true;
                continue;
            }
        }

        fclose(f);
        });
}

bool saveConfig() {
    return writeOverlayConfigToPath(SYSCLK_OVERLAY_CONFIG_PATH);
}

void loadConfig() {
    bool languageLoaded = false;
    bool hotkeyLoaded = false;

    parseOverlayConfigFile(SYSCLK_OVERLAY_CONFIG_PATH, languageLoaded, hotkeyLoaded);

    if (!languageLoaded || !hotkeyLoaded)
        parseOverlayConfigFile(EXTRAHAND_CONFIG_PATH, languageLoaded, hotkeyLoaded);

    if (!languageLoaded || !hotkeyLoaded)
        parseOverlayConfigFile(SYSCLK_OVERLAY_CONFIG_PATH_LEGACY, languageLoaded, hotkeyLoaded);

    if (languageLoaded || hotkeyLoaded)
        writeOverlayConfigToPath(SYSCLK_OVERLAY_CONFIG_PATH);

    if (!languageLoaded)
        currentLang = Lang::English;
    if (!hotkeyLoaded)
        currentHotkey = presetFromKeys(tsl::cfg::launchCombo);

    tsl::impl::updateCombo(getHotkeyKeys(currentHotkey));
}

void saveHotkey(HotkeyPreset preset) {
    currentHotkey = preset;
    tsl::impl::updateCombo(getHotkeyKeys(preset));
    saveConfig();
}

Result doReboot() {
    Result rc = 0;
    tsl::hlp::doWithSmSession([&] {
        rc = appletStartRebootSequenceForOverlay();
        if (R_SUCCEEDED(rc)) return;

        rc = appletRequestToReboot();
        if (R_SUCCEEDED(rc)) return;

        rc = spsmInitialize();
        if (R_FAILED(rc)) return;

        Result shutdownRc = spsmShutdown(true);
        spsmExit();
        rc = shutdownRc;
        });
    return rc;
}

Result doShutdown() {
    Result rc = 0;
    tsl::hlp::doWithSmSession([&] {
        rc = appletStartShutdownSequenceForOverlay();
        if (R_SUCCEEDED(rc)) return;

        rc = appletRequestToShutdown();
        if (R_SUCCEEDED(rc)) return;

        rc = spsmInitialize();
        if (R_FAILED(rc)) return;

        Result shutdownRc = spsmShutdown(false);
        spsmExit();
        rc = shutdownRc;
        });
    return rc;
}

void refreshSettingsTexts() {
    if (g_settingsGui != nullptr)
        g_settingsGui->refreshTexts();
    if (g_systemInfoGui != nullptr)
        g_systemInfoGui->refreshTexts();
}

void refreshVisibleTexts() {
    if (g_mainGui != nullptr)       g_mainGui->refreshTexts();
    if (g_settingsGui != nullptr)   g_settingsGui->refreshTexts();
    if (g_systemInfoGui != nullptr) g_systemInfoGui->refreshTexts();
    if (g_powerGui != nullptr)      g_powerGui->refreshTexts();
    sysclk::refreshVisibleTexts();
}

std::string formatStorageHuman(s64 freeBytes, s64 totalBytes) {
    const double freeGb = (double)freeBytes / (1024.0 * 1024.0 * 1024.0);
    const double totalGb = (double)totalBytes / (1024.0 * 1024.0 * 1024.0);

    char buf[96];
    std::snprintf(buf, sizeof(buf), tr(currentLang, TextId::StorageFormat), freeGb, totalGb);
    return std::string(buf);
}

static std::string getAtmosphereVersionString() {
    Result initRc = 0;
    Result cfgRc = 0;
    u64 value = 0;

    tsl::hlp::doWithSmSession([&] {
        initRc = splInitialize();
        if (R_FAILED(initRc))
            return;

        cfgRc = splGetConfig((SplConfigItem)65000, &value);
        splExit();
        });

    if (R_FAILED(initRc))
        return std::string("AMS init ") + resultToHex(initRc);

    if (R_FAILED(cfgRc))
        return std::string("AMS cfg ") + resultToHex(cfgRc);

    if (value == 0)
        return "AMS value=0";

    const u8 byte0 = static_cast<u8>((value >> 56) & 0xFF);
    const u8 byte1 = static_cast<u8>((value >> 48) & 0xFF);
    const u8 byte2 = static_cast<u8>((value >> 40) & 0xFF);

    const u32 major = static_cast<u32>(byte0);
    const u32 minor = static_cast<u32>(byte1);
    const u32 micro = static_cast<u32>(byte2);

    char buf[32];
    std::snprintf(buf, sizeof(buf), "AMS %u.%u.%u", major, minor, micro);
    return std::string(buf);
}

std::string getSystemVersionString() {
    SetSysFirmwareVersion fw{};
    std::string hos = "-";

    if (R_SUCCEEDED(setsysGetFirmwareVersion(&fw))) {
        if (fw.display_version[0] != '\0')
            hos = std::string(fw.display_version);
        else if (fw.display_title[0] != '\0')
            hos = std::string(fw.display_title);
    }

    const std::string ams = getAtmosphereVersionString();

    if (!ams.empty())
        return hos + " | " + ams;

    return hos;
}

static std::string getStorageLineFromFs(FsFileSystem* fs) {
    if (fs == nullptr)
        return "-";

    s64 freeSpace = 0;
    s64 totalSpace = 0;

    Result rcFree = fsFsGetFreeSpace(fs, "/", &freeSpace);
    Result rcTotal = fsFsGetTotalSpace(fs, "/", &totalSpace);

    if (R_FAILED(rcFree) || R_FAILED(rcTotal) || freeSpace < 0 || totalSpace <= 0)
        return "-";

    return formatStorageHuman(freeSpace, totalSpace);
}

std::string getSdStorageLine() {
    FsFileSystem sdFs{};
    Result rc = fsOpenSdCardFileSystem(&sdFs);
    if (R_FAILED(rc))
        return "-";

    const std::string out = getStorageLineFromFs(&sdFs);
    fsFsClose(&sdFs);
    return out;
}

std::string getInternalStorageLine() {
    FsFileSystem bisFs{};
    Result rc = fsOpenBisFileSystem(&bisFs, FsBisPartitionId_User, "");
    if (R_FAILED(rc))
        return "-";

    const std::string out = getStorageLineFromFs(&bisFs);
    fsFsClose(&bisFs);
    return out;
}

static const char* getControllerTypeName(u32 style) {
    return getControllerTypeDisplay(currentLang, style);
}

std::string getConsoleModelString() {
    SetSysProductModel model = SetSysProductModel_Invalid;
    if (R_FAILED(setsysGetProductModel(&model)))
        return "-";

    return getConsoleModelDisplay(model);
}

std::string getConsoleModeString() {
    AppletOperationMode mode = appletGetOperationMode();
    switch (mode) {
    case AppletOperationMode_Handheld: return tr(currentLang, TextId::Handheld);
    case AppletOperationMode_Console:  return tr(currentLang, TextId::Docked);
    default:                           return "-";
    }
}

std::string getResolutionString() {
    AppletOperationMode mode = appletGetOperationMode();
    char buf[64];

    switch (mode) {
    case AppletOperationMode_Handheld:
        std::snprintf(buf, sizeof(buf), tr(currentLang, TextId::ResolutionFormat), 1280, 720);
        return std::string(buf);
    case AppletOperationMode_Console:
        std::snprintf(buf, sizeof(buf), tr(currentLang, TextId::ResolutionFormat), 1920, 1080);
        return std::string(buf);
    default:
        return "-";
    }
}

std::vector<ControllerEntry> readCurrentlyConnectedControllers() {
    std::vector<ControllerEntry> out;

    const HidNpadIdType ids[] = {
        HidNpadIdType_No1, HidNpadIdType_No2, HidNpadIdType_No3, HidNpadIdType_No4,
        HidNpadIdType_No5, HidNpadIdType_No6, HidNpadIdType_No7, HidNpadIdType_No8
    };

    for (std::size_t i = 0; i < sizeof(ids) / sizeof(ids[0]); ++i) {
        const u32 style = hidGetNpadStyleSet(ids[i]);
        if (style == 0)
            continue;

        ControllerEntry entry;
        entry.id = ids[i];
        entry.style = style;
        entry.value = getControllerTypeName(style);
        out.push_back(entry);
    }

    return out;
}

void updateControllerOrderCache(bool force) {
    const uint64_t now = armGetSystemTick();

    if (!force &&
        g_lastControllerCacheTick != 0 &&
        armTicksToNs(now - g_lastControllerCacheTick) < 5000000000ULL) {
        return;
    }

    g_lastControllerCacheTick = now;
    const std::vector<ControllerEntry> current = readCurrentlyConnectedControllers();

    g_controllerOrderCache.erase(
        std::remove_if(
            g_controllerOrderCache.begin(),
            g_controllerOrderCache.end(),
            [&](const ControllerEntry& cached) {
                return std::find_if(
                    current.begin(),
                    current.end(),
                    [&](const ControllerEntry& nowEntry) {
                        return nowEntry.id == cached.id;
                    }
                ) == current.end();
            }
        ),
        g_controllerOrderCache.end()
    );

    for (std::size_t i = 0; i < g_controllerOrderCache.size(); ++i) {
        auto it = std::find_if(
            current.begin(),
            current.end(),
            [&](const ControllerEntry& nowEntry) {
                return nowEntry.id == g_controllerOrderCache[i].id;
            }
        );

        if (it != current.end()) {
            g_controllerOrderCache[i].style = it->style;
            g_controllerOrderCache[i].value = it->value;
        }
    }

    for (std::size_t i = 0; i < current.size(); ++i) {
        auto it = std::find_if(
            g_controllerOrderCache.begin(),
            g_controllerOrderCache.end(),
            [&](const ControllerEntry& cached) {
                return cached.id == current[i].id;
            }
        );

        if (it == g_controllerOrderCache.end())
            g_controllerOrderCache.push_back(current[i]);
    }
}

DeviceInfoSnapshot buildDeviceInfoSnapshot(bool forceControllerRefresh) {
    updateControllerOrderCache(forceControllerRefresh);

    DeviceInfoSnapshot s;
    s.modelLine = getConsoleModelString();
    s.systemVersion = getSystemVersionString();
    s.sdStorageLine = getSdStorageLine();
    s.internalStorageLine = getInternalStorageLine();
    s.consoleMode = getConsoleModeString();
    s.resolutionLine = getResolutionString();
    s.controllers = g_controllerOrderCache;
    return s;
}

tsl::elm::CustomDrawer* createCompactInfoRow(std::string* left, std::string* right) {
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
                x + w - (u16)size.first - 8,
                y + 18,
                14,
                tsl::gfx::Renderer::a({ 0xF, 0xF, 0xF, 0xF })
            );
        }
    );
}