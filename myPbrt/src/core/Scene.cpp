#include "Scene.h"

#include "Object.h"
#include "Interaction.h"
#include "Texture.h"

#include <filesystem>

#include <vendor/obj_loader/OBJ_Loader.h>

namespace MyPBRT {

	static objl::Loader modelLoader;

	bool Scene::Intersect(const Ray& ray, SurfaceInteraction* interaction) const
	{
		bool hit = false;
		int i = 0;
		for (auto& prim : primitives) {
			if (prim.shape->ObjectBound().HasIntersections(ray)) {
				if (prim.Intersect(ray, interaction)) {
					interaction->primitive = i;
					hit = true;
				}
			}
			i++;
		}
		return hit;
	}

	bool Scene::hasIntersections(const Ray& ray) const
	{
		for (auto& prim : primitives) {
			if (prim.hasIntersections(ray))
				return true;
		}
		return false;
	}

	bool Scene::IntersectAccel(const Ray& ray, SurfaceInteraction* interaction) const
	{
		return BVHAccel.Intersect(0, primitives, ray, interaction);
	}

	bool Scene::hasIntersectionsAccel(const Ray& ray) const
	{
		return BVHAccel.HasIntersections(0, primitives, ray);
	}

	void Scene::Preprocess() const
	{
		for (auto& prim : primitives) {
			prim.Preprocess();
		}
	}

	void Scene::DrawLines(const glm::vec2& resolution, const Camera& camera, const glm::vec3& color, IntegratorSetPixelFunctionPtr set_function) const
	{
		for (auto& prim : primitives) {
			prim.DrawLines(resolution, camera, color, set_function);
		}
	}

	void Scene::Load(const std::string& path)
	{
		if (modelLoader.LoadFile("models/" + path + ".obj")) {

			std::vector<MyPBRT::TriangleMesh::Vertex> trimeshvertices;

			for (auto& vertex : modelLoader.LoadedVertices) {
				glm::vec3 position(vertex.Position.X, vertex.Position.Y, vertex.Position.Z);
				glm::vec3 normal(vertex.Normal.X, vertex.Normal.Y, vertex.Normal.Z);
				glm::vec2 uv(vertex.TextureCoordinate.X, vertex.TextureCoordinate.Y);
				trimeshvertices.push_back({ position, normal, uv });
			}

			std::vector<uint32_t> trimeshindices = modelLoader.LoadedIndices;
			std::shared_ptr<MyPBRT::Shape> trimeshshape(new MyPBRT::TriangleMesh(trimeshvertices, trimeshindices));
			AddObject(Object(trimeshshape, 0));
		} 
	}

	void Scene::Build()
	{
		std::vector<Bounds> all_bounds;
		for (int i = 0; i < primitives.size(); i++) {
			all_bounds.push_back(primitives[i].shape->ObjectBound());
		}
		BVHAccel.Build(all_bounds);
		BVHAccel.PrintNode(0);
	}

	void Scene::AddObject(const Object& object)
	{
		primitives.push_back(object);
		Build();
	}

	void Scene::RemoveObject(int id)
	{
		primitives.erase(primitives.begin() + id);
		Build();
	}

	void Scene::RecalculateObject(int id)
	{
		BVHAccel.RecalculateObject(primitives, id);
	}

}

