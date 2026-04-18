#ifndef UNDOTOKENREMOVE_H
#define UNDOTOKENREMOVE_H

#include "undotokenbase.h"

class BattleDialogModelCombatant;
class BattleDialogModelEffect;

// Undo command for removing a combatant from a LayerTokens.
// The command takes ownership of the combatant while it is detached from the
// layer (i.e. after the first redo(), or before undo() re-attaches it).
class UndoTokenRemoveCombatant : public UndoTokenBase
{
public:
    UndoTokenRemoveCombatant(LayerTokens* layer, BattleDialogModelCombatant* combatant);
    virtual ~UndoTokenRemoveCombatant() override;

    virtual void undo() override;
    virtual void redo() override;

protected:
    BattleDialogModelCombatant* _combatant;
    bool _ownsCombatant;
};

// Undo command for removing an effect from a LayerTokens.
class UndoTokenRemoveEffect : public UndoTokenBase
{
public:
    UndoTokenRemoveEffect(LayerTokens* layer, BattleDialogModelEffect* effect);
    virtual ~UndoTokenRemoveEffect() override;

    virtual void undo() override;
    virtual void redo() override;

protected:
    BattleDialogModelEffect* _effect;
    bool _ownsEffect;
};

#endif // UNDOTOKENREMOVE_H
