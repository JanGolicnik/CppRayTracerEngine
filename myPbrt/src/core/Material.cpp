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

	glm::vec3 DiffuseMaterial::Evaluate(SurfaceInteraction* interaction) const
	{
		if (texture) {
			return texture->Evaluate(*interaction);
		}
		return glm::vec3(1, 0, 1);
	}

	bool DiffuseMaterial::ScatterRay(const SurfaceInteraction& interaction, glm::vec3& dir, bool& has_pdf) const
	{
		if (random_double(0, 1) < roughness) {
			has_pdf = true;
			dir = random_cosine_direction();
		}
		else {
			dir = glm::reflect(glm::normalize(dir), interaction.normal);
		}

		if (glm::dot(dir, interaction.normal) < 0) {
			dir = -dir;
		}

		return true;
	}

	void DiffuseMaterial::IMGUI_Edit()
	{
		ImGui::DragFloat("roguhness", &roughness, .1, 0, 1);
		Texture::CreateTextureFromMenuFull(&selected_texture, &texture, selectable_texture_types);
	}

	void DiffuseMaterial::IMGUI_Create(Scene* scene)
	{
		static float local_roughness = 0.0f;
		ImGui::DragFloat("roughness", &local_roughness, 0.01f, 0, 1);
		Texture::CreationMenuImGUI(&selected_texture, selectable_texture_types);
		if (ImGui::Button("Add")) {
			local_roughness = 0.0f;
			std::shared_ptr<Texture> tex = Texture::CreateTexture();
			if (tex.get() != nullptr) {
				std::shared_ptr<Material> mat(new DiffuseMaterial(tex, local_roughness));
				scene->materials.push_back(mat);
			}
		}
	}
	float DiffuseMaterial::Pdf_Value(const glm::vec3& direction, const glm::vec3& normal) const
	{
		float cosine_theta = glm::dot(glm::normalize(direction), normal);
		return fmax(0, cosine_theta * INVPIf);
	}
	void DiffuseMaterial::DeSerialize(const Json::Value& node)
	{
		Material::DeSerialize(node);
		roughness = node["roughness"].asFloat();
		if(node["texture"].isNull() == false)
			texture = Texture::ParseTexture(node["texture"]);
	}
	Json::Value DiffuseMaterial::Serialize() const
	{
		Json::Value ret = Material::Serialize();
		ret["roughness"] = roughness;
		if (texture)
			ret["texture"] = texture->Serialize();
		ret["type"] = GetType();
		return ret;
	}
	glm::vec3 EmissiveMaterial::EvaluateLight(const SurfaceInteraction& interaction) const
	{
		if (texture) {
			glm::vec4 tex = texture->Evaluate(interaction);
			return emission * glm::vec3(tex.x, tex.y, tex.z);
		}
		return glm::vec3(1.0f);
	};
	glm::vec3 EmissiveMaterial::Evaluate(SurfaceInteraction* interaction) const
	{
		return glm::vec3(1.0f);
	}

	bool EmissiveMaterial::ScatterRay(const SurfaceInteraction& interaction, glm::vec3& dir, bool& has_pdf) const
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

	float EmissiveMaterial::Pdf_Value(const glm::vec3& incoming, const glm::vec3& normal) const
	{
		return 0.0f;
	}

	void EmissiveMaterial::DeSerialize(const Json::Value& node)
	{
		Material::DeSerialize(node);
		int i = 0;
		for (auto& value : node["emission"]) {
			emission[i] = value.asFloat();
		}
		if (node["texture"].isNull() == false)
			texture = Texture::ParseTexture(node["texture"]);
	}

	Json::Value EmissiveMaterial::Serialize() const
	{
		Json::Value ret = Material::Serialize();
		for (int i = 0; i < 3; i++) {
			ret["emission"].append(emission[i]);
		}
		if (texture)
			ret["texture"] = texture->Serialize();
		ret["type"] = GetType();
		return ret;
	}

	glm::vec3 MetallicMaterial::Evaluate(SurfaceInteraction* interaction) const
	{
		if (texture) {
			return texture->Evaluate(*interaction);
		}
		return glm::vec3(1, 0, 1);
	}

	bool MetallicMaterial::ScatterRay(const SurfaceInteraction& interaction, glm::vec3& dir, bool& has_pdf) const
	{
		dir = glm::reflect(glm::normalize(dir), interaction.normal);
		if (random_double(0, 1) < roughness) {
			has_pdf = true;
			dir = glm::normalize(random_cosine_direction() + dir);
		}

		if (glm::dot(dir, interaction.normal) < 0) {
			dir = -dir;
		}

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

	float MetallicMaterial::Pdf_Value(const glm::vec3& direction, const glm::vec3& normal) const
	{
		float cosine_theta = glm::dot(glm::normalize(direction), normal);
		return fmax(0, cosine_theta * INVPIf);
	}

	void MetallicMaterial::DeSerialize(const Json::Value& node)
	{
		Material::DeSerialize(node);
		roughness = node["roughness"].asFloat();
		if (node["texture"].isNull() == false)
			texture = Texture::ParseTexture(node["texture"]);
	}

	Json::Value MetallicMaterial::Serialize() const
	{
		Json::Value ret = Material::Serialize();
		ret["roughness"] = roughness;
		if (texture)
			ret["texture"] = texture->Serialize();
		ret["type"] = GetType();
		return ret;
	}

	glm::vec3 GlassMaterial::Evaluate(SurfaceInteraction* interaction) const
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

	bool GlassMaterial::ScatterRay(const SurfaceInteraction& interaction, glm::vec3& dir, bool& has_pdf) const
	{
		glm::vec3 unit_direction = glm::normalize(dir);
		float refraction_ratio = glm::dot(unit_direction, interaction.normal) < 0 ? (1.0 / ior) : ior;
		double cos_theta = glm::abs(glm::dot(-unit_direction, interaction.normal));
		double reflectance = schlick_reflectance(cos_theta, refraction_ratio);

		if (reflectance > random_double())
			dir = glm::reflect(unit_direction, interaction.normal);
		else
			dir = glm::refract(unit_direction, interaction.normal, refraction_ratio);
		dir = glm::refract(unit_direction, interaction.normal, refraction_ratio);

		float roughness = roughness_map->Evaluate(interaction).x;
		if (random_double(0, 1) < roughness) {
			has_pdf = true;
			dir = glm::normalize(random_cosine_direction() + dir);
		}

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

	float GlassMaterial::Pdf_Value(const glm::vec3& direction, const glm::vec3& normal) const
	{
		float cosine_theta = glm::abs(glm::dot(glm::normalize(direction), normal));
		return cosine_theta * INVPIf;
	}

	void GlassMaterial::DeSerialize(const Json::Value& node)
	{
		Material::DeSerialize(node);
		ior = node["ior"].asFloat();
		if(node["texture"].isNull() == false)
			texture = Texture::ParseTexture(node["texture"]);
		if (node["roughness map"].isNull() == false)
			roughness_map = Texture::ParseTexture(node["roughness map"]);
	}

	Json::Value GlassMaterial::Serialize() const
	{
		Json::Value ret = Material::Serialize();
		ret["ior"] = ior;
		if(texture)
			ret["texture"] = texture->Serialize();
		if (roughness_map)
			ret["roughness map"] = roughness_map->Serialize();
		ret["type"] = GetType();
		return ret;
	}

	void Material::DeSerialize(const Json::Value& node)
	{
		has_pdf = node["has pdf"].asBool();
	}

	Json::Value Material::Serialize() const
	{
		Json::Value ret;
		ret["has pdf"] = has_pdf;
		return ret;
	}

	std::shared_ptr<Material> Material::ParseMaterial(const Json::Value& node)
	{
		std::shared_ptr<Material> ret;
		std::string type = node["type"].asString();

		if (type == "Emissive") {
			ret = std::make_shared<EmissiveMaterial>();
		}
		else if(type == "Diffuse") {
			ret = std::make_shared<DiffuseMaterial>();
		}
		else if (type == "Metallic") {
			ret = std::make_shared<MetallicMaterial>();
		}
		else if (type == "Glass") {
			ret = std::make_shared<GlassMaterial>();
		}

		ret->DeSerialize(node);
		return ret;
	}

}