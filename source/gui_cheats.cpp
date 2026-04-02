#include "gui_cheats.hpp"
#include "app_utils.hpp"
#include "gui_message.hpp"
#include "localization.hpp"
#include <sstream>

namespace {

    class CompactCheatListItem : public tsl::elm::ListItem {
    public:
        CompactCheatListItem(const std::string& text, const std::string& value = "")
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
                tsl::gfx::Renderer::a(tsl::style::color::ColorHighlight)
            );
        }
    };

    static std::string joinLines(const std::vector<std::string>& lines) {
        if (lines.empty())
            return std::string();

        std::ostringstream ss;
        for (std::size_t i = 0; i < lines.size(); ++i) {
            if (i != 0)
                ss << '\n';
            ss << lines[i];
        }
        return ss.str();
    }

} 

void CheatsGui::reload() {
    edizon::loadCheatContext(m_ctx);
    m_summary = buildEnabledSummary();
}

void CheatsGui::update() {
    reload();
}

std::string CheatsGui::buildEnabledSummary() const {
    if (!m_ctx.hasRunningApplication)
        return tr(currentLang, TextId::NoApplicationRunning);

    if (!m_ctx.hasCheatFile || m_ctx.cheats.empty())
        return tr(currentLang, TextId::NoCheatsFound);

    const std::vector<std::string> enabled = edizon::getEnabledCheatNames(m_ctx);
    std::string text = enabled.empty()
        ? std::string(tr(currentLang, TextId::NoActiveCheats))
        : joinLines(enabled);

    if (m_ctx.runtimeSyncRc != 0) {
        text += "\nRuntime RC: ";
        text += resultToHex(m_ctx.runtimeSyncRc);
    }

    return text;
}

tsl::elm::Element* CheatsGui::createUI() {
    reload();

    auto* frame = new tsl::elm::OverlayFrame(
        tr(currentLang, TextId::AppName),
        edizon::makeCheatsHeaderTitle(m_ctx)
    );

    auto* list = new tsl::elm::List();

    if (m_ctx.hasRunningApplication) {
        auto* selectItem = new tsl::elm::ListItem(tr(currentLang, TextId::SelectCheatCodes));
        selectItem->setClickListener([](u64 keys) {
            if (!(keys & HidNpadButton_A))
                return false;

            tsl::changeTo<CheatSelectionGui>();
            return true;
            });
        list->addItem(selectItem);
    }

    auto* header = new tsl::elm::CategoryHeader(tr(currentLang, TextId::ActiveCheatCodes));
    list->addItem(header);




    auto* activeDrawer = new tsl::elm::CustomDrawer(
        [this](tsl::gfx::Renderer* renderer, u16 x, u16 y, u16 w, u16 h) {
            renderer->drawRect(x, y + h - 1, w, 1, tsl::gfx::Renderer::a({ 0x3, 0x3, 0x3, 0xF }));

            const bool isInfoMessage =
                !m_ctx.hasRunningApplication ||
                !m_ctx.hasCheatFile ||
                m_ctx.cheats.empty() ||
                edizon::getEnabledCheatNames(m_ctx).empty();

            const auto textColor = isInfoMessage
                ? tsl::gfx::Renderer::a({ 0xB, 0xB, 0xB, 0xF })
                : tsl::gfx::Renderer::a({ 0x7, 0xD, 0xF, 0xF });

            std::stringstream stream(m_summary);
            std::string line;
            s32 currentY = y + 16;

            while (std::getline(stream, line, '\n')) {
                renderer->drawString(
                    line.c_str(),
                    false,
                    x + 14,
                    currentY,
                    14,
                    textColor
                );
                currentY += 16;
                if (currentY > y + h - 6)
                    break;
            }
        }
    );
    list->addItem(activeDrawer, 300);




    frame->setContent(list);
    return frame;
}

void CheatSelectionGui::reload(bool force) {
    const uint64_t now = armGetSystemTick();
    if (!force && m_lastReloadTick != 0 && armTicksToNs(now - m_lastReloadTick) < 250000000ULL)
        return;

    m_lastReloadTick = now;
    edizon::loadCheatContext(m_ctx);
}

void CheatSelectionGui::update() {
    reload(false);
}

tsl::elm::Element* CheatSelectionGui::createUI() {
    reload(true);

    auto* frame = new tsl::elm::OverlayFrame(
        tr(currentLang, TextId::AppName),
        tr(currentLang, TextId::SelectCheatCodes)
    );

    auto* list = new tsl::elm::List();

    if (!m_ctx.hasRunningApplication) {
        auto* item = new tsl::elm::ListItem(tr(currentLang, TextId::NoApplicationRunning));
        list->addItem(item);
        frame->setContent(list);
        return frame;
    }

    if (!m_ctx.hasCheatFile || m_ctx.cheats.empty()) {
        auto* item = new tsl::elm::ListItem(tr(currentLang, TextId::NoCheatsFound));
        list->addItem(item);
        frame->setContent(list);
        return frame;
    }

    for (std::size_t i = 0; i < m_ctx.cheats.size(); ++i) {
        auto* item = new CompactCheatListItem(
            m_ctx.cheats[i].name,
            m_ctx.cheats[i].enabled ? "+" : "-"
        );

        item->setClickListener([this, i, item](u64 keys) {
            if (!(keys & HidNpadButton_A))
                return false;

            if (i >= m_ctx.cheats.size())
                return true;

            const bool oldValue = m_ctx.cheats[i].enabled;
            m_ctx.cheats[i].enabled = !m_ctx.cheats[i].enabled;

            Result rc = edizon::saveCheatToggles(m_ctx);
            if (R_FAILED(rc)) {
                m_ctx.cheats[i].enabled = oldValue;

                std::string msg = "saveCheatToggles failed\nRC: ";
                msg += resultToHex(rc);

                if (m_ctx.runtimeSyncRc != 0) {
                    msg += "\nRuntime RC: ";
                    msg += resultToHex(m_ctx.runtimeSyncRc);
                }

                tsl::changeTo<MessageGui>(
                    tr(currentLang, TextId::Error),
                    msg
                );
                return true;
            }

            item->setValue(m_ctx.cheats[i].enabled ? "+" : "-", false);
            return true;
            });

        list->addItem(item);
    }

    frame->setContent(list);
    return frame;
}