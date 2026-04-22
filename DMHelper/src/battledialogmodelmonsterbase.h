#ifndef BATTLEDIALOGMODELMONSTERBASE_H
#define BATTLEDIALOGMODELMONSTERBASE_H

#include "battledialogmodelcombatant.h"
#include "combatant.h"
#include <QString>
#include <QStringList>

class MonsterClassv2;

class BattleDialogModelMonsterBase : public BattleDialogModelCombatant
{
    Q_OBJECT

public:
    enum
    {
        BattleMonsterType_Base = 0,
        BattleMonsterType_Combatant,
        BattleMonsterType_Class,
    };

    BattleDialogModelMonsterBase(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelMonsterBase(Combatant* combatant);
    explicit BattleDialogModelMonsterBase(Combatant* combatant, int initiative, const QPointF& position);
    virtual ~BattleDialogModelMonsterBase() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;

    virtual int getCombatantType() const override;
    virtual int getMonsterType() const = 0;
    virtual MonsterClassv2* getMonsterClass() const = 0;

    virtual void setMonsterName(const QString& monsterName) = 0;

    virtual int getSkillModifier(Combatant::Skills skill) const override;
    virtual QStringList getConditionList() const override;
    virtual bool hasConditionId(const QString& conditionId) const override;

    virtual int getLegendaryCount() const;

signals:
    void dataChanged(BattleDialogModelMonsterBase* monsterBase);
    void imageChanged(BattleDialogModelMonsterBase* monsterBase);

public slots:
    virtual void setConditionList(const QStringList& conditions) override;
    virtual void addConditionId(const QString& conditionId) override;
    virtual void removeConditionId(const QString& conditionId) override;
    virtual void clearConditions() override;
    virtual void setLegendaryCount(int legendaryCount);

protected:
    // From BattleDialogModelCombatant
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    int _legendaryCount;
    QStringList _conditionList;
};

#endif // BATTLEDIALOGMODELMONSTERBASE_H
