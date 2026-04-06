#include "battledialogmodelmonsterbase.h"
#include "conditions.h"
#include "monsterclassv2.h"
#include <QDomElement>

BattleDialogModelMonsterBase::BattleDialogModelMonsterBase(const QString& name, QObject *parent) :
    BattleDialogModelCombatant(name, parent),
    _legendaryCount(-1),
    _conditionList()
{
    connect(this, &BattleDialogModelMonsterBase::dataChanged, this, &BattleDialogModelMonsterBase::dirty);
    connect(this, &BattleDialogModelMonsterBase::imageChanged, this, &BattleDialogModelMonsterBase::dirty);
}

BattleDialogModelMonsterBase::BattleDialogModelMonsterBase(Combatant* combatant) :
    BattleDialogModelCombatant(combatant),
    _legendaryCount(-1),
    _conditionList()
{
    connect(this, &BattleDialogModelMonsterBase::dataChanged, this, &BattleDialogModelMonsterBase::dirty);
    connect(this, &BattleDialogModelMonsterBase::imageChanged, this, &BattleDialogModelMonsterBase::dirty);
}

BattleDialogModelMonsterBase::BattleDialogModelMonsterBase(Combatant* combatant, int initiative, const QPointF& position) :
    BattleDialogModelCombatant(combatant, initiative, position),
    _legendaryCount(-1),
    _conditionList()
{
    connect(this, &BattleDialogModelMonsterBase::dataChanged, this, &BattleDialogModelMonsterBase::dirty);
    connect(this, &BattleDialogModelMonsterBase::imageChanged, this, &BattleDialogModelMonsterBase::dirty);
}

BattleDialogModelMonsterBase::~BattleDialogModelMonsterBase()
{
}

void BattleDialogModelMonsterBase::inputXML(const QDomElement &element, bool isImport)
{
    BattleDialogModelCombatant::inputXML(element, isImport);

    _legendaryCount = element.attribute("legendaryCount", QString::number(-1)).toInt();

    // Condition migration: detect old int bitmask format vs new comma-separated string IDs
    QString condStr = element.attribute("conditions", QString());
    if(!condStr.isEmpty())
    {
        bool ok = false;
        int condInt = condStr.toInt(&ok);
        if(ok)
            _conditionList = Conditions::migrateFromBitmask(condInt);
        else
            _conditionList = condStr.split(QStringLiteral(","), Qt::SkipEmptyParts);
    }
}

void BattleDialogModelMonsterBase::copyValues(const CampaignObjectBase* other)
{
    const BattleDialogModelMonsterBase* otherMonsterBase = dynamic_cast<const BattleDialogModelMonsterBase*>(other);
    if(!otherMonsterBase)
        return;

    _legendaryCount = otherMonsterBase->_legendaryCount;
    _conditionList = otherMonsterBase->_conditionList;

    BattleDialogModelCombatant::copyValues(other);
}

int BattleDialogModelMonsterBase::getCombatantType() const
{
    return DMHelper::CombatantType_Monster;
}

int BattleDialogModelMonsterBase::getSkillModifier(Combatant::Skills skill) const
{
    MonsterClassv2* monsterClass = getMonsterClass();
    if(!monsterClass)
        return 0;

    // TODO: HACK
    switch(skill)
    {
        case Combatant::Skills_strengthSave:
            return monsterClass->getIntValue(QString("strengthSave"));
        case Combatant::Skills_athletics:
            return monsterClass->getIntValue(QString("athletics"));
        case Combatant::Skills_dexteritySave:
            return monsterClass->getIntValue(QString("dexteritySave"));
        case Combatant::Skills_stealth:
            return monsterClass->getIntValue(QString("stealth"));
        case Combatant::Skills_acrobatics:
            return monsterClass->getIntValue(QString("acrobatics"));
        case Combatant::Skills_sleightOfHand:
            return monsterClass->getIntValue(QString("sleightOfHand"));
        case Combatant::Skills_constitutionSave:
            return monsterClass->getIntValue(QString("constitutionSave"));
        case Combatant::Skills_intelligenceSave:
            return monsterClass->getIntValue(QString("intelligenceSave"));
        case Combatant::Skills_investigation:
            return monsterClass->getIntValue(QString("investigation"));
        case Combatant::Skills_arcana:
            return monsterClass->getIntValue(QString("arcana"));
        case Combatant::Skills_nature:
            return monsterClass->getIntValue(QString("nature"));
        case Combatant::Skills_history:
            return monsterClass->getIntValue(QString("history"));
        case Combatant::Skills_religion:
            return monsterClass->getIntValue(QString("religion"));
        case Combatant::Skills_wisdomSave:
            return monsterClass->getIntValue(QString("wisdomSave"));
        case Combatant::Skills_medicine:
            return monsterClass->getIntValue(QString("medicine"));
        case Combatant::Skills_animalHandling:
            return monsterClass->getIntValue(QString("animalHandling"));
        case Combatant::Skills_perception:
            return monsterClass->getIntValue(QString("perception"));
        case Combatant::Skills_insight:
            return monsterClass->getIntValue(QString("insight"));
        case Combatant::Skills_survival:
            return monsterClass->getIntValue(QString("survival"));
        case Combatant::Skills_charismaSave:
            return monsterClass->getIntValue(QString("charismaSave"));
        case Combatant::Skills_performance:
            return monsterClass->getIntValue(QString("performance"));
        case Combatant::Skills_deception:
            return monsterClass->getIntValue(QString("deception"));
        case Combatant::Skills_persuasion:
            return monsterClass->getIntValue(QString("persuasion"));
        case Combatant::Skills_intimidation:
            return monsterClass->getIntValue(QString("intimidation"));
        default:
            return 0;
    }
}

QStringList BattleDialogModelMonsterBase::getConditionList() const
{
    return _conditionList;
}

bool BattleDialogModelMonsterBase::hasConditionId(const QString& conditionId) const
{
    return _conditionList.contains(conditionId);
}

int BattleDialogModelMonsterBase::getLegendaryCount() const
{
    return _legendaryCount;
}

void BattleDialogModelMonsterBase::setConditionList(const QStringList& conditions)
{
    if(_conditionList != conditions)
    {
        _conditionList = conditions;
        emit dataChanged(this);
        emit conditionsChanged(this);
    }
}

void BattleDialogModelMonsterBase::addConditionId(const QString& conditionId)
{
    if(!conditionId.isEmpty() && !_conditionList.contains(conditionId))
    {
        _conditionList.append(conditionId);
        emit dataChanged(this);
        emit conditionsChanged(this);
    }
}

void BattleDialogModelMonsterBase::removeConditionId(const QString& conditionId)
{
    if(_conditionList.removeOne(conditionId))
    {
        emit dataChanged(this);
        emit conditionsChanged(this);
    }
}

void BattleDialogModelMonsterBase::clearConditions()
{
    if(!_conditionList.isEmpty())
    {
        _conditionList.clear();
        emit dataChanged(this);
        emit conditionsChanged(this);
    }
}

void BattleDialogModelMonsterBase::setLegendaryCount(int legendaryCount)
{
    if(_legendaryCount != legendaryCount)
    {
        _legendaryCount = legendaryCount;
        emit dataChanged(this);
    }
}

void BattleDialogModelMonsterBase::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("monsterType", getMonsterType());
    element.setAttribute("legendaryCount", _legendaryCount);
    element.setAttribute("conditions", _conditionList.join(QStringLiteral(",")));

    BattleDialogModelCombatant::internalOutputXML(doc, element, targetDirectory, isExport);
}
