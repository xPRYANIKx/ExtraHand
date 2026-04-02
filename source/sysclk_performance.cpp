#include "sysclk_performance.hpp"
#include "app_utils.hpp"
#include "gui_message.hpp"
#include "sysclk_common.hpp"
#include "sysclk_preset_picker.hpp"

namespace sysclk {

    namespace {

        class CompactProfileItem : public tsl::elm::ListItem {
        public:
            CompactProfileItem(const std::string& text, const std::string& value = "")
                : tsl::elm::ListItem(text, value) {
            }

            void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                tsl::elm::ListItem::layout(parentX, parentY, parentWidth, parentHeight);
                this->setBoundaries(this->getX(), this->getY(), this->getWidth(), 24);
            }

            void draw(tsl::gfx::Renderer* renderer) override {
                if (this->m_touched && Element::getInputMode() == tsl::InputMode::Touch) {
                    renderer->drawRect(ELEMENT_BOUNDS(this), tsl::gfx::Renderer::a(tsl::style::color::ColorClickAnimation));
                }

                if (this->m_maxWidth == 0) {
                    if (!this->m_value.empty()) {
                        auto [valueWidth, valueHeight] =
                            renderer->drawString(this->m_value.c_str(), false, 0, 0, 14, tsl::style::color::ColorTransparent);
                        (void)valueHeight;
                        this->m_maxWidth = this->getWidth() - valueWidth - 26;
                    }
                    else {
                        this->m_maxWidth = this->getWidth() - 16;
                    }

                    auto [width, height] =
                        renderer->drawString(this->m_text.c_str(), false, 0, 0, 14, tsl::style::color::ColorTransparent);
                    (void)height;
                    this->m_trunctuated = width > this->m_maxWidth;

                    if (this->m_trunctuated) {
                        this->m_scrollText = this->m_text + "     ";
                        auto [scrollWidth, scrollHeight] =
                            renderer->drawString(this->m_scrollText.c_str(), false, 0, 0, 14, tsl::style::color::ColorTransparent);
                        (void)scrollHeight;
                        this->m_scrollText += this->m_text;
                        this->m_textWidth = scrollWidth;
                        this->m_ellipsisText = renderer->limitStringLength(this->m_text, false, 14, this->m_maxWidth);
                    }
                    else {
                        this->m_textWidth = width;
                    }
                }

                renderer->drawRect(
                    this->getX(),
                    this->getY() + this->getHeight() - 1,
                    this->getWidth(),
                    1,
                    tsl::gfx::Renderer::a({ 0x3, 0x3, 0x3, 0xF })
                );

                if (this->m_trunctuated) {
                    if (this->m_focused) {
                        renderer->enableScissoring(this->getX(), this->getY(), this->m_maxWidth + 10, this->getHeight());
                        renderer->drawString(
                            this->m_scrollText.c_str(),
                            false,
                            this->getX() + 8 - this->m_scrollOffset,
                            this->getY() + 18,
                            14,
                            tsl::gfx::Renderer::a(tsl::style::color::ColorText)
                        );
                        renderer->disableScissoring();

                        if (this->m_scrollAnimationCounter >= 90) {
                            if (this->m_scrollOffset >= this->m_textWidth) {
                                this->m_scrollOffset = 0;
                                this->m_scrollAnimationCounter = 0;
                            }
                            else {
                                this->m_scrollOffset++;
                            }
                        }
                        else {
                            this->m_scrollAnimationCounter++;
                        }
                    }
                    else {
                        renderer->drawString(
                            this->m_ellipsisText.c_str(),
                            false,
                            this->getX() + 8,
                            this->getY() + 18,
                            14,
                            tsl::gfx::Renderer::a(tsl::style::color::ColorText)
                        );
                    }
                }
                else {
                    renderer->drawString(
                        this->m_text.c_str(),
                        false,
                        this->getX() + 8,
                        this->getY() + 18,
                        14,
                        tsl::gfx::Renderer::a(tsl::style::color::ColorText)
                    );
                }

                auto [valueWidth, valueHeight] =
                    renderer->drawString(this->m_value.c_str(), false, 0, 0, 14, tsl::style::color::ColorTransparent);
                (void)valueHeight;

                renderer->drawString(
                    this->m_value.c_str(),
                    false,
                    this->getX() + this->getWidth() - static_cast<s32>(valueWidth) - 8,
                    this->getY() + 18,
                    14,
                    this->m_faint
                    ? tsl::gfx::Renderer::a(tsl::style::color::ColorDescription)
                    : tsl::gfx::Renderer::a(tsl::style::color::ColorHighlight)
                );
            }
        };

