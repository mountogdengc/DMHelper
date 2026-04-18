#include "undotokenmove.h"
#include "battledialogmodelobject.h"

namespace
{
    // Stable id for QUndoCommand merge. Only consecutive moves of the same
    // object (pushed in a single drag) collapse; see mergeWith().
    constexpr int UNDO_TOKEN_MOVE_ID = 0x746d6f76; // "tmov"
}

UndoTokenMove::UndoTokenMove(LayerTokens* layer, BattleDialogModelObject* object,
                             const QPointF& oldPosition, const QPointF& newPosition) :
    UndoTokenBase(layer, QObject::tr("Move Token")),
    _object(object),
    _oldPosition(oldPosition),
    _newPosition(newPosition),
    _firstRedo(true)
{
}

void UndoTokenMove::undo()
{
    if(_object)
        _object->setPosition(_oldPosition);
}

void UndoTokenMove::redo()
{
    // Skip the first redo: the position was already set by the drag itself.
    // We only want redo() to drive the position on subsequent re-applies.
    if(_firstRedo)
    {
        _firstRedo = false;
        return;
    }

    if(_object)
        _object->setPosition(_newPosition);
}

int UndoTokenMove::id() const
{
    return UNDO_TOKEN_MOVE_ID;
}

bool UndoTokenMove::mergeWith(const QUndoCommand* other)
{
    const UndoTokenMove* otherMove = dynamic_cast<const UndoTokenMove*>(other);
    if((!otherMove) || (otherMove->_object != _object))
        return false;

    _newPosition = otherMove->_newPosition;
    return true;
}
