#include "Material.h"

#include <imgui.h>
#include "Scene.h"

namespace MyPBRT {

	const std::vector<Texture::TextureType> DiffuseMaterial::selectable_texture_types = { Texture::TextureType::ConstantColor, Texture::TextureType::Checkerboard, Texture::TextureType::UV, Texture::TextureType::Image };
	const std::vector<Texture::TextureType> EmissiveMaterial::selectable_texture_types = { Texture::TextureType::ConstantColor, Texture::TextureType::Checkerboard, Texture::TextureType::UV, Texture::TextureType::Image };
	const std::vector<Texture::TextureType> MetallicMaterial::selectable_texture_types = { Texture::TextureType::ConstantColor, Texture::TextureType::Checkerboard, Texture::TextureType::UV, Texture::TextureType::Image };
	const std::vector<Texture::TextureType> GlassMaterial::selectable_roughness_map_types = { Texture::TextureType::Image, Texture::TextureType::ConstantValue };
	int DiffuseMaterial::selected_texture = 0;
	int EmissiveMaterial::selected_texture = 0;
	int MetallicMaterial::selected_texture = 0;
	int GlassMaterial::selected_texture = 0;

	glm::vec3 DiffuseMaterial::Evaluate(SurfaceInteraction* interaction, const glm::vec3& wo, const glm::vec3& wi, glm::vec3* light) const
	{
		if (texture) {
			return texture->Evaluate(*interaction);
		}
		return glm::vec3(1, 0, 1);
	}

	bool DiffuseMaterial::ScatterRay(const SurfaceInteraction& interaction, Ray* ray) const
	{
		glm::vec3 scattered = random_in_unit_sphere();
		glm::vec3 reflected = glm::reflect(glm::normalize(ray->d), interaction.normal);
		ray->d = (1.0f - smoothness) * scattered +  smoothness * reflected;
		if (glm::dot(ray->d, interaction.normal) < 0) {
			ray->d = -ray->d;
		}
		return true;
	}

	void DiffuseMaterial::IMGUI_Edit()
	{
		ImGui::DragFloat("smoothness", &smoothness, .1, 0, 1);
		Texture::CreateTextureFromMenuFull(&selected_texture, &texture, selectable_texture_types);
	}

	void DiffuseMaterial::IMGUI_Create(Scene* scene)
	{
		static float smoothness = 0.0f;
		ImGui::DragFloat("smoothness", &smoothness, 0.01f, 0, 1);
		Texture::CreationMenuImGUI(&selected_texture, selectable_texture_types);
		if (ImGui::Button("Add")) {
			smoothness = 0.0f;
			std::shared_ptr<Texture> tex = Texture::CreateTexture();
			if (tex.get() != nullptr) {
				std::shared_ptr<Material> mat(new DiffuseMaterial(tex, smoothness));
				scene->materials.push_back(mat);
			}
		}
	}

	glm::vec3 EmissiveMaterial::Evaluate(SurfaceInteraction* interaction, const glm::vec3& wo, const glm::vec3& wi, glm::vec3* light) const
	{
		if (texture) {
			glm::vec4 tex = texture->Evaluate(*interaction);
			*light += emission * glm::vec3(tex.x, tex.y, tex.z);
		}
		return glm::vec3(1.0f);
	}

	bool EmissiveMaterial::ScatterRay(const SurfaceInteraction& interaction, Ray* ray) const
	{
		return false;
	}

	void EmissiveMaterial::IMGUI_Edit()
	{
		ImGui::ColorEdit3("emission", glm::value_ptr(emission));
		texture->CreateIMGUI();
	}

	void EmissiveMaterial::IMGUI_Create(Scene* scene)
	{
		static glm::vec3 emissivness(0.0f);
		ImGui::ColorEdit3("emissivness", glm::value_ptr(emissivness));
		Texture::CreationMenuImGUI(&selected_texture, selectable_texture_types);
		if (ImGui::Button("Add")) {
			emissivness = glm::vec3(0.0f);
			std::shared_ptr<Texture> tex = Texture::CreateTexture();
			if (tex.get() != nullptr) {
				std::shared_ptr<Material> mat(new EmissiveMaterial(tex, emissivness));
				scene->materials.push_back(mat);
			}
		}
	}

