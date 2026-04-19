#include "diceanimationwidget.h"

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QPointLight>

#include <QHBoxLayout>
#include <QTimer>
#include <QRandomGenerator>
#include <QSettings>
#include <QColor>
#include <QtMath>

namespace {
constexpr float kGravity = -22.0f;
constexpr float kFloorY = 0.0f;
constexpr float kDieHalfExtent = 0.8f;
constexpr float kLinearDamping = 0.25f;
constexpr float kAngularDamping = 0.35f;
constexpr float kRestoration = 0.30f;
constexpr float kFloorFriction = 0.82f;
constexpr float kAngularFloorDrag = 0.74f;
constexpr float kRestLinearThreshold = 0.30f;
constexpr float kRestAngularThreshold = 0.70f;
constexpr float kRestTimeSeconds = 0.18f;
constexpr int   kTimerIntervalMs = 16;
constexpr qint64 kMaxDurationMs = 2500;

QColor colorForDiceType(int diceType)
{
    switch(diceType)
    {
    case 4:   return QColor(232, 84, 84);
    case 6:   return QColor(84, 175, 232);
    case 8:   return QColor(84, 232, 114);
    case 10:  return QColor(232, 192, 84);
    case 12:  return QColor(170, 84, 232);
    case 20:  return QColor(232, 134, 84);
    case 100: return QColor(232, 230, 84);
    default:  return QColor(230, 230, 230);
    }
}

double randRange(QRandomGenerator *rng, double lo, double hi)
{
    return lo + rng->generateDouble() * (hi - lo);
}
}

DiceAnimationWidget::DiceAnimationWidget(QWidget *parent) :
    QWidget(parent),
    _view(nullptr),
    _rootEntity(nullptr),
    _diceGroupEntity(nullptr),
    _timer(nullptr),
    _elapsedMs(0),
    _bodies(),
    _currentDiceType(0)
{
    _view = new Qt3DExtras::Qt3DWindow();
    _view->defaultFrameGraph()->setClearColor(QColor(30, 22, 14));

    QWidget *container = QWidget::createWindowContainer(_view, this);
    container->setMinimumSize(200, 160);
    container->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(container);

    setupScene();

    _timer = new QTimer(this);
    _timer->setInterval(kTimerIntervalMs);
    connect(_timer, &QTimer::timeout, this, &DiceAnimationWidget::stepSimulation);
}

DiceAnimationWidget::~DiceAnimationWidget()
{
    if(_timer)
        _timer->stop();
}

bool DiceAnimationWidget::animationEnabled()
{
    QSettings settings;
    return settings.value(QStringLiteral("dice/animate3d"), true).toBool();
}

void DiceAnimationWidget::setupScene()
{
    _rootEntity = new Qt3DCore::QEntity();

    Qt3DRender::QCamera *camera = _view->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(0.0f, 8.0f, 10.0f));
    camera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
    camera->setViewCenter(QVector3D(0.0f, 0.5f, 0.0f));

    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(_rootEntity);
    Qt3DRender::QPointLight *light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor(Qt::white);
    light->setIntensity(1.2f);
    Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform();
    lightTransform->setTranslation(QVector3D(4.0f, 10.0f, 6.0f));
    lightEntity->addComponent(light);
    lightEntity->addComponent(lightTransform);

    _diceGroupEntity = new Qt3DCore::QEntity(_rootEntity);

    _view->setRootEntity(_rootEntity);
}

Qt3DCore::QEntity *DiceAnimationWidget::buildDieEntity(int diceType, Qt3DCore::QEntity *parent, Qt3DCore::QTransform *&outTransform)
{
    Qt3DCore::QEntity *dieEntity = new Qt3DCore::QEntity(parent);

    Qt3DExtras::QCuboidMesh *mesh = new Qt3DExtras::QCuboidMesh();
    const float side = 2.0f * kDieHalfExtent;
    mesh->setXExtent(side);
    mesh->setYExtent(side);
    mesh->setZExtent(side);

    Qt3DExtras::QPhongMaterial *material = new Qt3DExtras::QPhongMaterial();
    material->setDiffuse(colorForDiceType(diceType));
    material->setAmbient(colorForDiceType(diceType).darker(250));
    material->setShininess(50.0f);

    outTransform = new Qt3DCore::QTransform();

    dieEntity->addComponent(mesh);
    dieEntity->addComponent(material);
    dieEntity->addComponent(outTransform);

    return dieEntity;
}

void DiceAnimationWidget::clearScene()
{
    for(DieBody &b : _bodies)
    {
        if(b.entity)
        {
            b.entity->setParent(static_cast<Qt3DCore::QNode*>(nullptr));
            b.entity->deleteLater();
        }
    }
    _bodies.clear();
}

