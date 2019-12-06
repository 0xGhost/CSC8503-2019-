#pragma once
#include "NavigationMap.h"
#include <string>
#include <set>





namespace NCL {
	namespace CSC8503 {



		struct GridNode {
			GridNode* parent;

			GridNode* connected[4];
			int		  costs[4];

			Vector3		position;

			float f;
			float g;

			int type;

			GridNode() {
				for (int i = 0; i < 4; ++i) {
					connected[i] = nullptr;
					costs[i] = 0;
				}
				f = 0;
				g = 0;
				type = 0;
				parent = nullptr;
			}
			~GridNode() {	}

			bool operator<(GridNode rhs) const
			{
				return (f < rhs.f);
			}
		};

		bool  my_comparison(const GridNode* lhs, const GridNode* rhs);
		//typedef set_funcomp< GridNode, my_comparison >::t defaultSet;

		class NavigationGrid : public NavigationMap	{
		public:
			NavigationGrid();
			NavigationGrid(int nodeSize, int width, int height, int* node);
			NavigationGrid(const std::string&filename);
			~NavigationGrid();
			
			bool FindPath(const int& fromX, const int& fromZ, const int& toX, const int& toZ, NavigationPath& outPath) override;
			bool FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) override;
				
		protected:
			void BuildNodes();

			bool		NodeInList(GridNode* n, std::vector<GridNode*>& list) const;
			bool		NodeInSet(GridNode* n, std::set<GridNode*, decltype(&my_comparison)>& list) const;
			GridNode*	RemoveBestNode(std::vector<GridNode*>& list) const;
			float		Heuristic(GridNode* hNode, GridNode* endNode) const;
			int nodeSize;
			int gridWidth;
			int gridHeight;

			GridNode* allNodes;
		};
	}
}

