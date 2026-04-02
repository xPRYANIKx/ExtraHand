#include "edizon_api.hpp"
#include "app_utils.hpp"
#include "localization.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <map>
#include <set>
#include <sys/stat.h>
#include <vector>

namespace {

    typedef struct {
        u64 base;
        u64 size;
    } DmntMemoryRegionExtents;

    typedef struct {
        u64 process_id;
        u64 title_id;
        DmntMemoryRegionExtents main_nso_extents;
        DmntMemoryRegionExtents heap_extents;
        DmntMemoryRegionExtents alias_extents;
        DmntMemoryRegionExtents address_space_extents;
        u8 main_nso_build_id[0x20];
    } DmntCheatProcessMetadata;

    typedef struct {
        char readable_name[0x40];
        uint32_t num_opcodes;
        uint32_t opcodes[0x100];
    } DmntCheatDefinition;

    typedef struct {
        bool enabled;
        uint32_t cheat_id;
        DmntCheatDefinition definition;
    } DmntCheatEntry;

    static Result dmntchtHasCheatProcessCompat(bool* out) {
        if (out == nullptr)
            return MAKERESULT(Module_Libnx, LibnxError_BadInput);

        Result rc = 0;
        bool value = false;

        tsl::hlp::doWithSmSession([&] {
            Service srv;
            rc = smGetService(&srv, "dmnt:cht");
            if (R_FAILED(rc))
                return;

            u8 tmp = 0;
            rc = serviceDispatchOut(&srv, 65000, tmp);
            serviceClose(&srv);

            if (R_SUCCEEDED(rc))
                value = (tmp != 0);
            });

        if (R_SUCCEEDED(rc))
            *out = value;
        return rc;
    }

    static Result dmntchtForceOpenCheatProcessCompat(void) {
        Result rc = 0;

        tsl::hlp::doWithSmSession([&] {
            Service srv;
            rc = smGetService(&srv, "dmnt:cht");
            if (R_FAILED(rc))
                return;

            rc = serviceDispatch(&srv, 65003);
            serviceClose(&srv);
            });

        return rc;
    }

    static Result dmntchtGetCheatCountCompat(u64* out_count) {
        if (out_count == nullptr)
            return MAKERESULT(Module_Libnx, LibnxError_BadInput);

        Result rc = 0;
        u64 count = 0;

        tsl::hlp::doWithSmSession([&] {
            Service srv;
            rc = smGetService(&srv, "dmnt:cht");
            if (R_FAILED(rc))
                return;

            rc = serviceDispatchOut(&srv, 65200, count);
            serviceClose(&srv);
            });

        if (R_SUCCEEDED(rc))
            *out_count = count;
        return rc;
    }

    static Result dmntchtGetCheatsCompat(DmntCheatEntry* buffer, u64 max_count, u64 offset, u64* out_count) {
        if (buffer == nullptr || out_count == nullptr)
            return MAKERESULT(Module_Libnx, LibnxError_BadInput);

        Result rc = 0;
        u64 countOut = 0;
        const size_t bufferSize = static_cast<size_t>(max_count) * sizeof(DmntCheatEntry);

        tsl::hlp::doWithSmSession([&] {
            Service srv;
            rc = smGetService(&srv, "dmnt:cht");
            if (R_FAILED(rc))
                return;

            rc = serviceDispatchInOut(
                &srv,
                65201,
                offset,
                countOut,
                .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
                .buffers = { { buffer, bufferSize } }
            );

            serviceClose(&srv);
            });

        if (R_SUCCEEDED(rc))
            *out_count = countOut;
        return rc;
    }

    static Result dmntchtToggleCheatCompat(u32 cheat_id) {
        Result rc = 0;

        tsl::hlp::doWithSmSession([&] {
            Service srv;
            rc = smGetService(&srv, "dmnt:cht");
            if (R_FAILED(rc))
                return;

            rc = serviceDispatchIn(&srv, 65203, cheat_id);
            serviceClose(&srv);
            });

        return rc;
    }

    static bool directoryExists(const std::string& path) {
        bool ok = false;
        tsl::hlp::doWithSDCardHandle([&] {
            struct stat st {};
            ok = (stat(path.c_str(), &st) == 0) && S_ISDIR(st.st_mode);
            });
        return ok;
    }

