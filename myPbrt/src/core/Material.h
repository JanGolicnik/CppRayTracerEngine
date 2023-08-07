#pragma once

#include "core.h"
#include "Interaction.h"
#include "Texture.h"

namespace MyPBRT{

    class Material {
    public:
        bool has_pdf = false;
    public:
        virtual glm::vec3 Evaluate(SurfaceInteraction* interaction) const = 0;
        virtual glm::vec3 EvaluateLight(const SurfaceInteraction& interaction) const { return glm::vec3(0.0f); };
        virtual bool ScatterRay(const SurfaceInteraction& interaction, glm::vec3& dir) const = 0;
        virtual void IMGUI_Edit() = 0;
        virtual float Pdf_Value(const glm::vec3& incoming, const glm::vec3& normal) const = 0;
    };

    class EmissiveMaterial : public Material {
    public:
        glm::vec3 emission = glm::vec3(0.0f);
        std::shared_ptr<Texture> texture;

    public:
        EmissiveMaterial(std::shared_ptr<Texture> _texture, glm::vec3 _emission = glm::vec3(0.0f)) :texture(_texture), emission(_emission) { }
        glm::vec3 EvaluateLight(const SurfaceInteraction& interaction) const;
        glm::vec3 Evaluate(SurfaceInteraction* interaction) const;
        bool ScatterRay(const SurfaceInteraction& interaction, glm::vec3& dir) const;
        void IMGUI_Edit();
        static void IMGUI_Create(Scene* scene);
        float Pdf_Value(const glm::vec3& incoming, const glm::vec3& normal) const { return 0; }

    private:
        static const std::vector<Texture::TextureType> selectable_texture_types;
        static int selected_texture;
    };

    class DiffuseMaterial : public Material{
    public:
        float smoothness = 0.0f;
        std::shared_ptr<Texture> texture;

    public:
        DiffuseMaterial(std::shared_ptr<Texture> _texture, float _smoothness = 0.0f) :texture(_texture), smoothness(_smoothness) { has_pdf = true; }
        glm::vec3 Evaluate(SurfaceInteraction* interaction) const;
        bool ScatterRay(const SurfaceInteraction& interaction, glm::vec3& dir) const;
        void IMGUI_Edit();
        static void IMGUI_Create(Scene* scene);
        float Pdf_Value(const glm::vec3& incoming, const glm::vec3& normal) const;

    private:
        static const std::vector<Texture::TextureType> selectable_texture_types;
        static int selected_texture;
    };

    class MetallicMaterial : public Material {
    public:
        float roughness = 0.0f;
        std::shared_ptr<Texture> texture;

    public:
        MetallicMaterial(std::shared_ptr<Texture> _texture, float _roughness = 0.0f) :texture(_texture), roughness(_roughness) {}
        glm::vec3 Evaluate(SurfaceInteraction* interaction) const;
        bool ScatterRay(const SurfaceInteraction& interaction, glm::vec3& dir) const;
        void IMGUI_Edit();
        static void IMGUI_Create(Scene* scene);
        float Pdf_Value(const glm::vec3& incoming, const glm::vec3& normal) const { return 0; }

    private:
        static const  std::vector<Texture::TextureType> selectable_texture_types;
        static int selected_texture;
    };

    class GlassMaterial : public Material {
    public:
        float ior = 1.0f;
        std::shared_ptr<Texture> texture;
        std::shared_ptr<Texture> roughness_map;

    public:
        GlassMaterial(std::shared_ptr<Texture> _roughness_map, float _ior = 1.0f) : roughness_map(_roughness_map), ior(_ior) {}
        glm::vec3 Evaluate(SurfaceInteraction* interaction) const;
        bool ScatterRay(const SurfaceInteraction& interaction, glm::vec3& dir) const;
        void IMGUI_Edit();
        static void IMGUI_Create(Scene* scene);
        float Pdf_Value(const glm::vec3& incoming, const glm::vec3& normal) const { return 0; }

    private:
        static const std::vector<Texture::TextureType> selectable_roughness_map_types;
        static int selected_texture;

    private:
        static double schlick_reflectance(double cosine, double ref_idx) {
            double r0 = (1.0 - ref_idx) / (1.0 + ref_idx);
            r0 = r0 * r0;
            return r0 + (1.0 - r0) * pow((1.0 - cosine), 5);
        }
    };

}

