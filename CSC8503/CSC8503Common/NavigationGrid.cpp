#include "NavigationGrid.h"
#include "../../Common/Assets.h"

#include <fstream>
#define check(a, b) ((a) & (b) ? 1 : 0)
using namespace NCL;
using namespace CSC8503;

const int LEFT_NODE = 0;
const int RIGHT_NODE = 1;
const int TOP_NODE = 2;
const int BOTTOM_NODE = 3;

const char WALL_NODE = 'x';
const char FLOOR_NODE = '.';

NavigationGrid::NavigationGrid() {
	nodeSize = 0;
	gridWidth = 0;
	gridHeight = 0;
	allNodes = nullptr;
}

NCL::CSC8503::NavigationGrid::NavigationGrid(int tileSize, int width, int height, int* node)
{
	this->nodeSize = tileSize;
	gridWidth = width;
	gridHeight = height;
	allNodes = new GridNode[gridWidth * gridHeight];
	for (int x = 0; x < gridWidth; ++x) {
		for (int y = 0; y < gridHeight; ++y)
		{
			GridNode& n = allNodes[(gridHeight * x) + y];

			n.type = node[(gridHeight * x) + y];
			n.position = Vector3((float)(x * tileSize), 0, (float)(y * tileSize));
		}
	}
	BuildNodes();
}

NavigationGrid::NavigationGrid(const std::string& filename) : NavigationGrid() {
	std::ifstream infile(Assets::DATADIR + filename);

	infile >> nodeSize;
	infile >> gridWidth;
	infile >> gridHeight;

	allNodes = new GridNode[gridWidth * gridHeight];

	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode& n = allNodes[(gridWidth * y) + x];
			char type = 0;
			infile >> type;
			n.type = type;
			n.position = Vector3((float)(x * gridWidth), 0, (float)(y * gridHeight));
		}
	}
	BuildNodes();

}

NavigationGrid::~NavigationGrid() {
	delete[] allNodes;
}

void NCL::CSC8503::NavigationGrid::BuildNodes()
{
	//now to build the connectivity between the nodes
	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode& n = allNodes[(gridWidth * y) + x];

			if (y > 0) { //get the above node
				n.connected[0] = &allNodes[(gridWidth * (y - 1)) + x];
			}
			if (y < gridHeight - 1) { //get the below node
				n.connected[1] = &allNodes[(gridWidth * (y + 1)) + x];
			}
			if (x > 0) { //get left node
				n.connected[2] = &allNodes[(gridWidth * (y)) + (x - 1)];
			}
			if (x < gridWidth - 1) { //get right node
				n.connected[3] = &allNodes[(gridWidth * (y)) + (x + 1)];
			}
			for (int i = 0; i < 4; ++i) {

				if (n.connected[i]) {
					if (n.connected[i]->type & 1) {
						n.costs[i] = 1;
					}
					else {
						n.connected[i] = nullptr; //actually a wall, disconnect!
					}
				}
			}
		}
	}
}

#include <limits>
#include <queue>

bool NCL::CSC8503::my_comparison(const GridNode* lhs, const GridNode* rhs)
{
	return (lhs->f < rhs->f);
}

bool NCL::CSC8503::NavigationGrid::FindPath(const int& fromX, const int& fromZ, const int& toX, const int& toZ, NavigationPath& outPath)
{
	if (fromX < 0 || fromX > gridWidth - 1 ||
		fromZ < 0 || fromZ > gridHeight - 1) {
		return false; // outside of map region !
	}

	if (toX < 0 || toX > gridWidth - 1 ||
		toZ < 0 || toZ > gridHeight - 1) {
		return false; // outside of map region !
	}

	GridNode* startNode = &allNodes[(fromZ * gridWidth) + fromX];
	GridNode* endNode = &allNodes[(toZ * gridWidth) + toX];
	std::vector < GridNode*> q;

	for (int i = 0; i < gridWidth * gridHeight; i++) {
		if (allNodes + i == startNode)
			allNodes[i].f = allNodes[i].g = 0;
		else 
			allNodes[i].f = allNodes[i].g = std::numeric_limits<float>::max();
		allNodes[i].parent = nullptr;
	}

	std::set<GridNode*, decltype(&my_comparison)> openList(&my_comparison);
	//std::set<GridNode*, GridNode> openList;
	std::vector<GridNode*> closedList;

	openList.insert(startNode);

	startNode->f = 0;
	startNode->g = 0;
	startNode->parent = nullptr;

	GridNode* currentBestNode = nullptr;
	while (!openList.empty()) {
		currentBestNode = *openList.begin();
		openList.erase(openList.begin());
		if (currentBestNode == endNode) {// we ¡¯ve found the path !
			GridNode* node = endNode;
			while (node != nullptr) {
				outPath.PushWaypoint(node->position);
				node = node->parent; // Build up the waypoints
			}
			return true;
		}
		else {
			for (int i = 0; i < 4; ++i) {
				GridNode* neighbour = currentBestNode->connected[i];
				if (!neighbour) { // might not be connected ...
					continue;
				}
				bool inClosed = NodeInList(neighbour, closedList);
				if (inClosed) {
					continue; // already discarded this neighbour ...
				}

				float h = Heuristic(neighbour, endNode);
				float g = currentBestNode->g + neighbour->costs[i];
				float f =  g;

				bool inOpen = NodeInSet(neighbour, openList);

				if (!inOpen) { // first time we ¡¯ve seen this neighbour
					openList.insert(neighbour);
				}
				// might be a better route to this node !
				if (!inOpen || f < neighbour->f) {
					neighbour->parent = currentBestNode;
					neighbour->f = f;
					neighbour->g = g;
				}
				//openList.insert(neighbour);
			}
			closedList.emplace_back(currentBestNode);
		}

	}
	return false; // open list emptied out with no path !
}

bool NavigationGrid::FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) {
	// need to work out which node ¡¯from ¡¯ sits in , and ¡¯to ¡¯ sits in
	int fromX = from.x / nodeSize;
	int fromZ = from.z / nodeSize;

	int toX = (to.x + gridWidth / 2) / nodeSize;
	int toZ = (to.z + gridHeight / 2) / nodeSize;
	return FindPath(fromX, fromZ, toX, toZ, outPath);
}

bool NavigationGrid::NodeInList(GridNode* n, std::vector<GridNode*>& list) const {
	std::vector<GridNode*>::iterator i = std::find(list.begin(), list.end(), n);
	return i == list.end() ? false : true;
}

bool NCL::CSC8503::NavigationGrid::NodeInSet(GridNode* n, std::set<GridNode*, decltype(&my_comparison)>& list) const
{
	auto i = std::find(list.begin(), list.end(), n);
	return i == list.end() ? false : true;
}

GridNode* NavigationGrid::RemoveBestNode(std::vector<GridNode*>& list) const {
	std::vector<GridNode*>::iterator bestI = list.begin();

	GridNode* bestNode = *list.begin();

	for (auto i = list.begin(); i != list.end(); ++i) {
		if ((*i)->f < bestNode->f) {
			bestNode = (*i);
			bestI = i;
		}
	}
	list.erase(bestI);
	return bestNode;
}

float NavigationGrid::Heuristic(GridNode* hNode, GridNode* endNode) const {
	float dis = abs(hNode->position.x - hNode->position.x);
	dis += abs(hNode->position.z - hNode->position.z);
	return dis;
	//return (hNode->position - endNode->position).Length();
}