#include "Interaction.h"

namespace MyPBRT {
    SurfaceInteraction::SurfaceInteraction(const glm::vec3& p, const glm::vec3& normal, const glm::vec2& uv, const glm::vec3& wo, const Shape* shape) 
        : Interaction(p, normal, wo), uv(uv), shape(shape) {
    }
}