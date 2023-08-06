#include "Texture.h"
#include "imgui.h"

#include <imgui/imgui_stdlib.h>
#include <stb_image/stb_image.h>
#include <glad/glad.h>

namespace MyPBRT {

	const char* Texture::options[5] = { "ConstantColor", "Checkerboard", "UV", "Image", "ConstantValue"};
	Texture::TextureType Texture::selected_type = Texture::TextureType::ConstantColor;
	glm::vec3 Texture::constant_tex_color = glm::vec3(1.0f);
	float Texture::checkerboard_scale = 10.0f;
	std::string Texture::image_path = "";
	float Texture::constant_value_tex_value = 1.0f;

	glm::vec4 ConstantTexture<float>::Evaluate(const SurfaceInteraction&) const
	{
		return glm::vec4(value);
	}
	glm::vec4 ConstantTexture<glm::vec2>::Evaluate(const SurfaceInteraction&) const
	{
		return glm::vec4(value.x, value.y, 0, 0);
	}
	glm::vec4 ConstantTexture<glm::vec3>::Evaluate(const SurfaceInteraction&) const
	{
		return glm::vec4(value.x, value.y, value.z, 0);
	}
	void ConstantTexture<float>::CreateIMGUI()
	{
		ImGui::DragFloat("color", &value, 0.01, 0, std::numeric_limits<float>::max());
	}

	void ConstantTexture<glm::vec2>::CreateIMGUI()
	{
		ImGui::DragFloat2("color", glm::value_ptr(value), 0.01, 0, std::numeric_limits<float>::max());
	}

	void ConstantTexture<glm::vec3>::CreateIMGUI()
	{
		ImGui::ColorEdit3("color", glm::value_ptr(value));
	}

	glm::vec4 CheckerboardTexture::Evaluate(const SurfaceInteraction& interaction) const
	{
		float u = interaction.uv.x * scale;
		float v = interaction.uv.y * scale;

		return int(floor(u) + floor(v)) % 2 == 0 ? glm::vec4(0,0,0,1) : glm::vec4(1);
	}

	void CheckerboardTexture::CreateIMGUI()
	{
		ImGui::DragFloat("scale", &scale, 0.01, 0, std::numeric_limits<float>::max());
	}

	glm::vec4 UVTexture::Evaluate(const SurfaceInteraction& interaction) const
	{
		return glm::vec4(interaction.uv.x, interaction.uv.y, 0, 1);
	}

	ImageTexture::ImageTexture(std::vector<uint8_t> _data, uint32_t _width, uint32_t _height, uint8_t _channels, double _inverseMult)
		: data(_data), width(_width), height(_height), inverseMult(_inverseMult), channels(_channels)
	{
		glGenTextures(1, &image);
		glBindTexture(GL_TEXTURE_2D, image);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		GLuint data_format = GL_RED;
		GLuint internal_format = GL_RED; // Declare internal_format outside the switch statement

		switch (channels) {
		case 2:
			data_format = GL_RG;
			internal_format = GL_RG; // Assign internal_format correctly for 2 channels
			break;
		case 3:
			data_format = GL_RGB;
			internal_format = GL_RGB; // Assign internal_format correctly for 3 channels
			break;
		case 4:
			data_format = GL_RGBA;
			internal_format = GL_RGBA; // Assign internal_format correctly for 4 channels
			break;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, data_format, GL_UNSIGNED_BYTE, data.data());
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glm::vec4 ImageTexture::Evaluate(const SurfaceInteraction& interaction) const
	{
		float u = glm::clamp(interaction.uv.x, 0.0f, 1.0f);
		float v = 1.0 - glm::clamp(interaction.uv.y, 0.0f, 1.0f);
		int i = u * width;
		int j = v * height;

		if (i >= width)  i = width - 1;
		if (j >= height) j = height - 1;

		int pixel = (j * width + i) * channels;
		glm::vec4 val(0, 0, 0, 1);
		for (int c = 0; c < channels; c++) {
			val[c] = data[pixel + c] * inverseMult;
		}
		return glm::vec4(val);
	}
	ImageTexture::~ImageTexture()
	{
		glDeleteTextures(1, &image);
	}
	void ImageTexture::CreateIMGUI()
	{
		ImGui::Image((void*)(intptr_t)image, ImVec2(200, 200));
	}

	bool Texture::CreationMenuImGUI(int* selected_option, const std::vector<TextureType>& types)
	{
		if (types.size() == 0) return false;
		if (*selected_option >= types.size()) *selected_option = types.size() - 1;
		std::vector<const char*> filtered_options;
		for (int i = 0; i < types.size(); i++) {
			filtered_options.push_back(options[(int)types[i]]);
		}
		bool changed = ImGui::Combo("Texture Type", selected_option, filtered_options.data(), filtered_options.size());
		if (changed) {
			std::cout << "a\n";
		}
		selected_type = types[*selected_option];
		switch (selected_type) {
		case TextureType::ConstantColor:
			ImGui::ColorEdit3("Color", glm::value_ptr(constant_tex_color));
			break;
		case TextureType::Checkerboard:
			ImGui::DragFloat("Scale", &checkerboard_scale);
			break;
		case TextureType::UV:
			break;
		case TextureType::Image:
			ImGui::InputText("Path", &image_path);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Image name with extension (ex. \"Dog.jpg\", must be in Textures folder");
			break;
		case TextureType::ConstantValue:
			ImGui::DragFloat("Value", &constant_value_tex_value, 0.01f, std::numeric_limits<float>::min(), std::numeric_limits<float>::max());
			break;
		}
		return changed;
	}

	std::shared_ptr<Texture> Texture::CreateTexture() {
		switch (selected_type) {
		case TextureType::ConstantColor:
			return std::shared_ptr<Texture>(new ConstantTexture<glm::vec3>(constant_tex_color));
		case TextureType::Checkerboard:
			return std::shared_ptr<Texture>(new CheckerboardTexture(checkerboard_scale));
		case TextureType::UV:
			return std::shared_ptr<Texture>(new UVTexture());
		case TextureType::Image:
			return LoadImage(image_path);
		case TextureType::ConstantValue:
			return std::shared_ptr<Texture>(new ConstantTexture<float>(constant_value_tex_value));
		}
	}

	std::shared_ptr<Texture> Texture::CreateTextureFromMenu(int* selected_option, const std::vector<TextureType>& types) {
		if (CreationMenuImGUI(selected_option, types)) {
			return CreateTexture();
		}
		return nullptr;
	}

	void Texture::CreateTextureFromMenuFull(int* selected_option, std::shared_ptr<Texture>* ptr, const std::vector<TextureType>& types) {
		if (*ptr) {
			(*ptr)->CreateIMGUI();
			if (ImGui::Button("Remove Texture")) {
				(*ptr).reset();
			}
		}
		else {
			*ptr = Texture::CreateTextureFromMenu(selected_option, types);
		}
	}



	std::shared_ptr<Texture> Texture::LoadImage(const std::string& path) {
		int width, height, channels;
		std::string newPath = "textures/" + path;
		unsigned char* img = stbi_load(newPath.c_str(), &width, &height, &channels, 0);
		if (img == NULL) {
			return nullptr;
		}

		std::vector<uint8_t> data;
		data.resize(width * height * channels);
		for (int i = 0; i < width * height * channels; i++) {
			data[i] = img[i];
		}

		return std::shared_ptr<Texture>(new ImageTexture(data, width, height, channels));
	}

}

