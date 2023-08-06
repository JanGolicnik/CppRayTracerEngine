#pragma once

#include <limits>
#include <core/core.h>

namespace MyPBRT {

inline bool HasNaNs(const glm::vec3& v) {
    return std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z);
}

struct Ray {
    glm::vec3 o;
    glm::vec3 d;
    mutable float tMax;

    Ray() :o(.0f), d(0,0,-1.0f), tMax(INFINITY) {}

    Ray(const glm::vec3& o, const glm::vec3& d, float tMax = INFINITY)
        : o(o), d(d), tMax(tMax) {}

    glm::vec3 operator()(float t) const { return o + d * t; }

    bool HasNaNs() const { return (MyPBRT::HasNaNs(o) || MyPBRT::HasNaNs(d) || std::isnan(tMax)); }
};


struct Bounds {
    glm::vec3 min, max;

    Bounds() : min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest()) {};
    Bounds(const glm::vec3& p1, const glm::vec3& p2)
        :min(glm::min(p1, p2)), max(glm::max(p1, p2)) {}

    const glm::vec3& operator[](int i) const { return (i == 0) ? min : max; }

    glm::vec3& operator[](int i) { return (i == 0) ? min : max; }

    bool operator==(const Bounds& b) const {
        return b.min == min && b.max == max;
    }

    bool operator!=(const Bounds& b) const {
        return b.min != min || b.max != max;
    }

    glm::vec3 Corner(int corner) const {
        return glm::vec3((*this)[(corner & 1)].x,
            (*this)[(corner & 2) ? 1 : 0].y,
            (*this)[(corner & 4) ? 1 : 0].z);
    }

    glm::vec3 Diagonal() const { return max - min; }

    int MaximumExtent() const {
        glm::vec3 d = Diagonal();
        if (d.x > d.y && d.x > d.z)
            return 0;
        else if (d.y > d.z)
            return 1;
        else
            return 2;
    }

    glm::vec3 Center() const {
        return (min + max) * 0.5f;
    }

    Bounds Union(const glm::vec3& point) {
        return Bounds(glm::min(min, point), glm::max(max, point));
    }

    Bounds Union(const Bounds& bounds) {
        return Bounds(glm::min(bounds.min, min), glm::max(bounds.max, max));
    }

    bool Inside(const glm::vec3& p) {
        return (p.x >= min.x && p.x < max.x &&
                p.y >= min.y && p.y < max.y &&
                p.z >= min.z && p.z < max.z);
    }

    float DistanceSquared(const glm::vec3& p) {
        float dx = std::max(min.x - p.x, p.x - max.x);
        float dy = std::max(min.y - p.y, p.y - max.y);
        float dz = std::max(min.z - p.z, p.z - max.z);
        return dx * dx + dy * dy + dz * dz;
    }

    float Distance(const glm::vec3& p) {
        return std::sqrt(DistanceSquared(p));
    }

    void Translate(const glm::vec3& vec) {
        min += vec;
        max += vec;
    }

    bool Intersect(const Ray& ray, float* h0 = nullptr, float* h1 = nullptr) const {
        float t0 = 0, t1 = ray.tMax;
        for (int i = 0; i < 3; ++i) {
            float invRayDir = 1 / ray.d[i];
            float tNear = (min[i] - ray.o[i]) * invRayDir;
            float tFar = (max[i] - ray.o[i]) * invRayDir;

            if (tNear > tFar) std::swap(tNear, tFar);

            t0 = tNear > t0 ? tNear : t0;
            t1 = tFar < t1 ? tFar : t1;
            if (t0 > t1) return false;
        }
        if (h0) *h0 = t0;
        if (h1) *h1 = t1;
        return true;
    }

    bool HasIntersections(const Ray& ray) const {
        float t0 = 0, t1 = ray.tMax;
        for (int i = 0; i < 3; ++i) {
            float invRayDir = 1 / ray.d[i];
            float tNear = (min[i] - ray.o[i]) * invRayDir;
            float tFar = (max[i] - ray.o[i]) * invRayDir;

            if (tNear > tFar) std::swap(tNear, tFar);

            t0 = tNear > t0 ? tNear : t0;
            t1 = tFar < t1 ? tFar : t1;
            if (t0 > t1) return false;
        }
        return true;

    }
    bool HasIntersections(const Ray& ray, const glm::vec3& invRayDir) const {
        float t0 = 0, t1 = ray.tMax;
        for (int i = 0; i < 3; ++i) {
            float tNear = (min[i] - ray.o[i]) * invRayDir[i];
            float tFar = (max[i] - ray.o[i]) * invRayDir[i];

            if (tNear > tFar) std::swap(tNear, tFar);

            t0 = tNear > t0 ? tNear : t0;
            t1 = tFar < t1 ? tFar : t1;
            if (t0 > t1) return false;
        }
        return true;

    }

    float Area() const {
        glm::vec3 sides = max - min;
        return 2.0f * (sides.x * sides.y + sides.x * sides.z + sides.y * sides.z);
    }
};

inline float MaxComponent(const glm::vec3& v) {
    return std::max(v.x, std::max(v.y, v.z));
}

inline int MaxDimension(const glm::vec3& v) {
    return (v.x > v.y) ? ((v.x > v.z) ? 0 : 2) : ((v.y > v.z) ? 1 : 2);
}

inline void CoordinateSystem(const glm::vec3& v1, glm::vec3* v2,
    glm::vec3* v3) {
    if (std::abs(v1.x) > std::abs(v1.y))
        *v2 = glm::vec3(-v1.z, 0, v1.x) / std::sqrt(v1.x * v1.x + v1.z * v1.z);
    else
        *v2 = glm::vec3(0, v1.z, -v1.y) / std::sqrt(v1.y * v1.y + v1.z * v1.z);
    *v3 = glm::cross(v1, *v2);
}


inline uint32_t pcg_hash(uint32_t input)
{
    uint32_t state = input * 747796405u + 2891336453u;
    uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

inline float RandomFloat(uint32_t* seed) {
    *seed = pcg_hash(*seed);
    return (float)*seed / std::numeric_limits<uint32_t>::max();
}

inline glm::vec3 RandomInHemisphere(uint32_t* seed) {
    return glm::vec3(RandomFloat(seed) * 2.0f - 1.0f,
        RandomFloat(seed) * 2.0f - 1.0f,
        RandomFloat(seed) * 2.0f - 1.0f);
}
inline glm::vec3 RandomInHemisphere(uint32_t seed) {
    return glm::vec3(RandomFloat(&seed) * 2.0f - 1.0f,
        RandomFloat(&seed) * 2.0f - 1.0f,
        RandomFloat(&seed) * 2.0f - 1.0f);
}

inline glm::vec3 Reflect(const glm::vec3& wo, const glm::vec3& n) {
    return -wo + 2 * glm::dot(wo, n) * n;
}

} //MyPBRT