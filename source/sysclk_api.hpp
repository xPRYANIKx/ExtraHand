#pragma once
#include <switch.h>
#include <string>

namespace sysclk {

enum SysClkModule {
    SysClkModule_CPU = 0,
    SysClkModule_GPU = 1,
    SysClkModule_MEM = 2
};

enum SysClkProfile {
    SysClkProfile_Handheld = 0,
    SysClkProfile_HandheldCharging = 1,
    SysClkProfile_HandheldChargingUSB = 2,
    SysClkProfile_HandheldChargingOfficial = 3,
    SysClkProfile_Docked = 4
};

enum SysClkPowerSensor {
    SysClkPowerSensor_Now = 0,
    SysClkPowerSensor_Avg = 1
};

enum SysClkThermalSensor {
    SysClkThermalSensor_SOC = 0,
    SysClkThermalSensor_PCB = 1,
    SysClkThermalSensor_Skin = 2
};

enum class SysClkUiMode {
    Handheld = 0,
    Dock = 1,
    Charging = 2
};

#pragma pack(push, 1)
struct SysClkContext {
    u8 enabled;
    u8 _reserved[7];
    u64 applicationId;
    u32 profile;
    u32 freqs[3];
    u32 realFreqs[3];
    u32 overrideFreqs[3];
    u32 temps[3];
    s32 power[2];
    u32 ramLoad[2];
};
#pragma pack(pop)

static_assert(sizeof(SysClkContext) == 84, "SysClkContext layout must match sys-clk IPC");

struct SysClkPresetValues {
    u32 cpuMhz;
    u32 gpuMhz;
    u32 memMhz;

    SysClkPresetValues() : cpuMhz(0), gpuMhz(0), memMhz(0) {}
};

struct SysClkAppModePresetFile {
    SysClkPresetValues handheld;
    SysClkPresetValues dock;
    SysClkPresetValues charging;
};

void initialize();
void finalize();

bool isServiceCachedAvailable();

bool hasConfigDirHint();

bool ensureReady(std::string& outError);

Result getCurrentContext(SysClkContext* out_context);
int getIpcIndexForModule(SysClkModule module);
uint32_t getContextFreq(const SysClkContext& ctx, SysClkModule module);
std::string formatAppId16(uint64_t tid);

std::string getPresetFilePath(uint64_t tid);
std::string getLegacyPresetFilePath(uint64_t tid);

Result applyPresetToApp(uint64_t tid, SysClkUiMode mode, const SysClkPresetValues& values);

bool loadPresetFile(const char* path, SysClkAppModePresetFile& outFile);
bool savePresetFile(const char* path, const SysClkAppModePresetFile& fileData);
bool loadPresetForApp(uint64_t tid, SysClkAppModePresetFile& outFile, bool migrateIfNeeded = true);

bool tryGetCurrentAppPresetValues(uint64_t appId, SysClkUiMode mode, SysClkPresetValues& outValues);
bool tryGetCurrentAppAllModePresetValues(uint64_t appId, SysClkAppModePresetFile& outFile);

void refreshVisibleTexts();

} 
