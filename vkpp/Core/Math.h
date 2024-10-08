#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace vkpp
{

// Aixs-Aligned Bounding Box (AABB)
// https://github.com/mmp/pbrt-v4/blob/master/src/pbrt/util/vecmath.h
class AABB
{
public:
    glm::vec3 m_min;
    glm::vec3 m_max;

    AABB()
    {
        SetEmpty();
    }

    explicit AABB(const glm::vec3& p) :
        m_min(p),
        m_max(p)
    {}

    AABB(const glm::vec3& p1, const glm::vec3& p2) :
        m_min(glm::min(p1, p2)),
        m_max(glm::max(p1, p2))
    {}

    template<typename U>
    explicit AABB(const AABB& b)
    {
        if (!b.IsEmpty())
        {
            m_min = glm::vec3(b.m_min);
            m_max = glm::vec3(b.m_max);
        }
        else
        {
            SetEmpty();
        }
    }

    void SetEmpty()
    {
        constexpr float minValue = std::numeric_limits<float>::lowest();
        constexpr float maxValue = std::numeric_limits<float>::max();
        m_min = glm::vec3(maxValue, maxValue, maxValue);
        m_max = glm::vec3(minValue, minValue, minValue);
    }

    bool IsEmpty() const
    {
        return ((m_min.x >= m_max.x) || (m_min.y >= m_max.y) || (m_min.z >= m_max.z));
    }

    bool IsDegenerate() const
    {
        return ((m_min.x > m_max.x) || (m_min.y > m_max.y) || (m_min.z > m_max.z));
    }

    const glm::vec3& operator[](size_t i) const
    {
        assert(i == 0 || i == 1);
        const auto bounds = reinterpret_cast<glm::vec3*>(const_cast<AABB*>(this));
        return bounds[i];
    }

    glm::vec3& operator[](size_t i)
    {
        assert(i == 0 || i == 1);
        const auto bounds = reinterpret_cast<glm::vec3*>(this);
        return bounds[i];
    }

    glm::vec3 GetCorner(int index) const
    {
        assert(index >= 0 && index < 8);
        return glm::vec3(
            (*this)[(index & 1)].x,
            (*this)[(index & 2) ? 1 : 0].y,
            (*this)[(index & 4) ? 1 : 0].z);
    }

    glm::vec3 GetDiagonal() const
    {
        return (m_max - m_min);
    }

    float GetSurfaceArea() const
    {
        glm::vec3 d = GetDiagonal();
        return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
    }

    float GetVolume() const
    {
        glm::vec3 d = GetDiagonal();
        return d.x * d.y * d.z;
    }

    glm::vec3 Lerp(const glm::vec3& t) const
    {
        return glm::mix(m_min, m_max, t);
    }

    bool operator==(const AABB& b) const
    {
        return ((m_min == b.m_min) && (m_max == b.m_max));
    }

    bool operator!=(const AABB& b) const
    {
        return ((m_min != b.m_min) || (m_max != b.m_max));
    }

}; // class AABB

template<typename float>
inline AABB Unite(const AABB& b, const glm::vec3& p)
{
    AABB ret;
    ret.m_min = glm::min(b.m_min, p);
    ret.m_max = glm::max(b.m_max, p);
    return ret;
}

template<typename float>
inline AABB Unite(const AABB& b1, const AABB& b2)
{
    AABB ret;
    ret.m_min = glm::min(b1.m_min, b2.m_min);
    ret.m_max = glm::max(b1.m_max, b2.m_max);
    return ret;
}

template<typename float>
inline AABB Intersect(const AABB& b1, const AABB& b2)
{
    AABB ret;
    ret.m_min = glm::max(b1.m_min, b2.m_min);
    ret.m_max = glm::min(b1.m_max, b2.m_max);
    return ret;
}

template<typename float>
inline bool HasOverlap(const AABB& b1, const AABB& b2)
{
    bool x = (b1.m_max.x >= b2.m_min.x) && (b1.m_min.x <= b2.m_max.x);
    bool y = (b1.m_max.y >= b2.m_min.y) && (b1.m_min.y <= b2.m_max.y);
    bool z = (b1.m_max.z >= b2.m_min.z) && (b1.m_min.z <= b2.m_max.z);
    return (x && y && z);
}

template<typename float>
inline bool IsInside(const glm::vec3& p, const AABB& b)
{
    return (
        (p.x >= b.m_min.x) && (p.x <= b.m_max.x) &&
        (p.y >= b.m_min.y) && (p.y <= b.m_max.y) &&
        (p.z >= b.m_min.z) && (p.z <= b.m_max.z));
}

template<typename float>
inline bool IsInsideExclusive(const glm::vec3& p, const AABB& b)
{
    return (
        (p.x >= b.m_min.x) && (p.x < b.m_max.x) &&
        (p.y >= b.m_min.y) && (p.y < b.m_max.y) &&
        (p.z >= b.m_min.z) && (p.z < b.m_max.z));
}

template<typename float, typename U>
inline AABB Expand(const AABB& b, U delta)
{
    AABB ret;
    ret.m_min = b.m_min - glm::vec3(delta, delta, delta);
    ret.m_max = b.m_max + glm::vec3(delta, delta, delta);
    return ret;
}

} // namespace vkpp
