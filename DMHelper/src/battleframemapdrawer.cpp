#include "battleframemapdrawer.h"
#include "undofowpath.h"
#include "undofowfill.h"
#include "undofowshape.h"
#include "map.h"
#include "layerfow.h"
#include "layerwalls.h"
#include "layerscene.h"
#include <QPixmap>
#include <QPainter>
#include <QMessageBox>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QLineF>
#include <QPen>

BattleFrameMapDrawer::BattleFrameMapDrawer(QObject *parent) :
    QObject(parent),
    _mouseDown(false),
    _mouseDownPos(),
    _undoPath(nullptr),
    //_map(nullptr),
    _scene(nullptr),
    //_glFow(nullptr),
    _cursor(),
    _gridScale(10),
    _zoomScale(1.f),
    _size(10),
    _erase(true),
    _smooth(true),
    _brushMode(DMHelper::BrushType_Circle),
    _drawMode(DrawMode_Fow),
    _wallDragActive(false),
    _wallStart(),
    _wallPreview(nullptr),
    _wallPreviewScene(nullptr)
{
    createCursor();
}

/*
void BattleFrameMapDrawer::setMap(Map* map, QPixmap* fow, QImage* glFow)
{
    _map = map;
    _fow = fow;
    _glFow = glFow;

    if((_map) && (_fow) && (_glFow))
    {
        emit fowEdited(*_fow);
        emit fowChanged(*_glFow);
    }
}

Map* BattleFrameMapDrawer::getMap() const
{
    return _map;
}
*/

void BattleFrameMapDrawer::setScene(LayerScene* scene)
{
    _scene = scene;
}

LayerScene* BattleFrameMapDrawer::getScene() const
{
    return _scene;
}

void BattleFrameMapDrawer::setPreviewScene(QGraphicsScene* scene)
{
    _wallPreviewScene = scene;
}

const QCursor& BattleFrameMapDrawer::getCursor() const
{
    return _cursor;
}

BattleFrameMapDrawer::DrawMode BattleFrameMapDrawer::getDrawMode() const
{
    return _drawMode;
}

void BattleFrameMapDrawer::setDrawMode(DrawMode mode)
{
    if(_drawMode == mode)
        return;

    cancelWallInProgress();
    _drawMode = mode;
}

void BattleFrameMapDrawer::cancelWallInProgress()
{
    _wallDragActive = false;
    clearWallPreview();
}

void BattleFrameMapDrawer::handleMouseDown(const QPointF& pos)
{
    if(!_scene)
        return;

    if(_drawMode == DrawMode_Walls)
        handleWallMouseDown(pos);
    else
        handleFowMouseDown(pos);
}

void BattleFrameMapDrawer::handleMouseMoved(const QPointF& pos)
{
    if(_drawMode == DrawMode_Walls)
        handleWallMouseMoved(pos);
    else
        handleFowMouseMoved(pos);
}

void BattleFrameMapDrawer::handleMouseUp(const QPointF& pos)
{
    if(_drawMode == DrawMode_Walls)
        handleWallMouseUp(pos);
    else
        handleFowMouseUp(pos);
}

void BattleFrameMapDrawer::handleFowMouseDown(const QPointF& pos)
{
    _mouseDownPos = pos;
    _mouseDown = true;

    // TODO: Layers
    // Math says divide by 10: radius of 5 to adjust scale to "one square"
    LayerFow* layer = dynamic_cast<LayerFow*>(_scene->getNearest(_scene->getSelectedLayer(), DMHelper::LayerType_Fow));
    if(layer)
    {
        _undoPath = new UndoFowPath(layer, MapDrawPath(_gridScale * _size / 10, _brushMode, _erase, _smooth, pos.toPoint() - layer->getPosition()));
        layer->getUndoStack()->push(_undoPath);
        emit dirty();
    }
}

void BattleFrameMapDrawer::handleFowMouseMoved(const QPointF& pos)
{
    if((!_undoPath) || (!_undoPath->getLayer()))
        return;

    _undoPath->addPoint(pos.toPoint() - _undoPath->getLayer()->getPosition());
    emit dirty();
}

