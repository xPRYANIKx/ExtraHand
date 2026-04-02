# ExtraHand

ExtraHand — это оверлей для Nintendo Switch на базе Tesla Menu, объединяющий управление производительностью, чит-коды, системную информацию и управление энергопитанием в одном меню.

ExtraHand is a Nintendo Switch overlay built for Tesla Menu that combines performance control, cheat management, system information, and power management in a single menu.

---

## Возможности / Features

### Производительность / Performance

![ПРОИЗВОДИТЕЛЬНОСТЬ](https://github.com/user-attachments/assets/93716a59-4deb-431e-aa61-5dd2c685d180)
![ПРОФИЛИ](https://github.com/user-attachments/assets/7681f627-c2e7-41de-a33a-53976968cd78)

Управление частотами CPU, GPU и MEM через sys-clk, выбор профилей и настройка кастомных значений.

Control CPU, GPU, and MEM frequencies via sys-clk, select profiles, and configure custom values.

### Чит-коды / Cheat codes

![ЧИТЫ](https://github.com/user-attachments/assets/b12109d8-d6f5-4d27-9d8f-95f5cc00c9e8)

Поддержка Atmosphère dmnt:cht, включение и выключение читов, а также отображение активных чит-кодов.

Support for Atmosphère dmnt:cht, enabling and disabling cheats, and displaying active cheat codes.

### Информация о системе / System information

![ИНФОРМАЦИЯ](https://github.com/user-attachments/assets/46e65cd9-1f3d-4288-94e4-47914df94f8a)

Отображение модели консоли, версии системы, информации о памяти и подключённых контроллерах.

Display console model, system version, storage information, and connected controllers.

### Электропитание / Power management

![ПЕРЕЗАГРУЗКА](https://github.com/user-attachments/assets/bc010b91-0611-487c-999d-0ab5418785f0)


Показ текущего и среднего энергопотребления, температуры и действий питания.

Show current and average power usage, temperature, and power actions.

---

## Поддерживаемые языки / Supported languages
- English
- Deutsch
- Français
- Español
- Italiano
- Русский

---

## Особенности / Special note

В используемой версии `tesla.hpp` добавлена поддержка русского языка для нижней панели Tesla Menu (`Back` / `OK`).

The used `tesla.hpp` version includes built-in Russian support for the Tesla footer (`Back` / `OK` labels).

```cpp
enum class FooterLang {
    English,
    Russian
};

extern FooterLang footerLang;

[[maybe_unused]] static inline void setFooterLanguage(cfg::FooterLang lang) {
    cfg::footerLang = lang;
}

[[maybe_unused]] static inline cfg::FooterLang getFooterLanguage() {
    return cfg::footerLang;
}

[[maybe_unused]] static inline const char* getFooterBackLabel() {
    switch (cfg::footerLang) {
        case cfg::FooterLang::Russian:
            return "Назад";
        case cfg::FooterLang::English:
        default:
            return "Back";
    }
}

[[maybe_unused]] static inline const char* getFooterOkLabel() {
    switch (cfg::footerLang) {
        case cfg::FooterLang::Russian:
            return "OK";
        case cfg::FooterLang::English:
        default:
            return "OK";
    }
}

[[maybe_unused]] static inline std::string buildFooterLegend() {
    return "\uE0E1  " + std::string(getFooterBackLabel()) +
           "     \uE0E0  " + std::string(getFooterOkLabel());
}
```
---
## Установка / Installation

1. Установить sys-clk. Install sys-clk.
2. Скопировать `.ovl`-файл в папку overlays. Copy the `.ovl` file into the overlays folder (switch/.overlays).
3. Запустить оверлей через горячую клавишу. Launch the overlay using the hotkey (L + R3 + D-Pad Down)


---

## Лицензия / License

Этот проект распространяется под лицензией GNU General Public License v3.0.

This project is licensed under the GNU General Public License v3.0.
