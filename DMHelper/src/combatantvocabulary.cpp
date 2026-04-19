#include "combatantvocabulary.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QTextStream>
#include <QDebug>

namespace {

bool parseBool(const QString& raw, bool fallback = false)
{
    if(raw.isEmpty())
        return fallback;
    const QString v = raw.trimmed().toLower();
    return (v == "1" || v == "true" || v == "yes");
}

}

CombatantVocabulary::CombatantVocabulary() :
    _loaded(false)
{
}

void CombatantVocabulary::clear()
{
    _abilities.clear();
    _skills.clear();
    _sizes.clear();
    _formulas.clear();
    _sourceFile.clear();
    _loaded = false;
}

bool CombatantVocabulary::loadFromFile(const QString& path)
{
    clear();
    if(path.isEmpty())
        return false;

    QFile file(path);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[CombatantVocabulary] Unable to open vocabulary file: " << path << ", " << file.errorString();
        return false;
    }

    QDomDocument doc("DMHelperVocabulary");
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    const QDomDocument::ParseResult result = doc.setContent(in.readAll());
    file.close();

    if(!result)
    {
        qDebug() << "[CombatantVocabulary] Parse error in " << path << " at line " << result.errorLine << ", column " << result.errorColumn << ": " << result.errorMessage;
        return false;
    }

    const QDomElement root = doc.documentElement();
    if(root.isNull())
    {
        qDebug() << "[CombatantVocabulary] Vocabulary file has no root element: " << path;
        return false;
    }

    // Tolerate both "abilities" and "dmh:abilities" since the XML uses the
    // dmh: prefix without a declared namespace (convention, not enforcement).
    parseAbilities(root.firstChildElement(QString("abilities")));
    if(_abilities.isEmpty())
        parseAbilities(root.firstChildElement(QString("dmh:abilities")));

    parseSkills(root.firstChildElement(QString("skills")));
    if(_skills.isEmpty())
        parseSkills(root.firstChildElement(QString("dmh:skills")));

    parseSizes(root.firstChildElement(QString("sizes")));
    if(_sizes.isEmpty())
        parseSizes(root.firstChildElement(QString("dmh:sizes")));

    parseFormulas(root.firstChildElement(QString("formulas")));
    if(_formulas.isEmpty())
        parseFormulas(root.firstChildElement(QString("dmh:formulas")));

    _sourceFile = path;
    _loaded = true;
    return true;
}

void CombatantVocabulary::parseAbilities(const QDomElement& parent)
{
    if(parent.isNull())
        return;

    for(QDomElement el = parent.firstChildElement(); !el.isNull(); el = el.nextSiblingElement())
    {
        const QString tag = el.tagName();
        if(!tag.endsWith("ability", Qt::CaseInsensitive))
            continue;

        AbilityDef def;
        def.key = el.attribute("key");
        if(def.key.isEmpty())
            continue;
        def.displayName = el.attribute("display", def.key);
        def.shortName = el.attribute("short", def.displayName.left(3).toUpper());
        _abilities.append(def);
    }
}

void CombatantVocabulary::parseSkills(const QDomElement& parent)
{
    if(parent.isNull())
        return;

    for(QDomElement el = parent.firstChildElement(); !el.isNull(); el = el.nextSiblingElement())
    {
        const QString tag = el.tagName();
        if(!tag.endsWith("skill", Qt::CaseInsensitive))
            continue;

        SkillDef def;
        def.key = el.attribute("key");
        if(def.key.isEmpty())
            continue;
        def.displayName = el.attribute("display", def.key);
        def.ability = el.attribute("ability");
        def.save = parseBool(el.attribute("save"));
        _skills.append(def);
    }
}

void CombatantVocabulary::parseSizes(const QDomElement& parent)
{
    if(parent.isNull())
        return;

    for(QDomElement el = parent.firstChildElement(); !el.isNull(); el = el.nextSiblingElement())
    {
        const QString tag = el.tagName();
        if(!tag.endsWith("size", Qt::CaseInsensitive))
            continue;

        SizeDef def;
        def.key = el.attribute("key");
        if(def.key.isEmpty())
            continue;
        def.displayName = el.attribute("display", def.key);
        def.category = el.attribute("category", "0").toInt();
        bool ok = false;
        const qreal scale = el.attribute("scale", "1.0").toDouble(&ok);
        def.scaleFactor = ok ? scale : 1.0;
        _sizes.append(def);
    }
}

