#pragma once
#include <string>
#include <tesla.hpp>

class MessageGui : public tsl::Gui {
public:
    MessageGui(const std::string& title, const std::string& message);

    tsl::elm::Element* createUI() override;

private:
    std::string m_title;
    std::string m_message;
};