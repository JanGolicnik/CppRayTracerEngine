#ifndef CORE_H
#define CORE_H

constexpr double PI = 3.14159265359;
constexpr float PIf = 3.14159265359f;
constexpr float SHADOW_EPSILON = 0.0001f;
constexpr double INVPI = 0.31830988618379067154;
constexpr float INVPIf = 0.31830988618379067154f;

#define INFINITY std::numeric_limits<float>::max()
#define MACHINE_EPSILON std::numeric_limits<float>::epsilon() * 0.5

// Standard library stuff
#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <memory>
#include <random>
#include <functional>

//glm
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp >
#include <glm/gtc/type_ptr.hpp>

//MyPBRT
namespace MyPBRT {

	struct Ray;
	struct Bounds;
	class Light;
	struct Object;
	struct Scene;
	class Shape;
	class Texture;
	class Camera;
	class Integrator;
	class SurfaceInteraction;
	class Material;
	template <typename T>
	class ConstantTexture;
	class CheckerboardTexture;
	class UVTexture;
	class ImageTexture;

	using IntegratorSetPixelFunctionPtr = std::function<void(uint32_t, uint32_t, glm::vec4)>;

	inline uint32_t ToUint(const glm::vec4& p) {
		uint8_t r = (uint8_t)(glm::clamp(p.r, 0.0f, 1.f) * 255.0f);
		uint8_t g = (uint8_t)(glm::clamp(p.g, 0.0f, 1.f) * 255.0f);
		uint8_t b = (uint8_t)(glm::clamp(p.b, 0.0f, 1.f) * 255.0f);
		uint8_t w = (uint8_t)(glm::clamp(p.w, 0.0f, 1.f) * 255.0f);
		return (w << 24) | (b << 16) | (g << 8) | r;
	}

	inline double random_double(double min = std::numeric_limits<double>::lowest(), double max = std::numeric_limits<double>::max()) {
		thread_local std::random_device rd;
		thread_local std::mt19937 gen(rd());
		thread_local std::uniform_real_distribution<float> dis(min, max);
		return dis(gen);
	}

	inline double random_double_bad() {
		// Returns a random real in [0,1).
		return rand() / (RAND_MAX + 1.0);
	}

	inline double random_double_bad(double min, double max) {
		// Returns a random real in [min,max).
		return min + (max - min) * random_double_bad();
	}

	inline glm::vec3 random_vec3(double min, double max)
	{
		return glm::vec3(random_double(min, max), random_double(min, max), random_double(min, max));
	}

	inline glm::vec3 random_in_unit_sphere()
	{
		glm::vec3 vec;
		do {
			vec = random_vec3(-1, 1);
		} while (glm::length2(vec) > 1);
		return vec;
	}

	inline glm::vec3 random_in_unit_disk() {
		glm::vec3 p;

		do{
			p = glm::vec3(random_double_bad(-1, 1), random_double_bad(-1, 1), 0);
		
		} while (glm::length2(p) >= 1);
		
		return p;
	}

}


#endif 
