#pragma once
#include <string>
#include <tesla.hpp>

namespace sysclk {

class ComponentsGui : public tsl::Gui {
public:
    ComponentsGui();
    ~ComponentsGui() override;

    void refreshTexts();
    tsl::elm::Element* createUI() override;

private:
    tsl::elm::OverlayFrame* m_frame = nullptr;
    tsl::elm::CategoryHeader* m_componentsHeader = nullptr;

    std::string m_sysclkTitle;
    std::string m_sysclkValue;
    std::string m_helpLine1;
    std::string m_helpLine2;
    std::string m_helpLine3;

    tsl::elm::CustomDrawer* m_sysclkRow = nullptr;
    tsl::elm::CustomDrawer* m_helpTextBlock = nullptr;

    void rebuildStrings();
    tsl::elm::CustomDrawer* createCompactRow(std::string* left, std::string* right);
    tsl::elm::CustomDrawer* createSmallTextBlock(std::string* l1, std::string* l2, std::string* l3);
};

extern ComponentsGui* g_sysclkComponentsGui;

} 