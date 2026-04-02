#pragma once
#include <switch.h>
#include <cstddef>

enum class Lang {
    English,
    German,
    French,
    Spanish,
    Italian,
    Russian
};

enum class TextId {
    // =========================================================================
    // Общие / приложение
    // =========================================================================
    AppName,
    Error,
    CommandFailedPrefix,
    Done,
    Yes,
    No,

    // =========================================================================
    // Главное меню
    // =========================================================================
    Menu,
    Performance,
    CheatCodesMenu,
    SystemInformation,
    OverlaySettings,
    Power,

    // =========================================================================
    // Информация о системе
    // =========================================================================
    ConsoleModel,
    SystemVersion,
    FreeSdStorage,
    FreeInternalStorage,
    Resolution,
    ResolutionFormat,
    ExternalController,
    Controller,
    UnknownController,
    NoConnected,
    ControllerNumberFormat,
    StorageFormat,

    // =========================================================================
    // Параметры оверлея
    // =========================================================================
    Hotkeys,
    Language,
    ActiveComponents,
    Working,
    NotWorking,
    AtmosphereCheatsComponent,
    SysClkInlineDescription,
    EdizonInlineDescription,

    // =========================================================================
    // Электропитание
    // =========================================================================
    Info,
    Actions,
    Reboot,
    Shutdown,
    PowerShort,
    TempShort,
    PowerNowPrefix,
    PowerAvgPrefix,

    // =========================================================================
    // Чит-коды
    // =========================================================================
    ActiveCheatCodes,
    SelectCheatCodes,
    NoActiveCheats,
    NoCheatsFound,

    // =========================================================================
    // Производительность
    // =========================================================================
    CurrentType,
    UsbConnected,
    Handheld,
    Docked,
    Charging,
    Default,
    Base,
    Medium,
    MediumPlus,
    High,
    Ultra,
    Maximum,
    Custom,
    System,
    HandheldMode,
    DockMode,
    ChargingMode,
    SettingsForPrefix,
    Apply,
    NoApplicationRunning,
    SysclkUnavailable,
    PerformanceHelpText,
    Cpu,
    Gpu,
    Mem,
    Profiles
};

struct LocalizedText {
    const char* en;
    const char* de;
    const char* fr;
    const char* es;
    const char* it;
    const char* ru;
};

