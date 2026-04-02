#include "sysclk_api.hpp"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <tesla.hpp>
#include "app_utils.hpp"
#include "sysclk_common.hpp"
#include "sysclk_components.hpp"
#include "sysclk_performance.hpp"

#define SYSCLK_IPC_SERVICE_NAME       "sys:clk"
#define SYSCLK_PRESETS_DIR            "sdmc:/config/ExtraHand/sysclk"
#define SYSCLK_APP_PRESETS_DIR_LEGACY "sdmc:/config/ExtraHand/sysclk/apps"

namespace sysclk {

typedef struct {
    union {
        uint32_t mhz[15];
        uint32_t mhzMap[5][3];
    };
} SysClkTitleProfileList;

typedef struct {
    uint64_t tid;
    SysClkTitleProfileList profiles;
} SysClkIpc_SetProfiles_Args;

static Service g_sysclkSrv;
static bool g_sysclkServiceOpened = false;
static uint64_t g_lastInitAttemptTick = 0;
static Result g_lastInitResult = MAKERESULT(Module_Libnx, LibnxError_NotFound);

int getIpcIndexForModule(SysClkModule module) {
    switch (module) {
        case SysClkModule_CPU: return 0;
        case SysClkModule_GPU: return 1;
        case SysClkModule_MEM: return 2;
        default:               return 0;
    }
}

uint32_t getContextFreq(const SysClkContext& ctx, SysClkModule module) {
    const int index = (int)module;

    if (ctx.overrideFreqs[index] != 0)
        return ctx.overrideFreqs[index];

    if (ctx.freqs[index] != 0)
        return ctx.freqs[index];

    return ctx.realFreqs[index];
}

static uint32_t getCanonicalFreq(const uint32_t raw[3], SysClkModule module) {
    return raw[getIpcIndexForModule(module)];
}

static void setRawFreq(uint32_t raw[3], SysClkModule module, uint32_t value) {
    raw[getIpcIndexForModule(module)] = value;
}

static void readValuesFromProfile(const SysClkTitleProfileList& profiles, SysClkProfile profile, SysClkPresetValues& outValues) {
    outValues.cpuMhz = getCanonicalFreq(profiles.mhzMap[profile], SysClkModule_CPU);
    outValues.gpuMhz = getCanonicalFreq(profiles.mhzMap[profile], SysClkModule_GPU);
    outValues.memMhz = getCanonicalFreq(profiles.mhzMap[profile], SysClkModule_MEM);
}

static bool isPresetValuesEmpty(const SysClkPresetValues& values) {
    return values.cpuMhz == 0 && values.gpuMhz == 0 && values.memMhz == 0;
}

static bool arePresetValuesEqual(const SysClkPresetValues& a, const SysClkPresetValues& b) {
    return a.cpuMhz == b.cpuMhz &&
           a.gpuMhz == b.gpuMhz &&
           a.memMhz == b.memMhz;
}

static bool isChargingProfile(SysClkProfile profile) {
    return profile == SysClkProfile_HandheldCharging ||
           profile == SysClkProfile_HandheldChargingUSB ||
           profile == SysClkProfile_HandheldChargingOfficial;
}

static SysClkProfile chooseChargingProfileFromContext(const SysClkContext& ctx) {
    const SysClkProfile currentProfile = static_cast<SysClkProfile>(ctx.profile);
    if (isChargingProfile(currentProfile))
        return currentProfile;

    return SysClkProfile_HandheldCharging;
}

static void readAggregateChargingValues(const SysClkTitleProfileList& profiles, SysClkPresetValues& outValues) {
    SysClkPresetValues genericValues;
    SysClkPresetValues usbValues;
    SysClkPresetValues officialValues;

    readValuesFromProfile(profiles, SysClkProfile_HandheldCharging, genericValues);
    readValuesFromProfile(profiles, SysClkProfile_HandheldChargingUSB, usbValues);
    readValuesFromProfile(profiles, SysClkProfile_HandheldChargingOfficial, officialValues);

    SysClkContext ctx{};
    if (R_SUCCEEDED(getCurrentContext(&ctx))) {
        const SysClkProfile activeChargingProfile = chooseChargingProfileFromContext(ctx);
        if (activeChargingProfile == SysClkProfile_HandheldChargingUSB) {
            outValues = usbValues;
            return;
        }

        if (activeChargingProfile == SysClkProfile_HandheldChargingOfficial) {
            outValues = officialValues;
            return;
        }

        outValues = genericValues;
        return;
    }

    if (arePresetValuesEqual(genericValues, usbValues) && arePresetValuesEqual(genericValues, officialValues)) {
        outValues = genericValues;
        return;
    }

    if (!isPresetValuesEmpty(genericValues)) {
        outValues = genericValues;
        return;
    }

    if (!isPresetValuesEmpty(usbValues)) {
        outValues = usbValues;
        return;
    }

    outValues = officialValues;
}

static bool pathExists(const char* path) {
    struct stat st{};
    return path != nullptr && stat(path, &st) == 0;
}

static bool ensureSysclkPresetDirectory() {
    return ensureConfigDirectory() &&
           ensureDirectory(SYSCLK_PRESETS_DIR);
}

static Result sysclkIpcInitialize() {
    if (g_sysclkServiceOpened)
        return 0;

    const uint64_t now = armGetSystemTick();
    if (g_lastInitAttemptTick != 0 && armTicksToNs(now - g_lastInitAttemptTick) < 1500000000ULL)
        return g_lastInitResult;

    g_lastInitAttemptTick = now;
    g_lastInitResult = MAKERESULT(Module_Libnx, LibnxError_NotFound);

    Result rc = 0;
    bool done = false;

    tsl::hlp::doWithSmSession([&] {
        rc = smGetService(&g_sysclkSrv, SYSCLK_IPC_SERVICE_NAME);
        if (R_SUCCEEDED(rc)) {
            g_sysclkServiceOpened = true;
            g_lastInitResult = 0;
            done = true;
        } else {
            g_lastInitResult = rc;
        }
    });

    return done ? 0 : g_lastInitResult;
}

static void sysclkIpcExit() {
    if (g_sysclkServiceOpened) {
        serviceClose(&g_sysclkSrv);
        g_sysclkServiceOpened = false;
    }
}

static Result sysclkIpcGetCurrentContext(SysClkContext* out_context) {
    return serviceDispatchOut(&g_sysclkSrv, 2 /* GetCurrentContext */, *out_context);
}

static Result sysclkIpcGetProfiles(uint64_t tid, SysClkTitleProfileList* out_profiles) {
    return serviceDispatchInOut(&g_sysclkSrv, 5 /* GetProfiles */, tid, *out_profiles);
}

static Result sysclkIpcSetProfiles(uint64_t tid, SysClkTitleProfileList* profiles) {
    SysClkIpc_SetProfiles_Args args;
    args.tid = tid;
    std::memcpy(&args.profiles, profiles, sizeof(SysClkTitleProfileList));
    return serviceDispatchIn(&g_sysclkSrv, 6 /* SetProfiles */, args);
}

static void applyValuesToProfile(SysClkTitleProfileList& profiles, SysClkProfile profile, const SysClkPresetValues& values) {
    setRawFreq(profiles.mhzMap[profile], SysClkModule_CPU, values.cpuMhz);
    setRawFreq(profiles.mhzMap[profile], SysClkModule_GPU, values.gpuMhz);
    setRawFreq(profiles.mhzMap[profile], SysClkModule_MEM, values.memMhz);
}

void initialize() {
    ensureSysclkPresetDirectory();
}

void finalize() {
    sysclkIpcExit();
}

bool isServiceCachedAvailable() {
    std::string err;
    return ensureReady(err);
}

bool hasConfigDirHint() {
    bool found = false;

    tsl::hlp::doWithSDCardHandle([&] {
        found = pathExists("sdmc:/config/sys-clk") ||
                pathExists("sdmc:/config/sys-clk/config.ini") ||
                pathExists("sdmc:/config/sys-clk/config") ||
                pathExists("sdmc:/switch/.overlays/sys-clk.ovl") ||
                pathExists("sdmc:/switch/.overlays/sys-clk-overlay.ovl");
    });

    return found;
}

bool ensureReady(std::string& outError) {
    if (g_sysclkServiceOpened)
        return true;

    outError.clear();

    Result rc = sysclkIpcInitialize();
    if (R_FAILED(rc)) {
        outError = tr(currentLang, TextId::SysclkUnavailable);
        return false;
    }

    return true;
}

Result getCurrentContext(SysClkContext* out_context) {
    if (out_context == nullptr)
        return MAKERESULT(Module_Libnx, LibnxError_BadInput);

    Result rc = sysclkIpcGetCurrentContext(out_context);
    if (R_FAILED(rc))
        return rc;

    const uint32_t rawFreqs[3] = { out_context->freqs[0], out_context->freqs[1], out_context->freqs[2] };
    const uint32_t rawRealFreqs[3] = { out_context->realFreqs[0], out_context->realFreqs[1], out_context->realFreqs[2] };
    const uint32_t rawOverrideFreqs[3] = { out_context->overrideFreqs[0], out_context->overrideFreqs[1], out_context->overrideFreqs[2] };

    out_context->freqs[SysClkModule_CPU] = getCanonicalFreq(rawFreqs, SysClkModule_CPU);
    out_context->freqs[SysClkModule_GPU] = getCanonicalFreq(rawFreqs, SysClkModule_GPU);
    out_context->freqs[SysClkModule_MEM] = getCanonicalFreq(rawFreqs, SysClkModule_MEM);

    out_context->realFreqs[SysClkModule_CPU] = getCanonicalFreq(rawRealFreqs, SysClkModule_CPU);
    out_context->realFreqs[SysClkModule_GPU] = getCanonicalFreq(rawRealFreqs, SysClkModule_GPU);
    out_context->realFreqs[SysClkModule_MEM] = getCanonicalFreq(rawRealFreqs, SysClkModule_MEM);

    out_context->overrideFreqs[SysClkModule_CPU] = getCanonicalFreq(rawOverrideFreqs, SysClkModule_CPU);
    out_context->overrideFreqs[SysClkModule_GPU] = getCanonicalFreq(rawOverrideFreqs, SysClkModule_GPU);
    out_context->overrideFreqs[SysClkModule_MEM] = getCanonicalFreq(rawOverrideFreqs, SysClkModule_MEM);
    return rc;
}

std::string formatAppId16(uint64_t tid) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%016llX", (unsigned long long)tid);
    return std::string(buf);
}

