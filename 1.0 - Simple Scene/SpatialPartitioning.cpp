#include "SpatialPartitioning.h"
#include <array>

namespace SpatialPartitioning
{
	TreeNode* SpatialPartitioning::BuildOctTree(glm::vec3 center, float halfWidth, int level)
	{
		// Base case
		if (level < 0)
			return nullptr;
		// 1. Create new node Node( center, halfwidth )
		// 2. Assign pChildren = nullptr, pObjects = nullptr
		TreeNode* pNode = new TreeNode(center, halfWidth, nullptr);
		for (size_t i = 0; i < 8; i++)
		{
			// 3. Calculate new center and halfwidth
			pNode->pChildren[i] = BuildOctTree(center, halfWidth, level - 1);
		}
		return pNode;
	}

	int InsertIntoOctTree(TreeNode* pNode, GameObject* newObject)
	{
		// Calculate which child cell the center of newObject is in
		// Check if newObject straddles multiple child nodes
		std::pair<int, bool> result = getChildIndex(pNode, newObject);
		if (!result.second && pNode->pChildren[result.first])
		{
			// Does not straddle the child nodes
			// Push the object into the cell that contains its center 
			InsertIntoOctTree(pNode->pChildren[result.first], newObject);
		}
		else
		{ 
		// Node straddles the child cells, so store it at parent level 
		// Insert into this node’s list of objects
			pNode->pObjects.push_back(newObject);
		}

		return result.first; // can serve as an error code
	}

	std::pair<int, bool> getChildIndex(TreeNode* pNode, GameObject* newObject)
	{
		bool bStraddle = false;
		int index = 0, flag = 1;
		for (size_t axis = 0; axis < 3; axis++) //For each direction axis, { X = 0, Y = 1, Z = 2 }
		{
			float d = (newObject->transform.Position[axis] - pNode->center[axis]);
			// Check if d is within bounds of the BV
			if (abs(d) <= newObject->aabbBV.m_Max[axis] - newObject->aabbBV.m_Min[axis])
			{
				bStraddle = true;
				break;
			}

			// which of + or - value for the bit?
			if (d > 0)
				index |= (1 << axis);
		}
		return std::pair<int, bool> {index, bStraddle}; // can serve as an error code
	}


}

