#include "thememanager.h"

#include <QApplication>
#include <QFile>
#include <QGuiApplication>
#include <QPalette>
#include <QStyleHints>
#include <QWidget>

ThemeManager& ThemeManager::instance()
{
    static ThemeManager s_instance;
    return s_instance;
}

ThemeManager::ThemeManager(QObject* parent) :
    QObject(parent),
    _mode(ThemeMode::System),
    _app(nullptr),
    _defaultPalette(QGuiApplication::palette())
{
    populateRoles();

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if(QGuiApplication::styleHints())
        connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
                this, &ThemeManager::onSystemColorSchemeChanged);
#endif
}

ThemeManager::ThemeMode ThemeManager::mode() const
{
    return _mode;
}

ThemeManager::ThemeMode ThemeManager::effectiveMode() const
{
    if(_mode == ThemeMode::System)
        return detectSystemMode();
    return _mode;
}

bool ThemeManager::isDark() const
{
    return effectiveMode() == ThemeMode::Dark;
}

QColor ThemeManager::color(Role role) const
{
    const QHash<int, QColor>& roles = isDark() ? _darkRoles : _lightRoles;
    return roles.value(static_cast<int>(role));
}

QString ThemeManager::colorName(Role role) const
{
    return color(role).name();
}

void ThemeManager::setMode(ThemeMode m)
{
    if(_mode == m)
        return;

    _mode = m;
    if(_app)
        applyTo(_app);
    emit themeChanged(effectiveMode());
}

void ThemeManager::setModeFromString(const QString& s)
{
    setMode(modeFromString(s));
}

QString ThemeManager::modeToString(ThemeMode m)
{
    switch(m)
    {
    case ThemeMode::Light:  return QStringLiteral("light");
    case ThemeMode::Dark:   return QStringLiteral("dark");
    case ThemeMode::System: return QStringLiteral("system");
    }
    return QStringLiteral("system");
}

ThemeManager::ThemeMode ThemeManager::modeFromString(const QString& s)
{
    const QString v = s.toLower();
    if(v == QStringLiteral("light"))
        return ThemeMode::Light;
    if(v == QStringLiteral("dark"))
        return ThemeMode::Dark;
    return ThemeMode::System;
}

void ThemeManager::applyTo(QApplication* app)
{
    if(!app)
        return;

    _app = app;
    const bool dark = isDark();

    app->setPalette(buildPalette(dark));
    app->setStyleSheet(loadStyleSheet(dark));
}

void ThemeManager::applyPlayerPaletteTo(QWidget* widget)
{
    if(!widget)
        return;

    // Force the player-visible widget back to the Qt default palette captured
    // at construction so qApp->setPalette() for the DM's chosen theme never
    // leaks onto the projector/second screen. Paired with the object-name-
    // scoped stylesheet rules in light.qss/dark.qss, which only match the
    // DM main window.
    widget->setPalette(_defaultPalette);
}