std::string getPresetFilePath(uint64_t tid) {
    return std::string(SYSCLK_PRESETS_DIR) + "/" + formatAppId16(tid) + ".ini";
}

std::string getLegacyPresetFilePath(uint64_t tid) {
    return std::string(SYSCLK_APP_PRESETS_DIR_LEGACY) + "/" + formatAppId16(tid) + ".ini";
}

Result applyPresetToApp(uint64_t tid, SysClkUiMode mode, const SysClkPresetValues& values) {
    SysClkTitleProfileList profiles;
    Result rc = sysclkIpcGetProfiles(tid, &profiles);
    if (R_FAILED(rc))
        return rc;

    switch (mode) {
        case SysClkUiMode::Handheld:
            applyValuesToProfile(profiles, SysClkProfile_Handheld, values);
            break;

        case SysClkUiMode::Dock:
            applyValuesToProfile(profiles, SysClkProfile_Docked, values);
            break;

        case SysClkUiMode::Charging:
            applyValuesToProfile(profiles, SysClkProfile_HandheldCharging, values);
            applyValuesToProfile(profiles, SysClkProfile_HandheldChargingUSB, values);
            applyValuesToProfile(profiles, SysClkProfile_HandheldChargingOfficial, values);
            break;
    }

    return sysclkIpcSetProfiles(tid, &profiles);
}

