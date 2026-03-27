#include "layerparticle.h"
#include "layerparticlesettings.h"
#include "publishglrenderer.h"
#include "dmh_opengl.h"
#include <QImage>
#include <QPainter>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QMatrix4x4>
#include <QTimerEvent>
#include <QtMath>

const int LAYERPARTICLE_PREVIEWSIZE = 512;
const int LAYERPARTICLE_TIMERPERIOD = 30;

// Vertex shader: full-screen quad, passes tex coords to fragment
const char *particleVertexShaderSource = "#version 410 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "layout (location = 2) in vec2 aTexCoord;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "   TexCoord = aTexCoord;\n"
    "}\0";

// Fragment shader: procedural rain
const char *particleFragmentShaderSource = "#version 410 core\n"
    "out vec4 FragColor;\n"
    "in vec2 TexCoord;\n"
    "uniform float u_time;\n"
    "uniform float u_speed;\n"
    "uniform float u_angle;\n"
    "uniform float u_length;\n"
    "uniform vec4 u_color;\n"
    "\n"
    "float hash(vec2 p)\n"
    "{\n"
    "    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);\n"
    "}\n"
    "\n"
    "void main()\n"
    "{\n"
    "    float angle = radians(u_angle);\n"
    "    float ca = cos(angle);\n"
    "    float sa = sin(angle);\n"
    "\n"
    "    // Rotate texture coords into rain-aligned space\n"
    "    vec2 uv = TexCoord;\n"
    "    vec2 ruv;\n"
    "    ruv.x = uv.x * ca - uv.y * sa;\n"
    "    ruv.y = uv.x * sa + uv.y * ca;\n"
    "\n"
    "    float speedFactor = u_speed / 50.0;\n"
    "    float t = u_time * speedFactor * 0.001;\n"
    "\n"
    "    // Multi-layer rain for depth\n"
    "    float alpha = 0.0;\n"
    "    for(int i = 0; i < 3; i++)\n"
    "    {\n"
    "        float fi = float(i);\n"
    "        float scale = 20.0 + fi * 15.0;\n"
    "        float layerSpeed = 1.0 + fi * 0.5;\n"
    "        float layerAlpha = 0.3 - fi * 0.08;\n"
    "        float dropLen = u_length * (1.0 + fi * 0.3) / 100.0;\n"
    "\n"
    "        // Column hash for x-position variation\n"
    "        float col = floor(ruv.x * scale);\n"
    "        float colHash = hash(vec2(col, fi));\n"
    "\n"
    "        // Falling position with time offset per column\n"
    "        float fall = fract(ruv.y * scale * 0.5 - t * layerSpeed + colHash * 100.0);\n"
    "\n"
    "        // Horizontal position within the cell\n"
    "        float cellX = fract(ruv.x * scale) - 0.5;\n"
    "        float xOffset = (colHash - 0.5) * 0.3;\n"
    "\n"
    "        // Rain streak: narrow in x, elongated in y\n"
    "        float width = 0.04 + colHash * 0.02;\n"
    "        float streak = smoothstep(width, 0.0, abs(cellX - xOffset))\n"
    "                      * smoothstep(dropLen, 0.0, fall)\n"
    "                      * smoothstep(0.0, dropLen * 0.1, fall);\n"
    "\n"
    "        alpha += streak * layerAlpha;\n"
    "    }\n"
    "\n"
    "    alpha = clamp(alpha, 0.0, 1.0) * u_color.a;\n"
    "    FragColor = vec4(u_color.rgb, alpha);\n"
    "}\n";

LayerParticle::LayerParticle(const QString& name, int order, QObject *parent) :
    Layer{name, order, parent},
    _graphicsItem(nullptr),
    _scene(nullptr),
    _timerId(0),
    _milliseconds(0),
    _objectsDirty(true),
    _VAO(0),
    _VBO(0),
    _shaderTime(0),
    _shaderSpeed(0),
    _shaderAngle(0),
    _shaderLength(0),
    _shaderColor(0),
    _particleCount(defaultParticleCount),
    _rainSpeed(defaultRainSpeed),
    _rainAngle(defaultRainAngle),
    _rainColor(QColor(200, 200, 255, 180)),
    _rainLength(defaultRainLength)
{
}