void BattleFrameMapDrawer::handleFowMouseUp(const QPointF& pos)
{
    Q_UNUSED(pos);
    endPath();
    emit dirty();
}

void BattleFrameMapDrawer::handleWallMouseDown(const QPointF& pos)
{
    LayerWalls* wallsLayer = dynamic_cast<LayerWalls*>(
        _scene->getNearest(_scene->getSelectedLayer(), DMHelper::LayerType_Walls));
    if(!wallsLayer)
        return;

    _wallDragActive = true;
    _wallStart = pos;

    // Rubber-band preview: create a translucent line on the same QGraphicsScene
    // used by the battle view. We look it up lazily through any existing
    // graphics item on the scene.
    clearWallPreview();
    QGraphicsScene* gs = _wallPreviewScene;
    if(gs)
    {
        QPen previewPen(wallsLayer->wallColor());
        previewPen.setStyle(Qt::DashLine);
        previewPen.setWidthF(wallsLayer->wallThickness());
        previewPen.setCosmetic(true);
        _wallPreview = gs->addLine(QLineF(pos, pos), previewPen);
        if(_wallPreview)
            _wallPreview->setZValue(1e6); // keep on top while dragging
    }
}

void BattleFrameMapDrawer::handleWallMouseMoved(const QPointF& pos)
{
    if(!_wallDragActive)
        return;

    if(_wallPreview)
        _wallPreview->setLine(QLineF(_wallStart, pos));
}

void BattleFrameMapDrawer::handleWallMouseUp(const QPointF& pos)
{
    if(!_wallDragActive)
        return;

    LayerWalls* wallsLayer = dynamic_cast<LayerWalls*>(
        _scene->getNearest(_scene->getSelectedLayer(), DMHelper::LayerType_Walls));
    if(!wallsLayer)
    {
        _wallDragActive = false;
        clearWallPreview();
        return;
    }

    QLineF segment(_wallStart - QPointF(wallsLayer->getPosition()),
                   pos - QPointF(wallsLayer->getPosition()));

    // Ignore zero-length or accidental sub-pixel drags (e.g. click without drag).
    constexpr qreal kMinWallLength = 3.0;
    if(segment.length() >= kMinWallLength)
    {
        wallsLayer->addWall(segment);
        emit dirty();
    }

    _wallDragActive = false;
    clearWallPreview();
}

void BattleFrameMapDrawer::clearWallPreview()
{
    if(_wallPreview)
    {
        if(_wallPreview->scene())
            _wallPreview->scene()->removeItem(_wallPreview);
        delete _wallPreview;
        _wallPreview = nullptr;
    }
    // Intentionally retain _wallPreviewScene — the scene outlives any single
    // drag and is what the next handleWallMouseDown needs to draw its preview.
}

void BattleFrameMapDrawer::drawRect(const QRect& rect)
{
    //if((!_map) || (!_fow) || (!_glFow))
    if(!_scene)
        return;

    // TODO: Layers
    LayerFow* layer = dynamic_cast<LayerFow*>(_scene->getNearest(_scene->getSelectedLayer(), DMHelper::LayerType_Fow));
    if(layer)
    {
        UndoFowShape* undoShape = new UndoFowShape(layer, MapEditShape(rect.translated(-layer->getPosition()), _erase, false));
        layer->getUndoStack()->push(undoShape);
        emit dirty();
    }
    /*
    // Changed to ignore smoothing on an area
    UndoFowShape* undoShape = new UndoFowShape(_map, MapEditShape(rect, _erase, false));
    _map->getUndoStack()->push(undoShape);
    //_map->paintFoWRect(rect, undoShape->mapEditShape(), _fow, true);
    //_map->paintFoWRect(rect, undoShape->mapEditShape(), _glFow, false);
    emit fowEdited(*_fow);
    emit fowChanged(*_glFow);
    endPath();
    */
    emit dirty();
}

void BattleFrameMapDrawer::setSize(int size)
{
    if(_size == size)
        return;

    _size = size;
    endPath();
    createCursor();
}

