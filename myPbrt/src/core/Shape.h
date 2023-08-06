#pragma once

#include "core.h"
#include "BaseTypes.h"
#include "BVHAccelerator.h"
#include "Integrator.h"
#include <functional>

namespace MyPBRT {

	class Shape
	{
	public:
		Shape() {}
		virtual ~Shape();

		const Bounds& ObjectBound() const { return bounds; };
		Bounds& ObjectBound() { return bounds; };

		virtual void Preprocess() = 0;
		virtual bool Intersect(const Ray& ray, SurfaceInteraction* interaction, bool testAlphaTexture = true) const = 0;
		virtual bool hasIntersections(const Ray& ray, bool testAlphaTexture = true) const = 0;
		virtual float Area() const = 0;
		//true if scene should update
		virtual bool CreateIMGUI(const std::vector<std::shared_ptr<Material>>& materials) = 0;
		virtual void DrawLines(const glm::vec2& resolution, const Camera& camera, const glm::vec3& color, IntegratorSetPixelFunctionPtr set_function) const = 0;
		virtual void Translate(const glm::vec3& vec) = 0;
		virtual void RotateAround(const glm::quat& rotation, const glm::vec3& pivot) = 0;
		virtual glm::vec3 Center() const = 0;
		virtual std::vector < std::vector<std::pair<Integrator::RasterPixel, Integrator::RasterPixel>>> GetRasterizedEdges(const Camera& camera) const = 0;
	protected:
		Bounds bounds;
	};

	class Sphere : public Shape {
	public:
		Sphere(const glm::vec3& _position, float _radius)
			: radius(_radius), position(_position) {
			bounds = Bounds(position - radius, position + radius);
		}

		void Preprocess() override;
		bool Intersect(const Ray& ray, SurfaceInteraction* intersection, bool testAlphaTexture) const override;
		bool hasIntersections(const Ray& ray, bool testAlphaTexture) const override;
		float Area() const override;
		//true if scene should update
		bool CreateIMGUI(const std::vector<std::shared_ptr<Material>>& materials) override;
		void DrawLines(const glm::vec2& resolution, const Camera& camera, const glm::vec3& color, IntegratorSetPixelFunctionPtr set_function) const override;
		
		glm::vec3 Center() const override { return position; }
		void Translate(const glm::vec3& vec) override ;
		void RotateAround(const glm::quat& rotation, const glm::vec3& pivot) override;
		std::vector < std::vector<std::pair<Integrator::RasterPixel, Integrator::RasterPixel>>> GetRasterizedEdges(const Camera& camera) const { return {}; };

	private:
		float radius;
		glm::vec3 position;

		std::shared_ptr<Texture> normal_map;
	};

	class TriangleMesh : public Shape {
	public:
		struct Vertex {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 uv;
			glm::vec3 tangent;
			glm::vec3 bitangent;
			void CreateIMGUI(const std::string& name);
		};


	public:
		TriangleMesh(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices);

		void ApplyTransformation();

		void Preprocess() override;
		bool Intersect(const Ray& ray, SurfaceInteraction* intersection, bool testAlphaTexture) const override;
		bool hasIntersections(const Ray& ray, bool testAlphaTexture) const override;
		float Area() const override;
		//true if scene should update
		bool CreateIMGUI(const std::vector<std::shared_ptr<Material>>& materials) override;
		void DrawLines(const glm::vec2& resolution, const Camera& camera, const glm::vec3& color, IntegratorSetPixelFunctionPtr set_function) const override;
		
		glm::vec3 Center() const override { return position; };
		void Translate(const glm::vec3& vec) override;
		void RotateAround(const glm::quat& rotation, const glm::vec3& pivot) override;

		bool IntersectTriangle(const Ray& ray, SurfaceInteraction* interaction, int object) const;

		std::vector < std::vector<std::pair<Integrator::RasterPixel, Integrator::RasterPixel>>> GetRasterizedEdges(const Camera& camera) const;

	private:
		std::vector<Vertex> vertices;
		std::vector<Vertex> transformed_vertices;
		std::vector<uint32_t> indices;
		std::vector<std::pair<int, int>> edges;
		glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 position = glm::vec3(0.0f);
		glm::vec3 scale = glm::vec3(1.0f);

		std::shared_ptr<Texture> normal_map;
		float normal_map_strength = 1;
		int selected_normal_map_texture;

		BVHAccelerator accel;
		std::function<bool(const Ray&, SurfaceInteraction*, int)> triangle_intersection_func = [this](const Ray& ray, SurfaceInteraction* interaction, int object)->bool { return IntersectTriangle(ray, interaction, object); };
	};	

}
