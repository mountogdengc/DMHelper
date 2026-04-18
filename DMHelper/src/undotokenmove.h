#ifndef UNDOTOKENMOVE_H
#define UNDOTOKENMOVE_H

#include "undotokenbase.h"
#include <QPointF>

class BattleDialogModelObject;

class UndoTokenMove : public UndoTokenBase
{
public:
    UndoTokenMove(LayerTokens* layer, BattleDialogModelObject* object,
                  const QPointF& oldPosition, const QPointF& newPosition);

    virtual void undo() override;
    virtual void redo() override;

    virtual int id() const override;
    virtual bool mergeWith(const QUndoCommand* other) override;

protected:
    BattleDialogModelObject* _object;
    QPointF _oldPosition;
    QPointF _newPosition;
    bool _firstRedo;
};

#endif // UNDOTOKENMOVE_H
