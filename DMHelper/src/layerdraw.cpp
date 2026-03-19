#include "layerdraw.h"
#include "layerscene.h"
#include "undodrawaddobject.h"
#include "layerdrawshape.h"
#include "publishglbattlebackground.h"
#include "publishglbattleobject.h"
#include "publishglscene.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QUndoStack>
#include <QOpenGLContext>
#include <cmath>

LayerDraw::LayerDraw(const QString& name, int order, QObject *parent) :
    Layer{name, order, parent},
    //_graphicsItem{nullptr},
    //_pathItem{nullptr},
    _glObjects{},
    _dirtyObjects{},
    _scene{nullptr},
    _playerInitialized{false},
    _layerDrawState{},
//    _drawPath{},
    _undoStack{new QUndoStack(this)}
{
    connect(&_layerDrawState, &LayerDrawState::objectAdded, this, &LayerDraw::handleObjectAdded);
    connect(&_layerDrawState, &LayerDrawState::objectRemoved, this, &LayerDraw::handleObjectRemoved);
}

LayerDraw::~LayerDraw()
{
    //if(_imagePainter)
    //    endPainting();

    cleanupDM();
    cleanupPlayer();
}

void LayerDraw::inputXML(const QDomElement &element, bool isImport)
{
    _layerDrawState.inputXML(element, isImport);

    Layer::inputXML(element, isImport);
}

QRectF LayerDraw::boundingRect() const
{
//    return _pathItem ? _pathItem->boundingRect() : QRectF();
    return QRectF();
}

QImage LayerDraw::getLayerIcon() const
{
    return QImage(":/img/data/icon_pendashdot.png");
}

DMHelper::LayerType LayerDraw::getType() const
{
    return DMHelper::LayerType_Draw;
}

bool LayerDraw::defaultShader() const
{
    return false;
}

Layer* LayerDraw::clone() const
{
    LayerDraw* newLayer = new LayerDraw(this->getName(), this->getOrder());
    this->copyBaseValues(newLayer);
    return newLayer;
}

void LayerDraw::applyOrder(int order)
{
//    if(_pathItem)
//        _pathItem->setZValue(order);
}

void LayerDraw::applyLayerVisibleDM(bool layerVisible)
{
//    if(_pathItem)
//        _pathItem->setVisible(layerVisible);
}

void LayerDraw::applyLayerVisiblePlayer(bool layerVisible)
{
    Q_UNUSED(layerVisible);
}

void LayerDraw::applyOpacity(qreal opacity)
{
    _opacityReference = opacity;

//    if(_pathItem)
//        _pathItem->setOpacity(opacity);
}

void LayerDraw::applyPosition(const QPoint& position)
{
//    if(_pathItem)
//        _pathItem->setPos(position);
}

void LayerDraw::applySize(const QSize& size)
{
    /*
    if(size == _imageLayer.size())
        return;

    if(!_imageLayer.isNull())
        uninitialize();

    _size = size;
    initialize(size);

    QImage newImage = getImage();

    if(_graphicsItem)
        _graphicsItem->setPixmap(QPixmap::fromImage(newImage));
*/
}

QImage LayerDraw::getImage() const
{
    return QImage();
    //return _imageLayer;
    /*
    if(_imageFowTexture.isNull())
        return _imageFow;

    QImage combinedImage = _imageFow;
    QPainter p(&combinedImage);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.drawImage(0, 0, _imageFowTexture);
    p.end();
    return combinedImage;
*/
}

LayerDrawState& LayerDraw::getDrawState()
{
    return _layerDrawState;
}

QGraphicsItem* LayerDraw::createGraphicsItem(LayerDrawObject* drawObject)
{
    if((!drawObject) || (!_layerScene))
        return nullptr;

    if(_graphicsItems.contains(drawObject))
        return _graphicsItems.value(drawObject);

    QGraphicsScene* scene = _layerScene->getDMScene();
    if(!scene)
        return nullptr;

    QGraphicsItem* graphicsItem = nullptr;
    switch(drawObject->getType())
    {
        case DMHelper::ActionType_Path:
        {
            LayerDrawObjectPath* pathObject = dynamic_cast<LayerDrawObjectPath*>(drawObject);
            if(!pathObject)
                return nullptr;

            QPainterPath painterPath;
            const QList<QPointF>& points = pathObject->getPoints();
            if(points.count() < 2)
                return nullptr;

            painterPath.moveTo(points.first());
            for(int i = 1; i < points.size(); ++i)
                painterPath.lineTo(points[i]);

            QPen pathPen(QBrush(pathObject->getPenColor()), pathObject->getPenWidth(), pathObject->getPenStyle());
            QGraphicsPathItem* pathItem = new LayerDrawShapePath(painterPath, pathObject);
            pathItem->setPen(pathPen);
            scene->addItem(pathItem);

            pathItem->setPos(pathObject->getPosition());
            pathItem->setFlag(QGraphicsItem::ItemIsMovable, true);
            pathItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            pathItem->setZValue(getOrder());

            graphicsItem = pathItem;
            break;
        }
        default:
            return nullptr;
    }

    if(graphicsItem)
        _graphicsItems.insert(drawObject, graphicsItem);

    return graphicsItem;
}