    static std::string toUpperHex16(u64 value) {
        char buf[17];
        std::snprintf(buf, sizeof(buf), "%016llX", static_cast<unsigned long long>(value));
        return std::string(buf);
    }

    static std::string trimCopy(const std::string& s) {
        std::size_t begin = 0;
        while (begin < s.size() && std::isspace(static_cast<unsigned char>(s[begin])))
            ++begin;

        std::size_t end = s.size();
        while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1])))
            --end;

        return s.substr(begin, end - begin);
    }

    static bool endsWith(const std::string& value, const std::string& suffix) {
        return value.size() >= suffix.size() &&
            value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    static std::string stripEnabledSuffix(const std::string& name, bool* enabledByDefault = nullptr) {
        static const std::string suffix = ":ENABLED";

        if (endsWith(name, suffix)) {
            if (enabledByDefault != nullptr)
                *enabledByDefault = true;
            return name.substr(0, name.size() - suffix.size());
        }

        if (enabledByDefault != nullptr)
            *enabledByDefault = false;
        return name;
    }

    static std::string getFileNameStem(const std::string& path) {
        const std::size_t slash = path.find_last_of("/\\");
        const std::string fileName = (slash == std::string::npos) ? path : path.substr(slash + 1);

        const std::size_t dot = fileName.find_last_of('.');
        if (dot == std::string::npos)
            return fileName;

        return fileName.substr(0, dot);
    }

    static std::string joinPath(const std::string& a, const std::string& b) {
        if (a.empty())
            return b;
        if (a.back() == '/')
            return a + b;
        return a + "/" + b;
    }

    static bool listDirectoryFiles(const std::string& path, std::vector<std::string>& outFiles) {
        bool ok = false;

        tsl::hlp::doWithSDCardHandle([&] {
            DIR* dir = opendir(path.c_str());
            if (dir == nullptr)
                return;

            struct dirent* entry = nullptr;
            while ((entry = readdir(dir)) != nullptr) {
                if (std::strcmp(entry->d_name, ".") == 0 || std::strcmp(entry->d_name, "..") == 0)
                    continue;

                outFiles.push_back(entry->d_name);
            }

            closedir(dir);
            ok = true;
            });

        return ok;
    }

    static bool chooseCheatFile(const std::string& cheatsDir, std::string& outPath) {
        std::vector<std::string> files;
        if (!listDirectoryFiles(cheatsDir, files))
            return false;

        std::sort(files.begin(), files.end());

        for (const std::string& file : files) {
            if (!endsWith(file, ".txt"))
                continue;
            if (file == "toggles.txt")
                continue;

            outPath = joinPath(cheatsDir, file);
            return true;
        }

        return false;
    }

    static std::set<std::string> readToggleSet(const std::string& togglesPath) {
        std::set<std::string> enabledNames;

        tsl::hlp::doWithSDCardHandle([&] {
            FILE* f = std::fopen(togglesPath.c_str(), "rb");
            if (f == nullptr)
                return;

            char line[1024] = {};
            while (std::fgets(line, sizeof(line), f) != nullptr) {
                trimInPlace(line);
                if (line[0] == '\0')
                    continue;

                enabledNames.insert(line);
            }

            std::fclose(f);
            });

        return enabledNames;
    }

    static std::vector<edizon::CheatEntry> parseCheatFile(const std::string& cheatFilePath) {
        std::vector<edizon::CheatEntry> cheats;

        tsl::hlp::doWithSDCardHandle([&] {
            FILE* f = std::fopen(cheatFilePath.c_str(), "rb");
            if (f == nullptr)
                return;

            char line[1024] = {};
            while (std::fgets(line, sizeof(line), f) != nullptr) {
                trimInPlace(line);
                if (line[0] != '[')
                    continue;

                std::string raw = line;
                if (raw.size() < 2 || raw.back() != ']')
                    continue;

                raw = raw.substr(1, raw.size() - 2);
                raw = trimCopy(raw);

                bool enabledByDefault = false;
                std::string cleanName = stripEnabledSuffix(raw, &enabledByDefault);

                edizon::CheatEntry entry;
                entry.name = cleanName;
                entry.enabled = enabledByDefault;
                cheats.push_back(entry);
            }

            std::fclose(f);
            });

        return cheats;
    }

    static bool hasAtmosphereCheatRoots() {
        return directoryExists("/atmosphere/contents") || directoryExists("/atmosphere/titles");
    }

    static Result tryGetProgramIdViaPmDmnt(u64& outProgramId) {
        outProgramId = 0;

        Result rc = pmdmntInitialize();
        if (R_FAILED(rc))
            return rc;

        u64 pid = 0;
        rc = pmdmntGetApplicationProcessId(&pid);
        if (R_SUCCEEDED(rc) && pid != 0) {
            Result rcProgram = pmdmntGetProgramId(&outProgramId, pid);
            pmdmntExit();
            return rcProgram;
        }

        pmdmntExit();
        return rc;
    }

    static Result tryGetProgramIdViaShellThenPm(u64& outProgramId) {
        outProgramId = 0;

        Result rc = pmshellInitialize();
        if (R_FAILED(rc))
            return rc;

        u64 pid = 0;
        rc = pmshellGetApplicationProcessIdForShell(&pid);
        pmshellExit();

        if (R_FAILED(rc) || pid == 0)
            return R_FAILED(rc) ? rc : MAKERESULT(Module_Libnx, LibnxError_BadInput);

        rc = pmdmntInitialize();
        if (R_SUCCEEDED(rc)) {
            Result rcProgram = pmdmntGetProgramId(&outProgramId, pid);
            pmdmntExit();
            if (R_SUCCEEDED(rcProgram) && outProgramId != 0)
                return 0;
        }

        rc = pminfoInitialize();
        if (R_FAILED(rc))
            return rc;

        Result rcProgram = pminfoGetProgramId(&outProgramId, pid);
        pminfoExit();
        return rcProgram;
    }

} // namespace

