#include "sysclk_custom_preset.hpp"
#include "gui_message.hpp"
#include "sysclk_api.hpp"
#include "sysclk_common.hpp"
#include "sysclk_custom_value_picker.hpp"
#include "sysclk_preset_picker.hpp"

namespace sysclk {

    namespace {

        class CompactCustomPresetItem : public tsl::elm::ListItem {
        public:
            CompactCustomPresetItem(const std::string& text, const std::string& value = "")
                : tsl::elm::ListItem(text, value) {
            }

            void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                tsl::elm::ListItem::layout(parentX, parentY, parentWidth, parentHeight);
                this->setBoundaries(this->getX(), this->getY(), this->getWidth(), 40);
            }

            void draw(tsl::gfx::Renderer* renderer) override {
                const s32 fontSize = 23;
                const s32 textY = this->getY() + (this->getHeight() / 2) + (fontSize / 2) - 3;

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
                            textY,
                            fontSize,
                            tsl::gfx::Renderer::a(tsl::style::color::ColorText)
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
                        tsl::gfx::Renderer::a(tsl::style::color::ColorText)
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

        class LargeBlueApplyItem : public tsl::elm::ListItem {
        public:
            explicit LargeBlueApplyItem(const std::string& text)
                : tsl::elm::ListItem(text) {
            }

            void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                tsl::elm::ListItem::layout(parentX, parentY, parentWidth, parentHeight);
                this->setBoundaries(this->getX(), this->getY(), this->getWidth(), 46);
            }

            void draw(tsl::gfx::Renderer* renderer) override {
                const s32 fontSize = 26;
                const s32 textY = this->getY() + (this->getHeight() / 2) + (fontSize / 2) - 4;

                if (this->m_touched && Element::getInputMode() == tsl::InputMode::Touch) {
                    renderer->drawRect(ELEMENT_BOUNDS(this), tsl::gfx::Renderer::a(tsl::style::color::ColorClickAnimation));
                }

                renderer->drawRect(
                    this->getX(),
                    this->getY() + this->getHeight() - 1,
                    this->getWidth(),
                    1,
                    tsl::gfx::Renderer::a({ 0x3, 0x3, 0x3, 0xF })
                );

                renderer->drawString(
                    this->m_text.c_str(),
                    false,
                    this->getX() + 8,
                    textY,
                    fontSize,
                    tsl::gfx::Renderer::a({ 0x7, 0xD, 0xF, 0xF })
                );
            }
        };

    } 

    CustomPresetGui::CustomPresetGui(
        PresetPickerGui* parentPicker,
        uint64_t appId,
        SysClkUiMode mode,
        const SysClkPresetValues& initialValues
    )
        : m_appId(appId)
        , m_mode(mode)
        , m_values(initialValues)
        , m_parentPicker(parentPicker) {
    }

    void CustomPresetGui::refreshValues() {
        if (m_cpuItem != nullptr) m_cpuItem->setValue(formatCustomValue(m_values.cpuMhz), false);
        if (m_gpuItem != nullptr) m_gpuItem->setValue(formatCustomValue(m_values.gpuMhz), false);
        if (m_memItem != nullptr) m_memItem->setValue(formatCustomValue(m_values.memMhz), false);
    }

    void CustomPresetGui::setValue(SysClkModule module, uint32_t value) {
        if (module == SysClkModule_CPU) m_values.cpuMhz = value;
        if (module == SysClkModule_GPU) m_values.gpuMhz = value;
        if (module == SysClkModule_MEM) m_values.memMhz = value;
        refreshValues();
    }

    const SysClkPresetValues& CustomPresetGui::getValues() const {
        return m_values;
    }

    tsl::elm::Element* CustomPresetGui::createUI() {
        const std::string subtitle = getModeDisplayName(m_mode);

        m_frame = new tsl::elm::OverlayFrame(
            tr(currentLang, TextId::AppName),
            subtitle
        );

        auto* list = new tsl::elm::List();

        m_cpuItem = new CompactCustomPresetItem(tr(currentLang, TextId::Cpu), formatCustomValue(m_values.cpuMhz));
        m_cpuItem->setClickListener([this](u64 keys) {
            if (!(keys & HidNpadButton_A))
                return false;

            tsl::changeTo<CustomValuePickerGui>(this, SysClkModule_CPU);
            return true;
            });
        list->addItem(m_cpuItem, 40);

        m_gpuItem = new CompactCustomPresetItem(tr(currentLang, TextId::Gpu), formatCustomValue(m_values.gpuMhz));
        m_gpuItem->setClickListener([this](u64 keys) {
            if (!(keys & HidNpadButton_A))
                return false;

            tsl::changeTo<CustomValuePickerGui>(this, SysClkModule_GPU);
            return true;
            });
        list->addItem(m_gpuItem, 40);

        m_memItem = new CompactCustomPresetItem(tr(currentLang, TextId::Mem), formatCustomValue(m_values.memMhz));
        m_memItem->setClickListener([this](u64 keys) {
            if (!(keys & HidNpadButton_A))
                return false;

            tsl::changeTo<CustomValuePickerGui>(this, SysClkModule_MEM);
            return true;
            });
        list->addItem(m_memItem, 40);

        auto* applyItem = new LargeBlueApplyItem(tr(currentLang, TextId::Apply));
        applyItem->setClickListener([this](u64 keys) {
            if (!(keys & HidNpadButton_A))
                return false;

            Result rc = applyPresetToApp(this->m_appId, this->m_mode, this->m_values);
            if (R_FAILED(rc)) {
                tsl::changeTo<MessageGui>(
                    tr(currentLang, TextId::Error),
                    std::string(tr(currentLang, TextId::CommandFailedPrefix)) + resultToHex(rc)
                );
                return true;
            }

            const std::string path = getPresetFilePath(this->m_appId);

            SysClkAppModePresetFile fileData;
            loadPresetForApp(this->m_appId, fileData);

            switch (this->m_mode) {
            case SysClkUiMode::Handheld:
                fileData.handheld = this->m_values;
                break;
            case SysClkUiMode::Dock:
                fileData.dock = this->m_values;
                break;
            case SysClkUiMode::Charging:
                fileData.charging = this->m_values;
                break;
            }

            savePresetFile(path.c_str(), fileData);

            if (this->m_parentPicker != nullptr)
                this->m_parentPicker->notifyCustomApplied(this->m_values);

            tsl::goBack();
            return true;
            });
        list->addItem(applyItem, 46);

        m_frame->setContent(list);
        refreshValues();
        return m_frame;
    }

}