void LayerDraw::dmInitialize(QGraphicsScene* scene)
{
    if(!scene)
        return;

    // Add existing objects to the scene
    QList<LayerDrawObject*> drawObjects = _layerDrawState.getObjects();
    for(LayerDrawObject* object : std::as_const(drawObjects))
    {
        if((object) && (!_graphicsItems.contains(object)))
            createGraphicsItem(object);
    }

    /*
    if(_pathItem)
    {
        qDebug() << "[LayerDraw] ERROR: dmInitialize called although the path item already exists!";
        return;
    }

    _pathItem = scene->addPath(_drawPath, QPen(Qt::red, 5));
    if(_pathItem)
    {
        _pathItem->setPos(_position);
        _pathItem->setFlag(QGraphicsItem::ItemIsMovable, false);
        _pathItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
        _pathItem->setZValue(getOrder());
    }
*/

    Layer::dmInitialize(scene);
}

void LayerDraw::dmUninitialize()
{
    cleanupDM();
}

void LayerDraw::dmUpdate()
{
}

void LayerDraw::playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene)
{
    if(!scene)
        return;

    _scene = scene;

    QList<LayerDrawObject*> drawObjects = _layerDrawState.getObjects();
    for(LayerDrawObject* object : std::as_const(drawObjects))
    {
        if((object) && (!_glObjects.contains(object)))
            createGLObject(object);
    }

    _playerInitialized = true;
    Layer::playerGLInitialize(renderer, scene);
}

void LayerDraw::playerGLUninitialize()
{
    _playerInitialized = false;
    cleanupPlayer();
}

void LayerDraw::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    Q_UNUSED(defaultModelMatrix);

    if(!functions)
        return;

    DMH_DEBUG_OPENGL_PAINTGL();

    if(_shaderProgramRGBA == 0)
        return;

    // Re-upload textures for any dirty objects
    for(LayerDrawObject* dirtyObj : std::as_const(_dirtyObjects))
    {
        if(!dirtyObj)
            continue;

        // Remove old GL object and recreate
        PublishGLBattleBackground* oldGLObj = _glObjects.take(dirtyObj);
        delete oldGLObj;
        createGLObject(dirtyObj);
    }
    _dirtyObjects.clear();

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGBA);
    functions->glUseProgram(_shaderProgramRGBA);
    DMH_DEBUG_OPENGL_glUniformMatrix4fv4(_shaderProjectionMatrixRGBA, 1, GL_FALSE, projectionMatrix);
    functions->glUniformMatrix4fv(_shaderProjectionMatrixRGBA, 1, GL_FALSE, projectionMatrix);
    functions->glActiveTexture(GL_TEXTURE0);

    QList<LayerDrawObject*> drawObjects = _layerDrawState.getObjects();
    for(LayerDrawObject* object : std::as_const(drawObjects))
    {
        if(!object)
            continue;

        PublishGLBattleBackground* glObj = _glObjects.value(object);
        if(!glObj)
            continue;

        QMatrix4x4 localMatrix = glObj->getMatrix();
        localMatrix.translate(_position.x(), _position.y());
        DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, localMatrix.constData(), localMatrix);
        functions->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, localMatrix.constData());
        DMH_DEBUG_OPENGL_glUniform1f(_shaderAlphaRGBA, _opacityReference);
        functions->glUniform1f(_shaderAlphaRGBA, _opacityReference);

        glObj->paintGL(functions, projectionMatrix);
    }

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGB);
    functions->glUseProgram(_shaderProgramRGB);
}

void LayerDraw::playerGLResize(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}

bool LayerDraw::playerIsInitialized()
{
    return _playerInitialized;
}

void LayerDraw::initialize(const QSize& sceneSize)
{
    //if(!_imageLayer.isNull())
    //    return;

    if(getSize().isEmpty())
        setSize(sceneSize);

    //_imageLayer = QImage(getSize(), QImage::Format_ARGB32_Premultiplied);
    //_imageLayer.fill(Qt::transparent);

    //initializeUndoStack();
}

void LayerDraw::uninitialize()
{
    //_imageLayer = QImage();
}
/*
QPainterPath* LayerDraw::beginPainting()
{
    return &_drawPath;
}

void LayerDraw::endPainting()
{
}
*/

