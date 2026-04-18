#include "undotokenadd.h"
#include "layertokens.h"
#include "battledialogmodelcombatant.h"
#include "battledialogmodeleffect.h"

UndoTokenAddCombatant::UndoTokenAddCombatant(LayerTokens* layer, BattleDialogModelCombatant* combatant) :
    UndoTokenBase(layer, QObject::tr("Add Combatant")),
    _combatant(combatant),
    _ownsCombatant(true) // Combatant is detached until first redo().
{
}

UndoTokenAddCombatant::~UndoTokenAddCombatant()
{
    if((_ownsCombatant) && (_combatant))
        delete _combatant;
}

void UndoTokenAddCombatant::undo()
{
    if((!_layer) || (!_combatant))
        return;

    _layer->removeCombatant(_combatant);
    _ownsCombatant = true;
}

void UndoTokenAddCombatant::redo()
{
    if((!_layer) || (!_combatant))
        return;

    _layer->addCombatant(_combatant);
    _ownsCombatant = false;
}


UndoTokenAddEffect::UndoTokenAddEffect(LayerTokens* layer, BattleDialogModelEffect* effect) :
    UndoTokenBase(layer, QObject::tr("Add Effect")),
    _effect(effect),
    _ownsEffect(true)
{
}

UndoTokenAddEffect::~UndoTokenAddEffect()
{
    if((_ownsEffect) && (_effect))
        delete _effect;
}

void UndoTokenAddEffect::undo()
{
    if((!_layer) || (!_effect))
        return;

    _layer->removeEffect(_effect);
    _ownsEffect = true;
}

void UndoTokenAddEffect::redo()
{
    if((!_layer) || (!_effect))
        return;

    _layer->addEffect(_effect);
    _ownsEffect = false;
}
