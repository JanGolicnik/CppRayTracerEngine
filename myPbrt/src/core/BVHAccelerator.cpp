#include "BVHAccelerator.h"

#include "Object.h"
#include "Mesh.h"
#include "Interaction.h"

#include <algorithm>

namespace MyPBRT {

	struct BVHAccelerator::FlatNode {

		Bounds bounds;//24 bytes
		union {
			int primitivesOffset;
			int secondChildOffset;//first child is right next to parent
		}; // 4 bytes
		uint16_t nPrimitives; // 0 -> interior 2 byte
		uint8_t axis; //1 byte
		uint8_t pad; // 1 byte
	}; // 32 bytes

	BVHAccelerator::Node* BVHAccelerator::Build(const std::vector<Bounds>& objects_bounds)
	{
		totalNodes = 0;
		orderedObjects.clear();
		if (flattenedNodes != nullptr);
		delete[] flattenedNodes;
		flattenedNodes = nullptr;

		if (objects_bounds.size() == 0)
			return nullptr;

		std::vector<int> objectIndexes;
		for (int i = 0; i < objects_bounds.size(); i++)
			objectIndexes.push_back(i);

		std::vector<int> orderedObjects;
		orderedObjects.reserve(objects_bounds.size());

		Node* root = CreateNode(objects_bounds, objectIndexes, 0, objectIndexes.size());
		flattenedNodes = new FlatNode[totalNodes];
		int offset = 0;
		Flatten(root, &offset);
		DestroyTree(root);
	}

	BVHAccelerator::Node* BVHAccelerator::CreateNode(const std::vector<Bounds>& objects_bounds, std::vector<int>& objectIndexes,  uint32_t start, uint32_t end)
	{
		Node* node = new Node;
		int thisNodeIndex = totalNodes;
		totalNodes++;

		Bounds bounds;
		for (int i = start; i < end; i++)
			bounds = bounds.Union(objects_bounds[objectIndexes[i]]);

		uint32_t nObjects = end - start;
		if (nObjects == 1) {

			int firstOffset = orderedObjects.size();
			for (int i = start; i < end; i++)
				orderedObjects.push_back(objectIndexes[i]);
			node->MakeLeaf(firstOffset, nObjects, bounds);
			return node;
		}
		else {
			Bounds centroidBounds;
			for (int i = start; i < end; i++)
				centroidBounds = centroidBounds.Union(objects_bounds[objectIndexes[i]].Center());
			int axis = centroidBounds.MaximumExtent();

			int mid = (start + end) / 2;
			if (centroidBounds.max[axis] == centroidBounds.min[axis]) {
				int firstOffset = orderedObjects.size();
				for (int i = start; i < end; i++)
					orderedObjects.push_back(objectIndexes[i]);
				node->MakeLeaf(firstOffset, nObjects, bounds);
				return node;
			}
			
			float pmid = (centroidBounds.min[axis] + centroidBounds.max[axis]) / 2.0f;
			int* midPtr =
				std::partition(&objectIndexes[start], &objectIndexes[end - 1] + 1,
					[axis, pmid, objects_bounds](const int& pi) {
						return objects_bounds[pi].Center()[axis] < pmid;
					});
			mid = midPtr - &objectIndexes[0];

			Node* firstChild = CreateNode(objects_bounds, objectIndexes, start, mid);
			Node* secondChild = CreateNode(objects_bounds, objectIndexes, mid, end);
			node->MakeInterior(axis, firstChild, secondChild);
		}

		return node;
	}

	void BVHAccelerator::DestroyTree(Node* node)
	{
		if (node->children[0] != nullptr) {
			DestroyTree(node->children[0]);
			DestroyTree(node->children[1]);
		}
		delete node;
		return;
	}

	void BVHAccelerator::PrintNode(int node, int depth)
	{
		if (node >= totalNodes) return;
		int n = depth;
		if (depth > 1) n++;
		for (int i = 0; i < n; i++) {
			std::cout << "|";
		}
		if (depth > 0) std::cout << "-";
		if (flattenedNodes[node].nPrimitives > 0) {
			std::cout << node << " leaf holds " << flattenedNodes[node].nPrimitives << " primitives\n";
			return;
		}
		std::cout << node << " node holds two children: " << node + 1 << " " << flattenedNodes[node].secondChildOffset << "\n";
		PrintNode(node+1, depth+1);
		PrintNode(flattenedNodes[node].secondChildOffset, depth + 1);
	}

