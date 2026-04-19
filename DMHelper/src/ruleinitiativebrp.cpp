#include "ruleinitiativebrp.h"
#include "battledialogmodelcombatant.h"

QString RuleInitiativeBrp::InitiativeType = QString("brp");
QString RuleInitiativeBrp::InitiativeDescription = QString("BRP DEX-Rank Initiative");

RuleInitiativeBrp::RuleInitiativeBrp(QObject *parent) :
    RuleInitiative{parent}
{}

QString RuleInitiativeBrp::getInitiativeType()
{
    return RuleInitiativeBrp::InitiativeType;
}

bool RuleInitiativeBrp::compareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b)
{
    return RuleInitiativeBrp::CompareCombatants(a, b);
}

bool RuleInitiativeBrp::internalRollInitiative(QList<BattleDialogModelCombatant*>& combatants, bool previousResult)
{
    if((combatants.isEmpty()) || (!previousResult))
        return false;

    // BRP has no initiative roll: each combatant's initiative is their
    // DEX score. Assign it directly; no dialog needed.
    for(BattleDialogModelCombatant* combatant : combatants)
    {
        if(combatant)
            combatant->setInitiative(combatant->getDexterity());
    }
    return true;
}

void RuleInitiativeBrp::internalSortInitiative(QList<BattleDialogModelCombatant*>& combatants)
{
    std::sort(combatants.begin(), combatants.end(), CompareCombatants);
}

bool RuleInitiativeBrp::CompareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b)
{
    if((!a) || (!b))
        return false;

    // Higher initiative (= higher DEX after internalRollInitiative) acts
    // first. When initiative has not yet been rolled for one combatant,
    // fall back to raw DEX so the order remains stable.
    const int aInit = (a->getInitiative() > 0) ? a->getInitiative() : a->getDexterity();
    const int bInit = (b->getInitiative() > 0) ? b->getInitiative() : b->getDexterity();
    if(aInit == bInit)
        return a->getDexterity() > b->getDexterity();
    return aInit > bInit;
}
