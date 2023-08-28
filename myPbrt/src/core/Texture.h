#pragma once

#include "Interaction.h"
#include "imgui.h"

#include "Serializable.h"

namespace MyPBRT {

	class Texture : public Serializable
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

		static bool CreationMenuImGUI(int* selected_option, const std::vector<TextureType>& types);
		static std::shared_ptr<Texture> CreateTexture();
		static std::shared_ptr<Texture> CreateTextureFromMenu(int* selected_option, const std::vector<TextureType>& types);
		static void CreateTextureFromMenuFull(int* selected_option, std::shared_ptr<Texture>* ptr, const std::vector<TextureType>& types);
		static std::shared_ptr<Texture> LoadImage(const std::string& path);
		static std::shared_ptr<Texture> ParseTexture(const Json::Value& node);

		void DeSerialize(const Json::Value& node) override {}
		Json::Value Serialize() const override { return Json::Value(); }

	public:
		virtual glm::vec4 Evaluate(const SurfaceInteraction& interaction) const = 0;
		virtual ~Texture() {}
		virtual void CreateIMGUI() {};
	};

	template<typename T>
	class ConstantTexture : public Texture {
	public:
		static std::string GetType();

	public:
		ConstantTexture() {}
		ConstantTexture(T value) : value(value) {}
		glm::vec4 Evaluate(const SurfaceInteraction&) const override;
		void CreateIMGUI() override;

		void DeSerialize(const Json::Value& node) override;
		Json::Value Serialize() const override;

	private:
		T value;
	};

	class CheckerboardTexture : public Texture {
	public:
		static std::string GetType() { return "Checkerboard"; }

	public:
		CheckerboardTexture() {}
		CheckerboardTexture(float _scale) : scale(_scale) {}
		glm::vec4 Evaluate(const SurfaceInteraction& interaction) const override;
		void CreateIMGUI() override;

		void DeSerialize(const Json::Value& node) override;
		Json::Value Serialize() const override;

	private:
		float scale = 1;
	};

	class UVTexture : public Texture {
	public:
		static std::string GetType() { return "UV"; }

	public:
		UVTexture() {}
		glm::vec4 Evaluate(const SurfaceInteraction& interaction) const override;
	
		void DeSerialize(const Json::Value& node) override;
		Json::Value Serialize() const override;

	};

	class ImageTexture : public Texture {
	public:
		static std::string GetType() { return "Image"; }

	public:
		ImageTexture() {};
		ImageTexture(std::vector<uint8_t> _data, uint32_t _width, uint32_t _height, uint8_t _channels, double _inverseMult = 1.0f / 255.0f);
		~ImageTexture();
		glm::vec4 Evaluate(const SurfaceInteraction& interaction) const override;
		void CreateIMGUI() override;

		void DeSerialize(const Json::Value& node) override;
		Json::Value Serialize() const override;

	public:
		const double inverseMult = 1.0f / 255.0f;
		uint8_t channels = -1;
		uint32_t width = -1;
		uint32_t height = -1;
		std::vector<uint8_t> data;
		unsigned int image = 0;
	};

}