bool loadPresetFile(const char* path, SysClkAppModePresetFile& outFile) {
    bool loaded = false;

    outFile.handheld = SysClkPresetValues();
    outFile.dock = SysClkPresetValues();
    outFile.charging = SysClkPresetValues();

    tsl::hlp::doWithSDCardHandle([&] {
        FILE* f = fopen(path, "rb");
        if (!f)
            return;

        bool hasDock = false;
        bool hasCharging = false;
        bool hasPowered = false;
        bool hasLegacySysclk = false;

        enum Section {
            Section_None = 0,
            Section_Handheld,
            Section_Dock,
            Section_Charging,
            Section_Powered,
            Section_Legacy
        } section = Section_None;

        char line[256] = {};
        while (fgets(line, sizeof(line), f)) {
            char* p = line;
            while (*p == ' ' || *p == '\t') ++p;

            if (*p == ';' || *p == '#' || *p == '\n' || *p == '\r' || *p == '\0')
                continue;

            if (*p == '[') {
                if (std::strncmp(p, "[handheld]", 10) == 0) {
                    section = Section_Handheld;
                } else if (std::strncmp(p, "[dock]", 6) == 0) {
                    section = Section_Dock;
                    hasDock = true;
                } else if (std::strncmp(p, "[charging]", 10) == 0) {
                    section = Section_Charging;
                    hasCharging = true;
                } else if (std::strncmp(p, "[powered]", 9) == 0) {
                    section = Section_Powered;
                    hasPowered = true;
                } else if (std::strncmp(p, "[sys-clk]", 9) == 0) {
                    section = Section_Legacy;
                    hasLegacySysclk = true;
                } else {
                    section = Section_None;
                }
                continue;
            }

            auto parseKey = [&](const char* key, uint32_t& dst) {
                const std::size_t klen = std::strlen(key);
                if (std::strncmp(p, key, klen) == 0) {
                    const char* eq = std::strchr(p, '=');
                    if (!eq)
                        return;
                    ++eq;
                    dst = (uint32_t)std::strtoul(eq, nullptr, 10);
                }
            };

            if (section == Section_Handheld) {
                parseKey("cpu_mhz", outFile.handheld.cpuMhz);
                parseKey("gpu_mhz", outFile.handheld.gpuMhz);
                parseKey("mem_mhz", outFile.handheld.memMhz);
            } else if (section == Section_Dock) {
                parseKey("cpu_mhz", outFile.dock.cpuMhz);
                parseKey("gpu_mhz", outFile.dock.gpuMhz);
                parseKey("mem_mhz", outFile.dock.memMhz);
            } else if (section == Section_Charging) {
                parseKey("cpu_mhz", outFile.charging.cpuMhz);
                parseKey("gpu_mhz", outFile.charging.gpuMhz);
                parseKey("mem_mhz", outFile.charging.memMhz);
            } else if (section == Section_Powered) {
                parseKey("cpu_mhz", outFile.dock.cpuMhz);
                parseKey("gpu_mhz", outFile.dock.gpuMhz);
                parseKey("mem_mhz", outFile.dock.memMhz);
                parseKey("cpu_mhz", outFile.charging.cpuMhz);
                parseKey("gpu_mhz", outFile.charging.gpuMhz);
                parseKey("mem_mhz", outFile.charging.memMhz);
            } else if (section == Section_Legacy) {
                parseKey("cpu_mhz", outFile.handheld.cpuMhz);
                parseKey("gpu_mhz", outFile.handheld.gpuMhz);
                parseKey("mem_mhz", outFile.handheld.memMhz);
                parseKey("cpu_mhz", outFile.dock.cpuMhz);
                parseKey("gpu_mhz", outFile.dock.gpuMhz);
                parseKey("mem_mhz", outFile.dock.memMhz);
                parseKey("cpu_mhz", outFile.charging.cpuMhz);
                parseKey("gpu_mhz", outFile.charging.gpuMhz);
                parseKey("mem_mhz", outFile.charging.memMhz);
            }
        }

        fclose(f);

        if (!hasDock && !hasCharging && hasPowered)
            outFile.charging = outFile.dock;

        if (!hasDock && !hasCharging && hasLegacySysclk) {
            outFile.dock = outFile.handheld;
            outFile.charging = outFile.handheld;
        }

        loaded = true;
    });

    return loaded;
}

