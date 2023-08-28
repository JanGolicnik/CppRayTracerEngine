#include "Scene.h"

#include "Object.h"
#include "Interaction.h"
#include "Texture.h"
#include "Material.h"
#include "Light.h"

#include <filesystem>

#include <vendor/obj_loader/OBJ_Loader.h>

#include <json/json.h>

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

	void Scene::LoadOBJ(const std::string& path)
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

	void Scene::Clear()
	{
		materials.clear();
		lights.clear();
		meshes.clear();
		objects.clear();
		Build();
	}

	void Scene::Save(const std::string& foldername, Json::Value& root) const
	{
		for (const auto& light : lights) {
			root["lights"].append(light->Serialize());
		}

		for (const auto& material : materials) {
			root["materials"].append(material->Serialize());
		}

		for (const auto& object : objects) {
			Json::Value val;
			val["shape"] = object.shape;
			val["material"] = object.material;
			root["objects"].append(val);
		}

		std::ofstream file(foldername + "/meshes.bin", std::ios::binary | std::ios::out);

		uint32_t numMeshes = (uint32_t)(meshes.size());
		file.write((const char*)&numMeshes, sizeof(numMeshes));

		int i = 0;
		for (const auto& mesh : meshes) {
			Json::Value meshNode = mesh->Serialize();
			meshNode["data"] = i;
			root["meshes"].append(meshNode);
			i++;

			const std::vector<Mesh::Vertex>& vertices = mesh->GetVertices();
			uint32_t numVertices = (uint32_t)(vertices.size());
			file.write((const char*)&numVertices, sizeof(numVertices));
			
			const std::vector<uint32_t>& indices = mesh->GetIndices();
			uint32_t numIndices = (uint32_t)(indices.size());
			file.write((const char*)&numIndices, sizeof(numIndices));

			for (const auto& vertex : vertices) {
				file.write((const char*)(&vertex.position), sizeof(vertex.position));
				file.write((const char*)(&vertex.normal), sizeof(vertex.normal));
				file.write((const char*)(&vertex.uv), sizeof(vertex.uv));
			}
			
			for (const auto& index : indices) {
				file.write((const char*)(&index), sizeof(index));
			}
		}

		file.close();
	}

	void Scene::Load(const std::string& foldername, const Json::Value& node)
	{
		Clear();
		MergeLoad(foldername, node);
	}

	void Scene::MergeLoad(const std::string& foldername, const Json::Value& node)
	{
		int PrevNumMeshes = meshes.size();
		int PrevNumMaterials = materials.size();

		//parse lights
		for (const auto& light : node["lights"]) {
			lights.push_back(Light::ParseLight(light));
		}

		//parse materials
		for (const auto& mat : node["materials"]) {
			materials.push_back(Material::ParseMaterial(mat));
		}

		for (const auto& node : node["objects"]) {
			Object& o = objects.emplace_back();
			o.shape = PrevNumMeshes + node["shape"].asInt();
			o.material= PrevNumMaterials + node["material"].asInt();
		}

		std::vector<std::vector<Mesh::Vertex>> vertices_per_mesh;
		std::vector<std::vector<uint32_t>> indices_per_mesh;
		
		std::ifstream file(foldername + "/meshes.bin", std::ios::binary | std::ios::in);
		
		uint32_t numMeshes;
		file.read((char*)(&numMeshes), sizeof(uint32_t));
		
		for (int i = 0; i < numMeshes; i++) {
			uint32_t numVertices;
			uint32_t numIndices;
			file.read((char*)(&numVertices), sizeof(uint32_t));
			file.read((char*)(&numIndices), sizeof(uint32_t));

			std::vector<Mesh::Vertex> vertices;
			vertices.resize(numVertices);
			for (int v = 0; v < numVertices; v++) {
				file.read((char*)(&vertices[v].position), sizeof(glm::vec3));
				file.read((char*)(&vertices[v].normal), sizeof(glm::vec3));
				file.read((char*)(&vertices[v].uv), sizeof(glm::vec2));
			}

			std::vector<uint32_t> indices;
			indices.resize(numIndices);
			for (int j = 0; j < numIndices; j++) {
				uint32_t data;
				file.read((char*)(&data), sizeof(uint32_t));
				indices.push_back(data);
			}

			vertices_per_mesh.push_back(vertices);
			indices_per_mesh.push_back(indices);
		}

		file.close();
		
		for (const auto& mesh : node["meshes"]) {
			meshes.push_back(Mesh::ParseMesh(mesh));
			meshes[meshes.size() - 1]->GetVertices() = vertices_per_mesh[mesh["data"].asInt()];
			meshes[meshes.size() - 1]->GetIndices() = indices_per_mesh[mesh["data"].asInt()];
			meshes[meshes.size() - 1]->ApplyTransformation();
		}

		Build();
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

