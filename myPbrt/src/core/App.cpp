#include "App.h"

#include "imgui.h"
#include "imgui/imgui_stdlib.h"

#include "core.h"

#include "Object.h"
#include "Material.h"

#include "BVHAccelerator.h"

namespace MyPBRT {

    App::App() : camera(50, 0.01, 100, 0, 0), integrator(8, { 1200, 720 }, SCREENSCALE) {
		std::shared_ptr<MyPBRT::Texture> st(new ConstantTexture<glm::vec3>(glm::vec3(.75, .1, .1)));
		std::shared_ptr<MyPBRT::Material> sphereMaterial(new MyPBRT::DiffuseMaterial(st));
		scene.materials.push_back(sphereMaterial);
        std::shared_ptr<MyPBRT::Texture> st2(new ConstantTexture<float>(0));
		std::shared_ptr<MyPBRT::Material> sphereMaterial2(new MyPBRT::GlassMaterial(st2));
		scene.materials.push_back(sphereMaterial2);
	}

    App::~App() {}

    void App::RenderFrame(double dt, glm::ivec2 resolution)
    {
        afk_timer += dt;
        if (afk_timer > 1.5) {
            scene.Build();
            afk_timer = -300;
        }
        integrator.OnResize(resolution);
        scaled_resolution = integrator.ScaledResolution();
        camera.OnResize(scaled_resolution);
        if (camera.Update(dt)) {
            integrator.ResetFrameIndex();
            integrator.Clear();
        }
        integrator.Render(scene, camera);
    }

	uint32_t* App::GetImage()
	{
		return integrator.GetImage();
	}

    void App::MousePressed(int button)
    {
        afk_timer = 0;
        camera.MouseButtonCallback(button, true);
        
        if (button == 0) {
            
            if (!holding_lmouse) {
                SurfaceInteraction interaction;
                Ray ray = camera.GetMouseRay(normalized_mouse_position);
                scene.IntersectAccel(ray, &interaction);
                std::cout << interaction.uv.x << " " << interaction.uv.y << "\n";
                if(!holding_shift)
                    integrator.selected_objects.clear();
                if (interaction.primitive >= 0 && interaction.primitive < scene.primitives.size())
                    integrator.selected_objects.insert(interaction.primitive);
            }
            holding_lmouse = true;
        }
    }

    void App::MouseReleased(int button)
    {
        afk_timer = 0;
        camera.MouseButtonCallback(button, false);

        if (button == 0) holding_lmouse = false;
    }

    void App::ButtonPressed(int key)
	{
        afk_timer = 0;
        camera.ButtonCallback(key, true);
        if (key == 340) {
            holding_shift = true;
        }
        //if (key == 48) {
        //    uint32_t s = time(NULL);
        //    for (int i = 0; i < 4; i++) {
        //        std::shared_ptr<MyPBRT::Shape> ss(new MyPBRT::Sphere(RandomInHemisphere(time(NULL)) * 10.0f * RandomFloat(&s), RandomFloat(&s) * 2.0f));
        //        scene.AddObject(Object(ss, 0));
        //    }
        //}
        if (key == 84) {
            scene.Build();
        }
    }

	void App::ButtonReleased(int key)
	{
        afk_timer = 0;
        camera.ButtonCallback(key, false);
        if (key == 340) {
            holding_shift = false;
        }
    }

    void App::MouseMoved(double xpos, double ypos)
    {
        camera.MouseMotionCallback({ xpos, ypos });
    }

    void App::ScrollMoved(double offset)
    {
        camera.ScrollCallback(offset);
    }