LayerParticle::~LayerParticle()
{
    cleanupDM();
    cleanupPlayer();
}

void LayerParticle::inputXML(const QDomElement &element, bool isImport)
{
    _particleCount = element.attribute("particleCount", QString::number(defaultParticleCount)).toInt();
    _rainSpeed     = element.attribute("rainSpeed", QString::number(defaultRainSpeed)).toInt();
    _rainAngle     = element.attribute("rainAngle", QString::number(defaultRainAngle)).toInt();
    _rainLength    = element.attribute("rainLength", QString::number(defaultRainLength)).toInt();
    _rainColor     = QColor(element.attribute("rainColor", QColor(200, 200, 255, 180).name(QColor::HexArgb)));

    Layer::inputXML(element, isImport);
}

QRectF LayerParticle::boundingRect() const
{
    return QRectF();
}

QImage LayerParticle::getLayerIcon() const
{
    return QImage(":/img/data/icon_clouds.png");
}

bool LayerParticle::defaultShader() const
{
    return false;
}

bool LayerParticle::hasSettings() const
{
    return true;
}

DMHelper::LayerType LayerParticle::getType() const
{
    return DMHelper::LayerType_Particle;
}

Layer* LayerParticle::clone() const
{
    LayerParticle* newLayer = new LayerParticle(_name, _order);
    copyBaseValues(newLayer);
    newLayer->_particleCount = _particleCount;
    newLayer->_rainSpeed     = _rainSpeed;
    newLayer->_rainAngle     = _rainAngle;
    newLayer->_rainColor     = _rainColor;
    newLayer->_rainLength    = _rainLength;
    return newLayer;
}

void LayerParticle::applyOrder(int order)
{
    if(_graphicsItem)
        _graphicsItem->setZValue(order);
}

void LayerParticle::applyLayerVisibleDM(bool layerVisible)
{
    if(_graphicsItem)
        _graphicsItem->setVisible(layerVisible);
}

void LayerParticle::applyLayerVisiblePlayer(bool layerVisible)
{
    Q_UNUSED(layerVisible);
}

void LayerParticle::applyOpacity(qreal opacity)
{
    _opacityReference = opacity;
    if(_graphicsItem)
        _graphicsItem->setOpacity(opacity);
}

void LayerParticle::applyPosition(const QPoint& position)
{
    if(_graphicsItem)
        _graphicsItem->setPos(position);
}

void LayerParticle::applySize(const QSize& size)
{
    Q_UNUSED(size);
    _objectsDirty = true;
    refreshDMPreview();
}

// DM Window Interface

void LayerParticle::dmInitialize(QGraphicsScene* scene)
{
    if(!scene)
        return;

    if(_graphicsItem)
    {
        qDebug() << "[LayerParticle] ERROR: dmInitialize called although the graphics item already exists!";
        return;
    }

    QImage preview = createRainPreview(_size.isEmpty() ? QSize(LAYERPARTICLE_PREVIEWSIZE, LAYERPARTICLE_PREVIEWSIZE) : _size);
    _graphicsItem = scene->addPixmap(QPixmap::fromImage(preview));

    if(_graphicsItem)
    {
        _graphicsItem->setPos(_position);
        _graphicsItem->setFlag(QGraphicsItem::ItemIsMovable, false);
        _graphicsItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
        _graphicsItem->setZValue(getOrder());
        _graphicsItem->setOpacity(_opacityReference);
    }

    Layer::dmInitialize(scene);
}

void LayerParticle::dmUninitialize()
{
    cleanupDM();
}

