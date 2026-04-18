#include "visioncalculator.h"
#include <QtMath>
#include <algorithm>
#include <limits>

namespace
{
    constexpr qreal kEpsilon = 1e-9;
    constexpr qreal kAngleOffset = 1e-4; // radians; small peek around corners

    struct RayHit
    {
        QPointF point;
        qreal angle;
    };

    QList<VisionCalculator::Segment> boundsAsSegments(const QRectF& bounds)
    {
        const QPointF tl = bounds.topLeft();
        const QPointF tr = bounds.topRight();
        const QPointF br = bounds.bottomRight();
        const QPointF bl = bounds.bottomLeft();
        return {
            {tl, tr},
            {tr, br},
            {br, bl},
            {bl, tl}
        };
    }
}

bool VisionCalculator::segmentIntersection(const QPointF& p0, const QPointF& p1,
                                           const QPointF& q0, const QPointF& q1,
                                           QPointF* outPoint)
{
    const qreal r_x = p1.x() - p0.x();
    const qreal r_y = p1.y() - p0.y();
    const qreal s_x = q1.x() - q0.x();
    const qreal s_y = q1.y() - q0.y();

    const qreal denom = r_x * s_y - r_y * s_x;
    if(qAbs(denom) < kEpsilon)
        return false; // parallel or collinear

    const qreal dx = q0.x() - p0.x();
    const qreal dy = q0.y() - p0.y();

    const qreal t = (dx * s_y - dy * s_x) / denom;
    const qreal u = (dx * r_y - dy * r_x) / denom;

    if(t < -kEpsilon || t > 1.0 + kEpsilon)
        return false;
    if(u < -kEpsilon || u > 1.0 + kEpsilon)
        return false;

    if(outPoint)
        *outPoint = QPointF(p0.x() + t * r_x, p0.y() + t * r_y);

    return true;
}

QPointF VisionCalculator::castRay(const QPointF& origin, const QPointF& direction,
                                  const QList<Segment>& walls, const QRectF& bounds)
{
    // Build a far endpoint along the ray direction, clamped well outside bounds.
    qreal len = std::hypot(direction.x(), direction.y());
    if(len < kEpsilon)
        return origin;

    const qreal diag = std::hypot(bounds.width(), bounds.height());
    const qreal reach = diag + std::hypot(origin.x() - bounds.center().x(),
                                          origin.y() - bounds.center().y()) + 1.0;

    const QPointF unit(direction.x() / len, direction.y() / len);
    const QPointF far(origin.x() + unit.x() * reach,
                      origin.y() + unit.y() * reach);

    QPointF bestHit = far;
    qreal bestDistSq = std::numeric_limits<qreal>::max();
    bool haveHit = false;

    const auto considerHit = [&](const QPointF& hit) {
        const qreal dx = hit.x() - origin.x();
        const qreal dy = hit.y() - origin.y();
        const qreal distSq = dx * dx + dy * dy;
        if(distSq < bestDistSq)
        {
            bestDistSq = distSq;
            bestHit = hit;
            haveHit = true;
        }
    };

    for(const Segment& seg : walls)
    {
        QPointF hit;
        if(segmentIntersection(origin, far, seg.a, seg.b, &hit))
            considerHit(hit);
    }

    const QList<Segment> boundSegs = boundsAsSegments(bounds);
    for(const Segment& seg : boundSegs)
    {
        QPointF hit;
        if(segmentIntersection(origin, far, seg.a, seg.b, &hit))
            considerHit(hit);
    }

    return haveHit ? bestHit : far;
}

QPolygonF VisionCalculator::computeVisionPolygon(const QPointF& viewpoint,
                                                 const QList<Segment>& walls,
                                                 const QRectF& bounds,
                                                 qreal maxRadius,
                                                 int circleSteps)
{
    if(!bounds.isValid() || bounds.isEmpty())
        return QPolygonF();

    // Collect candidate angles: corners of bounds, each wall endpoint,
    // plus flanking angles to peek around corners.
    QList<qreal> angles;
    angles.reserve(walls.size() * 6 + 8);

    const auto addAngle = [&](const QPointF& target) {
        const qreal dx = target.x() - viewpoint.x();
        const qreal dy = target.y() - viewpoint.y();
        if(qAbs(dx) < kEpsilon && qAbs(dy) < kEpsilon)
            return;
        const qreal a = std::atan2(dy, dx);
        angles.append(a);
        angles.append(a + kAngleOffset);
        angles.append(a - kAngleOffset);
    };

    addAngle(bounds.topLeft());
    addAngle(bounds.topRight());
    addAngle(bounds.bottomRight());
    addAngle(bounds.bottomLeft());

    for(const Segment& seg : walls)
    {
        addAngle(seg.a);
        addAngle(seg.b);
    }

    // If vision is radius-limited, add evenly spaced angles so the clipped
    // circle stays smooth.
    if(maxRadius > 0.0)
    {
        const int steps = std::max(circleSteps, 8);
        for(int i = 0; i < steps; ++i)
            angles.append((2.0 * M_PI * i) / steps);
    }

    // Cast rays and collect hits.
    QList<RayHit> hits;
    hits.reserve(angles.size());

    for(qreal a : angles)
    {
        QPointF dir(std::cos(a), std::sin(a));
        QPointF hit = castRay(viewpoint, dir, walls, bounds);

        // Clip to maxRadius if requested.
        if(maxRadius > 0.0)
        {
            const qreal dx = hit.x() - viewpoint.x();
            const qreal dy = hit.y() - viewpoint.y();
            const qreal dist = std::hypot(dx, dy);
            if(dist > maxRadius)
            {
                hit = QPointF(viewpoint.x() + dir.x() * maxRadius,
                              viewpoint.y() + dir.y() * maxRadius);
            }
        }

        RayHit rh;
        rh.point = hit;
        rh.angle = a;
        hits.append(rh);
    }

    std::sort(hits.begin(), hits.end(),
              [](const RayHit& lhs, const RayHit& rhs) {
                  return lhs.angle < rhs.angle;
              });

    QPolygonF polygon;
    polygon.reserve(hits.size());
    for(const RayHit& h : hits)
        polygon.append(h.point);

    return polygon;
}

QPolygonF VisionCalculator::computeVisionPolygon(const QPointF& viewpoint,
                                                 const QList<QLineF>& walls,
                                                 const QRectF& bounds,
                                                 qreal maxRadius,
                                                 int circleSteps)
{
    QList<Segment> segs;
    segs.reserve(walls.size());
    for(const QLineF& line : walls)
        segs.append({line.p1(), line.p2()});
    return computeVisionPolygon(viewpoint, segs, bounds, maxRadius, circleSteps);
}