static constexpr LocalizedText g_localizedTextTable[] = {
    // =========================================================================
    // Общие / приложение
    // =========================================================================
    { "ExtraHand", "ExtraHand", "ExtraHand", "ExtraHand", "ExtraHand", "ExtraHand" }, // AppName
    { "Error", "Fehler", "Erreur", "Error", "Errore", "Ошибка" }, // Error
    { "Command failed: ", "Befehl fehlgeschlagen: ", "Échec de la commande : ", "Error del comando: ", "Comando fallito: ", "Ошибка выполнения: " }, // CommandFailedPrefix
    { "Done", "Done", "Done", "Done", "Done", "Готово" }, // Done
    { "Yes", "Ja", "Oui", "Sí", "Sì", "Да" }, // Yes
    { "No", "Nein", "Non", "No", "No", "Нет" }, // No

    // =========================================================================
    // Главное меню
    // =========================================================================
    { "Menu", "Menü", "Menu", "Menú", "Menu", "Меню" }, // Menu
    { "Performance", "Leistung", "Performances", "Rendimiento", "Prestazioni", "Производительность" }, // Performance
    { "Cheat codes", "Cheat-Codes", "Codes de triche", "Códigos de trucos", "Codici cheat", "Чит-коды" }, // CheatCodesMenu
    { "System information", "Systeminformationen", "Informations système", "Información del sistema", "Informazioni di sistema", "Информация о системе" }, // SystemInformation
    { "Overlay settings", "Overlay-Einstellungen", "Paramètres de l'overlay", "Configuración del overlay", "Impostazioni overlay", "Параметры оверлея" }, // OverlaySettings
    { "Reboot", "Neustart", "Redémarrage", "Reinicio", "Riavvio", "Перезагрузка" }, // Power

    // =========================================================================
    // Информация о системе
    // =========================================================================
    { "Console model", "Konsolenmodell", "Modèle de console", "Modelo de consola", "Modello console", "Модель консоли" }, // ConsoleModel
    { "System version", "Systemversion", "Version du système", "Versión del sistema", "Versione del sistema", "Версия системы" }, // SystemVersion
    { "Free SD storage", "Freier SD-Speicher", "Espace libre sur SD", "Espacio libre en SD", "Spazio libero su SD", "Свободно памяти на SD" }, // FreeSdStorage
    { "Free internal storage", "Freier interner Speicher", "Espace libre interne", "Espacio libre interno", "Spazio libero interno", "Свободно памяти на консоли" }, // FreeInternalStorage
    { "Resolution", "Auflösung", "Résolution", "Resolución", "Risoluzione", "Разрешение" }, // Resolution
    { "%d x %d", "%d x %d", "%d x %d", "%d x %d", "%d x %d", "%d x %d" }, // ResolutionFormat
    { "External controller", "Externer Controller", "Manette externe", "Controlador externo", "Controller esterno", "Внешний контроллер" }, // ExternalController
    { "Controller", "Controller", "Manette", "Controlador", "Controller", "Контроллер" }, // Controller
    { "Unknown controller", "Unbekannter Controller", "Manette inconnue", "Controlador desconocido", "Controller sconosciuto", "Неизвестный контроллер" }, // UnknownController
    { "No connected", "Nicht verbunden", "Aucune connectée", "Ninguno conectado", "Nessuno connesso", "Нет подключённых" }, // NoConnected
    { "Controller #%d", "Controller #%d", "Manette #%d", "Controlador #%d", "Controller #%d", "Контроллер #%d" }, // ControllerNumberFormat
    { "%.1f of %.1f GB", "%.1f von %.1f GB", "%.1f sur %.1f GB", "%.1f de %.1f GB", "%.1f di %.1f GB", "%.1f из %.1f GB" }, // StorageFormat

    // =========================================================================
    // Параметры оверлея
    // =========================================================================
    { "Hotkeys", "Hotkeys", "Raccourcis", "Atajos", "Scorciatoie", "Клавиши" }, // Hotkeys
    { "Language", "Sprache", "Langue", "Idioma", "Lingua", "Язык" }, // Language
    { "Active components", "Aktive Komponenten", "Composants actifs", "Componentes activos", "Componenti attivi", "Активные компоненты" }, // ActiveComponents
    { "Working", "Funktioniert", "Fonctionne", "Funciona", "Funziona", "Работает" }, // Working
    { "Not working", "Funktioniert nicht", "Ne fonctionne pas", "No funciona", "Non funziona", "Не работает" }, // NotWorking
    { "dmnt:cht", "dmnt:cht", "dmnt:cht", "dmnt:cht", "dmnt:cht", "dmnt:cht" }, // AtmosphereCheatsComponent
    { "Adjusts CPU, GPU and MEM frequencies", "Passt CPU-, GPU- und MEM-Frequenzen an", "Ajuste les fréquences CPU, GPU et MEM", "Ajusta las frecuencias de CPU, GPU y MEM", "Regola le frequenze di CPU, GPU e MEM", "Настраивает частоты CPU, GPU, MEM" }, // SysClkInlineDescription
    { "Cheat support through Atmosphère", "Cheat-Unterstützung über Atmosphère", "Support des cheats via Atmosphère", "Soporte de trucos mediante Atmosphère", "Supporto cheat tramite Atmosphère", "Поддержка чит-кодов через Atmosphère" }, // EdizonInlineDescription

    // =========================================================================
    // Электропитание
    // =========================================================================
    { "Info", "Info", "Info", "Info", "Info", "Информация" }, // Info
    { "Actions", "Aktionen", "Actions", "Acciones", "Azioni", "Действия" }, // Actions
    { "Reboot", "Neustart", "Redémarrer", "Reiniciar", "Riavvia", "Перезагрузка" }, // Reboot
    { "Shutdown", "Herunterfahren", "Éteindre", "Apagar", "Spegni", "Выключение" }, // Shutdown
    { "Power", "Strom", "Alimentation", "Energía", "Alimentazione", "Питание" }, // PowerShort
    { "Temp.", "Temp.", "Temp.", "Temp.", "Temp.", "Темп." }, // TempShort
    { "N", "N", "N", "N", "N", "N" }, // PowerNowPrefix
    { "A", "A", "A", "A", "A", "A" }, // PowerAvgPrefix

    // =========================================================================
    // Чит-коды
    // =========================================================================
    { "Active cheat codes", "Aktive Cheat-Codes", "Codes de triche actifs", "Códigos activos", "Cheat attivi", "Активные чит-коды" }, // ActiveCheatCodes
    { "Select cheat codes", "Cheat-Codes auswählen", "Choisir les codes de triche", "Seleccionar trucos", "Seleziona cheat", "Выбрать чит-коды" }, // SelectCheatCodes
    { "No active cheats", "Keine aktiven Cheats", "Aucun code actif", "No hay trucos activos", "Nessun cheat attivo", "Нет активных чит-кодов" }, // NoActiveCheats
    { "No cheats found", "Keine Cheats gefunden", "Aucun code trouvé", "No se encontraron trucos", "Nessun cheat trovato", "Чит-коды не найдены" }, // NoCheatsFound

    // =========================================================================
    // Производительность
    // =========================================================================
    { "Current type", "Aktueller Typ", "Type actuel", "Tipo actual", "Tipo corrente", "Текущий тип консоли" }, // CurrentType
    { "USB connected", "Per USB verbunden", "Connecté en USB", "Conectado por USB", "Collegato via USB", "Подключено по USB" }, // UsbConnected
    { "Handheld", "Handheld", "Portable", "Portátil", "Portatile", "Портативный" }, // Handheld
    { "Dock station", "Dockingstation", "Station d'accueil", "Base", "Dock station", "Док-станция" }, // Docked
    { "USB mode", "USB-Modus", "Mode USB", "Modo USB", "Modalità USB", "Подключение по USB" }, // Charging
    { "Default", "Standard", "Par défaut", "Predeterminado", "Predefinito", "По умолчанию" }, // Default
    { "Base", "Basis", "Base", "Base", "Base", "Базовые" }, // Base
    { "Medium", "Mittel", "Moyen", "Medio", "Medio", "Средние" }, // Medium
    { "Medium+", "Mittel+", "Moyen+", "Medio+", "Medio+", "Средние+" }, // MediumPlus
    { "High", "Hoch", "Élevé", "Alto", "Alto", "Высокие" }, // High
    { "Ultra", "Ultra", "Ultra", "Ultra", "Ultra", "Ультра" }, // Ultra
    { "Maximum", "Maximum", "Maximum", "Máximo", "Massimo", "Максимум" }, // Maximum
    { "Custom", "Benutzerdefiniert", "Personnalisé", "Personalizado", "Personalizzato", "Свои" }, // Custom
    { "System", "System", "Système", "Sistema", "Sistema", "Системные" }, // System
    { "Handheld mode", "Handheld-Modus", "Mode portable", "Modo portátil", "Modalità portatile", "Портативный режим" }, // HandheldMode
    { "Dock station", "Dockingstation", "Station d'accueil", "Base", "Dock station", "Док-станция" }, // DockMode
    { "USB mode", "USB-Modus", "Mode USB", "Modo USB", "Modalità USB", "Подключение по USB" }, // ChargingMode
    { "Profile settings", "Profileinstellungen", "Paramètres du profil", "Configuración del perfil", "Impostazioni profilo", "Настройка профиля для" }, // SettingsForPrefix
    { "Apply", "Anwenden", "Appliquer", "Aplicar", "Applica", "Применить" }, // Apply
    { "No application running.", "Keine Anwendung läuft.", "Aucune application en cours.", "No hay ninguna aplicación en ejecución.", "Nessuna applicazione in esecuzione.", "Приложение не запущено." }, // NoApplicationRunning
    { "sys-clk is not active or not installed.", "sys-clk ist nicht aktiv oder nicht installiert.", "sys-clk n'est pas actif ou n'est pas installé.", "sys-clk no está activo o no está instalado.", "sys-clk non è attivo o non è installato.", "sys-clk не активен или не установлен." }, // SysclkUnavailable
    { "Adjusts the frequencies of the console components.", "Passt die Frequenzen der Konsolenkomponenten an.", "Ajuste les fréquences des composants de la console.", "Ajusta las frecuencias de los componentes de la consola.", "Regola le frequenze dei componenti della console.", "Настройка частот компонентов консоли." }, // PerformanceHelpText
    { "CPU", "CPU", "CPU", "CPU", "CPU", "CPU" }, // Cpu
    { "GPU", "GPU", "GPU", "GPU", "GPU", "GPU" }, // Gpu
    { "MEM", "MEM", "MEM", "MEM", "MEM", "MEM" }, // Mem
    { "Clock profiles", "Taktprofile", "Profils d'horloge", "Perfiles de reloj", "Profili clock", "Профили частот" } // Profiles
};

static_assert(
    sizeof(g_localizedTextTable) / sizeof(g_localizedTextTable[0]) == static_cast<size_t>(TextId::Profiles) + 1,
    "Localization table size mismatch"
    );

static inline const char* tr(Lang lang, TextId id) {
    const LocalizedText& row = g_localizedTextTable[static_cast<size_t>(id)];
    switch (lang) {
    case Lang::English: return row.en;
    case Lang::German:  return row.de;
    case Lang::French:  return row.fr;
    case Lang::Spanish: return row.es;
    case Lang::Italian: return row.it;
    case Lang::Russian: return row.ru;
    default:            return row.en;
    }
}

static inline Lang getNextLang(Lang lang) {
    switch (lang) {
    case Lang::English: return Lang::German;
    case Lang::German:  return Lang::French;
    case Lang::French:  return Lang::Spanish;
    case Lang::Spanish: return Lang::Italian;
    case Lang::Italian: return Lang::Russian;
    case Lang::Russian: return Lang::English;
    default:            return Lang::English;
    }
}