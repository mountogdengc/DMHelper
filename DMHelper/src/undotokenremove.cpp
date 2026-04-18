#include "undotokenremove.h"
#include "layertokens.h"
#include "battledialogmodelcombatant.h"
#include "battledialogmodeleffect.h"

UndoTokenRemoveCombatant::UndoTokenRemoveCombatant(LayerTokens* layer, BattleDialogModelCombatant* combatant) :
    UndoTokenBase(layer, QObject::tr("Remove Combatant")),
    _combatant(combatant),
    _ownsCombatant(false) // Combatant is attached to the layer until first redo().
{
}

UndoTokenRemoveCombatant::~UndoTokenRemoveCombatant()
{
    if((_ownsCombatant) && (_combatant))
        delete _combatant;
}

void UndoTokenRemoveCombatant::undo()
{
    if((!_layer) || (!_combatant))
        return;

    _layer->addCombatant(_combatant);
    _ownsCombatant = false;
}

void UndoTokenRemoveCombatant::redo()
{
    if((!_layer) || (!_combatant))
        return;

    _layer->removeCombatant(_combatant);
    _ownsCombatant = true;
}


UndoTokenRemoveEffect::UndoTokenRemoveEffect(LayerTokens* layer, BattleDialogModelEffect* effect) :
    UndoTokenBase(layer, QObject::tr("Remove Effect")),
    _effect(effect),
    _ownsEffect(false)
{
}

UndoTokenRemoveEffect::~UndoTokenRemoveEffect()
{
    if((_ownsEffect) && (_effect))
        delete _effect;
}

void UndoTokenRemoveEffect::undo()
{
    if((!_layer) || (!_effect))
        return;

    _layer->addEffect(_effect);
    _ownsEffect = false;
}

void UndoTokenRemoveEffect::redo()
{
    if((!_layer) || (!_effect))
        return;

    _layer->removeEffect(_effect);
    _ownsEffect = true;
}
