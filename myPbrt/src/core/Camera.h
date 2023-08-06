#pragma once

#include "core.h"
#include "BaseTypes.h"

namespace MyPBRT {

	class Camera
	{
	public:
		Camera(float verticalFOV, float nearClip, float farClip, float aperture, float lens_radius);
		~Camera();

		bool Update(float dt);
		void OnResize(glm::ivec2 resolution);

		//-1 for toggle
		void SetActive(int state = -1);

		void ButtonCallback(int key, bool pressed);
		void MouseButtonCallback(int key, bool pressed);
		void MouseMotionCallback(glm::vec2 mouse_position);
		void ScrollCallback(double offset);

		const glm::ivec2& GetResolution() const { return {viewportWidth, viewportHeight}; }

		const glm::mat4& GetProjection() const { return projection; }
		const glm::mat4& GetInverseProjection() const { return inverseProjection; }
		const glm::mat4& GetView() const { return view; }
		const glm::mat4& GetInverseView() const { return inverseView; }

		const glm::vec3 GetPosition() const { return position; }
		const glm::vec3 GetDirection() const { return direction; }

		float& GetFocalDistance() { return focal_distance; }
		float& GetLensRadius() { return lens_radius; }

		const std::vector<glm::vec3>& GetRayDirections() const { return rayDirections; }
		const Ray GetRay(const glm::ivec2& pos) const;
		const Ray GetMouseRay(const glm::vec2& pos) const;

	private:
		void RecalculateProjection();
		void RecalculateView();
		void RecalculateRayDirections();

	private:
		glm::mat4 projection{ 1.0f };
		glm::mat4 view{ 1.0f };
		glm::mat4 inverseProjection{ 1.0f };
		glm::mat4 inverseView{ 1.0f };

		float verticalFOV = 45.0f;
		float nearClip = .1f;
		float farClip = 100.0f;

		float focal_distance;
		float lens_radius;

		glm::vec3 right;

		glm::vec3 position{ 0.0f, 0.0f, 0.0f };
		glm::vec3 direction{ 0.0f, 0.0f, 0.0f };
		glm::vec2 velocity{ 0.0f, 0.0f };

		std::vector<glm::vec3> rayDirections;

		glm::vec2 last_mouse_position{ 0.0f, 0.0f };

		uint32_t viewportWidth;
		uint32_t viewportHeight;

		//when right mouse button is held
		bool active = false;

		const float speed = 5.0f;

		bool should_update = false;
	};
}