void LayerParticle::dmUpdate()
{
}

// Player Window Interface

void LayerParticle::playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene)
{
    _scene = scene;
    Layer::playerGLInitialize(renderer, scene);

    if((_timerId == 0) && (renderer))
    {
        connect(this, &LayerParticle::update, renderer, &PublishGLRenderer::updateWidget);
        _timerId = startTimer(LAYERPARTICLE_TIMERPERIOD);
    }
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

    if((_VAO == 0) || _objectsDirty)
        createObjects();

    if((!QOpenGLContext::currentContext()) || (!functions) || (_shaderProgramRGBA == 0) || (_VAO == 0))
        return;

    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if(!e)
        return;

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGBA);
    functions->glUseProgram(_shaderProgramRGBA);
    DMH_DEBUG_OPENGL_glUniformMatrix4fv4(_shaderProjectionMatrixRGBA, 1, GL_FALSE, projectionMatrix);
    functions->glUniformMatrix4fv(_shaderProjectionMatrixRGBA, 1, GL_FALSE, projectionMatrix);

    QMatrix4x4 modelMatrix;
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, modelMatrix.constData(), modelMatrix);
    functions->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, modelMatrix.constData());

    functions->glUniform1f(_shaderTime, static_cast<float>(_milliseconds));
    functions->glUniform1f(_shaderSpeed, static_cast<float>(_rainSpeed));
    functions->glUniform1f(_shaderAngle, static_cast<float>(_rainAngle));
    functions->glUniform1f(_shaderLength, static_cast<float>(_rainLength));
    functions->glUniform4f(_shaderColor,
                           _rainColor.redF(), _rainColor.greenF(), _rainColor.blueF(),
                           _opacityReference);

    e->glBindVertexArray(_VAO);
    functions->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void LayerParticle::playerGLResize(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);

    _objectsDirty = true;
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

void LayerParticle::editSettings()
{
    LayerParticleSettings* dlg = new LayerParticleSettings();

    connect(dlg, &LayerParticleSettings::particleCountChanged, this, &LayerParticle::setParticleCount);
    connect(dlg, &LayerParticleSettings::rainSpeedChanged,     this, &LayerParticle::setRainSpeed);
    connect(dlg, &LayerParticleSettings::rainAngleChanged,     this, &LayerParticle::setRainAngle);
    connect(dlg, &LayerParticleSettings::rainColorChanged,     this, &LayerParticle::setRainColor);
    connect(dlg, &LayerParticleSettings::rainLengthChanged,    this, &LayerParticle::setRainLength);

    dlg->setParticleCount(_particleCount);
    dlg->setRainSpeed(_rainSpeed);
    dlg->setRainAngle(_rainAngle);
    dlg->setRainColor(_rainColor);
    dlg->setRainLength(_rainLength);

    dlg->exec();

    refreshDMPreview();

    dlg->deleteLater();
}

void LayerParticle::setParticleCount(int count)
{
    if(_particleCount == count)
        return;
    _particleCount = count;
    emit dirty();
    emit update();
}

void LayerParticle::setRainSpeed(int speed)
{
    if(_rainSpeed == speed)
        return;
    _rainSpeed = speed;
    emit dirty();
    emit update();
}

void LayerParticle::setRainAngle(int angle)
{
    if(_rainAngle == angle)
        return;
    _rainAngle = angle;
    emit dirty();
    emit update();
}

void LayerParticle::setRainColor(const QColor& color)
{
    if(_rainColor == color)
        return;
    _rainColor = color;
    emit dirty();
    emit update();
}

void LayerParticle::setRainLength(int length)
{
    if(_rainLength == length)
        return;
    _rainLength = length;
    emit dirty();
    emit update();
}

void LayerParticle::timerEvent(QTimerEvent *event)
{
    if((event) && (event->timerId() > 0) && (event->timerId() == _timerId))
    {
        _milliseconds += LAYERPARTICLE_TIMERPERIOD;
        emit update();
    }
}