void LayerDraw::addObject(LayerDrawObject* drawObject)
{
    if(!drawObject)
        return;

    _undoStack->push(new UndoDrawAddObject(&_layerDrawState, drawObject));
}

void LayerDraw::removeObject(LayerDrawObject* drawObject)
{
    if(!drawObject)
        return;

    _graphicsItems.remove(drawObject);
    _dirtyObjects.remove(drawObject);
    delete _glObjects.take(drawObject);
}

void LayerDraw::handleObjectAdded(LayerDrawObject* object, int index)
{
    Q_UNUSED(index);

    if(!object)
        return;

    if(!_graphicsItems.contains(object))
        createGraphicsItem(object);

    if((_playerInitialized) && (!_glObjects.contains(object)))
        createGLObject(object);

    connect(object, &LayerDrawObject::objectMoved, this, &LayerDraw::handleObjectMoved);
}

void LayerDraw::handleObjectRemoved(LayerDrawObject* object, int index)
{
    Q_UNUSED(index);

    disconnect(object, &LayerDrawObject::objectMoved, this, &LayerDraw::handleObjectMoved);

    _dirtyObjects.remove(object);
    delete _glObjects.take(object);
}

void LayerDraw::handleObjectMoved(LayerDrawObject* object)
{
    if((!object) || (!_scene))
        return;

    PublishGLBattleBackground* glObj = _glObjects.value(object);
    if(!glObj)
        return;

    // Recompute the world position for the moved object
    QRectF sceneBounds;
    renderObjectToImage(object, sceneBounds); // only need bounds, discard image
    QPointF worldPos = PublishGLBattleObject::sceneToWorld(_scene->getSceneRect(),
                                                          sceneBounds.topLeft() + object->getPosition());
    glObj->setPosition(QPoint(static_cast<int>(worldPos.x()), static_cast<int>(worldPos.y())));
}

void LayerDraw::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    _layerDrawState.outputXML(doc, element, targetDirectory, isExport);

    Layer::internalOutputXML(doc, element, targetDirectory, isExport);
}

void LayerDraw::cleanupDM()
{
    QList<QGraphicsItem*> items = _graphicsItems.values();
    for(QGraphicsItem* item : std::as_const(items))
    {
        if((item) && (item->scene()))
            item->scene()->removeItem(item);

        delete item;
    }
    _graphicsItems.clear();
}

void LayerDraw::cleanupPlayer()
{
    qDeleteAll(_glObjects);
    _glObjects.clear();
    _dirtyObjects.clear();

    _scene = nullptr;
}

PublishGLBattleBackground* LayerDraw::createGLObject(LayerDrawObject* drawObject)
{
    if((!drawObject) || (!_scene))
        return nullptr;

    QRectF sceneBounds;
    QImage image = renderObjectToImage(drawObject, sceneBounds);
    if(image.isNull())
        return nullptr;

    PublishGLBattleBackground* glObj = new PublishGLBattleBackground(_scene, image, GL_LINEAR);

    QPointF worldPos = PublishGLBattleObject::sceneToWorld(_scene->getSceneRect(),
                                                          sceneBounds.topLeft() + drawObject->getPosition());
    glObj->setPosition(QPoint(static_cast<int>(worldPos.x()), static_cast<int>(worldPos.y())));

    _glObjects.insert(drawObject, glObj);
    return glObj;
}

QImage LayerDraw::renderObjectToImage(LayerDrawObject* drawObject, QRectF& sceneBounds)
{
    if(!drawObject)
        return QImage();

    switch(drawObject->getType())
    {
        case DMHelper::ActionType_Path:
        {
            LayerDrawObjectPath* pathObject = dynamic_cast<LayerDrawObjectPath*>(drawObject);
            if(!pathObject)
                return QImage();

            const QList<QPointF>& points = pathObject->getPoints();
            if(points.count() < 2)
                return QImage();

            QPainterPath painterPath;
            painterPath.moveTo(points.first());
            for(int i = 1; i < points.size(); ++i)
                painterPath.lineTo(points[i]);

            qreal penHalf = pathObject->getPenWidth() / 2.0 + 1.0;
            QRectF pathRect = painterPath.boundingRect();
            sceneBounds = pathRect.adjusted(-penHalf, -penHalf, penHalf, penHalf);

            int imgW = qMax(1, static_cast<int>(std::ceil(sceneBounds.width())));
            int imgH = qMax(1, static_cast<int>(std::ceil(sceneBounds.height())));
            QImage image(imgW, imgH, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QPainter painter(&image);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.translate(-sceneBounds.topLeft());
            QPen pathPen(QBrush(pathObject->getPenColor()), pathObject->getPenWidth(), pathObject->getPenStyle());
            painter.setPen(pathPen);
            painter.drawPath(painterPath);
            painter.end();

            return image;
        }
        default:
            return QImage();
    }
}
