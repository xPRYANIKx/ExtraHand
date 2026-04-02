#include <sstream>
#include <vector>
#include "gui_message.hpp"
#include "localization.hpp"
#include "app_utils.hpp"

static std::vector<std::string> wrapTextSimple(
    const std::string& text,
    size_t maxCharsPerLine
) {
    std::vector<std::string> result;

    std::stringstream paragraphStream(text);
    std::string paragraph;

    while (std::getline(paragraphStream, paragraph, '\n')) {
        if (paragraph.empty()) {
            result.push_back("");
            continue;
        }

        std::stringstream wordStream(paragraph);
        std::string word;
        std::string currentLine;

        while (wordStream >> word) {
            std::string candidate = currentLine.empty()
                ? word
                : currentLine + " " + word;

            if (candidate.length() <= maxCharsPerLine) {
                currentLine = candidate;
            }
            else {
                if (!currentLine.empty()) {
                    result.push_back(currentLine);
                    currentLine.clear();
                }

                while (word.length() > maxCharsPerLine) {
                    result.push_back(word.substr(0, maxCharsPerLine));
                    word = word.substr(maxCharsPerLine);
                }

                currentLine = word;
            }
        }

        if (!currentLine.empty()) {
            result.push_back(currentLine);
        }
    }

    return result;
}

MessageGui::MessageGui(const std::string& title, const std::string& message)
    : m_title(title), m_message(message) {
}

tsl::elm::Element* MessageGui::createUI() {
    auto* frame = new tsl::elm::OverlayFrame(
        tr(currentLang, TextId::AppName),
        m_title
    );

    auto* drawer = new tsl::elm::CustomDrawer(
        [this](tsl::gfx::Renderer* renderer, u16 x, u16 y, u16 w, u16 h) {

            const s32 left = x + 20;
            const s32 top = y + 20;
            const s32 fontSize = 20;
            const s32 lineHeight = 26;
            const size_t maxCharsPerLine = 32;

            std::vector<std::string> lines = wrapTextSimple(m_message, maxCharsPerLine);

            s32 currentY = top;

            for (const auto& line : lines) {

                if (line.empty()) {
                    currentY += lineHeight;
                    continue;
                }

                if (currentY + lineHeight > y + h - 10) {
                    break;
                }

                renderer->drawString(
                    line.c_str(),
                    false,
                    left,
                    currentY,
                    fontSize,
                    renderer->a(0xFFFF)
                );

                currentY += lineHeight;
            }
        }
    );

    frame->setContent(drawer);
    return frame;
}