#pragma once

#include "core.h"
#include "BaseTypes.h"

namespace MyPBRT {

    struct Object {
        std::shared_ptr<Shape> shape;
        int material;
        
        Object(std::shared_ptr<Shape> shape, int material);

        bool Intersect(const Ray& r, SurfaceInteraction* isect) const;
        bool hasIntersections(const Ray& r) const;
        void Preprocess() const;
        void DrawLines(const glm::vec2& resolution, const Camera& camera, const glm::vec3& color, IntegratorSetPixelFunctionPtr set_function) const;
        
        //true if scene should update
        bool CreateIMGUI(const std::vector<std::shared_ptr<Material>>& materials);
    };

}

