#pragma once

#include "BaseTypes.h"
#include "core.h"

namespace MyPBRT {

	class Interaction
	{
	public:
		glm::vec3 pos = glm::vec3(0.0f);
		glm::vec3 wo = glm::vec3(0.0f);
		glm::vec3 normal = glm::vec3(0.0f);
		glm::vec3 uv = glm::vec3(0.0f);

	public:
		Interaction() {}
		Interaction(const glm::vec3& pos, const glm::vec3& normal, const glm::vec3& wo)
			: pos(pos), normal(normal), wo(wo) {}

		bool isSurfaceInteraction() const { return normal != glm::vec3(); }
	};

	class SurfaceInteraction : public Interaction {
	public:
		glm::vec2 uv = glm::vec2(0.0f);
		const Shape* shape = nullptr;
		int primitive = -1;
		bool front_face = true;

	public:
		SurfaceInteraction() {}
		SurfaceInteraction(const glm::vec3& p, const glm::vec3& normal, const glm::vec2& uv, const glm::vec3& wo, const Shape* shape);
		void SetNormal(const glm::vec3& _rayDirection, const glm::vec3& _normal) {
			front_face = glm::dot(_rayDirection, normal) < 0;
			normal = front_face ? _normal : -_normal;
			normal = _normal;
		}
	};

}