        static std::string buildPerformanceSubtitle(bool ctxValid, u64 applicationId) {
            std::string subtitle = tr(currentLang, TextId::Performance);
            subtitle += " [";
            subtitle += (ctxValid && applicationId != 0) ? formatAppId16(applicationId) : "-";
            subtitle += "]";
            return subtitle;
        }

    }

    PerformanceGui* g_sysclkPerformanceGui = nullptr;

    PerformanceGui::PerformanceGui() {
        g_sysclkPerformanceGui = this;

        m_consoleModelTitle = tr(currentLang, TextId::ConsoleModel);
        m_currentTypeTitle = tr(currentLang, TextId::CurrentType);
        m_usbConnectedTitle = tr(currentLang, TextId::UsbConnected);
        m_resolutionTitle = tr(currentLang, TextId::Resolution);

        m_profileHandheldTitle = tr(currentLang, TextId::HandheldMode);
        m_profileDockTitle = tr(currentLang, TextId::DockMode);
        m_profileChargingTitle = tr(currentLang, TextId::ChargingMode);

        m_cpuTitle = tr(currentLang, TextId::Cpu);
        m_gpuTitle = tr(currentLang, TextId::Gpu);
        m_memTitle = tr(currentLang, TextId::Mem);
        m_powerTitle = tr(currentLang, TextId::PowerShort);
        m_tempTitle = tr(currentLang, TextId::TempShort);
    }

    PerformanceGui::~PerformanceGui() {
        if (g_sysclkPerformanceGui == this)
            g_sysclkPerformanceGui = nullptr;
    }

    void PerformanceGui::refreshTexts() {
        if (m_frame != nullptr)
            m_frame->setSubtitle(buildPerformanceSubtitle(m_ctxValid, m_ctx.applicationId));

        m_consoleModelTitle = tr(currentLang, TextId::ConsoleModel);
        m_currentTypeTitle = tr(currentLang, TextId::CurrentType);
        m_usbConnectedTitle = tr(currentLang, TextId::UsbConnected);
        m_resolutionTitle = tr(currentLang, TextId::Resolution);

        m_profileHandheldTitle = tr(currentLang, TextId::HandheldMode);
        m_profileDockTitle = tr(currentLang, TextId::DockMode);
        m_profileChargingTitle = tr(currentLang, TextId::ChargingMode);

        m_cpuTitle = tr(currentLang, TextId::Cpu);
        m_gpuTitle = tr(currentLang, TextId::Gpu);
        m_memTitle = tr(currentLang, TextId::Mem);
        m_powerTitle = tr(currentLang, TextId::PowerShort);
        m_tempTitle = tr(currentLang, TextId::TempShort);

        rebuildStrings();

        if (m_profilesHeader != nullptr)
            m_profilesHeader->setText(m_profilesHeaderText);

        if (m_handheldProfileItem != nullptr)
            m_handheldProfileItem->setText(tr(currentLang, TextId::HandheldMode));
        if (m_dockProfileItem != nullptr)
            m_dockProfileItem->setText(tr(currentLang, TextId::DockMode));
        if (m_chargingProfileItem != nullptr)
            m_chargingProfileItem->setText(tr(currentLang, TextId::ChargingMode));
    }

    void PerformanceGui::refreshModePresets() {
        if (!m_ctxValid || m_ctx.applicationId == 0) {
            m_modePresetsValid = false;
            m_modePresets = SysClkAppModePresetFile();
            return;
        }

        uint64_t ticks = armGetSystemTick();
        if (!m_modePresetsValid || armTicksToNs(ticks - m_lastPresetTick) > 1000000000ULL) {
            m_lastPresetTick = ticks;
            m_modePresetsValid = tryGetCurrentAppAllModePresetValues(m_ctx.applicationId, m_modePresets);

            if (!m_modePresetsValid)
                m_modePresetsValid = loadPresetForApp(m_ctx.applicationId, m_modePresets);
        }
    }

    void PerformanceGui::refreshContext() {
        std::string err;
        if (!ensureReady(err)) {
            m_ctxValid = false;
            rebuildStrings();
            return;
        }

        uint64_t ticks = armGetSystemTick();
        if (!m_ctxValid || armTicksToNs(ticks - m_lastCtxTick) > 250000000ULL) {
            m_lastCtxTick = ticks;
            Result rc = getCurrentContext(&m_ctx);
            m_ctxValid = R_SUCCEEDED(rc);
        }

        refreshModePresets();
        rebuildStrings();
    }

    void PerformanceGui::rebuildStrings() {
        m_infoHeaderText = tr(currentLang, TextId::Info);
        m_profilesHeaderText = tr(currentLang, TextId::Profiles);

        if (m_frame != nullptr)
            m_frame->setSubtitle(buildPerformanceSubtitle(m_ctxValid, m_ctx.applicationId));

        if (m_infoHeader != nullptr)
            m_infoHeader->setText(m_infoHeaderText);

        m_consoleModelValue = getConsoleModelString();
        m_resolutionValue = getResolutionString();

        if (!m_ctxValid) {
            m_currentTypeValue = "-";
            m_usbConnectedValue = "-";
            m_profileHandheldValue = "-";
            m_profileDockValue = "-";
            m_profileChargingValue = "-";
            m_cpuValue = "-";
            m_gpuValue = "-";
            m_memValue = "-";
            m_powerValue = "-";
            m_tempValue = "-";
        }
        else {
            m_currentTypeValue = (appletGetOperationMode() == AppletOperationMode_Console) ? tr(currentLang, TextId::DockMode) : (appletGetOperationMode() == AppletOperationMode_Handheld ? tr(currentLang, TextId::HandheldMode) : "-");

            m_usbConnectedValue = tr(currentLang, isExternalPowerProfile(static_cast<SysClkProfile>(m_ctx.profile)) ? TextId::Yes : TextId::No);

            m_profileHandheldValue = m_modePresetsValid ? getCurrentProfileName(m_modePresets.handheld) : "-";
            m_profileDockValue = m_modePresetsValid ? getCurrentProfileName(m_modePresets.dock) : "-";
            m_profileChargingValue = m_modePresetsValid ? getCurrentProfileName(m_modePresets.charging) : "-";

            auto formatTargetAndReal = [&](SysClkModule module) -> std::string {
                const int index = static_cast<int>(module);
                const uint32_t targetHz =
                    (m_ctx.overrideFreqs[index] != 0) ? m_ctx.overrideFreqs[index] :
                    (m_ctx.freqs[index] != 0) ? m_ctx.freqs[index] :
                    m_ctx.realFreqs[index];

                const uint32_t realHz = m_ctx.realFreqs[index];
                return formatFreqHzPrecise(targetHz) + " | " + formatFreqHzPrecise(realHz);
                };

            m_cpuValue = formatTargetAndReal(SysClkModule_CPU);
            m_gpuValue = formatTargetAndReal(SysClkModule_GPU);
            m_memValue = formatTargetAndReal(SysClkModule_MEM);

            m_powerValue =
                std::string(tr(currentLang, TextId::PowerNowPrefix)) + " " +
                formatPowerMilliwatts((int)m_ctx.power[SysClkPowerSensor_Now]) +
                " | " +
                tr(currentLang, TextId::PowerAvgPrefix) + " " +
                formatPowerMilliwatts((int)m_ctx.power[SysClkPowerSensor_Avg]);

            m_tempValue = formatTemperatureSummary(m_ctx);
        }

        if (m_handheldProfileItem != nullptr)
            m_handheldProfileItem->setValue(m_profileHandheldValue, false);
        if (m_dockProfileItem != nullptr)
            m_dockProfileItem->setValue(m_profileDockValue, false);
        if (m_chargingProfileItem != nullptr)
            m_chargingProfileItem->setValue(m_profileChargingValue, false);
    }

    void PerformanceGui::update() {
        refreshContext();
    }

    tsl::elm::CustomDrawer* PerformanceGui::createCompactRow(std::string* left, std::string* right) {
        return new tsl::elm::CustomDrawer(
            [left, right](tsl::gfx::Renderer* renderer, u16 x, u16 y, u16 w, u16 h) {
                renderer->drawRect(x, y + h - 1, w, 1, tsl::gfx::Renderer::a({ 0x3, 0x3, 0x3, 0xF }));
                renderer->drawString(left->c_str(), false, x + 8, y + 18, 14, tsl::gfx::Renderer::a({ 0x9, 0x9, 0x9, 0xF }));

                auto size = renderer->drawString(right->c_str(), false, 0, 0, 14, tsl::style::color::ColorTransparent);
                renderer->drawString(
                    right->c_str(),
                    false,
                    x + w - static_cast<u16>(size.first) - 8,
                    y + 18,
                    14,
                    tsl::gfx::Renderer::a({ 0x7, 0xD, 0xF, 0xF })
                );
            }
        );
    }

    tsl::elm::Element* PerformanceGui::createUI() {
        std::string err;
        bool ok = ensureReady(err);

        m_frame = new tsl::elm::OverlayFrame(tr(currentLang, TextId::AppName), buildPerformanceSubtitle(false, 0));
        auto* list = new tsl::elm::List();

        if (!ok) {
            list->addItem(new tsl::elm::ListItem(err));
            m_frame->setContent(list);
            return m_frame;
        }

        refreshContext();

        m_infoHeader = new tsl::elm::CategoryHeader(m_infoHeaderText);
        list->addItem(m_infoHeader);

        m_consoleModelRow = createCompactRow(&m_consoleModelTitle, &m_consoleModelValue);
        list->addItem(m_consoleModelRow, 28);

        m_currentTypeRow = createCompactRow(&m_currentTypeTitle, &m_currentTypeValue);
        list->addItem(m_currentTypeRow, 28);

        m_usbConnectedRow = createCompactRow(&m_usbConnectedTitle, &m_usbConnectedValue);
        list->addItem(m_usbConnectedRow, 28);

        m_resolutionRow = createCompactRow(&m_resolutionTitle, &m_resolutionValue);
        list->addItem(m_resolutionRow, 28);

        m_cpuRow = createCompactRow(&m_cpuTitle, &m_cpuValue);
        list->addItem(m_cpuRow, 28);

        m_gpuRow = createCompactRow(&m_gpuTitle, &m_gpuValue);
        list->addItem(m_gpuRow, 28);

        m_memRow = createCompactRow(&m_memTitle, &m_memValue);
        list->addItem(m_memRow, 28);

        m_powerRow = createCompactRow(&m_powerTitle, &m_powerValue);
        list->addItem(m_powerRow, 28);

        m_tempRow = createCompactRow(&m_tempTitle, &m_tempValue);
        list->addItem(m_tempRow, 28);

        m_profilesHeader = new tsl::elm::CategoryHeader(m_profilesHeaderText);
        list->addItem(m_profilesHeader);

        m_handheldProfileItem = new CompactProfileItem(tr(currentLang, TextId::HandheldMode), m_profileHandheldValue);
        m_handheldProfileItem->setClickListener([this](u64 keys) {
            if (!(keys & HidNpadButton_A))
                return false;

            this->refreshContext();
            if (!this->m_ctxValid || this->m_ctx.applicationId == 0) {
                tsl::changeTo<MessageGui>(tr(currentLang, TextId::Error), tr(currentLang, TextId::NoApplicationRunning));
                return true;
            }

            tsl::changeTo<PresetPickerGui>(this->m_ctx.applicationId, SysClkUiMode::Handheld);
            return true;
            });
        list->addItem(m_handheldProfileItem, 24);

        m_dockProfileItem = new CompactProfileItem(tr(currentLang, TextId::DockMode), m_profileDockValue);
        m_dockProfileItem->setClickListener([this](u64 keys) {
            if (!(keys & HidNpadButton_A))
                return false;

            this->refreshContext();
            if (!this->m_ctxValid || this->m_ctx.applicationId == 0) {
                tsl::changeTo<MessageGui>(tr(currentLang, TextId::Error), tr(currentLang, TextId::NoApplicationRunning));
                return true;
            }

            tsl::changeTo<PresetPickerGui>(this->m_ctx.applicationId, SysClkUiMode::Dock);
            return true;
            });
        list->addItem(m_dockProfileItem, 24);

        m_chargingProfileItem = new CompactProfileItem(tr(currentLang, TextId::ChargingMode), m_profileChargingValue);
        m_chargingProfileItem->setClickListener([this](u64 keys) {
            if (!(keys & HidNpadButton_A))
                return false;

            this->refreshContext();
            if (!this->m_ctxValid || this->m_ctx.applicationId == 0) {
                tsl::changeTo<MessageGui>(tr(currentLang, TextId::Error), tr(currentLang, TextId::NoApplicationRunning));
                return true;
            }

            tsl::changeTo<PresetPickerGui>(this->m_ctx.applicationId, SysClkUiMode::Charging);
            return true;
            });
        list->addItem(m_chargingProfileItem, 24);

        m_frame->setContent(list);
        return m_frame;
    }

}