void LayerParticle::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    if(_particleCount != defaultParticleCount)
        element.setAttribute("particleCount", _particleCount);

    if(_rainSpeed != defaultRainSpeed)
        element.setAttribute("rainSpeed", _rainSpeed);

    if(_rainAngle != defaultRainAngle)
        element.setAttribute("rainAngle", _rainAngle);

    if(_rainLength != defaultRainLength)
        element.setAttribute("rainLength", _rainLength);

    QColor defaultColor(200, 200, 255, 180);
    if(_rainColor != defaultColor)
        element.setAttribute("rainColor", _rainColor.name(QColor::HexArgb));

    Layer::internalOutputXML(doc, element, targetDirectory, isExport);
}

void LayerParticle::cleanupDM()
{
    if(!_graphicsItem)
        return;

    if(_graphicsItem->scene())
        _graphicsItem->scene()->removeItem(_graphicsItem);

    delete _graphicsItem;
    _graphicsItem = nullptr;
}

void LayerParticle::cleanupPlayer()
{
    disconnect(this, &LayerParticle::update, nullptr, nullptr);

    if(_timerId > 0)
    {
        killTimer(_timerId);
        _timerId = 0;
    }

    destroyObjects();
    destroyShaders();

    _scene = nullptr;
}

void LayerParticle::createShaders()
{
    if((!QOpenGLContext::currentContext()) || (_shaderProgramRGBA != 0))
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

    int success;
    char infoLog[512];

    // Vertex shader
    unsigned int vertexShader = f->glCreateShader(GL_VERTEX_SHADER);
    f->glShaderSource(vertexShader, 1, &particleVertexShaderSource, NULL);
    f->glCompileShader(vertexShader);

    f->glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        qDebug() << "[LayerParticle] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

    // Fragment shader
    unsigned int fragmentShader = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShader, 1, &particleFragmentShaderSource, NULL);
    f->glCompileShader(fragmentShader);

    f->glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        qDebug() << "[LayerParticle] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    // Link program
    _shaderProgramRGBA = f->glCreateProgram();
    DMH_DEBUG_OPENGL_glCreateProgram(_shaderProgramRGBA, "_shaderProgramRGBA");

    f->glAttachShader(_shaderProgramRGBA, vertexShader);
    f->glAttachShader(_shaderProgramRGBA, fragmentShader);
    f->glLinkProgram(_shaderProgramRGBA);

    f->glGetProgramiv(_shaderProgramRGBA, GL_LINK_STATUS, &success);
    if(!success)
    {
        f->glGetProgramInfoLog(_shaderProgramRGBA, 512, NULL, infoLog);
        qDebug() << "[LayerParticle] ERROR::SHADER::PROGRAM::LINK_FAILED: " << infoLog;
        return;
    }

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGBA);
    f->glUseProgram(_shaderProgramRGBA);
    f->glDeleteShader(vertexShader);
    f->glDeleteShader(fragmentShader);

    // Get uniform locations
    _shaderModelMatrixRGBA = f->glGetUniformLocation(_shaderProgramRGBA, "model");
    _shaderProjectionMatrixRGBA = f->glGetUniformLocation(_shaderProgramRGBA, "projection");
    _shaderTime   = f->glGetUniformLocation(_shaderProgramRGBA, "u_time");
    _shaderSpeed  = f->glGetUniformLocation(_shaderProgramRGBA, "u_speed");
    _shaderAngle  = f->glGetUniformLocation(_shaderProgramRGBA, "u_angle");
    _shaderLength = f->glGetUniformLocation(_shaderProgramRGBA, "u_length");
    _shaderColor  = f->glGetUniformLocation(_shaderProgramRGBA, "u_color");

    // Set view matrix
    QMatrix4x4 modelMatrix;
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));

    f->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, modelMatrix.constData());
    int viewLoc = f->glGetUniformLocation(_shaderProgramRGBA, "view");
    f->glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMatrix.constData());

    qDebug() << "[LayerParticle] Shaders created. Program:" << _shaderProgramRGBA;
}