bool savePresetFile(const char* path, const SysClkAppModePresetFile& fileData) {
    if (!ensureSysclkPresetDirectory())
        return false;

    bool ok = false;

    tsl::hlp::doWithSDCardHandle([&] {
        std::string tmp = std::string(path) + ".tmp";
        FILE* f = fopen(tmp.c_str(), "wb");
        if (!f)
            return;

        int written = std::fprintf(
            f,
            "[handheld]\n"
            "cpu_mhz=%u\n"
            "gpu_mhz=%u\n"
            "mem_mhz=%u\n"
            "\n"
            "[dock]\n"
            "cpu_mhz=%u\n"
            "gpu_mhz=%u\n"
            "mem_mhz=%u\n"
            "\n"
            "[charging]\n"
            "cpu_mhz=%u\n"
            "gpu_mhz=%u\n"
            "mem_mhz=%u\n",
            (unsigned)fileData.handheld.cpuMhz,
            (unsigned)fileData.handheld.gpuMhz,
            (unsigned)fileData.handheld.memMhz,
            (unsigned)fileData.dock.cpuMhz,
            (unsigned)fileData.dock.gpuMhz,
            (unsigned)fileData.dock.memMhz,
            (unsigned)fileData.charging.cpuMhz,
            (unsigned)fileData.charging.gpuMhz,
            (unsigned)fileData.charging.memMhz
        );

        if (written <= 0) {
            fclose(f);
            remove(tmp.c_str());
            return;
        }

        std::fflush(f);
        fclose(f);

        remove(path);
        if (rename(tmp.c_str(), path) != 0) {
            remove(tmp.c_str());
            return;
        }

        ok = true;
    });

    return ok;
}