	glm::vec3 MetallicMaterial::Evaluate(SurfaceInteraction* interaction, const glm::vec3& wo, const glm::vec3& wi, glm::vec3* light) const
	{
		if (texture) {
			return texture->Evaluate(*interaction);
		}
		return glm::vec3(1, 0, 1);
	}

	bool MetallicMaterial::ScatterRay(const SurfaceInteraction& interaction, Ray* ray) const
	{
		glm::vec3 scattered = roughness * random_in_unit_sphere();
		glm::vec3 reflected = glm::reflect(glm::normalize(ray->d), interaction.normal);
		ray->d = glm::normalize(scattered + reflected);
		return true;
	}

	void MetallicMaterial::IMGUI_Edit()
	{
		ImGui::DragFloat("roughness", &roughness, 0.01, 0, 1);
		texture->CreateIMGUI();
	}

	void MetallicMaterial::IMGUI_Create(Scene* scene)
	{
		static float roughess(0.0f);
		ImGui::DragFloat("roughess", &roughess, 0.01f, 0, 1);
		Texture::CreationMenuImGUI(&selected_texture, selectable_texture_types);
		if (ImGui::Button("Add")) {
			roughess = 0.0f;
			std::shared_ptr<Texture> tex = Texture::CreateTexture();
			if (tex.get() != nullptr) {
				std::shared_ptr<Material> mat(new MetallicMaterial(tex, roughess));
				scene->materials.push_back(mat);
			}
		}
	}

	glm::vec3 GlassMaterial::Evaluate(SurfaceInteraction* interaction, const glm::vec3& wo, const glm::vec3& wi, glm::vec3* light) const
	{
		return glm::vec3(1.0f);
	}

	double FresnelFactor(const glm::vec3& incoming, const glm::vec3& normal, double ior1, double ior2) {
		double cos_theta_i = glm::dot(incoming, normal);

		bool entering = cos_theta_i > 0.f;
		if (!entering) {
			std::swap(ior1, ior2);
			cos_theta_i = std::abs(cos_theta_i);
		}

		double refraction_ratio = ior1 / ior2;

		double sin_theta_t = refraction_ratio * sqrt(std::max(0.0, 1.0 - cos_theta_i * cos_theta_i));

		double cos_theta_t = sqrt(std::max(0.0, 1.0 - sin_theta_t * sin_theta_t));

		double R_parallel = (ior2 * cos_theta_i - ior1 * cos_theta_t) / (ior2 * cos_theta_i + ior1 * cos_theta_t);
		double R_perpendicular = (ior1 * cos_theta_i - ior2 * cos_theta_t) / (ior1 * cos_theta_i + ior2 * cos_theta_t);

		double Fresnel = 0.5 * (R_parallel * R_parallel + R_perpendicular * R_perpendicular);

		return Fresnel;
	}

	bool GlassMaterial::ScatterRay(const SurfaceInteraction& interaction, Ray* ray) const
	{
		glm::vec3 normal = interaction.normal;
		glm::vec3 unit_direction = glm::normalize(ray->d);
		float refraction_ratio = glm::dot(unit_direction, normal) < 0 ? (1.0 / ior) : ior;

		double cos_theta = glm::abs(glm::dot(-unit_direction, normal));
		double reflectance = schlick_reflectance(cos_theta, refraction_ratio);

		float roughness = roughness_map->Evaluate(interaction).x;

		glm::vec3 scattered = roughness * random_in_unit_sphere();
		normal = glm::normalize(normal + scattered);
		double rand = random_double_bad();

		if (reflectance > rand)
			ray->d = glm::reflect(unit_direction, normal);
		else
			ray->d = glm::refract(unit_direction, normal, refraction_ratio);

		return true;
	}

	void GlassMaterial::IMGUI_Edit()
	{
		ImGui::DragFloat("ior", &ior, 0.01, 0, std::numeric_limits<float>::max());
		roughness_map->CreateIMGUI();
	}

	void GlassMaterial::IMGUI_Create(Scene* scene)
	{
		static float ior = 1.49f;
		ImGui::DragFloat("ior", &ior, 0.01f, 0, 1);
		Texture::CreationMenuImGUI(&selected_texture, selectable_roughness_map_types);
		if (ImGui::Button("Add")) {
			ior = 1.49f;
			std::shared_ptr<Texture> roughnesstex = Texture::CreateTexture();
			if (roughnesstex) {
				std::shared_ptr<Material> mat(new GlassMaterial(roughnesstex, ior));
				scene->materials.push_back(mat);
			}
		}
	}

}