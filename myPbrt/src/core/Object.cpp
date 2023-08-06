#include "Object.h"

#include "Interaction.h"
#include "Material.h"
#include "Scene.h"
#include "Shape.h"
#include <imgui.h>

namespace MyPBRT {

    Object::Object(std::shared_ptr<Shape> shape, int material)
        : shape(shape), material(material) {}

    bool Object::hasIntersections(const Ray& r) const { return shape->hasIntersections(r); }

    bool Object::Intersect(const Ray& r, SurfaceInteraction* interaction) const {
        return shape->Intersect(r, interaction);
    }

    void Object::Preprocess() const { shape->Preprocess(); };

    void Object::DrawLines(const glm::vec2& resolution, const Camera& camera, const glm::vec3& color, IntegratorSetPixelFunctionPtr set_function) const
    {
        shape->DrawLines(resolution, camera, color, set_function);
    }
    bool Object::CreateIMGUI(const std::vector<std::shared_ptr<Material>>& materials)
    {
        ImGui::DragInt("material", &material, 1, 0, materials.size()-1);
        return shape->CreateIMGUI(materials);
    }

}
