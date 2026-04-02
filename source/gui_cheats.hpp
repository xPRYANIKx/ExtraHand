#pragma once
#include <string>
#include <vector>
#include <tesla.hpp>
#include "edizon_api.hpp"

class CheatSelectionGui;

class CheatsGui : public tsl::Gui {
public:
    CheatsGui() = default;
    ~CheatsGui() override = default;

    void update() override;
    tsl::elm::Element* createUI() override;

private:
    edizon::CheatContext m_ctx;
    std::string m_summary;

    void reload();
    std::string buildEnabledSummary() const;
};

class CheatSelectionGui : public tsl::Gui {
public:
    CheatSelectionGui() = default;
    ~CheatSelectionGui() override = default;

    void update() override;
    tsl::elm::Element* createUI() override;

private:
    edizon::CheatContext m_ctx;
    uint64_t m_lastReloadTick = 0;

    void reload(bool force = false);
};