void DiceAnimationWidget::rollDice(int diceType, const QVector<int>& results)
{
    if(_timer->isActive())
        _timer->stop();

    clearScene();
    _currentDiceType = diceType;

    if(results.isEmpty())
    {
        emit animationFinished();
        return;
    }

    QRandomGenerator *rng = QRandomGenerator::global();
    const int count = results.size();

    for(int i = 0; i < count; ++i)
    {
        DieBody body;
        body.restTimer = 0.0f;
        body.atRest = false;
        body.targetValue = results.at(i);

        Qt3DCore::QTransform *transform = nullptr;
        body.entity = buildDieEntity(diceType, _diceGroupEntity, transform);
        body.transform = transform;

        const float spread = 1.6f;
        const float xOffset = (count > 1) ? (i - (count - 1) / 2.0f) * spread : 0.0f;
        body.position = QVector3D(static_cast<float>(xOffset + randRange(rng, -0.3, 0.3)),
                                  static_cast<float>(4.0 + randRange(rng, 0.0, 1.5)),
                                  static_cast<float>(randRange(rng, -0.4, 0.4)));

        body.linearVelocity = QVector3D(static_cast<float>(randRange(rng, -1.5, 1.5)),
                                        static_cast<float>(randRange(rng, -0.5, 0.5)),
                                        static_cast<float>(randRange(rng, -1.0, 1.0)));

        body.angularVelocity = QVector3D(static_cast<float>(randRange(rng, -8.0, 8.0)),
                                         static_cast<float>(randRange(rng, -8.0, 8.0)),
                                         static_cast<float>(randRange(rng, -8.0, 8.0)));

        body.orientation = QQuaternion::fromEulerAngles(
            static_cast<float>(randRange(rng, 0.0, 360.0)),
            static_cast<float>(randRange(rng, 0.0, 360.0)),
            static_cast<float>(randRange(rng, 0.0, 360.0)));

        body.transform->setTranslation(body.position);
        body.transform->setRotation(body.orientation);

        _bodies.append(body);
    }

    _elapsedMs = 0;
    _timer->start();
}

bool DiceAnimationWidget::isAnimating() const
{
    return _timer && _timer->isActive();
}

void DiceAnimationWidget::stepSimulation()
{
    const float dt = kTimerIntervalMs / 1000.0f;
    _elapsedMs += kTimerIntervalMs;

    bool allAtRest = true;

    for(DieBody &b : _bodies)
    {
        if(!b.atRest)
        {
            b.linearVelocity.setY(b.linearVelocity.y() + kGravity * dt);

            b.linearVelocity *= (1.0f - kLinearDamping * dt);
            b.angularVelocity *= (1.0f - kAngularDamping * dt);

            b.position += b.linearVelocity * dt;

            const float angMag = b.angularVelocity.length();
            if(angMag > 0.0001f)
            {
                const QVector3D axis = b.angularVelocity.normalized();
                QQuaternion dq = QQuaternion::fromAxisAndAngle(axis, qRadiansToDegrees(angMag * dt));
                b.orientation = (dq * b.orientation).normalized();
            }

            const float floorLimit = kFloorY + kDieHalfExtent;
            if(b.position.y() < floorLimit)
            {
                b.position.setY(floorLimit);
                if(b.linearVelocity.y() < 0.0f)
                    b.linearVelocity.setY(-b.linearVelocity.y() * kRestoration);
                b.linearVelocity.setX(b.linearVelocity.x() * kFloorFriction);
                b.linearVelocity.setZ(b.linearVelocity.z() * kFloorFriction);
                b.angularVelocity *= kAngularFloorDrag;
            }

            if(b.linearVelocity.length() < kRestLinearThreshold &&
               b.angularVelocity.length() < kRestAngularThreshold &&
               b.position.y() <= floorLimit + 0.05f)
            {
                b.restTimer += dt;
                if(b.restTimer >= kRestTimeSeconds)
                {
                    b.atRest = true;
                    b.linearVelocity = QVector3D(0.0f, 0.0f, 0.0f);
                    b.angularVelocity = QVector3D(0.0f, 0.0f, 0.0f);
                    b.position.setY(floorLimit);
                }
            }
            else
            {
                b.restTimer = 0.0f;
            }
        }

        if(!b.atRest)
            allAtRest = false;

        b.transform->setTranslation(b.position);
        b.transform->setRotation(b.orientation);
    }

    if(allAtRest || _elapsedMs >= kMaxDurationMs)
    {
        _timer->stop();
        for(DieBody &b : _bodies)
        {
            b.orientation = QQuaternion();
            b.position.setY(kFloorY + kDieHalfExtent);
            b.transform->setTranslation(b.position);
            b.transform->setRotation(b.orientation);
        }
        emit animationFinished();
    }
}
