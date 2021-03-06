#include "Ray3f.h"

namespace rwe
{
    Ray3f::Ray3f(const Vector3f& position, const Vector3f& direction) : origin(position), direction(direction) {}

    Vector3f Ray3f::pointAt(const float t) const
    {
        return origin + (direction * t);
    }

    bool Ray3f::isLessFar(const Vector3f& a, const Vector3f& b) const
    {
        return (a - origin).dot(direction) < (b - origin).dot(direction);
    }

    Ray3f Ray3f::fromLine(const Line3f& line)
    {
        return Ray3f(line.start, line.end - line.start);
    }

    Line3f Ray3f::toLine() const
    {
        return Line3f(origin, pointAt(1.0f));
    }
}