void ThemeManager::populateRoles()
{
    auto L = [this](Role r, const QColor& c) { _lightRoles[static_cast<int>(r)] = c; };
    auto D = [this](Role r, const QColor& c) { _darkRoles[static_cast<int>(r)] = c; };

    // Light values match today's hardcoded on-screen colors so switching to
    // Light is a visual no-op relative to the pre-feature baseline.
    L(Role::WindowBackground,      QColor(240, 240, 240));
    L(Role::WindowText,            QColor(0, 0, 0));
    L(Role::DiceSuccess,           QColor("#00b000"));
    L(Role::DiceFailure,           QColor("#ff0000"));
    L(Role::DicePrimaryText,       QColor("#000000"));
    L(Role::CombatantActive,       QColor(115, 18, 0));
    L(Role::CombatantSelected,     QColor(196, 196, 196));
    L(Role::CombatantHover,        QColor(64, 64, 64));
    L(Role::CombatantInactive,     QColor(0, 0, 0, 0));
    L(Role::OverlaySelected,       QColor(64, 64, 64));
    L(Role::OverlayBackground,     QColor(0, 0, 0, 0));
    L(Role::LayerSelected,         QColor(64, 64, 64));
    L(Role::LayerInactive,         QColor(0, 0, 0, 0));
    L(Role::SoundboardActive,      QColor(115, 18, 0));
    L(Role::PublishButtonActive,   QColor("#ff0000"));
    L(Role::PublishButtonInactive, QColor("#000000"));
    L(Role::SpellbookDisabledText, QColor(196, 196, 196));
    L(Role::SpellbookEnabledText,  QColor(0, 0, 0));
    L(Role::EffectPreviewBg,       QColor(242, 242, 242));

    D(Role::WindowBackground,      QColor(45, 45, 45));
    D(Role::WindowText,            QColor(230, 230, 230));
    D(Role::DiceSuccess,           QColor("#5aff5a"));
    D(Role::DiceFailure,           QColor("#ff6060"));
    D(Role::DicePrimaryText,       QColor("#e0e0e0"));
    D(Role::CombatantActive,       QColor(170, 45, 20));
    D(Role::CombatantSelected,     QColor(90, 90, 90));
    D(Role::CombatantHover,        QColor(70, 70, 70));
    D(Role::CombatantInactive,     QColor(0, 0, 0, 0));
    D(Role::OverlaySelected,       QColor(90, 90, 90));
    D(Role::OverlayBackground,     QColor(0, 0, 0, 0));
    D(Role::LayerSelected,         QColor(90, 90, 90));
    D(Role::LayerInactive,         QColor(0, 0, 0, 0));
    D(Role::SoundboardActive,      QColor(170, 45, 20));
    D(Role::PublishButtonActive,   QColor("#ff6060"));
    D(Role::PublishButtonInactive, QColor("#e0e0e0"));
    D(Role::SpellbookDisabledText, QColor(120, 120, 120));
    D(Role::SpellbookEnabledText,  QColor(230, 230, 230));
    D(Role::EffectPreviewBg,       QColor(60, 60, 60));
}

QPalette ThemeManager::buildPalette(bool dark) const
{
    QPalette p;
    if(!dark)
    {
        // Leave Light as Qt default style palette so the existing parchment-
        // themed dialogs and widgets render exactly as they do today.
        return p;
    }

    const QColor window(45, 45, 45);
    const QColor base(30, 30, 30);
    const QColor alt(40, 40, 40);
    const QColor text(230, 230, 230);
    const QColor disabled(130, 130, 130);
    const QColor highlight(45, 108, 170);

    p.setColor(QPalette::Window, window);
    p.setColor(QPalette::WindowText, text);
    p.setColor(QPalette::Base, base);
    p.setColor(QPalette::AlternateBase, alt);
    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::Button, window);
    p.setColor(QPalette::ButtonText, text);
    p.setColor(QPalette::BrightText, Qt::red);
    p.setColor(QPalette::Highlight, highlight);
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::ToolTipBase, base);
    p.setColor(QPalette::ToolTipText, text);
    p.setColor(QPalette::PlaceholderText, disabled);
    p.setColor(QPalette::Link, QColor(100, 160, 220));
    p.setColor(QPalette::LinkVisited, QColor(170, 120, 210));

    p.setColor(QPalette::Disabled, QPalette::Text, disabled);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
    p.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
    p.setColor(QPalette::Disabled, QPalette::Highlight, QColor(70, 70, 70));

    return p;
}

QString ThemeManager::loadStyleSheet(bool dark) const
{
    const QString path = dark ? QStringLiteral(":/styles/dark.qss")
                              : QStringLiteral(":/styles/light.qss");
    QFile f(path);
    if(!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    return QString::fromUtf8(f.readAll());
}

ThemeManager::ThemeMode ThemeManager::detectSystemMode() const
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if(QGuiApplication::styleHints())
    {
        const Qt::ColorScheme scheme = QGuiApplication::styleHints()->colorScheme();
        if(scheme == Qt::ColorScheme::Dark)
            return ThemeMode::Dark;
        if(scheme == Qt::ColorScheme::Light)
            return ThemeMode::Light;
    }
#endif
    // Fallback: inspect the default palette's window color.
    const QColor win = QGuiApplication::palette().color(QPalette::Window);
    return (win.lightness() < 128) ? ThemeMode::Dark : ThemeMode::Light;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
void ThemeManager::onSystemColorSchemeChanged()
{
    if(_mode != ThemeMode::System)
        return;

    if(_app)
        applyTo(_app);
    emit themeChanged(effectiveMode());
}
#endif
