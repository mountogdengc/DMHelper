#include "layerdraw.h"
#include <QPainter>

LayerDraw::LayerDraw(const QString& name, int order, QObject *parent) :
    Layer{name, order, parent},
    _graphicsItem{nullptr},
    _drawGLObject{nullptr},
    _scene{nullptr},
    _imageLayer{},
    _imagePainter{nullptr}
{}

LayerDraw::~LayerDraw()
{
    if(_imagePainter)
        endPainting();

    cleanupDM();
    cleanupPlayer();
}

QRectF LayerDraw::boundingRect() const
{
    return _imageLayer.isNull() ? QRectF() : QRectF(_position, _imageLayer.size());
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
    Q_UNUSED(order);
}

void LayerDraw::applyLayerVisibleDM(bool layerVisible)
{
    Q_UNUSED(layerVisible);
}

void LayerDraw::applyLayerVisiblePlayer(bool layerVisible)
{
    Q_UNUSED(layerVisible);
}

void LayerDraw::applyOpacity(qreal opacity)
{
    Q_UNUSED(opacity);
}

void LayerDraw::applyPosition(const QPoint& position)
{
    Q_UNUSED(position);
}

void LayerDraw::applySize(const QSize& size)
{
    Q_UNUSED(size);
}

QImage LayerDraw::getImage() const
{
    return _imageLayer;
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

void LayerDraw::dmInitialize(QGraphicsScene* scene)
{
    if(!scene)
        return;

    /*
    if(_graphicsItem)
    {
        qDebug() << "[LayerEffect] ERROR: dmInitialize called although the graphics item already exists!";
        return;
    }

    QImage effectImage = LayerEffectSettings::createNoiseImage(QSize(LAYEREFFECT_PREVIEWSIZE, LAYEREFFECT_PREVIEWSIZE),
                                                               static_cast<qreal>(_effectWidth) / 10.f,
                                                               static_cast<qreal>(_effectHeight) / 10.f,
                                                               _effectColor,
                                                               static_cast<qreal>(_effectThickness) / 100.f);
    _graphicsItem = scene->addPixmap(QPixmap::fromImage(effectImage.scaled(_size)));

    if(_graphicsItem)
    {
        _graphicsItem->setPos(_position);
        _graphicsItem->setFlag(QGraphicsItem::ItemIsMovable, false);
        _graphicsItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
        _graphicsItem->setZValue(getOrder());
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
    if(!_imageLayer.isNull())
        return;

    if(getSize().isEmpty())
        setSize(sceneSize);

    _imageLayer = QImage(getSize(), QImage::Format_ARGB32_Premultiplied);
    _imageLayer.fill(Qt::transparent);

    //initializeUndoStack();
}

void LayerDraw::uninitialize()
{
    _imageLayer = QImage();
}

QPainter* LayerDraw::beginPainting()
{
    if(_imagePainter)
        return nullptr;

    _imagePainter = new QPainter(&_imageLayer);
    return _imagePainter;
}

void LayerDraw::endPainting()
{
    if(!_imagePainter)
        return;

    _imagePainter->end();
    delete _imagePainter;
    _imagePainter = nullptr;
}

void LayerDraw::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    Layer::internalOutputXML(doc, element, targetDirectory, isExport);
}

void LayerDraw::cleanupDM()
{
    /*
    if(!_graphicsItem)
        return;

    if(_graphicsItem->scene())
        _graphicsItem->scene()->removeItem(_graphicsItem);

    delete _graphicsItem;
    _graphicsItem = nullptr;
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
