#ifndef BATTLEFRAMEMAPDRAWER_H
#define BATTLEFRAMEMAPDRAWER_H

#include <QObject>
#include <QCursor>
#include <QPointF>

class UndoFowPath;
class LayerScene;
class QGraphicsLineItem;
class QGraphicsScene;

class BattleFrameMapDrawer : public QObject
{
    Q_OBJECT
public:
    enum DrawMode
    {
        DrawMode_Fow = 0,
        DrawMode_Walls
    };

    explicit BattleFrameMapDrawer(QObject *parent = nullptr);

    //void setMap(Map* map, QPixmap* fow, QImage* glFow);
    //Map* getMap() const;
    void setScene(LayerScene* scene);
    LayerScene* getScene() const;
    const QCursor& getCursor() const;

    // Optional: graphics scene used for the wall rubber-band preview.
    // If nullptr, wall drawing still works but with no live preview.
    void setPreviewScene(QGraphicsScene* scene);

    DrawMode getDrawMode() const;

signals:
    //void fowEdited(const QPixmap& fow);
    //void fowChanged(const QImage& glFow);
    void dirty();
    void cursorChanged(const QCursor& cursor);

public slots:
    void handleMouseDown(const QPointF& pos);
    void handleMouseMoved(const QPointF& pos);
    void handleMouseUp(const QPointF& pos);

    void drawRect(const QRect& rect);

    void setSize(int size);
    void setScale(int gridScale, qreal zoomScale);
    void fillFoW();
    void resetFoW();
    void clearFoW();
    void setErase(bool erase);
    void setSmooth(bool smooth);
    void setBrushMode(int brushMode);

    void setDrawMode(DrawMode mode);
    void cancelWallInProgress();

private:

    void endPath();
    void createCursor();

    // Wall-drawing helpers
    void handleFowMouseDown(const QPointF& pos);
    void handleFowMouseMoved(const QPointF& pos);
    void handleFowMouseUp(const QPointF& pos);
    void handleWallMouseDown(const QPointF& pos);
    void handleWallMouseMoved(const QPointF& pos);
    void handleWallMouseUp(const QPointF& pos);
    void clearWallPreview();

    bool _mouseDown;
    QPointF _mouseDownPos;
    UndoFowPath* _undoPath;
    //Map* _map;
    LayerScene* _scene;
    //QPixmap* _fow;
    //QImage* _glFow;
    QCursor _cursor;

    int _gridScale;
    qreal _zoomScale;
    int _size;
    bool _erase;
    bool _smooth;
    int _brushMode;

    DrawMode _drawMode;

    // Wall drag state
    bool _wallDragActive;
    QPointF _wallStart;
    QGraphicsLineItem* _wallPreview;
    QGraphicsScene* _wallPreviewScene;
};

#endif // BATTLEFRAMEMAPDRAWER_H
