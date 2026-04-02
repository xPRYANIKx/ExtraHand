#include "sysclk_custom_value_picker.hpp"
#include "sysclk_common.hpp"
#include "sysclk_custom_preset.hpp"
#include "localization.hpp"

namespace sysclk {

    namespace {

        class CompactValuePickerItem : public tsl::elm::ListItem {
        public:
            CompactValuePickerItem(const std::string& text, bool highlightTitle = false)
                : tsl::elm::ListItem(text)
                , m_highlightTitle(highlightTitle) {
            }

            void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                tsl::elm::ListItem::layout(parentX, parentY, parentWidth, parentHeight);
                this->setBoundaries(this->getX(), this->getY(), this->getWidth(), 38);
            }

            void draw(tsl::gfx::Renderer* renderer) override {
                const s32 fontSize = 22;
                const s32 textY = this->getY() + (this->getHeight() / 2) + (fontSize / 2) - 3;
                const auto titleColor = m_highlightTitle
                    ? tsl::gfx::Renderer::a(tsl::style::color::ColorHighlight)
                    : tsl::gfx::Renderer::a(tsl::style::color::ColorText);

                if (this->m_touched && Element::getInputMode() == tsl::InputMode::Touch) {
                    renderer->drawRect(ELEMENT_BOUNDS(this), tsl::gfx::Renderer::a(tsl::style::color::ColorClickAnimation));
                }

                if (this->m_maxWidth == 0) {
                    this->m_maxWidth = this->getWidth() - 16;

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
            }

        private:
            bool m_highlightTitle;
        };

    }

    CustomValuePickerGui::CustomValuePickerGui(CustomPresetGui* parent, SysClkModule module)
        : m_parent(parent)
        , m_module(module) {
    }

    tsl::elm::Element* CustomValuePickerGui::createUI() {
        const char* subtitle =
            (m_module == SysClkModule_CPU) ? tr(currentLang, TextId::Cpu) :
            (m_module == SysClkModule_GPU) ? tr(currentLang, TextId::Gpu) :
            tr(currentLang, TextId::Mem);

        auto* frame = new tsl::elm::OverlayFrame(
            tr(currentLang, TextId::AppName),
            subtitle
        );

        auto* list = new tsl::elm::List();

        const std::vector<uint32_t> values =
            (m_module == SysClkModule_CPU) ? getCpuOptions() :
            (m_module == SysClkModule_GPU) ? getGpuOptions() :
            getMemOptions();

        for (std::size_t i = 0; i < values.size(); ++i) {
            const uint32_t val = values[i];
            const bool highlightSystem = (val == 0);

            auto* item = new CompactValuePickerItem(
                formatCustomValue(val),
                highlightSystem
            );

            item->setClickListener([this, val](u64 keys) {
                if (!(keys & HidNpadButton_A))
                    return false;

                this->m_parent->setValue(this->m_module, val);
                tsl::goBack();
                return true;
                });

            list->addItem(item, 38);
        }

        frame->setContent(list);
        return frame;
    }

}