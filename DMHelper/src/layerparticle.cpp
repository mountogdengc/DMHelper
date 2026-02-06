#include "layerparticle.h"

LayerParticle::LayerParticle(const QString& name, int order, QObject *parent) :
    Layer{name, order, parent}
{}

LayerParticle::~LayerParticle()
{
    cleanupDM();
    cleanupPlayer();
}

DMHelper::LayerType LayerParticle::getType() const
{
    return DMHelper::LayerType_Particle;
}

Layer* LayerParticle::clone() const
{
    LayerParticle* newLayer = new LayerParticle(this->getName(), this->getOrder());
    this->copyBaseValues(newLayer);
    return newLayer;
}

void LayerParticle::applyOrder(int order)
{
    Q_UNUSED(order);
}

void LayerParticle::applyLayerVisibleDM(bool layerVisible)
{
    Q_UNUSED(layerVisible);
}

void LayerParticle::applyLayerVisiblePlayer(bool layerVisible)
{
    Q_UNUSED(layerVisible);
}

void LayerParticle::applyOpacity(qreal opacity)
{
    Q_UNUSED(opacity);
}

void LayerParticle::applyPosition(const QPoint& position)
{
    Q_UNUSED(position);
}

void LayerParticle::applySize(const QSize& size)
{
    Q_UNUSED(size);
}

void LayerParticle::dmInitialize(QGraphicsScene* scene)
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

void LayerParticle::dmUninitialize()
{
    cleanupDM();
}

void LayerParticle::dmUpdate()
{
}

void LayerParticle::playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene)
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

void LayerParticle::playerGLUninitialize()
{
    cleanupPlayer();
}

void LayerParticle::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
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

void LayerParticle::playerGLResize(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);

    destroyObjects();
    destroyShaders();
}

void LayerParticle::playerSetShaders(unsigned int programRGB, int modelMatrixRGB, int projectionMatrixRGB, unsigned int programRGBA, int modelMatrixRGBA, int projectionMatrixRGBA, int alphaRGBA)
{
    Q_UNUSED(programRGB);
    Q_UNUSED(modelMatrixRGB);
    Q_UNUSED(projectionMatrixRGB);
    Q_UNUSED(programRGBA);
    Q_UNUSED(modelMatrixRGBA);
    Q_UNUSED(projectionMatrixRGBA);
    Q_UNUSED(alphaRGBA);
}

bool LayerParticle::playerIsInitialized()
{
    return _scene != nullptr;
}

void LayerParticle::initialize(const QSize& sceneSize)
{
    if(getSize().isEmpty())
        setSize(sceneSize);
}

void LayerParticle::uninitialize()
{
}

void LayerParticle::timerEvent(QTimerEvent *event)
{
    /*
    if((event) && (event->timerId() > 0) && (event->timerId() == _timerId))
    {
        _milliseconds += LAYEREFFECT_TIMERPERIOD;
        emit update();
    }
*/
}

void LayerParticle::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    Layer::internalOutputXML(doc, element, targetDirectory, isExport);
}

void LayerParticle::cleanupDM()
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

void LayerParticle::cleanupPlayer()
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

void LayerParticle::createShaders()
{
}

void LayerParticle::destroyShaders()
{
}

void LayerParticle::createObjects()
{
}

void LayerParticle::destroyObjects()
{
}
