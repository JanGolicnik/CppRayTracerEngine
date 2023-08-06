#pragma once

#include "core.h"
#include "Shape.h"
#include "BVHAccelerator.h"

namespace MyPBRT {

struct Scene
{
	std::vector<std::shared_ptr<Light>> lights;
	std::vector<std::shared_ptr<Shape>> shapes;
	std::vector<std::shared_ptr<Material>> materials;
	std::vector<Object> primitives;
	BVHAccelerator BVHAccel;

	bool Intersect(const Ray& ray, SurfaceInteraction* interaction) const;
	bool IntersectAccel(const Ray& ray, SurfaceInteraction* interaction) const;
	bool hasIntersections(const Ray& ray) const;
	bool hasIntersectionsAccel(const Ray& ray) const;
	void Preprocess() const;

	void DrawLines(const glm::vec2& resolution, const Camera& camera, const glm::vec3& color, IntegratorSetPixelFunctionPtr set_function) const;

	void Load(const std::string& path);
	
	void Build();

	void AddObject(const Object& object);
	void RemoveObject(int id);
	void RecalculateObject(int id);


};
}
