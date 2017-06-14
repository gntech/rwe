#include "Vector3f.h"
#include <cmath>

namespace rwe
{
    Vector3f::Vector3f(float x, float y, float z) : x(x), y(y), z(z) {}

    Vector3f& Vector3f::operator+=(const Vector3f& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    Vector3f& Vector3f::operator-=(const Vector3f& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    Vector3f& Vector3f::operator*=(float v)
    {
        x *= v;
        y *= v;
        z *= v;
        return *this;
    }

    Vector3f& Vector3f::operator/=(float v)
    {
        x /= v;
        y /= v;
        z /= v;
        return *this;
    }

    float Vector3f::lengthSquared() const
    {
        return (x * x) + (y * y) + (z * z);
    }

    float Vector3f::length() const
    {
        return std::sqrt(lengthSquared());
    }

    float Vector3f::distanceSquared(const Vector3f& rhs)
    {
        return (rhs - *this).lengthSquared();
    }

    float Vector3f::distance(const Vector3f& rhs)
    {
        return (rhs - *this).length();
    }

    void Vector3f::normalize()
    {
        float n = length();
        if (n == 0.0f)
        {
            throw std::logic_error("Attempted to normalize a zero-length rwe_math");
        }

        x /= n;
        y /= n;
        z /= n;
    }

    Vector3f Vector3f::normalized()
    {
        float n = length();
        if (n == 0.0f)
        {
            throw std::logic_error("Attempted to normalize a zero-length rwe_math");
        }

        return Vector3f(x / n, y / n, z / n);
    }

    float Vector3f::dot(const Vector3f& rhs) const
    {
        return (x * rhs.x) + (y * rhs.y) + (z * rhs.z);
    }

    Vector3f Vector3f::cross(const Vector3f& rhs) const
    {
        return Vector3f(
            (y * rhs.z) - (z * rhs.y),
            (z * rhs.x) - (x * rhs.z),
            (x * rhs.y) - (y * rhs.x));
    }

    Vector3f operator+(const Vector3f& lhs, const Vector3f& rhs)
    {
        return Vector3f(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
    }

    Vector3f operator-(const Vector3f& lhs, const Vector3f& rhs)
    {
        return Vector3f(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
    }

    Vector3f operator*(const Vector3f& lhs, float rhs)
    {
        return Vector3f(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);
    }

    Vector3f operator/(const Vector3f& lhs, float rhs)
    {
        return Vector3f(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);
    }

    std::ostream& operator<<(std::ostream& lhs, const Vector3f& rhs)
    {
        lhs << "(" << rhs.x << ", " << rhs.y << ", " << rhs.z << ")";
        return lhs;
    }

    float scalarTriple(const Vector3f& a, const Vector3f& b, const Vector3f& c)
    {
        return a.cross(b).dot(c);
    }

    const boost::optional<Vector3f>& closestTo(
        const Vector3f& v,
        const boost::optional<Vector3f>& a,
        const boost::optional<Vector3f>& b)
    {
        if (!a)
        {
            return b;
        }

        if (!b)
        {
            return a;
        }

        if ((*a - v).lengthSquared() < (*b - v).lengthSquared())
        {
            return a;
        }

        return b;
    }

    bool isCloserTo(const Vector3f& v, const Vector3f& a, const Vector3f& b)
    {
        return (a - v).lengthSquared() < (b - v).lengthSquared();
    }
}
