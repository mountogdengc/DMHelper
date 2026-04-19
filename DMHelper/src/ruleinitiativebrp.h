#ifndef RULEINITIATIVEBRP_H
#define RULEINITIATIVEBRP_H

#include "ruleinitiative.h"

// Basic Roleplaying initiative: there is no initiative roll in BRP. A
// combatant acts on their DEX rank, with higher DEX going first. This
// implementation auto-assigns each combatant's initiative value to their
// DEX score at the start of a round and sorts by it, so the existing
// initiative-order UI in the rest of DMHelper renders sensibly. No
// dialog is presented to the user.
class RuleInitiativeBrp : public RuleInitiative
{
    Q_OBJECT
public:
    explicit RuleInitiativeBrp(QObject *parent = nullptr);

    static QString InitiativeType;
    static QString InitiativeDescription;

    virtual QString getInitiativeType() override;

    virtual bool compareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b) override;

protected:
    virtual bool internalRollInitiative(QList<BattleDialogModelCombatant*>& combatants, bool previousResult) override;
    virtual void internalSortInitiative(QList<BattleDialogModelCombatant*>& combatants) override;

private:
    static bool CompareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b);
};

#endif // RULEINITIATIVEBRP_H