bool loadPresetForApp(uint64_t tid, SysClkAppModePresetFile& outFile, bool migrateIfNeeded) {
    const std::string newPath = getPresetFilePath(tid);
    if (loadPresetFile(newPath.c_str(), outFile))
        return true;

    const std::string legacyPath = getLegacyPresetFilePath(tid);
    if (!loadPresetFile(legacyPath.c_str(), outFile))
        return false;

    if (migrateIfNeeded)
        savePresetFile(newPath.c_str(), outFile);

    return true;
}

bool tryGetCurrentAppPresetValues(uint64_t appId, SysClkUiMode mode, SysClkPresetValues& outValues) {
    if (appId == 0)
        return false;

    SysClkTitleProfileList profiles;
    Result rc = sysclkIpcGetProfiles(appId, &profiles);
    if (R_FAILED(rc))
        return false;

    switch (mode) {
        case SysClkUiMode::Handheld:
            readValuesFromProfile(profiles, SysClkProfile_Handheld, outValues);
            return true;

        case SysClkUiMode::Dock:
            readValuesFromProfile(profiles, SysClkProfile_Docked, outValues);
            return true;

        case SysClkUiMode::Charging:
            readAggregateChargingValues(profiles, outValues);
            return true;
    }

    return false;
}

bool tryGetCurrentAppAllModePresetValues(uint64_t appId, SysClkAppModePresetFile& outFile) {
    const bool okHandheld = tryGetCurrentAppPresetValues(appId, SysClkUiMode::Handheld, outFile.handheld);
    const bool okDock     = tryGetCurrentAppPresetValues(appId, SysClkUiMode::Dock, outFile.dock);
    const bool okCharging = tryGetCurrentAppPresetValues(appId, SysClkUiMode::Charging, outFile.charging);
    return okHandheld && okDock && okCharging;
}

void refreshVisibleTexts() {
    if (g_sysclkPerformanceGui != nullptr)
        g_sysclkPerformanceGui->refreshTexts();
}

} // namespace sysclk