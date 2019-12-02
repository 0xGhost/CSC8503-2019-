#pragma once
#include "../../Common/Vector2.h"
#include "Debug.h"
#include <list>
#include "Ray.h"
#include <functional>

namespace NCL {
	using namespace NCL::Maths;
	namespace CSC8503 {
		template<class T>
		class QuadTree;

		template<class T>
		struct QuadTreeEntry {
			Vector3 pos;
			Vector3 size;
			T object;

			QuadTreeEntry(T obj, Vector3 pos, Vector3 size) {
				object = obj;
				this->pos = pos;
				this->size = size;
			}
		};

		template<class T>
		class QuadTreeNode {
		public:
			typedef std::function<void(std::list<QuadTreeEntry<T>>&)> QuadTreeFunc;
		protected:
			friend class QuadTree<T>;

			QuadTreeNode() {}

			QuadTreeNode(Vector2 pos, Vector2 size, QuadTree<T>* t) {
				children = nullptr;
				this->tree = t;
				this->position = pos;
				this->size = size;
			}

			~QuadTreeNode() {

				DeleteFourChildren(children);
				//delete[] children;
			}

			void Insert(T& object, const Vector3& objectPos, const Vector3& objectSize, int depthLeft, int maxSize) {
				if (!CollisionDetection::AABBTest(objectPos,
					Vector3(position.x, 0, position.y), objectSize,
					Vector3(size.x, 1000.0f, size.y))) {
					return;
				}
				if (children) { // not a leaf node , just descend the tree
					for (int i = 0; i < 4; ++i) {
						children[i].Insert(object, objectPos, objectSize,
							depthLeft - 1, maxSize);
					}
				}
				else { // currently a leaf node , can just expand
					contents.push_back(QuadTreeEntry <T >(object, objectPos, objectSize));
					if ((int)contents.size() > maxSize && depthLeft > 0) {
						if (!children) {
							Split();
							// we need to reinsert the contents so far !
							for (const auto& i : contents) {
								for (int j = 0; j < 4; ++j) {
									auto entry = i;
									children[j].Insert(entry.object, entry.pos,
										entry.size, depthLeft - 1, maxSize);
								}
							}
							contents.clear(); // contents now distributed !
						}
					}
				}

			}

			void Split() {
				Vector2 halfSize = size / 2.0f;
				children = tree->GetFourChildren();
				//children = new QuadTreeNode <T>[4];
				children[0] = QuadTreeNode<T>(position +
					Vector2(-halfSize.x, halfSize.y), halfSize, tree);
				children[1] = QuadTreeNode<T>(position +
					Vector2(halfSize.x, halfSize.y), halfSize, tree);
				children[2] = QuadTreeNode<T>(position +
					Vector2(-halfSize.x, -halfSize.y), halfSize, tree);
				children[3] = QuadTreeNode<T>(position +
					Vector2(halfSize.x, -halfSize.y), halfSize, tree);
			}

			void DebugDraw() {
				Debug::DrawCube(Vector3(position.x, 10, position.y), Vector3(size.x, 20, size.y), Vector4(0.7f,0.2f,0,1));
				if (children) {
					for (int i = 0; i < 4; ++i) {
						children[i].DebugDraw();
					}
				}
			}

			void OperateOnContents(QuadTreeFunc& func) {
				if (children) {
					for (int i = 0; i < 4; ++i) {
						children[i].OperateOnContents(func);
					}
				}
				else {
					if (!contents.empty()) {
						func(contents);
					}
				}
			}

			void RayCastList(Ray& r, std::list<T>& list)
			{
				RayCollision c;
				if (CollisionDetection::RayBoxIntersection(r, Vector3(position.x, 0, position.y), Vector3(size.x, 100, size.y), c))
				{
					if (children) {
						for (int i = 0; i < 4; ++i) {
							children[i].RayCastList(r, list);
						}
					}
					else {
						if (!contents.empty()) {
							for(auto i : contents)
								list.push_back(i.object);
						}
					}
				}
			}

			void DeleteFourChildren(QuadTreeNode<T>* children)
			{
				if (!children) return;
				for (int i = 0; i < 3; i++)
				{
					DeleteFourChildren(children[i].children);
				}
				tree->DeleteFourChildren(children);
			}


		protected:
			std::list<QuadTreeEntry<T>>	contents;

			
			Vector2 position;
			Vector2 size;

			QuadTreeNode<T>* children;
			QuadTree<T>* tree;
		};
	}
}


namespace NCL {
	using namespace NCL::Maths;
	namespace CSC8503 {
		template<class T>
		class QuadTree
		{
		public:
			QuadTree(Vector2 size, int maxDepth = 6, int maxSize = 5) {
				root = new QuadTreeNode<T>(Vector2(), size, this);
				this->size = size;
				this->maxDepth = maxDepth;
				this->maxSize = maxSize;
			}
			~QuadTree() {
			}

			void Update() {
				delete root;
				root = new QuadTreeNode<T>(Vector2(), size, this);
			}

			void Insert(T object, const Vector3& pos, const Vector3& size) {
				root->Insert(object, pos, size, maxDepth, maxSize);
			}

			//void Check()

			void DebugDraw() {
				root->DebugDraw();
			}

			void OperateOnContents(typename QuadTreeNode<T>::QuadTreeFunc  func) {
				root->OperateOnContents(func);
			}

			std::list<T> RayCastList(Ray& r) {
				std::list<T> list;
				root->RayCastList(r, list);
				return list;
			}

			
			QuadTreeNode<T>* GetFourChildren()
			{
				if (freefourQuadTreeNodes.empty())
					return new QuadTreeNode<T>[4];
				else
				{
					QuadTreeNode<T>* temp = freefourQuadTreeNodes.back();
					freefourQuadTreeNodes.pop_back();
					return temp;
				}
			}

			void DeleteFourChildren(QuadTreeNode<T>* children)
			{
				freefourQuadTreeNodes.push_back(children);
			}

		protected:
			QuadTreeNode<T> *root;
			std::vector<QuadTreeNode<T>*> freefourQuadTreeNodes;
			Vector2 size;
			int maxDepth;
			int maxSize;
		};
	}
}