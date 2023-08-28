#pragma once

#include "core.h"

#include <json/json.h>
#include "Serializable.h"

namespace MyPBRT {

	class Light : public Serializable
	{
	public:

		Light(const glm::vec3 _color, float _strength) : color(_color), strength(_strength) {}

		virtual glm::vec3 Sample(const SurfaceInteraction& interaction) const = 0;
		virtual float PDF_Value(const SurfaceInteraction& interacton, const glm::vec3& direction) const = 0;
		virtual glm::vec3 Color() const { return color * strength; }
		virtual void DrawLines(const glm::vec2& resolution, const Camera& camera, const glm::vec3& color, IntegratorSetPixelFunctionPtr set_function) const {};

		virtual bool CreateIMGUI() = 0;

		virtual void DeSerialize(const Json::Value& node) override;
		virtual Json::Value Serialize() const override;

		static std::shared_ptr<Light> ParseLight(const Json::Value& node);

	protected:
		float strength;
		glm::vec3 color;
	};

	class SphericalLight : public Light {
	public:
		SphericalLight() :Light(glm::vec3(0), 0) {}
		SphericalLight(const glm::vec3 _color, float _strength, const glm::vec3& _position, float _radius) : Light(_color, _strength), position(_position), radius(_radius) {}
	
		glm::vec3 Sample(const SurfaceInteraction& interaction) const override;
		float PDF_Value(const SurfaceInteraction& interacton, const glm::vec3& direction) const override;

		void DrawLines(const glm::vec2& resolution, const Camera& camera, const glm::vec3& color, IntegratorSetPixelFunctionPtr set_function) const override;

		bool CreateIMGUI() override;
	
		void DeSerialize(const Json::Value& node) override;
		Json::Value Serialize() const override;
		std::string GetType() const override;

	private:
		glm::vec3 position = glm::vec3(0);
		float radius = 0;
	};
}

