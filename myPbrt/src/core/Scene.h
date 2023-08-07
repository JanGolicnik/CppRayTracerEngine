#pragma once

#include "core.h"
#include "Mesh.h"
#include "BVHAccelerator.h"

namespace MyPBRT {

struct Scene
{
	std::vector<std::shared_ptr<Material>> materials;
	std::vector<std::shared_ptr<Light>> lights;
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<Object> objects;
	BVHAccelerator BVHAccel;

	bool Intersect(const Ray& ray, SurfaceInteraction* interaction) const;
	bool IntersectAccel(const Ray& ray, SurfaceInteraction* interaction) const;
	bool hasIntersections(const Ray& ray) const;
	bool hasIntersectionsAccel(const Ray& ray) const;
	void Preprocess();

	void DrawLines(const glm::vec2& resolution, const Camera& camera, const glm::vec3& color, IntegratorSetPixelFunctionPtr set_function) const;

	void Load(const std::string& path);
	
	void Build();

	void AddObject(const Object& object);
	void RemoveObject(int id);
	void RecalculateObject(int id);

	const Mesh& ObjectToMesh(int object) const;
	Mesh& ObjectToMesh(int object);

};
}
