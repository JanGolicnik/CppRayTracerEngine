#pragma once

#include "core.h"
#include "BaseTypes.h"
#include <set>

namespace MyPBRT {

	class BVHAccelerator
	{
	public:

		using IntersectionFunction = std::function<bool(const Ray& ray, SurfaceInteraction* interaction, int object)>;

		struct Node {
			void MakeLeaf(int first, int n, const Bounds& b) {
				firstPrimOffset = first;
				nPrimitives = n;
				bounds = b;
				children[0] = children[1] = nullptr;
			}

			void MakeInterior(int axis, Node* c0, Node* c1) {
				children[0] = c0;
				children[1] = c1;
				bounds = c0->bounds.Union(c1->bounds);
				splitAxis = axis;
				nPrimitives = 0;
			}

			Bounds bounds;
			Node* children[2];
			int splitAxis, firstPrimOffset, nPrimitives;
		};

		struct FlatNode;
		enum class SplitMethod { SAH, HLBVH, Middle, EqualCounts };

	public:
		BVHAccelerator(SplitMethod _splitMethod = SplitMethod::Middle):splitMethod(_splitMethod) {}
		~BVHAccelerator() { delete[] flattenedNodes; }

		Node* Build(const std::vector<Bounds>& objects_bounds);

		void PrintNode(int node, int depth = 0);
		uint32_t Flatten(Node* node, int* offset);

		FlatNode* GetRoot();

		bool Intersect(int nodeIndex, const std::vector<Object>& objects, const Ray& ray, SurfaceInteraction* interaction) const;
		bool Intersect(int nodeIndex, const Ray& ray, SurfaceInteraction* interaction, const IntersectionFunction& intersction_func) const;
		bool HasIntersections(int nodeIndex, const std::vector<Object>& objects, const Ray& ray) const;

		//called when a object inside the leaf moves
		void RecalculateObject(const std::vector<Object>& objects, int objectIndex);

	private:
		const int maxPrimsInNode = 1;
		const SplitMethod splitMethod;
		FlatNode* flattenedNodes = nullptr;
		int totalNodes = 0;
		std::vector<int> orderedObjects;
		std::unordered_map<int, int> obj_to_leaf;

	private:
		Node* CreateNode(const std::vector<Bounds>& objects, std::vector<int>& objectIndexes, uint32_t start, uint32_t end);
		void DestroyTree(Node* node);
	};

}

