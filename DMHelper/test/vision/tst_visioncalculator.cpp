// Qt Test-style unit tests for VisionCalculator.
//
// Build standalone with (adjust include path as needed):
//   qmake -project "CONFIG += qtestlib c++11" \
//         "SOURCES += tst_visioncalculator.cpp ../../src/visioncalculator.cpp" \
//         "HEADERS += ../../src/visioncalculator.h" \
//         "INCLUDEPATH += ../../src"
//   qmake && make && ./vision
//
// The tests exercise the geometry engine in isolation — no Qt GUI / OpenGL
// context required — so they run in any Qt environment.

#include "visioncalculator.h"
#include <QtTest/QtTest>
#include <QtMath>

class TestVisionCalculator : public QObject
{
    Q_OBJECT

private slots:
    void segmentIntersectionCrosses();
    void segmentIntersectionParallel();
    void segmentIntersectionEndpoints();

    void emptyRoomReturnsRoomCorners();
    void singleWallBlocksOppositeSide();
    void maxRadiusClipsToCircle();
    void wallEndpointNotOccludedFromClosePoint();
    void visionPolygonIsFinite();

    void qLineFOverloadMatchesSegment();
};

void TestVisionCalculator::segmentIntersectionCrosses()
{
    QPointF hit;
    const bool intersects = VisionCalculator::segmentIntersection(
        QPointF(0, 0), QPointF(10, 10),
        QPointF(0, 10), QPointF(10, 0),
        &hit);

    QVERIFY(intersects);
    QVERIFY(qAbs(hit.x() - 5.0) < 1e-6);
    QVERIFY(qAbs(hit.y() - 5.0) < 1e-6);
}

void TestVisionCalculator::segmentIntersectionParallel()
{
    const bool intersects = VisionCalculator::segmentIntersection(
        QPointF(0, 0), QPointF(10, 0),
        QPointF(0, 5), QPointF(10, 5));
    QVERIFY(!intersects);
}

void TestVisionCalculator::segmentIntersectionEndpoints()
{
    // Segments sharing an endpoint count as intersecting there.
    QPointF hit;
    const bool intersects = VisionCalculator::segmentIntersection(
        QPointF(0, 0), QPointF(10, 0),
        QPointF(10, 0), QPointF(10, 10),
        &hit);

    QVERIFY(intersects);
    QVERIFY(qAbs(hit.x() - 10.0) < 1e-6);
    QVERIFY(qAbs(hit.y() - 0.0) < 1e-6);
}

void TestVisionCalculator::emptyRoomReturnsRoomCorners()
{
    // No walls: the visible polygon should span the full bounds, so all four
    // corners must lie on or extremely close to the polygon.
    const QRectF bounds(0, 0, 100, 100);
    const QList<VisionCalculator::Segment> walls;

    const QPolygonF poly = VisionCalculator::computeVisionPolygon(
        QPointF(50, 50), walls, bounds);

    QVERIFY(!poly.isEmpty());

    // The polygon must contain the viewpoint.
    QVERIFY(poly.containsPoint(QPointF(50, 50), Qt::OddEvenFill));

    // With no obstruction, the four corners should be "visible" — i.e. they
    // must be inside or on the boundary of the polygon.
    QVERIFY(poly.containsPoint(QPointF(0.5, 0.5), Qt::OddEvenFill));
    QVERIFY(poly.containsPoint(QPointF(99.5, 0.5), Qt::OddEvenFill));
    QVERIFY(poly.containsPoint(QPointF(99.5, 99.5), Qt::OddEvenFill));
    QVERIFY(poly.containsPoint(QPointF(0.5, 99.5), Qt::OddEvenFill));
}

