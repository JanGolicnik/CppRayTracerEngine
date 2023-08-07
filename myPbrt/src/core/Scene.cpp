#include "Scene.h"

#include "Object.h"
#include "Interaction.h"
#include "Texture.h"
#include "Material.h"

#include <filesystem>

#include <vendor/obj_loader/OBJ_Loader.h>

namespace MyPBRT {

	static objl::Loader modelLoader;

	bool Scene::Intersect(const Ray& ray, SurfaceInteraction* interaction) const
	{
		bool hit = false;
		int i = 0;
		for (auto& object : objects) {
			const Mesh& mesh = *meshes[object.shape];
			if (mesh.GetBounds().HasIntersections(ray)) {
				if (mesh.Intersect(ray, interaction)) {
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
		for (auto& object : objects) {
			const Mesh& mesh = *meshes[object.shape];
			if (mesh.hasIntersections(ray))
				return true;
		}
		return false;
	}

	bool Scene::IntersectAccel(const Ray& ray, SurfaceInteraction* interaction) const
	{
		return BVHAccel.Intersect(0, objects, ray, interaction, meshes);
	}

	bool Scene::hasIntersectionsAccel(const Ray& ray) const
	{
		return BVHAccel.HasIntersections(0, objects, ray, meshes);
	}

	void Scene::Preprocess()
	{
		for (auto& mesh : meshes) {
			mesh->Preprocess();
		}
	}

	void Scene::DrawLines(const glm::vec2& resolution, const Camera& camera, const glm::vec3& color, IntegratorSetPixelFunctionPtr set_function) const
	{
		for (auto& object : objects) {
			const Mesh& mesh = *meshes[object.shape];
			mesh.DrawLines(resolution, camera, color, set_function);
		}
	}

	void Scene::Load(const std::string& path)
	{
		if (modelLoader.LoadFile("models/" + path + ".obj")) {

			std::vector<MyPBRT::Mesh::Vertex> trimeshvertices;

			for (auto& vertex : modelLoader.LoadedVertices) {
				glm::vec3 position(vertex.Position.X, vertex.Position.Y, vertex.Position.Z);
				glm::vec3 normal(vertex.Normal.X, vertex.Normal.Y, vertex.Normal.Z);
				glm::vec2 uv(vertex.TextureCoordinate.X, vertex.TextureCoordinate.Y);
				trimeshvertices.push_back({ position, normal, uv });
			}

			std::vector<uint32_t> trimeshindices = modelLoader.LoadedIndices;
			meshes.push_back(std::shared_ptr<Mesh>(new Mesh(trimeshvertices, trimeshindices)));
			objects.push_back(Object(meshes.size() - 1, 0));
			Build(); 
		} 
	}

	void Scene::Build()
	{
		std::vector<Bounds> all_bounds;
		for (auto& object : objects) {
			const Mesh& mesh = *meshes[object.shape];
			all_bounds.push_back(mesh.GetBounds());
		}
		BVHAccel.Build(all_bounds);
		BVHAccel.PrintNode(0);
	}

	void Scene::AddObject(const Object& object)
	{
		objects.push_back(object);
		Build();
	}

	void Scene::RemoveObject(int id)
	{
		objects.erase(objects.begin() + id);
		Build();
	}

	void Scene::RecalculateObject(int id)
	{
		BVHAccel.RecalculateObject(objects, id, meshes);
	}

	const Mesh& Scene::ObjectToMesh(int object) const {
		return *meshes[objects[object].shape];
	}

	Mesh& Scene::ObjectToMesh(int object)
	{
		return *meshes[objects[object].shape];
	}

}