namespace edizon {

    bool hasInstalledComponent() {
        if (hasAtmosphereCheatRoots())
            return true;

        bool hasProcess = false;
        if (R_SUCCEEDED(dmntchtHasCheatProcessCompat(&hasProcess)))
            return true;

        Result rc = pmdmntInitialize();
        if (R_SUCCEEDED(rc)) {
            pmdmntExit();
            return true;
        }

        rc = pmshellInitialize();
        if (R_SUCCEEDED(rc)) {
            pmshellExit();
            return true;
        }

        return false;
    }

    static Result applyRuntimeState(CheatContext& ctx) {
        ctx.runtimeSyncRc = 0;

        Result rc = dmntchtForceOpenCheatProcessCompat();
        if (R_FAILED(rc)) {
            ctx.runtimeSyncRc = rc;
            return rc;
        }

        bool hasCheatProcess = false;
        rc = dmntchtHasCheatProcessCompat(&hasCheatProcess);
        if (R_FAILED(rc) || !hasCheatProcess) {
            ctx.runtimeSyncRc = R_FAILED(rc) ? rc : MAKERESULT(Module_Libnx, LibnxError_NotFound);
            return ctx.runtimeSyncRc;
        }

        u64 count = 0;
        rc = dmntchtGetCheatCountCompat(&count);
        if (R_FAILED(rc)) {
            ctx.runtimeSyncRc = rc;
            return rc;
        }

        if (count == 0) {
            ctx.runtimeSyncRc = 0;
            return 0;
        }

        std::vector<DmntCheatEntry> runtimeCheats(static_cast<std::size_t>(count));
        u64 outCount = 0;
        rc = dmntchtGetCheatsCompat(runtimeCheats.data(), count, 0, &outCount);
        if (R_FAILED(rc)) {
            ctx.runtimeSyncRc = rc;
            return rc;
        }

        std::map<std::string, bool> desiredStates;
        for (const auto& cheat : ctx.cheats)
            desiredStates[cheat.name] = cheat.enabled;

        for (u64 i = 0; i < outCount; ++i) {
            const DmntCheatEntry& runtimeEntry = runtimeCheats[static_cast<std::size_t>(i)];
            std::string runtimeName = trimCopy(runtimeEntry.definition.readable_name);

            auto it = desiredStates.find(runtimeName);
            if (it == desiredStates.end())
                continue;

            const bool desiredEnabled = it->second;
            if (runtimeEntry.enabled != desiredEnabled) {
                Result toggleRc = dmntchtToggleCheatCompat(runtimeEntry.cheat_id);
                if (R_FAILED(toggleRc)) {
                    ctx.runtimeSyncRc = toggleRc;
                    return toggleRc;
                }
            }
        }

        ctx.runtimeSyncRc = 0;
        return 0;
    }