void TestVisionCalculator::singleWallBlocksOppositeSide()
{
    // Viewpoint at (10, 50). A vertical wall from (50, 0) to (50, 100) splits
    // the room. Points well past the wall must NOT be inside the polygon;
    // points on the viewpoint's side MUST be inside.
    const QRectF bounds(0, 0, 100, 100);
    QList<VisionCalculator::Segment> walls;
    walls.append({QPointF(50, 0), QPointF(50, 100)});

    const QPolygonF poly = VisionCalculator::computeVisionPolygon(
        QPointF(10, 50), walls, bounds);

    QVERIFY(!poly.isEmpty());
    QVERIFY(poly.containsPoint(QPointF(10, 50), Qt::OddEvenFill));

    // Clear line of sight on the viewpoint's side.
    QVERIFY(poly.containsPoint(QPointF(20, 50), Qt::OddEvenFill));
    QVERIFY(poly.containsPoint(QPointF(40, 50), Qt::OddEvenFill));

    // Past the wall: should be occluded (not in polygon). Pick a point well
    // past the wall so numerical precision is irrelevant.
    QVERIFY(!poly.containsPoint(QPointF(80, 50), Qt::OddEvenFill));
    QVERIFY(!poly.containsPoint(QPointF(99, 50), Qt::OddEvenFill));
}

void TestVisionCalculator::maxRadiusClipsToCircle()
{
    const QRectF bounds(0, 0, 1000, 1000);
    const QList<VisionCalculator::Segment> walls;

    const qreal radius = 30.0;
    const QPolygonF poly = VisionCalculator::computeVisionPolygon(
        QPointF(500, 500), walls, bounds, radius);

    QVERIFY(!poly.isEmpty());

    // Every polygon vertex should lie within radius + tiny slack.
    for(const QPointF& pt : poly)
    {
        const qreal dx = pt.x() - 500.0;
        const qreal dy = pt.y() - 500.0;
        const qreal dist = std::hypot(dx, dy);
        QVERIFY2(dist <= radius + 1e-3,
                 qPrintable(QString("vertex at (%1, %2) is %3 from viewpoint")
                                .arg(pt.x()).arg(pt.y()).arg(dist)));
    }

    // A point just past the radius should not be visible.
    QVERIFY(!poly.containsPoint(QPointF(500 + radius + 5.0, 500),
                                Qt::OddEvenFill));
}

void TestVisionCalculator::wallEndpointNotOccludedFromClosePoint()
{
    // If the viewpoint is almost at a wall endpoint, the polygon should still
    // be well-formed (no NaN, reasonable area).
    const QRectF bounds(0, 0, 100, 100);
    QList<VisionCalculator::Segment> walls;
    walls.append({QPointF(40, 40), QPointF(60, 60)});

    const QPolygonF poly = VisionCalculator::computeVisionPolygon(
        QPointF(40.0001, 40.0001), walls, bounds);

    QVERIFY(poly.size() >= 3);
    for(const QPointF& pt : poly)
    {
        QVERIFY(!qIsNaN(pt.x()));
        QVERIFY(!qIsNaN(pt.y()));
    }
}

void TestVisionCalculator::visionPolygonIsFinite()
{
    // All polygon vertices must be finite, even with many overlapping walls.
    const QRectF bounds(0, 0, 500, 500);
    QList<VisionCalculator::Segment> walls;
    for(int i = 0; i < 20; ++i)
    {
        const qreal x = 50.0 + i * 20.0;
        walls.append({QPointF(x, 100), QPointF(x, 400)});
    }

    const QPolygonF poly = VisionCalculator::computeVisionPolygon(
        QPointF(40, 250), walls, bounds);

    QVERIFY(poly.size() >= 3);
    for(const QPointF& pt : poly)
    {
        QVERIFY(qIsFinite(pt.x()));
        QVERIFY(qIsFinite(pt.y()));
    }
}

void TestVisionCalculator::qLineFOverloadMatchesSegment()
{
    const QRectF bounds(0, 0, 100, 100);

    QList<VisionCalculator::Segment> segs;
    segs.append({QPointF(50, 20), QPointF(50, 80)});

    QList<QLineF> lines;
    lines.append(QLineF(50, 20, 50, 80));

    const QPolygonF polyA = VisionCalculator::computeVisionPolygon(
        QPointF(10, 50), segs, bounds);
    const QPolygonF polyB = VisionCalculator::computeVisionPolygon(
        QPointF(10, 50), lines, bounds);

    QCOMPARE(polyA.size(), polyB.size());
    for(int i = 0; i < polyA.size(); ++i)
    {
        QVERIFY(qAbs(polyA[i].x() - polyB[i].x()) < 1e-6);
        QVERIFY(qAbs(polyA[i].y() - polyB[i].y()) < 1e-6);
    }
}

QTEST_APPLESS_MAIN(TestVisionCalculator)
#include "tst_visioncalculator.moc"
