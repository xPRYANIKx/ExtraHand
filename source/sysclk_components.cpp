#include "sysclk_components.hpp"
#include "app_utils.hpp"
#include "localization.hpp"
#include "sysclk_api.hpp"

namespace sysclk {

ComponentsGui* g_sysclkComponentsGui = nullptr;

ComponentsGui::ComponentsGui() {
    g_sysclkComponentsGui = this;
    m_sysclkTitle = "sys-clk";
}

ComponentsGui::~ComponentsGui() {
    if (g_sysclkComponentsGui == this)
        g_sysclkComponentsGui = nullptr;
}

void ComponentsGui::refreshTexts() {
    if (m_frame != nullptr)
        m_frame->setSubtitle(tr(currentLang, TextId::ActiveComponents));
    rebuildStrings();
}

void ComponentsGui::rebuildStrings() {
    if (m_componentsHeader != nullptr)
        m_componentsHeader->setText(tr(currentLang, TextId::ActiveComponents));

    std::string err;
    m_sysclkValue = tr(currentLang, ensureReady(err) ? TextId::Working : TextId::NotWorking);

    m_helpLine1 = tr(currentLang, TextId::PerformanceHelpText);
    m_helpLine2.clear();
    m_helpLine3.clear();
}

tsl::elm::CustomDrawer* ComponentsGui::createCompactRow(std::string* left, std::string* right) {
    return new tsl::elm::CustomDrawer(
        [left, right](tsl::gfx::Renderer* renderer, u16 x, u16 y, u16 w, u16 h) {
            renderer->drawRect(x, y + h - 1, w, 1, tsl::gfx::Renderer::a({0x3, 0x3, 0x3, 0xF}));
            renderer->drawString(left->c_str(), false, x + 8, y + 18, 16, tsl::gfx::Renderer::a({0x9, 0x9, 0x9, 0xF}));

            auto size = renderer->drawString(right->c_str(), false, 0, 0, 16, tsl::style::color::ColorTransparent);
            renderer->drawString(right->c_str(), false, x + w - (u16)size.first - 8, y + 18, 16, tsl::gfx::Renderer::a({0x7, 0xD, 0xF, 0xF}));
        }
    );
}

tsl::elm::CustomDrawer* ComponentsGui::createSmallTextBlock(std::string* l1, std::string* l2, std::string* l3) {
    return new tsl::elm::CustomDrawer(
        [l1, l2, l3](tsl::gfx::Renderer* renderer, u16 x, u16 y, u16 w, u16 h) {
            renderer->drawRect(x, y + h - 1, w, 1, tsl::gfx::Renderer::a({0x3, 0x3, 0x3, 0xF}));
            const auto color = tsl::gfx::Renderer::a({0x7, 0x7, 0x7, 0xF});

            if (!l1->empty())
                renderer->drawString(l1->c_str(), false, x + 8, y + 14, 12, color);

            if (!l2->empty())
                renderer->drawString(l2->c_str(), false, x + 8, y + 28, 12, color);

            if (!l3->empty())
                renderer->drawString(l3->c_str(), false, x + 8, y + 42, 12, color);
        }
    );
}

tsl::elm::Element* ComponentsGui::createUI() {
    m_frame = new tsl::elm::OverlayFrame(tr(currentLang, TextId::AppName), tr(currentLang, TextId::ActiveComponents));
    auto* list = new tsl::elm::List();

    rebuildStrings();

    m_componentsHeader = new tsl::elm::CategoryHeader(tr(currentLang, TextId::ActiveComponents));
    list->addItem(m_componentsHeader);

    m_sysclkRow = createCompactRow(&m_sysclkTitle, &m_sysclkValue);
    list->addItem(m_sysclkRow, 30);

    m_helpTextBlock = createSmallTextBlock(&m_helpLine1, &m_helpLine2, &m_helpLine3);
    list->addItem(m_helpTextBlock, 28);

    m_frame->setContent(list);
    return m_frame;
}

} 