#include "sysclk_preset_picker.hpp"
#include "gui_message.hpp"
#include "sysclk_common.hpp"
#include "sysclk_custom_preset.hpp"

namespace sysclk {

    namespace {

        class CompactPresetListItem : public tsl::elm::ListItem {
        public:
            CompactPresetListItem(const std::string& text, const std::string& value = "")
                : tsl::elm::ListItem(text, value) {
            }

            void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                tsl::elm::ListItem::layout(parentX, parentY, parentWidth, parentHeight);
                this->setBoundaries(this->getX(), this->getY(), this->getWidth(), 34);
            }

            void draw(tsl::gfx::Renderer* renderer) override {
                const s32 fontSize = 18;
                const s32 textY = this->getY() + (this->getHeight() / 2) + (fontSize / 2) - 3;
                const auto titleColor = tsl::gfx::Renderer::a(tsl::style::color::ColorText);

                if (this->m_touched && Element::getInputMode() == tsl::InputMode::Touch) {
                    renderer->drawRect(ELEMENT_BOUNDS(this), tsl::gfx::Renderer::a(tsl::style::color::ColorClickAnimation));
                }

                if (this->m_maxWidth == 0) {
                    if (!this->m_value.empty()) {
                        auto [valueWidth, valueHeight] =
                            renderer->drawString(this->m_value.c_str(), false, 0, 0, fontSize, tsl::style::color::ColorTransparent);
                        (void)valueHeight;
                        this->m_maxWidth = this->getWidth() - valueWidth - 28;
                    }
                    else {
                        this->m_maxWidth = this->getWidth() - 16;
                    }

                    auto [width, height] =
                        renderer->drawString(this->m_text.c_str(), false, 0, 0, fontSize, tsl::style::color::ColorTransparent);
                    (void)height;
                    this->m_trunctuated = width > this->m_maxWidth;

                    if (this->m_trunctuated) {
                        this->m_scrollText = this->m_text + "     ";
                        auto [scrollWidth, scrollHeight] =
                            renderer->drawString(this->m_scrollText.c_str(), false, 0, 0, fontSize, tsl::style::color::ColorTransparent);
                        (void)scrollHeight;
                        this->m_scrollText += this->m_text;
                        this->m_textWidth = scrollWidth;
                        this->m_ellipsisText = renderer->limitStringLength(this->m_text, false, fontSize, this->m_maxWidth);
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
                            textY,
                            fontSize,
                            titleColor
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
                            textY,
                            fontSize,
                            titleColor
                        );
                    }
                }
                else {
                    renderer->drawString(
                        this->m_text.c_str(),
                        false,
                        this->getX() + 8,
                        textY,
                        fontSize,
                        titleColor
                    );
                }

                auto [valueWidth, valueHeight] =
                    renderer->drawString(this->m_value.c_str(), false, 0, 0, fontSize, tsl::style::color::ColorTransparent);
                (void)valueHeight;

