#pragma once

#include "core.h"

namespace MyPBRT {

	class Light
	{
	public:

		Light(const glm::vec3 _color, float _strength) : color(_color), strength(_strength) {}

		virtual glm::vec3 Sample(const SurfaceInteraction& interaction) const = 0;
		virtual float PDF_Value(const SurfaceInteraction& interacton, const glm::vec3& direction) const = 0;
		virtual glm::vec3 Color() const { return color * strength; }
		virtual void DrawLines(const glm::vec2& resolution, const Camera& camera, const glm::vec3& color, IntegratorSetPixelFunctionPtr set_function) const {};

		virtual bool CreateIMGUI() = 0;
	
	protected:
		float strength;
		glm::vec3 color;
	};

	class SphericalLight : public Light {
	public:
		SphericalLight(const glm::vec3 _color, float _strength, const glm::vec3& _position, float _radius) : Light(_color, _strength), position(_position), radius(_radius) {}
	
		glm::vec3 Sample(const SurfaceInteraction& interaction) const override;
		float PDF_Value(const SurfaceInteraction& interacton, const glm::vec3& direction) const override;

		void DrawLines(const glm::vec2& resolution, const Camera& camera, const glm::vec3& color, IntegratorSetPixelFunctionPtr set_function) const override;

		bool CreateIMGUI() override;
	
	private:
		glm::vec3 position;
		float radius;
	};
}

