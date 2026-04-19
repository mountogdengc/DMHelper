#ifndef DICEANIMATIONWIDGET_H
#define DICEANIMATIONWIDGET_H

#include <QWidget>
#include <QVector>
#include <QVector3D>
#include <QQuaternion>

namespace Qt3DCore {
class QEntity;
class QTransform;
}
namespace Qt3DExtras {
class Qt3DWindow;
}

class QTimer;

class DiceAnimationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DiceAnimationWidget(QWidget *parent = nullptr);
    ~DiceAnimationWidget() override;

    static bool animationEnabled();

    void rollDice(int diceType, const QVector<int>& results);
    bool isAnimating() const;

signals:
    void animationFinished();

private slots:
    void stepSimulation();

private:
    struct DieBody {
        Qt3DCore::QEntity *entity;
        Qt3DCore::QTransform *transform;
        QVector3D position;
        QVector3D linearVelocity;
        QQuaternion orientation;
        QVector3D angularVelocity;
        float restTimer;
        bool atRest;
        int targetValue;
    };

    void clearScene();
    void setupScene();
    Qt3DCore::QEntity *buildDieEntity(int diceType, Qt3DCore::QEntity *parent, Qt3DCore::QTransform *&outTransform);

    Qt3DExtras::Qt3DWindow *_view;
    Qt3DCore::QEntity *_rootEntity;
    Qt3DCore::QEntity *_diceGroupEntity;

    QTimer *_timer;
    qint64 _elapsedMs;

    QVector<DieBody> _bodies;
    int _currentDiceType;
};

#endif // DICEANIMATIONWIDGET_H
