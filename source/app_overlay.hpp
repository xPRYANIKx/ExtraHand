#pragma once
#include <memory>
#include <tesla.hpp>
#include "gui_main.hpp"

class ExtraHandOverlay : public tsl::Overlay {
public:
    ExtraHandOverlay();                 
    ~ExtraHandOverlay() override = default;

    std::unique_ptr<tsl::Gui> loadInitialGui() override;
};