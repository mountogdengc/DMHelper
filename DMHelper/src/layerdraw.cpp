#include "layerdraw.h"
#include "layerscene.h"
#include "undodrawaddobject.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QUndoStack>

LayerDraw::LayerDraw(const QString& name, int order, QObject *parent) :
    Layer{name, order, parent},
    //_graphicsItem{nullptr},
    //_pathItem{nullptr},
    _drawGLObject{nullptr},
    _scene{nullptr},
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
            QGraphicsPathItem* pathItem = scene->addPath(painterPath, pathPen);

            pathItem->setPos(getPosition());
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
    _scene = scene;
    Layer::playerGLInitialize(renderer, scene);

/*
    if((_timerId == 0) && (renderer))
    {
        connect(this, &LayerEffect::update, renderer, &PublishGLRenderer::updateWidget);
        _timerId = startTimer(30);
    }
*/
}

void LayerDraw::playerGLUninitialize()
{
    cleanupPlayer();
}

void LayerDraw::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    Q_UNUSED(defaultModelMatrix);

    if(!functions)
        return;

    DMH_DEBUG_OPENGL_PAINTGL();

    if(_shaderProgramRGBA == 0)
        createShaders();

    //if(_VAO == 0)
     //   createObjects();

    /*
    if((_shaderProgramRGBA == 0) || (_shaderModelMatrixRGBA == -1) || (_shaderAlphaRGBA == -1))
    {
        qDebug() << "[LayerImage] ERROR: invalid shaders set! _shaderProgramRGBA: " << _shaderProgramRGBA << ", _shaderProjectionMatrixRGBA: " << _shaderProjectionMatrixRGBA << ", _shaderModelMatrixRGBA: " << _shaderModelMatrixRGBA << ", _shaderAlphaRGBA: " << _shaderAlphaRGBA;
        return;
    }

    DMH_DEBUG_OPENGL_PAINTGL();

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGBA);
    functions->glUseProgram(_shaderProgramRGBA);
    DMH_DEBUG_OPENGL_glUniformMatrix4fv4(_shaderProjectionMatrixRGBA, 1, GL_FALSE, projectionMatrix);
    functions->glUniformMatrix4fv(_shaderProjectionMatrixRGBA, 1, GL_FALSE, projectionMatrix);
    functions->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, _imageGLObject->getMatrixData(), _imageGLObject->getMatrix());
    functions->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, _imageGLObject->getMatrixData());
    DMH_DEBUG_OPENGL_glUniform1f(_shaderAlphaRGBA, _opacityReference);
    functions->glUniform1f(_shaderAlphaRGBA, _opacityReference);

    _imageGLObject->paintGL(functions, projectionMatrix);
    */

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGB);
    functions->glUseProgram(_shaderProgramRGB);
}

void LayerDraw::playerGLResize(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);

    destroyObjects();
    destroyShaders();
}

void LayerDraw::playerSetShaders(unsigned int programRGB, int modelMatrixRGB, int projectionMatrixRGB, unsigned int programRGBA, int modelMatrixRGBA, int projectionMatrixRGBA, int alphaRGBA)
{
    Q_UNUSED(programRGB);
    Q_UNUSED(modelMatrixRGB);
    Q_UNUSED(projectionMatrixRGB);
    Q_UNUSED(programRGBA);
    Q_UNUSED(modelMatrixRGBA);
    Q_UNUSED(projectionMatrixRGBA);
    Q_UNUSED(alphaRGBA);
}

bool LayerDraw::playerIsInitialized()
{
    return _scene != nullptr;
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
}

void LayerDraw::handleObjectAdded(LayerDrawObject* object, int index)
{
    Q_UNUSED(index);

    if(!object)
        return;

    if(!_graphicsItems.contains(object))
        createGraphicsItem(object);
}

void LayerDraw::handleObjectRemoved(LayerDrawObject* object, int index)
{
    Q_UNUSED(object);
    Q_UNUSED(index);
}

void LayerDraw::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    _layerDrawState.outputXML(doc, element, targetDirectory, isExport);

    Layer::internalOutputXML(doc, element, targetDirectory, isExport);
}

void LayerDraw::cleanupDM()
{
    /*
    if(!_pathItem)
        return;

    if(_pathItem->scene())
        _pathItem->scene()->removeItem(_pathItem);

    delete _pathItem;
    _pathItem = nullptr;
*/
}

void LayerDraw::cleanupPlayer()
{
    /*
    disconnect(this, &LayerEffect::update, nullptr, nullptr);

    if(_timerId > 0)
    {
        killTimer(_timerId);
        _timerId = 0;
    }
*/

    destroyObjects();
    destroyShaders();

    _scene = nullptr;
}

void LayerDraw::createShaders()
{
}

void LayerDraw::destroyShaders()
{
}

void LayerDraw::createObjects()
{
}

void LayerDraw::destroyObjects()
{
}
