#pragma once
#include <string>
#include <vector>
#include <switch.h>

namespace edizon {

    struct CheatEntry {
        std::string name;
        bool enabled = false;
    };

    struct CheatContext {
        bool hasRunningApplication = false;
        bool hasCheatFile = false;
        bool hasAtmosphereCheats = false;

        Result detectApplicationRc = 0;
        Result runtimeSyncRc = 0;

        std::string titleId;
        std::string buildId;
        std::string cheatFilePath;
        std::string cheatsDirectoryPath;
        std::vector<CheatEntry> cheats;
    };

    bool hasInstalledComponent();

    Result loadCheatContext(CheatContext& outCtx);
    Result saveCheatToggles(CheatContext& ctx);
    Result syncRuntimeCheats(CheatContext& ctx);

    std::vector<std::string> getEnabledCheatNames(const CheatContext& ctx);
    std::string makeCheatsHeaderTitle(const CheatContext& ctx);

}