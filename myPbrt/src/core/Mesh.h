#pragma once

#include "core.h"
#include "BaseTypes.h"
#include "BVHAccelerator.h"
#include "Integrator.h"
#include <functional>

#include <json/json.h>

namespace MyPBRT {

	class Mesh : Serializable {
	public:
		struct Vertex {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 uv;
			glm::vec3 tangent;
			glm::vec3 bitangent;
			void CreateIMGUI(const std::string& name);
			Json::Value Serialize() const;
			void DeSerialize(const Json::Value& node);
		};

	public:
		static std::shared_ptr<Mesh> ParseMesh(const Json::Value& node);

	public:
		Mesh(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices);

		void ApplyTransformation();

		void Preprocess();
		bool Intersect(const Ray& ray, SurfaceInteraction* intersection, bool testAlphaTexture = false) const;
		bool hasIntersections(const Ray& ray, bool testAlphaTexture = false) const;
		float Area() const;
		//true if scene should update
		bool CreateIMGUI();
		void DrawLines(const glm::vec2& resolution, const Camera& camera, const glm::vec3& color, IntegratorSetPixelFunctionPtr set_function) const;
		
		glm::vec3 Center() const { return position; };
		void Translate(const glm::vec3& vec);
		void RotateAround(const glm::quat& rotation, const glm::vec3& pivot);

		bool IntersectTriangle(const Ray& ray, SurfaceInteraction* interaction, int object) const;

		std::vector < std::vector<std::pair<Integrator::RasterPixel, Integrator::RasterPixel>>> GetRasterizedEdges(const Camera& camera) const;

		const Bounds& GetBounds() const { return bounds; }

		void DeSerialize(const Json::Value& node) override;
		Json::Value Serialize() const override;
		std::string GetType() const { return "Mesh"; };
		const std::vector<Vertex>& GetVertices() const { return vertices; };
		std::vector<Vertex>& GetVertices() { return vertices; };
		const std::vector<uint32_t>& GetIndices() const { return indices; };
		std::vector<uint32_t>& GetIndices() { return indices; };

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
		
		std::vector<float> triangle_areas;
		float total_area;
		glm::vec3 triangle_center;

		Bounds bounds;
	};	

}
