#ifndef VISIONCALCULATOR_H
#define VISIONCALCULATOR_H

#include <QList>
#include <QLineF>
#include <QPointF>
#include <QPolygonF>
#include <QRectF>

/*
 * VisionCalculator - wall-based line-of-sight polygon builder.
 *
 * Given a viewpoint, a list of occluding wall segments, and a bounding
 * rectangle, computeVisionPolygon() returns the polygon of points the
 * viewpoint can see. The algorithm casts rays at each wall endpoint
 * (plus two small angular offsets to "peek around" corners) and at the
 * four corners of the bounding rectangle, finds the nearest intersection
 * along each ray, and returns the resulting points sorted by angle.
 *
 * Optionally, a maxRadius > 0 clips the polygon to a circle approximation
 * representing vision range (e.g. a torch's 40ft radius).
 *
 * The output polygon is in the same coordinate space as the input walls;
 * the caller is responsible for rendering (e.g. as a clip path or
 * QGraphicsPolygonItem on top of the fog-of-war layer).
 */
class VisionCalculator
{
public:
    struct Segment
    {
        QPointF a;
        QPointF b;
    };

    // Clip bounds may be any convex rectangle containing the viewpoint.
    // If maxRadius <= 0, no radial clipping is applied.
    // If circleSteps < 8, a reasonable default is used for the radial arc.
    static QPolygonF computeVisionPolygon(const QPointF& viewpoint,
                                          const QList<Segment>& walls,
                                          const QRectF& bounds,
                                          qreal maxRadius = 0.0,
                                          int circleSteps = 64);

    // Convenience overload for QLineF-based wall lists (matches LayerWalls).
    static QPolygonF computeVisionPolygon(const QPointF& viewpoint,
                                          const QList<QLineF>& walls,
                                          const QRectF& bounds,
                                          qreal maxRadius = 0.0,
                                          int circleSteps = 64);

    // Segment-segment intersection. Returns true and writes the intersection
    // point if the two segments cross (inclusive of endpoints).
    static bool segmentIntersection(const QPointF& p0, const QPointF& p1,
                                    const QPointF& q0, const QPointF& q1,
                                    QPointF* outPoint = nullptr);

    // Cast a ray from origin in the given direction. Returns the closest
    // hit point against the list of walls and the four sides of bounds.
    // Bounds are treated as occluders of last resort so rays always hit.
    static QPointF castRay(const QPointF& origin, const QPointF& direction,
                           const QList<Segment>& walls, const QRectF& bounds);
};

#endif // VISIONCALCULATOR_H