                renderer->drawString(
                    this->m_value.c_str(),
                    false,
                    this->getX() + this->getWidth() - static_cast<s32>(valueWidth) - 8,
                    textY,
                    fontSize,
                    tsl::gfx::Renderer::a(tsl::style::color::ColorHighlight)
                );
            }
        };

    }

    PresetPickerGui::PresetPickerGui(uint64_t appId, SysClkUiMode mode)
        : m_appId(appId), m_mode(mode) {
    }

    tsl::elm::CustomDrawer* PresetPickerGui::createCompactRow(std::string* left, std::string* right) {
        return new tsl::elm::CustomDrawer(
            [left, right](tsl::gfx::Renderer* renderer, u16 x, u16 y, u16 w, u16 h) {
                renderer->drawRect(x, y + h - 1, w, 1, tsl::gfx::Renderer::a({ 0x3, 0x3, 0x3, 0xF }));

                const s32 fontSize = 14;
                const s32 textY = y + (h / 2) + (fontSize / 2) - 3;

                renderer->drawString(left->c_str(), false, x + 8, textY, fontSize, tsl::gfx::Renderer::a({ 0x9, 0x9, 0x9, 0xF }));

                auto size = renderer->drawString(right->c_str(), false, 0, 0, fontSize, tsl::style::color::ColorTransparent);
                renderer->drawString(
                    right->c_str(),
                    false,
                    x + w - (u16)size.first - 8,
                    textY,
                    fontSize,
                    tsl::gfx::Renderer::a({ 0x7, 0xD, 0xF, 0xF })
                );
            }
        );
    }

    bool PresetPickerGui::loadCurrentFromSystemOrFile() {
        m_currentOk = tryGetCurrentAppPresetValues(m_appId, m_mode, m_current);

        if (!m_currentOk) {
            SysClkAppModePresetFile fileData;
            if (loadPresetForApp(m_appId, fileData)) {
                switch (m_mode) {
                case SysClkUiMode::Handheld: m_current = fileData.handheld; break;
                case SysClkUiMode::Dock:     m_current = fileData.dock; break;
                case SysClkUiMode::Charging: m_current = fileData.charging; break;
                }
                m_currentOk = true;
            }
        }

        if (!m_currentOk)
            m_current = SysClkPresetValues();

        return m_currentOk;
    }

    std::string PresetPickerGui::buildHeaderText() const {
        return std::string(tr(currentLang, TextId::SettingsForPrefix)) + " " + formatAppId16(m_appId);
    }

    void PresetPickerGui::refreshHeader() {
        if (m_profilesHeader != nullptr)
            m_profilesHeader->setText(buildHeaderText());
    }

    void PresetPickerGui::rebuildCurrentProfileStrings() {
        m_currentProfileTitle = getModeDisplayName(m_mode);
        m_currentProfileValue = getCurrentProfileName(m_current);
    }

    void PresetPickerGui::refreshCustomItem() {
        if (m_customItem != nullptr)
            m_customItem->setValue(formatPresetValueForMenuValues(m_current), false);
    }

    Result PresetPickerGui::applyCurrentSelection(const SysClkPresetValues& values) {
        Result rc = applyPresetToApp(m_appId, m_mode, values);
        if (R_FAILED(rc))
            return rc;

        const std::string path = getPresetFilePath(m_appId);
        SysClkAppModePresetFile fileData;
        loadPresetForApp(m_appId, fileData);

        switch (m_mode) {
        case SysClkUiMode::Handheld:
            fileData.handheld = values;
            break;
        case SysClkUiMode::Dock:
            fileData.dock = values;
            break;
        case SysClkUiMode::Charging:
            fileData.charging = values;
            break;
        }

        savePresetFile(path.c_str(), fileData);

        m_current = values;
        m_currentOk = true;
        refreshHeader();
        rebuildCurrentProfileStrings();
        refreshCustomItem();
        return 0;
    }

    void PresetPickerGui::notifyCustomApplied(const SysClkPresetValues& values) {
        m_current = values;
        m_currentOk = true;
        refreshHeader();
        rebuildCurrentProfileStrings();
        refreshCustomItem();
    }

    void PresetPickerGui::update() {
        uint64_t ticks = armGetSystemTick();
        if (m_lastRefreshTick == 0 || armTicksToNs(ticks - m_lastRefreshTick) > 1000000000ULL) {
            m_lastRefreshTick = ticks;
            loadCurrentFromSystemOrFile();
            refreshHeader();
            rebuildCurrentProfileStrings();
            refreshCustomItem();
        }
    }

    tsl::elm::Element* PresetPickerGui::createUI() {
        std::string err;
        if (!ensureReady(err)) {
            auto* frame = new tsl::elm::OverlayFrame(tr(currentLang, TextId::AppName), tr(currentLang, TextId::Performance));
            auto* list = new tsl::elm::List();
            list->addItem(new tsl::elm::ListItem(err));
            frame->setContent(list);
            return frame;
        }

        loadCurrentFromSystemOrFile();
        rebuildCurrentProfileStrings();

        std::string subtitle = getModeDisplayName(m_mode);

        m_frame = new tsl::elm::OverlayFrame(tr(currentLang, TextId::AppName), subtitle);
        auto* list = new tsl::elm::List();

        m_profilesHeader = new tsl::elm::CategoryHeader(buildHeaderText());
        list->addItem(m_profilesHeader);

        m_currentProfileRow = createCompactRow(&m_currentProfileTitle, &m_currentProfileValue);
        list->addItem(m_currentProfileRow, 54);

        const std::size_t presetCount = sizeof(g_sysclkPresets) / sizeof(g_sysclkPresets[0]);

        for (std::size_t i = 0; i < presetCount; ++i) {
            const SysClkPresetDef& p = g_sysclkPresets[i];
            const SysClkPresetValues pv = presetToValues(p);

            std::string title = tr(currentLang, p.nameTextId);
            std::string value = p.showFrequencies
                ? formatTripleMhz(p.cpuMhz, p.gpuMhz, p.memMhz)
                : tr(currentLang, TextId::System);

            auto* item = new CompactPresetListItem(title, value);

            item->setClickListener([this, pv](u64 keys) {
                if (!(keys & HidNpadButton_A))
                    return false;

                Result rc = this->applyCurrentSelection(pv);
                if (R_FAILED(rc)) {
                    tsl::changeTo<MessageGui>(
                        tr(currentLang, TextId::Error),
                        std::string(tr(currentLang, TextId::CommandFailedPrefix)) + resultToHex(rc)
                    );
                    return true;
                }

                return true;
                });

            list->addItem(item, 34);
        }

        m_customItem = new CompactPresetListItem(
            tr(currentLang, TextId::Custom),
            formatPresetValueForMenuValues(m_current)
        );

        m_customItem->setClickListener([this](u64 keys) {
            if (!(keys & HidNpadButton_A))
                return false;

            tsl::changeTo<CustomPresetGui>(this, this->m_appId, this->m_mode, this->m_current);
            return true;
            });

        list->addItem(m_customItem, 34);

        m_frame->setContent(list);
        return m_frame;
    }

}