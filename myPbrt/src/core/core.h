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
	class Mesh;
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
	class Pdf;

	using IntegratorSetPixelFunctionPtr = std::function<void(uint32_t, uint32_t, glm::vec4)>;

	inline uint32_t ToUint(const glm::vec4& p) {
		uint8_t r = (uint8_t)(glm::clamp(p.r, 0.0f, 1.f) * 255.0f);
		uint8_t g = (uint8_t)(glm::clamp(p.g, 0.0f, 1.f) * 255.0f);
		uint8_t b = (uint8_t)(glm::clamp(p.b, 0.0f, 1.f) * 255.0f);
		uint8_t w = (uint8_t)(glm::clamp(p.w, 0.0f, 1.f) * 255.0f);
		return (w << 24) | (b << 16) | (g << 8) | r;
	}

	inline uint32_t pcg_hash(uint32_t input)
	{
		uint32_t state = input * 747796405u + 2891336453u;
		uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}

	inline uint32_t random_uint() {
		thread_local uint32_t seed = rand();
		seed = pcg_hash(seed);
		return seed;
	}

	inline int random_int(int min, int max) {
		uint32_t val = random_uint();
		return (val % (max - min + 1)) + min;
	}

	inline double random_double() {
		return (double)random_uint() / (double)std::numeric_limits<uint32_t>::max();
	}

	inline double random_double(double min, double max) {
		// Returns a random real in [min,max).
		return min + (max - min) * random_double();
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

	inline glm::vec3 random_in_unit_hemisphere(const glm::vec3& normal)
	{
		glm::vec3 vec = random_in_unit_sphere();

		if (glm::dot(vec, normal) < 0) {
			return -vec;
		}
		return vec;
	}

	inline glm::vec3 random_in_unit_disk() {

		glm::vec3 p;
		do{
			p = glm::vec3(random_double(-1, 1), random_double(-1, 1), 0);
		} while (glm::length2(p) >= 1);
		
		return p;
	}

	inline glm::vec3 random_cosine_direction() {
		float r1 = random_double();
		float r2 = random_double();

		float phi = 2 * PI * r1;
		float x = cos(phi) * sqrt(r2);
		float y = sin(phi) * sqrt(r2);
		float z = sqrt(1 - r2);

		return glm::vec3(x, y, z);
	}

	inline glm::vec3 random_to_sphere(double radius, double distance_squared) {
		auto r1 = random_double();
		auto r2 = random_double();
		auto z = 1 + r2 * (sqrt(1 - radius * radius / distance_squared) - 1);

		auto phi = 2 * PIf * r1;
		auto x = cos(phi) * sqrt(1 - z * z);
		auto y = sin(phi) * sqrt(1 - z * z);

		return glm::vec3(x, y, z);
	}

}

#endif 