	void App::CreateIMGUI()
	{
        IMGUISettings();
        IMGUIScene();
        IMGUIMaterials();
        IMGUIRendering();
        IMGUISelection();

	}
    void App::IMGUISettings()
    {
        ImGui::Begin("Settings");


        bool changed = ImGui::DragFloat("focal distance", &camera.GetFocalDistance(), 0.01, 0, std::numeric_limits<float>::max());
        changed |= ImGui::DragFloat("lens radius", &camera.GetLensRadius(), 0.01, 0, std::numeric_limits<float>::max());
        
        if (changed) {
            integrator.ResetFrameIndex();
        }
        
        ImGui::End();
    }
    void App::IMGUIScene()
    {
        ImGui::Begin("Scene");
        
        if (ImGui::CollapsingHeader("Add a sphere")) {
            ImGui::DragFloat3("Position", &next_sphere_pos[0], .1, -1000.0f, 1000.0f);
            ImGui::DragFloat("Radius", &next_sphere_radius, .1, -1000.0f, 1000.0f);
            if (ImGui::Button("Add")) {
                std::shared_ptr<MyPBRT::Shape> ss(new MyPBRT::Sphere(next_sphere_pos, next_sphere_radius));
                scene.AddObject(Object(ss, 0));
            }
        }
        if (ImGui::CollapsingHeader("Load a .obj")) {
            ImGui::InputText("Filename", &obj_file_to_load);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Model name only (ex. \"cube\"), file must be in the models folder");
            if (ImGui::Button("Load")) {
                scene.Load(obj_file_to_load);
            }
        }

        if (ImGui::CollapsingHeader("Primitives")) {
            for (int i = 0; i < scene.primitives.size(); i++) {
                ImGui::PushID(i);
                if (scene.primitives[i].CreateIMGUI(scene.materials)) {
                    scene.RecalculateObject(i);
                }
                if (ImGui::Button("Delete")) {
                    scene.RemoveObject(i);
                    i--;
                    integrator.selected_objects.clear();
                }
                ImGui::PopID();
            }
        }
        ImGui::End();
    }
    void App::IMGUIMaterials()
    {
        ImGui::Begin("Materials");
        {
            ImGui::Combo("type", &current_selected_material_option, material_options, IM_ARRAYSIZE(material_options));
            if (str_mat_opt[current_selected_material_option] == "Diffuse") {
                MyPBRT::DiffuseMaterial::IMGUI_Create(&scene);
            }
            else if (str_mat_opt[current_selected_material_option] == "Emissive") {
                MyPBRT::EmissiveMaterial::IMGUI_Create(&scene);
            }
            else if (str_mat_opt[current_selected_material_option] == "Metallic") {
                MyPBRT::MetallicMaterial::IMGUI_Create(&scene);
            }
            else if (str_mat_opt[current_selected_material_option] == "Glass") {
                MyPBRT::GlassMaterial::IMGUI_Create(&scene);
            }
        }
        for (int i = 0; i < scene.materials.size(); i++) {
            ImGui::PushID(i);
            std::string text = "Material " + std::to_string(i);
            ImGui::Text(text.c_str());
            scene.materials[i]->IMGUI_Edit();
            if (ImGui::Button("Delete")) {
                scene.materials.erase(scene.materials.begin() + i);
                for (int j = 0; j < scene.primitives.size(); j++) {
                    int* mat = &scene.primitives[j].material;
                    if (*mat == i) *mat = 0; else
                        if (*mat > i) (*mat)--;
                }
                i--;
            }
            ImGui::PopID();
        }
        ImGui::End();
    }
    void App::IMGUIRendering()
    {
        ImGui::Begin("Rendering");
        integrator.CreateIMGUI();
        ImGui::End();
    }
    void App::IMGUISelection() {

        static glm::quat rotation(1.0f, 0.0f, 0.0f, 0.0f);
        if (integrator.selected_objects.size() == 0) {
            rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            return;
        }
        if (integrator.selected_objects.size() == 1) {
            ImGui::Begin("Selection");
            int obj = *integrator.selected_objects.begin();
            if (scene.primitives[obj].CreateIMGUI(scene.materials)) {
                scene.RecalculateObject(obj);
            }
            if (ImGui::Button("Delete")) {
                scene.RemoveObject(obj);
                integrator.selected_objects.clear();
            }
            ImGui::End();
            return;
        }

        glm::vec3 center(0.0f);
        for (auto& object_id : integrator.selected_objects) {
            Object* object = &scene.primitives[object_id];
            center += object->shape->Center();
        }
        center /= (float)integrator.selected_objects.size();

        glm::vec3 center_offset = center;
        glm::quat rotation_offset = rotation;
        glm::vec3 rotation_offset_euler = glm::degrees(glm::eulerAngles(rotation_offset));

        bool changed = false;

        ImGui::Begin("Selection");
        changed |= ImGui::DragFloat3("position", glm::value_ptr(center_offset), 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
        changed |= ImGui::DragFloat3("rotation", glm::value_ptr(rotation_offset_euler), 0.1, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
        ImGui::End();

        if (rotation_offset_euler.y < -90) {
            rotation_offset_euler.x -= 180;
            rotation_offset_euler.z -= 180;
        }
        else if (rotation_offset_euler.y > 90) {
            rotation_offset_euler.x += 180;
            rotation_offset_euler.z += 180;
        }

        if (!changed) return;

        rotation_offset = glm::quat(glm::radians(rotation_offset_euler));

        glm::vec3 center_difference = center_offset - center;
        glm::quat rotation_difference = glm::inverse(rotation) * rotation_offset;
        rotation = rotation_offset;

        for (auto& object_index : integrator.selected_objects) {
            Object* object = &scene.primitives[object_index];
            object->shape->Translate(center_difference);
            object->shape->RotateAround(rotation_difference, center_offset);
            scene.RecalculateObject(object_index);
            std::cout << "recalculated " << object_index << "\n";
        }

    }
}