void CombatantVocabulary::parseFormulas(const QDomElement& parent)
{
    if(parent.isNull())
        return;

    for(QDomElement el = parent.firstChildElement(); !el.isNull(); el = el.nextSiblingElement())
    {
        const QString tag = el.tagName();
        if(!tag.endsWith("formula", Qt::CaseInsensitive))
            continue;

        const QString name = el.attribute("name");
        const QString expr = el.attribute("expression");
        if(name.isEmpty() || expr.isEmpty())
            continue;
        _formulas.insert(name, expr);
    }
}

void CombatantVocabulary::loadDefaults5e()
{
    clear();

    _abilities = {
        {"strength",     "Strength",     "STR"},
        {"dexterity",    "Dexterity",    "DEX"},
        {"constitution", "Constitution", "CON"},
        {"intelligence", "Intelligence", "INT"},
        {"wisdom",       "Wisdom",       "WIS"},
        {"charisma",     "Charisma",     "CHA"},
    };

    _skills = {
        {"strengthSave",     "Strength Save",     "strength",     true},
        {"athletics",        "Athletics",         "strength",     false},
        {"dexteritySave",    "Dexterity Save",    "dexterity",    true},
        {"stealth",          "Stealth",           "dexterity",    false},
        {"acrobatics",       "Acrobatics",        "dexterity",    false},
        {"sleightOfHand",    "Sleight of Hand",   "dexterity",    false},
        {"constitutionSave", "Constitution Save", "constitution", true},
        {"intelligenceSave", "Intelligence Save", "intelligence", true},
        {"investigation",    "Investigation",     "intelligence", false},
        {"arcana",           "Arcana",            "intelligence", false},
        {"nature",           "Nature",            "intelligence", false},
        {"history",          "History",           "intelligence", false},
        {"religion",         "Religion",          "intelligence", false},
        {"wisdomSave",       "Wisdom Save",       "wisdom",       true},
        {"medicine",         "Medicine",          "wisdom",       false},
        {"animalHandling",   "Animal Handling",   "wisdom",       false},
        {"perception",       "Perception",        "wisdom",       false},
        {"insight",          "Insight",           "wisdom",       false},
        {"survival",         "Survival",          "wisdom",       false},
        {"charismaSave",     "Charisma Save",     "charisma",     true},
        {"performance",      "Performance",       "charisma",     false},
        {"deception",        "Deception",         "charisma",     false},
        {"persuasion",       "Persuasion",        "charisma",     false},
        {"intimidation",     "Intimidation",      "charisma",     false},
    };

    _sizes = {
        {"tiny",       "Tiny",       0, 0.5},
        {"small",      "Small",      1, 0.75},
        {"medium",     "Medium",     2, 1.0},
        {"large",      "Large",      3, 2.0},
        {"huge",       "Huge",       4, 3.0},
        {"gargantuan", "Gargantuan", 5, 4.0},
        {"colossal",   "Colossal",   6, 8.0},
    };

    _formulas.insert("abilityMod", "(v-10)/2");

    _sourceFile = QStringLiteral("<builtin:5e>");
    _loaded = true;
}

int CombatantVocabulary::abilityIndex(const QString& key) const
{
    for(int i = 0; i < _abilities.size(); ++i)
    {
        if(_abilities.at(i).key == key)
            return i;
    }
    return -1;
}

int CombatantVocabulary::skillIndex(const QString& key) const
{
    for(int i = 0; i < _skills.size(); ++i)
    {
        if(_skills.at(i).key == key)
            return i;
    }
    return -1;
}

const CombatantVocabulary::SizeDef* CombatantVocabulary::sizeByKey(const QString& key) const
{
    for(const SizeDef& size : _sizes)
    {
        if(QString::compare(size.key, key, Qt::CaseInsensitive) == 0)
            return &size;
    }
    return nullptr;
}

const CombatantVocabulary::SizeDef* CombatantVocabulary::sizeByCategory(int category) const
{
    for(const SizeDef& size : _sizes)
    {
        if(size.category == category)
            return &size;
    }
    return nullptr;
}

QString CombatantVocabulary::derivedFormula(const QString& name, const QString& fallback) const
{
    return _formulas.value(name, fallback);
}
