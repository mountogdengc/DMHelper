#include "undotokenbase.h"

UndoTokenBase::UndoTokenBase(LayerTokens* layer, const QString& text) :
    QUndoCommand(text),
    _layer(layer)
{
}

UndoTokenBase::~UndoTokenBase()
{
}

LayerTokens* UndoTokenBase::getLayer() const
{
    return _layer;
}
