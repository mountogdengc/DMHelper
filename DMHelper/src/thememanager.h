#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QColor>
#include <QHash>
#include <QPalette>
#include <QString>

class QApplication;
class QWidget;

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    enum class ThemeMode
    {
        Light,
        Dark,
        System
    };

    enum class Role
    {
        WindowBackground,
        WindowText,
        DiceSuccess,
        DiceFailure,
        DicePrimaryText,
        CombatantActive,
        CombatantSelected,
        CombatantHover,
        CombatantInactive,
        OverlaySelected,
        OverlayBackground,
        LayerSelected,
        LayerInactive,
        SoundboardActive,
        PublishButtonActive,
        PublishButtonInactive,
        SpellbookDisabledText,
        SpellbookEnabledText,
        EffectPreviewBg
    };

    static ThemeManager& instance();

    ThemeMode mode() const;
    ThemeMode effectiveMode() const;
    bool isDark() const;

    QColor color(Role role) const;
    QString colorName(Role role) const;

    void setMode(ThemeMode m);
    void setModeFromString(const QString& s);
    static QString modeToString(ThemeMode m);
    static ThemeMode modeFromString(const QString& s);

    void applyTo(QApplication* app);
    void applyPlayerPaletteTo(QWidget* widget);

signals:
    void themeChanged(ThemeManager::ThemeMode effectiveMode);

private:
    explicit ThemeManager(QObject* parent = nullptr);
    Q_DISABLE_COPY(ThemeManager)

    void populateRoles();
    QPalette buildPalette(bool dark) const;
    QString loadStyleSheet(bool dark) const;
    ThemeMode detectSystemMode() const;
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    void onSystemColorSchemeChanged();
#endif

    ThemeMode _mode;
    QApplication* _app;
    QPalette _defaultPalette;
    QHash<int, QColor> _lightRoles;
    QHash<int, QColor> _darkRoles;
};

#endif // THEMEMANAGER_H
