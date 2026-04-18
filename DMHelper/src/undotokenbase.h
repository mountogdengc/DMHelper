#ifndef UNDOTOKENBASE_H
#define UNDOTOKENBASE_H

#include <QUndoCommand>

class LayerTokens;

class UndoTokenBase : public QUndoCommand
{
public:
    UndoTokenBase(LayerTokens* layer, const QString& text);
    virtual ~UndoTokenBase() override;

    LayerTokens* getLayer() const;

protected:
    LayerTokens* _layer;
};

#endif // UNDOTOKENBASE_H
