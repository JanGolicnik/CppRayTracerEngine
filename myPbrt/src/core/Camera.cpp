#include "Camera.h"

#include "BaseTypes.h"

namespace MyPBRT {

	void approach(glm::vec3& from, const glm::vec3& to, float step) {
		float inverseStep = 1.0f / step;
		from += (to - from) * inverseStep;
	}

	Camera::Camera(float _verticalFOV, float _nearClip, float _farClip, float _focal_distance, float _lens_radius)
		:verticalFOV(_verticalFOV), nearClip(_nearClip), farClip(_farClip), focal_distance(_focal_distance), lens_radius(_lens_radius)
	{
		direction = glm::vec3(0, 0, -1);
		position = glm::vec3(0, 0, 10);
		should_update = true;
	}

	Camera::~Camera()
	{
	}

	bool Camera::Update(float dt)
	{
		if (!active) velocity = glm::vec2(0);
		if (velocity != glm::vec2(0)) {
			glm::vec3 rightDirection = glm::cross(direction, glm::vec3(.0f, 1.0f, .0f));
			position += direction * dt * velocity.x;
			position += rightDirection * dt * velocity.y;
			should_update = true;
		}

		if (should_update) {
			RecalculateView();
			RecalculateProjection();
			RecalculateRayDirections();
			should_update = false;
			return true;
		}
		return false;
	}

	void Camera::OnResize(glm::ivec2 resolution)
	{
		if (resolution.x == viewportWidth && resolution.y == viewportHeight)
			return;

		viewportWidth = resolution.x;
		viewportHeight = resolution.y;
		should_update = true;
	}

	void Camera::SetActive(int state)
	{
		if (state == -1) {
			active = !active;
		}
		else {
			active = state;
		}
	}

	void Camera::ButtonCallback(int key, bool pressed)
	{
		if (!active) return;

		switch (key) {
		case 87: //W
			velocity.x = speed * (float)pressed;
			break;
		case 83: //S
			velocity.x = -speed * (float)pressed;
			break;
		case 65: //A
			velocity.y = -speed * (float)pressed;
			break;
		case 68: //D
			velocity.y = speed * (float)pressed;
			break;
		}
	}

	void Camera::MouseButtonCallback(int key, bool pressed)
	{
		if (key == 1) {
			active = pressed;
		}
	}

	void Camera::MouseMotionCallback(glm::vec2 mouse_position)
	{
		right = glm::cross(direction, glm::vec3(.0f, 1.0f, .0f));
		if (last_mouse_position == mouse_position) return;
		glm::vec2 delta = (mouse_position - last_mouse_position) * 0.007f;
		last_mouse_position = mouse_position;
		/*std::cout << last_mouse_position.x << " " << last_mouse_position.y << "\n";
		std::cout << viewportWidth << " " << viewportHeight << "\n";*/
		if (!active) return;

		float pitchDelta = delta.y * .3f;
		float yawDelta = delta.x * .3f;

		glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, right), glm::angleAxis(-yawDelta, glm::vec3(.0f, 1.0f, .0f))));
		direction = glm::rotate(q, direction);

		should_update = true;
	}

	void Camera::ScrollCallback(double offset)
	{
		if (!active) return;
		if (!offset) return;

		verticalFOV += offset;
		should_update = true;
	}

	const Ray Camera::GetRay(const glm::ivec2& pos) const
	{
		if (lens_radius == 0) {
			return Ray(position, rayDirections[pos.x + pos.y * viewportWidth]);
		}
		glm::vec2 random_offset = lens_radius * random_in_unit_disk();

		glm::vec3 offset = random_offset.x * right + random_offset.y * glm::vec3(0, 1, 0);
		glm::vec3 normalized_dir = glm::normalize(rayDirections[pos.x + pos.y * viewportWidth]);
		glm::vec3 resized_dir = normalized_dir * focal_distance;
		glm::vec3 dir = glm::normalize(resized_dir - offset);
		
		Ray ray(position + offset, dir);
		
		return ray;
	}

	const Ray Camera::GetMouseRay(const glm::vec2& pos) const
	{
		return GetRay(glm::ivec2(pos.x * viewportWidth, pos.y * viewportHeight));
	}

	void Camera::RecalculateProjection()
	{
		projection = glm::perspectiveFov(glm::radians(verticalFOV), (float)viewportWidth, (float)viewportHeight, nearClip, farClip);
		inverseProjection = glm::inverse(projection);
	}

	void Camera::RecalculateView()
	{
		view = glm::lookAt(position, position + direction, glm::vec3(0, 1, 0));
		inverseView = glm::inverse(view);
	}

	void Camera::RecalculateRayDirections()
	{
		rayDirections.resize(viewportWidth * viewportHeight);

		for (uint32_t y = 0; y < viewportHeight; y++) {
			for (uint32_t x = 0; x < viewportWidth; x++) {

				glm::vec2 coord = { (float)x / (float)viewportWidth, (float)y / (float)viewportHeight };
				coord = coord * 2.0f - 1.0f; // -1 -> 1

				glm::vec4 target = inverseProjection * glm::normalize(glm::vec4(coord.x, coord.y, 0, 1));
				glm::vec3 rayDirection = glm::vec3(inverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0)); // World space

				rayDirections[x + y * viewportWidth] = rayDirection;
			}
		}
	}
}