#include "Light.h"

#include "imgui.h"
#include "Interaction.h"
#include "Camera.h"

namespace MyPBRT {

	glm::vec3 SphericalLight::Sample(const SurfaceInteraction& interaction) const
	{
		glm::vec3 dir = position - interaction.pos;
		auto distance_squared = glm::length2(dir);

		glm::vec3 unit_w = glm::normalize(dir);
		glm::vec3 a = (fabs(unit_w.x) > 0.9) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
		glm::vec3 v = glm::normalize(glm::cross(unit_w, a));
		glm::vec3 u = glm::cross(unit_w, v);

		glm::vec3 random = random_to_sphere(radius, distance_squared);

		return random.x * u + random.y * v + random.z * unit_w;
	}

	float SphericalLight::PDF_Value(const SurfaceInteraction& interacton, const glm::vec3& direction) const
	{
		glm::vec3 dir = position - interacton.pos;
		auto cos_theta_max = sqrt(1.0f - radius * radius / glm::length2(dir));
		auto solid_angle = 2 * PIf * (1 - cos_theta_max);

		return  1 / solid_angle;
	}

	void SphericalLight::DrawLines(const glm::vec2& resolution, const Camera& camera, const glm::vec3& color, IntegratorSetPixelFunctionPtr set_function) const
	{
		glm::vec4 pos = camera.GetView() * glm::vec4(position,1);
	    pos = camera.GetProjection() * pos;
	    float w = pos.w;
	    if (w == 0) return;
	    pos /= w;
	    pos += 1;
	    pos *= 0.5;
	    if (pos.z > 1) return;
	    if (pos.x < 0 || pos.x > 1) return;
	    if (pos.y < 0 || pos.y > 1) return;

	    glm::ivec2 screenPosition = glm::ivec2(pos.x * resolution.x, pos.y * resolution.y);
	    float screenRadius = (radius * resolution.y) / w;
	    screenRadius *= 1.1;
	    set_function(screenPosition.x, screenPosition.y, glm::vec4(color, 0.0f));

	    float circumference = 2 * PIf * screenRadius;
	    float ratio = (2*PIf) / circumference;

	    for (int i = 0; i <= circumference; i++) {
	        glm::ivec2 pos = screenPosition + 1;
	        pos.x += cos(i * ratio) * screenRadius;
	        pos.y += sin(i * ratio) * screenRadius;
	        if (pos.x > 0 && pos.x < resolution.x && pos.y > 0 && pos.y < resolution.y)
	            set_function(pos.x, pos.y, glm::vec4(color, 0.0f));
	    }
	}

	bool SphericalLight::CreateIMGUI()
	{
		bool changed = false;
		changed |= ImGui::DragFloat3("position", glm::value_ptr(position), 0.01, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		changed |= ImGui::DragFloat("radius", &radius, 0.01, 0, std::numeric_limits<float>::max());
		changed |= ImGui::ColorEdit3("color", glm::value_ptr(color));
		changed |= ImGui::DragFloat("strength", &strength, 0.01, 0, std::numeric_limits<float>::max());
		return changed;
	}

}
