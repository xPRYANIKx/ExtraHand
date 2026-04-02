#pragma once
#include <cstdio>
#include <string>
#include <vector>
#include "app_utils.hpp"
#include "sysclk_api.hpp"

namespace sysclk {

    struct SysClkPresetDef {
        const char* id;
        TextId nameTextId;
        uint32_t cpuMhz;
        uint32_t gpuMhz;
        uint32_t memMhz;
        bool showFrequencies;
    };

    static const SysClkPresetDef g_sysclkPresets[] = {
        { "default",   TextId::Default,    0,    0,   0,    false },
        { "base",      TextId::Base,       1020, 384, 1331, true  },
        { "medium",    TextId::Medium,     1224, 460, 1600, true  },
        { "mediumplus",TextId::MediumPlus, 1224, 460, 2133, true  },
        { "high",      TextId::High,       1581, 768, 2133, true  },
        { "ultra",     TextId::Ultra,      1683, 844, 2133, true  },
        { "maximum",   TextId::Maximum,    1785, 921, 2133, true  }
    };

    inline SysClkPresetValues presetToValues(const SysClkPresetDef& p) {
        SysClkPresetValues v;
        v.cpuMhz = p.cpuMhz;
        v.gpuMhz = p.gpuMhz;
        v.memMhz = p.memMhz;
        return v;
    }

    inline bool equalsPresetValues(const SysClkPresetValues& a, const SysClkPresetValues& b) {
        return a.cpuMhz == b.cpuMhz &&
            a.gpuMhz == b.gpuMhz &&
            a.memMhz == b.memMhz;
    }

    inline std::string formatCustomValue(uint32_t mhz) {
        if (mhz == 0)
            return tr(currentLang, TextId::System);

        return std::to_string((unsigned)mhz);
    }

    inline std::string formatMenuValuePart(uint32_t mhz) {
        if (mhz == 0)
            return "-";

        return std::to_string((unsigned)mhz);
    }

    inline std::string formatProfileFreqMhzOnlyIfCustom(uint32_t mhz) {
        if (mhz == 0)
            return tr(currentLang, TextId::System);
        return std::to_string((unsigned)mhz);
    }

    inline std::string formatTripleMhz(uint32_t cpu, uint32_t gpu, uint32_t mem) {
        char buf[64];
        std::snprintf(
            buf,
            sizeof(buf),
            "%u | %u | %u",
            (unsigned)cpu,
            (unsigned)gpu,
            (unsigned)mem
        );
        return std::string(buf);
    }

    inline std::string formatPresetValueForMenuValues(const SysClkPresetValues& values) {
        return formatMenuValuePart(values.cpuMhz) + " | " +
            formatMenuValuePart(values.gpuMhz) + " | " +
            formatMenuValuePart(values.memMhz);
    }

    inline std::string getCurrentProfileName(const SysClkPresetValues& current) {
        for (std::size_t i = 0; i < sizeof(g_sysclkPresets) / sizeof(g_sysclkPresets[0]); ++i) {
            SysClkPresetValues pv = presetToValues(g_sysclkPresets[i]);
            if (equalsPresetValues(current, pv))
                return tr(currentLang, g_sysclkPresets[i].nameTextId);
        }

        if (current.cpuMhz == 0 && current.gpuMhz == 0 && current.memMhz == 0)
            return tr(currentLang, TextId::System);

        return tr(currentLang, TextId::Custom);
    }

    inline const char* getModeDisplayName(SysClkUiMode mode) {
        switch (mode) {
        case SysClkUiMode::Handheld: return tr(currentLang, TextId::HandheldMode);
        case SysClkUiMode::Dock:     return tr(currentLang, TextId::DockMode);
        case SysClkUiMode::Charging: return tr(currentLang, TextId::ChargingMode);
        default:                     return tr(currentLang, TextId::HandheldMode);
        }
    }

    inline std::vector<uint32_t> getCpuOptions() {
        return { 0, 816, 918, 1020, 1122, 1224, 1326, 1428, 1581, 1683, 1785 };
    }

    inline std::vector<uint32_t> getGpuOptions() {
        return { 0, 307, 384, 460, 537, 614, 691, 768, 844, 921 };
    }

    inline std::vector<uint32_t> getMemOptions() {
        return { 0, 665, 800, 1065, 1331, 1600, 2133 };
    }

    inline std::string formatFreqHz(uint32_t hz) {
        if (hz == 0)
            return "-";

        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.1f MHz", (double)hz / 1000000.0);
        return std::string(buf);
    }

    inline std::string formatFreqHzPrecise(uint32_t hz) {
        if (hz == 0)
            return "-";

        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.1f MHz", (double)hz / 1000000.0);
        return std::string(buf);
    }

    inline std::string formatTempMilli(uint32_t milliC) {
        char buf[24];
        std::snprintf(buf, sizeof(buf), "%u.%u C", milliC / 1000, (milliC % 1000) / 100);
        return std::string(buf);
    }

    inline std::string formatPowerMilliwatts(int microwatts) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.1f mW", (double)microwatts / 1000.0);
        return std::string(buf);
    }

    inline std::string getPowerSourceDisplay(bool isExternalPower) {
        return isExternalPower ? "AC" : "BAT";
    }

    inline bool isExternalPowerProfile(SysClkProfile p) {
        return p == SysClkProfile_Docked ||
            p == SysClkProfile_HandheldCharging ||
            p == SysClkProfile_HandheldChargingUSB ||
            p == SysClkProfile_HandheldChargingOfficial;
    }

    inline void appendTempPart(std::string& dst, const char* label, uint32_t milliC) {
        if (milliC == 0)
            return;

        if (!dst.empty())
            dst += " | ";

        dst += label;
        dst += " ";
        dst += formatTempMilli(milliC);
    }

    inline std::string formatTemperatureSummary(const SysClkContext& ctx) {
        std::string result;
        appendTempPart(result, "SOC", ctx.temps[SysClkThermalSensor_SOC]);
        appendTempPart(result, "PCB", ctx.temps[SysClkThermalSensor_PCB]);
        appendTempPart(result, "Skin", ctx.temps[SysClkThermalSensor_Skin]);
        return result.empty() ? "-" : result;
    }

    inline const char* localizeProfile(SysClkProfile p) {
        switch (p) {
        case SysClkProfile_Handheld: return tr(currentLang, TextId::Handheld);
        case SysClkProfile_Docked: return tr(currentLang, TextId::Docked);
        case SysClkProfile_HandheldCharging:
        case SysClkProfile_HandheldChargingUSB:
        case SysClkProfile_HandheldChargingOfficial:
            return tr(currentLang, TextId::Charging);
        default:
            return "-";
        }
    }

    inline const char* localizeCurrentType(u32 p) {
        switch ((SysClkProfile)p) {
        case SysClkProfile_Handheld: return tr(currentLang, TextId::Handheld);
        case SysClkProfile_Docked: return tr(currentLang, TextId::Docked);
        case SysClkProfile_HandheldCharging:
        case SysClkProfile_HandheldChargingUSB:
        case SysClkProfile_HandheldChargingOfficial:
            return tr(currentLang, TextId::Charging);
        default: return "-";
        }
    }

}