void BattleFrameMapDrawer::setScale(int gridScale, qreal zoomScale)
{
    if((_gridScale == gridScale) && (_zoomScale == zoomScale))
        return;

    _gridScale = gridScale;
    _zoomScale = zoomScale;
    endPath();
    createCursor();
}

void BattleFrameMapDrawer::fillFoW()
{
    if(_erase)
        clearFoW();
    else
        resetFoW();
}

void BattleFrameMapDrawer::resetFoW()
{
    //if((!_map) || (!_fow) || (!_glFow))
    if(!_scene)
        return;

    if(QMessageBox::question(nullptr, QString("Confirm Fill FoW"), QString("Are you sure you would like to fill the entire Fog of War?")) == QMessageBox::No)
        return;

    // TODO: Layers
    LayerFow* layer = dynamic_cast<LayerFow*>(_scene->getNearest(_scene->getSelectedLayer(), DMHelper::LayerType_Fow));
    if(layer)
    {
        UndoFowFill* undoFill = new UndoFowFill(layer, MapEditFill(QColor(0, 128, 0, 255)));
        layer->getUndoStack()->push(undoFill);
        emit dirty();
    }
    /*
    UndoFowFill* undoFill = new UndoFowFill(_map, MapEditFill(QColor(0, 0, 0, 255)));
    _map->getUndoStack()->push(undoFill);
    //_map->fillFoW(QColor(0, 0, 0, 128), _fow);
    //_map->fillFoW(QColor(0, 0, 0, 255), _glFow);
    endPath();
    emit fowEdited(*_fow);
    emit fowChanged(*_glFow);
    */
}

void BattleFrameMapDrawer::clearFoW()
{
    //if((!_map) || (!_fow) || (!_glFow))
    if(!_scene)
        return;

    if(QMessageBox::question(nullptr, QString("Confirm Clear FoW"), QString("Are you sure you would like to clear the entire Fog of War?")) == QMessageBox::No)
        return;

    // TODO: Layers
    LayerFow* layer = dynamic_cast<LayerFow*>(_scene->getNearest(_scene->getSelectedLayer(), DMHelper::LayerType_Fow));
    if(layer)
    {
        UndoFowFill* undoFill = new UndoFowFill(layer, MapEditFill(QColor(0, 128, 0, 0)));
        layer->getUndoStack()->push(undoFill);
        emit dirty();
    }
    /*
    UndoFowFill* undoFill = new UndoFowFill(_map, MapEditFill(QColor(0, 0, 0, 0)));
    _map->getUndoStack()->push(undoFill);
    //_map->fillFoW(QColor(0, 0, 0, 0), _fow);
    //_map->fillFoW(QColor(0, 0, 0, 0), _glFow);
    endPath();
    emit fowEdited(*_fow);
    emit fowChanged(*_glFow);
    */
}

void BattleFrameMapDrawer::setErase(bool erase)
{
    if(_erase == erase)
        return;

    _erase = erase;
}

void BattleFrameMapDrawer::setSmooth(bool smooth)
{
    if(_smooth == smooth)
        return;

    _smooth = smooth;
}

void BattleFrameMapDrawer::setBrushMode(int brushMode)
{
    if(_brushMode == brushMode)
        return;

    _brushMode = brushMode;
    endPath();
    createCursor();
}

void BattleFrameMapDrawer::endPath()
{
    _undoPath = nullptr;
    _mouseDown = false;
}

void BattleFrameMapDrawer::createCursor()
{
    //int cursorSize = _scale * _mapSource->getPartyScale() * _brushSize / 5;
    //int cursorSize = _viewWidth * _size / 5;
    int cursorSize = _gridScale * _zoomScale * _size / 5;
    QPixmap cursorPixmap(QSize(cursorSize, cursorSize));
    cursorPixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&cursorPixmap);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QBrush(Qt::black), 4));
        if(_brushMode == DMHelper::BrushType_Circle)
            painter.drawEllipse(0, 0, cursorSize, cursorSize);
        else
            painter.drawRect(0, 0, cursorSize, cursorSize);
    painter.end();

    _cursor = QCursor(cursorPixmap);
    emit cursorChanged(_cursor);
}
