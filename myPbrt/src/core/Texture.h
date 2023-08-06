#pragma once

#include "Interaction.h"
#include "imgui.h"

namespace MyPBRT {

	class Texture
	{
	public:
		enum class TextureType {
			ConstantColor = 0,
			Checkerboard = 1,
			UV = 2,
			Image = 3,
			ConstantValue = 4
		};
	public:
		static const char* options[5];
		static TextureType selected_type;
		static glm::vec3 constant_tex_color;
		static float checkerboard_scale;
		static std::string image_path;
		static float constant_value_tex_value;

	public:
		virtual glm::vec4 Evaluate(const SurfaceInteraction& interaction) const = 0;
		virtual ~Texture() {}
		virtual void CreateIMGUI() {};

		static bool CreationMenuImGUI(int* selected_option, const std::vector<TextureType>& types);
		static std::shared_ptr<Texture> CreateTexture();
		static std::shared_ptr<Texture> CreateTextureFromMenu(int* selected_option, const std::vector<TextureType>& types);
		static void CreateTextureFromMenuFull(int* selected_option, std::shared_ptr<Texture>* ptr, const std::vector<TextureType>& types);
		static std::shared_ptr<Texture> LoadImage(const std::string& path);
	};

	template<typename T>
	class ConstantTexture : public Texture {
	public:
		ConstantTexture(T value) : value(value) {}
		glm::vec4 Evaluate(const SurfaceInteraction&) const;
		void CreateIMGUI();
	private:
		T value;
	};

	class CheckerboardTexture : public Texture {
	public:
		CheckerboardTexture(float _scale) : scale(_scale) {}
		glm::vec4 Evaluate(const SurfaceInteraction& interaction) const;
		void CreateIMGUI();
	private:
		float scale;
	};

	class UVTexture : public Texture {
	public:
		UVTexture() {}
		glm::vec4 Evaluate(const SurfaceInteraction& interaction) const;
	};

	class ImageTexture : public Texture {
	public:
		ImageTexture(std::vector<uint8_t> _data, uint32_t _width, uint32_t _height, uint8_t _channels, double _inverseMult = 1.0f / 255.0f);
		~ImageTexture();
		glm::vec4 Evaluate(const SurfaceInteraction& interaction) const;
		void CreateIMGUI();
	public:
		const double inverseMult;
		uint8_t channels;
		uint32_t width;
		uint32_t height;
		std::vector<uint8_t> data;
		unsigned int image;
	};

}

