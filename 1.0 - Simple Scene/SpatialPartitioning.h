#pragma once
#include "GameObject.h"

namespace SpatialPartitioning
{
	struct TreeNode
	{
		glm::vec3 center;
		float halfwidth;
		TreeNode* pChildren[8];
		std::vector<GameObject*> pObjects{};
		unsigned int depth;
		TreeNode(glm::vec3 center, float halfWidth, TreeNode* childrenArray) :
			center{ center }, halfwidth{ halfWidth }, pChildren{ childrenArray } {}
	};

	TreeNode* BuildOctTree(glm::vec3 center, float halfWidth, int level);
	int InsertIntoOctTree(TreeNode* pNode, GameObject* newObject);
	std::pair<int, bool> getChildIndex(TreeNode* pNode, GameObject* newObject);
	

}
