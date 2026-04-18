#ifndef UNDOTOKENADD_H
#define UNDOTOKENADD_H

#include "undotokenbase.h"

class BattleDialogModelCombatant;
class BattleDialogModelEffect;

// Undo command for adding a combatant to a LayerTokens.
// The command takes ownership of the combatant while it is detached from the
// layer (i.e. after undo, or before the first redo).
class UndoTokenAddCombatant : public UndoTokenBase
{
public:
    UndoTokenAddCombatant(LayerTokens* layer, BattleDialogModelCombatant* combatant);
    virtual ~UndoTokenAddCombatant() override;

    virtual void undo() override;
    virtual void redo() override;

protected:
    BattleDialogModelCombatant* _combatant;
    bool _ownsCombatant;
};

// Undo command for adding an effect to a LayerTokens.
class UndoTokenAddEffect : public UndoTokenBase
{
public:
    UndoTokenAddEffect(LayerTokens* layer, BattleDialogModelEffect* effect);
    virtual ~UndoTokenAddEffect() override;

    virtual void undo() override;
    virtual void redo() override;

protected:
    BattleDialogModelEffect* _effect;
    bool _ownsEffect;
};

#endif // UNDOTOKENADD_H
