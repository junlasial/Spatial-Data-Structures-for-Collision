#pragma once
#include "GameObject.h"
#include "Collision.h"

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
		int col{};
		glm::vec3 colour{};
	};

	TreeNode* BuildOctTree(glm::vec3 center, float halfWidth, int level, int col, std::vector<GameObject>& objs);
	int InsertIntoOctTree(TreeNode* pNode, GameObject* newObject);
	std::pair<int, bool> getChildIndex(TreeNode* pNode, GameObject* newObject);

	struct Polygon //should be just triangles
	{
		std::vector<glm::vec3> vertices;
		//std::vector<unsigned int> indices;
		Polygon() {};
		Polygon(std::vector<glm::vec3>& vertices) : vertices { vertices } {}
	};

	struct BSPNode
	{
		enum class Type
		{
			INTERNAL,
			LEAF
		};
		//std::vector<Polygon*> geometry{}; 
		Model geometry; //leaf nodes need to store geometry data
		Type currType{};
		BSPNode* frontTree{}; //child node
		BSPNode* backTree{}; //child node
		unsigned int depth{}; //depth
		glm::vec3 colour{};
		BSPNode(BSPNode* front, BSPNode* back) :
			frontTree{ front }, backTree{ back }
		{
			currType = Type::INTERNAL;
		}

		BSPNode(const std::vector<Polygon>& polygons);// create a leaf node
	};
	
	enum class POINT_ATTRIB
	{
		POINT_IN_FRONT_OF_PLANE,
		POINT_BEHIND_PLANE,
		POINT_ON_PLANE
	};
	POINT_ATTRIB ClassifyPointToPlane(glm::vec3 p, Collision::Plane plane);

	enum class POLYGON_ATTRIB
	{
		POLYGON_STRADDLING_PLANE,
		POLYGON_IN_FRONT_OF_PLANE,
		POLYGON_BEHIND_PLANE,
		POLYGON_COPLANAR_WITH_PLANE
	};

	std::vector<Polygon> getPolygonsFromModel(const Model& model);
	std::vector<Polygon> getPolygonsOfObj(const std::vector<Polygon>& modelPolys, GameObject& obj);

	Collision::Plane GetPlaneFromPolygon(Polygon poly);
	POLYGON_ATTRIB ClassifyPolygonToPlane(Polygon poly, Collision::Plane plane);
	Collision::Plane PickSplittingPlane(const std::vector<Polygon>& polygons);
	void SplitPolygon(Polygon& poly, Collision::Plane plane, Polygon& frontPoly, Polygon& backPoly);


	BSPNode* BuildBSPTree(const std::vector<Polygon>& polygons, int depth);
	
}