	uint32_t BVHAccelerator::Flatten(Node* node, int* offset)
	{
		FlatNode* flatNode = &flattenedNodes[*offset];
		flatNode->bounds = node->bounds;
		uint32_t thisNodeOffset = (*offset)++;
		if (node->children[0] == nullptr) {
			flatNode->primitivesOffset = node->firstPrimOffset;
			flatNode->nPrimitives = node->nPrimitives;
			for (int i = 0; i < flatNode->nPrimitives; i++) {
				obj_to_leaf.insert(std::make_pair(orderedObjects[node->firstPrimOffset + i], thisNodeOffset));
			}
		}
		else {
			flatNode->axis = node->splitAxis;
			flatNode->nPrimitives = 0;
			Flatten(node->children[0], offset);
			flatNode->secondChildOffset = Flatten(node->children[1], offset);
		}
		return thisNodeOffset;
	}

	BVHAccelerator::FlatNode* BVHAccelerator::GetRoot()
	{
		return &flattenedNodes[0];
	}

	bool BVHAccelerator::Intersect(int nodeIndex, const std::vector<Object>& objects, const Ray& ray, SurfaceInteraction* interaction, const std::vector<std::shared_ptr<Mesh>>& meshes) const
	{
		if (nodeIndex >= totalNodes) return false;

		FlatNode* node = &flattenedNodes[nodeIndex];
		if (!node->bounds.HasIntersections(ray)) return false;

		if (node->nPrimitives > 0) {
			bool hit = false;
			for (int i = 0; i < node->nPrimitives; i++)
				if (meshes[objects[orderedObjects[node->primitivesOffset + i]].shape]->Intersect(ray, interaction)) {
					hit = true;
					interaction->primitive = orderedObjects[node->primitivesOffset + i];
					interaction->shape = objects[orderedObjects[node->primitivesOffset + i]].shape;
				}
			return hit;
		}

		bool hit = Intersect(nodeIndex + 1, objects, ray, interaction, meshes);
		hit |= Intersect(node->secondChildOffset, objects, ray, interaction, meshes);
		
		return hit;
	}

	bool BVHAccelerator::Intersect(int nodeIndex, const Ray& ray, SurfaceInteraction* interaction, const IntersectionFunction& intersection_function) const
	{
		if (nodeIndex >= totalNodes) return false;

		FlatNode* node = &flattenedNodes[nodeIndex];
		if (!node->bounds.HasIntersections(ray)) return false;

		if (node->nPrimitives > 0) {
			bool hit = false;
			for (int i = 0; i < node->nPrimitives; i++)
				if (intersection_function(ray, interaction, orderedObjects[node->primitivesOffset + i])) {
					hit = true;
					interaction->primitive = orderedObjects[node->primitivesOffset + i];
				}
			return hit;
		}

		bool hit = Intersect(nodeIndex + 1, ray, interaction, intersection_function);
		hit |= Intersect(node->secondChildOffset, ray, interaction, intersection_function);

		return hit;
	}

	bool BVHAccelerator::HasIntersections(int nodeIndex, const std::vector<Object>& objects, const Ray& ray, const std::vector<std::shared_ptr<Mesh>>& meshes) const {
		if (nodeIndex >= totalNodes) return false;
		
		FlatNode* node = &flattenedNodes[nodeIndex];
		if (!node->bounds.HasIntersections(ray)) return false;

		if (node->nPrimitives > 0) {
			for (int i = 0; i < node->nPrimitives; i++)
				if (meshes[objects[orderedObjects[node->primitivesOffset + i]].shape]->hasIntersections(ray))
					return true;
			return false;
		}

		if (HasIntersections(nodeIndex + 1, objects, ray, meshes)) return true;
		if (HasIntersections(node->secondChildOffset, objects, ray, meshes)) return true;

		return false;
	}

	void BVHAccelerator::RecalculateObject(const std::vector<Object>& objects, int objectIndex, const std::vector<std::shared_ptr<Mesh>>& meshes)
	{
		int targetNode = obj_to_leaf[objectIndex];

		int currentNode = 0;
		while (true) {
			FlatNode* node = &flattenedNodes[currentNode];
			
			node->bounds = node->bounds.Union(meshes[objects[objectIndex].shape]->GetBounds());

			if (node->nPrimitives > 0) return;

			if (targetNode < node->secondChildOffset)
				currentNode++;
			else currentNode = node->secondChildOffset;
		}
	}

}