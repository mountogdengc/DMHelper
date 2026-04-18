#ifndef CAMPAIGNOBJECTFRAME_H
#define CAMPAIGNOBJECTFRAME_H

#include <QFrame>

class CampaignObjectBase;
class PublishGLRenderer;
class QAction;

class CampaignObjectFrame : public QFrame
{
    Q_OBJECT

public:
    CampaignObjectFrame(QWidget *parent = nullptr);
    virtual ~CampaignObjectFrame();

    virtual void activateObject(CampaignObjectBase* object, PublishGLRenderer* currentRenderer);
    virtual void deactivateObject();

    // Override to return an undo/redo action bound to this frame's content.
    // Default returns nullptr; MainWindow falls back to a disabled action.
    virtual QAction* getUndoAction(QObject* parent);
    virtual QAction* getRedoAction(QObject* parent);

signals:
    void setPublishEnabled(bool enabled, bool layered);
    void checkedChanged(bool checked);
    void checkableChanged(bool checkable);
    void rotationChanged(int rotation);
    void backgroundColorChanged(const QColor& color);

public slots:
    virtual void publishClicked(bool checked);
    virtual void setRotation(int rotation);
    virtual void setBackgroundColor(const QColor& color);
    virtual void editLayers();
};

#endif // CAMPAIGNOBJECTFRAME_H
