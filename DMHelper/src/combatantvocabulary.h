#ifndef COMBATANTVOCABULARY_H
#define COMBATANTVOCABULARY_H

#include <QString>
#include <QList>
#include <QHash>

class QDomElement;

// Per-ruleset vocabulary describing the structural 5e-style enums that are
// still hardcoded in Combatant: abilities, skills, sizes, and a small bag of
// derived-value formulas. Conditions are NOT included here — v3.8.1 already
// has a dedicated Conditions class for that (see conditions.h).
//
// Loaded from an XML file referenced by the <ruleset vocabulary="..."/>
// attribute in ruleset.xml; when absent, loadDefaults5e() provides the
// built-in 5e vocabulary so existing campaigns continue to behave identically.
class CombatantVocabulary
{
public:
    struct AbilityDef
    {
        QString key;
        QString displayName;
        QString shortName;
    };

    struct SkillDef
    {
        QString key;
        QString displayName;
        QString ability;
        bool save = false;
    };

    struct SizeDef
    {
        QString key;
        QString displayName;
        int category = 0;
        qreal scaleFactor = 1.0;
    };

    CombatantVocabulary();

    bool loadFromFile(const QString& path);
    void loadDefaults5e();
    bool isLoaded() const { return _loaded; }
    QString sourceFile() const { return _sourceFile; }

    // Active instance singleton. Ruleset sets this during setValues/inputXML
    // so static helpers elsewhere (e.g. MonsterClassv2::convertSizeToCategory)
    // can consult the current ruleset's vocabulary without plumbing a Ruleset
    // pointer through. Matches the pattern established by Conditions::
    // activeConditions(). When null, callers fall back to built-in 5e.
    static const CombatantVocabulary* activeVocabulary();
    static void setActiveVocabulary(const CombatantVocabulary* vocabulary);

    const QList<AbilityDef>& abilities() const { return _abilities; }
    const QList<SkillDef>& skills() const { return _skills; }
    const QList<SizeDef>& sizes() const { return _sizes; }

    int abilityIndex(const QString& key) const;
    int skillIndex(const QString& key) const;
    const SizeDef* sizeByKey(const QString& key) const;
    const SizeDef* sizeByCategory(int category) const;

    QString derivedFormula(const QString& name, const QString& fallback = QString()) const;

private:
    void clear();
    void parseAbilities(const QDomElement& parent);
    void parseSkills(const QDomElement& parent);
    void parseSizes(const QDomElement& parent);
    void parseFormulas(const QDomElement& parent);

    QList<AbilityDef> _abilities;
    QList<SkillDef> _skills;
    QList<SizeDef> _sizes;
    QHash<QString, QString> _formulas;
    QString _sourceFile;
    bool _loaded;
};

#endif // COMBATANTVOCABULARY_H
