#include "Object.h"

#include "Interaction.h"
#include "Material.h"
#include "Scene.h"
#include <imgui.h>

namespace MyPBRT {

    Object::Object(int _shape, int _material)
        : shape(_shape), material(_material) {}

    bool Object::CreateIMGUI(const std::vector<std::shared_ptr<Material>>& materials)
    {
        return ImGui::DragInt("material", &material, 1, 0, materials.size()-1);
    }

}