    Result syncRuntimeCheats(CheatContext& ctx) {
        return applyRuntimeState(ctx);
    }

    Result loadCheatContext(CheatContext& outCtx) {
        outCtx = {};
        outCtx.hasAtmosphereCheats = hasAtmosphereCheatRoots();

        u64 programId = 0;

        Result rc = tryGetProgramIdViaPmDmnt(programId);
        if (R_FAILED(rc) || programId == 0) {
            Result fallbackRc = tryGetProgramIdViaShellThenPm(programId);
            if (R_SUCCEEDED(fallbackRc) && programId != 0) {
                rc = 0;
            }
            else {
                outCtx.detectApplicationRc = R_FAILED(fallbackRc) ? fallbackRc : rc;
                return outCtx.detectApplicationRc;
            }
        }

        outCtx.hasRunningApplication = true;
        outCtx.detectApplicationRc = 0;
        outCtx.titleId = toUpperHex16(programId);

        const std::string contentsDir = "/atmosphere/contents/" + outCtx.titleId + "/cheats";
        const std::string legacyDir = "/atmosphere/titles/" + outCtx.titleId + "/cheats";

        if (directoryExists(contentsDir))
            outCtx.cheatsDirectoryPath = contentsDir;
        else if (directoryExists(legacyDir))
            outCtx.cheatsDirectoryPath = legacyDir;
        else
            return 0;

        std::string cheatFilePath;
        if (!chooseCheatFile(outCtx.cheatsDirectoryPath, cheatFilePath))
            return 0;

        outCtx.hasCheatFile = true;
        outCtx.cheatFilePath = cheatFilePath;
        outCtx.buildId = getFileNameStem(cheatFilePath);
        outCtx.cheats = parseCheatFile(cheatFilePath);

        const std::string togglesPath = joinPath(outCtx.cheatsDirectoryPath, "toggles.txt");
        const std::set<std::string> toggles = readToggleSet(togglesPath);

        if (!toggles.empty()) {
            for (auto& cheat : outCtx.cheats)
                cheat.enabled = (toggles.find(cheat.name) != toggles.end());
        }

        return 0;
    }

    Result saveCheatToggles(CheatContext& ctx) {
        if (ctx.cheatsDirectoryPath.empty())
            return 0;

        bool ok = false;
        const std::string togglesPath = joinPath(ctx.cheatsDirectoryPath, "toggles.txt");
        const std::string tempPath = togglesPath + ".tmp";

        tsl::hlp::doWithSDCardHandle([&] {
            FILE* f = std::fopen(tempPath.c_str(), "wb");
            if (f == nullptr)
                return;

            for (const auto& cheat : ctx.cheats) {
                if (!cheat.enabled)
                    continue;

                std::fprintf(f, "%s\n", cheat.name.c_str());
            }

            std::fflush(f);
            std::fclose(f);

            std::remove(togglesPath.c_str());
            if (std::rename(tempPath.c_str(), togglesPath.c_str()) != 0) {
                std::remove(tempPath.c_str());
                return;
            }

            ok = true;
            });

        if (!ok)
            return MAKERESULT(Module_Libnx, LibnxError_IoError);

        return applyRuntimeState(ctx);
    }

    std::vector<std::string> getEnabledCheatNames(const CheatContext& ctx) {
        std::vector<std::string> out;
        for (const auto& cheat : ctx.cheats) {
            if (cheat.enabled)
                out.push_back(cheat.name);
        }
        return out;
    }

    std::string makeCheatsHeaderTitle(const CheatContext& ctx) {
        if (!ctx.hasRunningApplication || ctx.titleId.empty())
            return tr(currentLang, TextId::ActiveCheatCodes);

        return std::string(tr(currentLang, TextId::ActiveCheatCodes)) + " [" + ctx.titleId + "]";
    }

}