void LayerParticle::destroyShaders()
{
    if(QOpenGLContext::currentContext())
    {
        QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
        if((e) && (_shaderProgramRGBA > 0))
        {
            DMH_DEBUG_OPENGL_Singleton::removeProgram(_shaderProgramRGBA);
            e->glDeleteProgram(_shaderProgramRGBA);
        }
    }

    _shaderProgramRGBA = 0;
    _shaderModelMatrixRGBA = 0;
    _shaderProjectionMatrixRGBA = 0;
    _shaderTime = 0;
    _shaderSpeed = 0;
    _shaderAngle = 0;
    _shaderLength = 0;
    _shaderColor = 0;
}

void LayerParticle::createObjects()
{
    if(!QOpenGLContext::currentContext())
        return;

    if(_size.isEmpty())
        return;

    destroyObjects();

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    float hw = static_cast<float>(_size.width())  / 2.f;
    float hh = static_cast<float>(_size.height()) / 2.f;

    float vertices[] = {
        // positions          // colors           // texture coords
        -hw,  hh, 0.0f,      1.f, 1.f, 1.f,      0.0f, 1.0f,  // top left
         hw,  hh, 0.0f,      1.f, 1.f, 1.f,      1.0f, 1.0f,  // top right
         hw, -hh, 0.0f,      1.f, 1.f, 1.f,      1.0f, 0.0f,  // bottom right
        -hw, -hh, 0.0f,      1.f, 1.f, 1.f,      0.0f, 0.0f,  // bottom left
    };

    e->glGenVertexArrays(1, &_VAO);
    f->glGenBuffers(1, &_VBO);

    e->glBindVertexArray(_VAO);

    f->glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    f->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    f->glEnableVertexAttribArray(0);
    // color attribute
    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    f->glEnableVertexAttribArray(1);
    // texture attribute
    f->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    f->glEnableVertexAttribArray(2);

    _objectsDirty = false;

    qDebug() << "[LayerParticle] Objects created, size:" << _size;
}

void LayerParticle::destroyObjects()
{
    if(QOpenGLContext::currentContext())
    {
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
        QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();

        if((f) && (e))
        {
            if(_VAO > 0)
                e->glDeleteVertexArrays(1, &_VAO);
            if(_VBO > 0)
                f->glDeleteBuffers(1, &_VBO);
        }
    }

    _VAO = 0;
    _VBO = 0;
}

void LayerParticle::refreshDMPreview()
{
    if(!_graphicsItem)
        return;

    QSize previewSize = _size.isEmpty() ? QSize(LAYERPARTICLE_PREVIEWSIZE, LAYERPARTICLE_PREVIEWSIZE) : _size;
    QImage preview = createRainPreview(previewSize);
    _graphicsItem->setPixmap(QPixmap::fromImage(preview));
}

QImage LayerParticle::createRainPreview(const QSize& size) const
{
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen pen(_rainColor);
    pen.setWidthF(1.5);
    painter.setPen(pen);

    qreal angleRad = qDegreesToRadians(static_cast<qreal>(_rainAngle));
    qreal dx = qSin(angleRad) * _rainLength;
    qreal dy = qCos(angleRad) * _rainLength;

    // Deterministic pseudo-random rain line positions
    uint seed = 42;
    for(int i = 0; i < _particleCount; ++i)
    {
        seed = seed * 1103515245 + 12345;
        qreal x = static_cast<qreal>(seed % static_cast<uint>(size.width()));
        seed = seed * 1103515245 + 12345;
        qreal y = static_cast<qreal>(seed % static_cast<uint>(size.height()));

        painter.drawLine(QPointF(x, y), QPointF(x + dx, y + dy));
    }

    painter.end();
    return image;
}
