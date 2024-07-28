#pragma once
#include "VisualEntity.h"
#include "Collision.h"

namespace SpatialPartitioning
{

	// Enum Classes
	enum class point_place
	{
		point_front,
		point_back,
		point_plane
	};

	enum class poly_place
	{
		onplane,
		planefront,
		planeback,
		coplane
	};

	// BSPNode Structure
	struct BSPNode
	{
		Model data_g;           // Leaf nodes need to store data_g data
		unsigned int depth{};   // Depth of the node in the tree
		glm::vec3 colour{};     // Colour of the node

		enum class Type
		{
			INTERNAL,
			LEAF
		};
		Type currType{};        // Current type of the node

		BSPNode* tree_front{};  // Pointer to the front child node
		BSPNode* tree_back{};   // Pointer to the back child node

		// Constructor for internal nodes
		BSPNode(BSPNode* front, BSPNode* back) :
			tree_front{ front }, tree_back{ back }
		{
			currType = Type::INTERNAL;
		}

		// Constructor for leaf nodes
		BSPNode(const std::vector<poly_shape>& polygons);
	};


	// TreeNode Structure
	struct TreeNode
	{
		glm::vec3 colour{};     // Colour of the node
		int col{};              // Additional attribute (could be used for color index, etc.)
		Model data_g;           // Store data_g data
		glm::vec3 center;       // Center of the node
		float mid_width;        // Half-width of the node
		TreeNode* childp[8];    // Array of pointers to child nodes
		unsigned int depth;     // Depth of the node in the tree

		// Constructor
		TreeNode(glm::vec3 center, float halfWidth, TreeNode* childrenArray) :
			center{ center }, mid_width{ halfWidth }, childp{ childrenArray } {}
	};


	// Poly Shape Structure
	struct poly_shape // should be just triangles
	{
		std::vector<glm::vec3> vertices;
		// std::vector<unsigned int> indices;
		poly_shape() {};
		poly_shape(std::vector<glm::vec3>& vertices) : vertices{ vertices } {}
	};

	

	// Function Declarations
	std::vector<poly_shape> getPolygonsFromModel(const Model& model);
	std::vector<poly_shape> getPolygonsOfObj(const std::vector<poly_shape>& modelPolys, VisualEntity& obj);
	TreeNode* BuildOctTree(glm::vec3 center, float halfWidth, int level, int col, std::vector<poly_shape>& objs);
	int InsertIntoOctTree(TreeNode* pNode, const std::vector<poly_shape>& objpolys);
	point_place ClassifyPointToPlane(glm::vec3 p, Collision::Plane plane);
	Collision::Plane polygon_GetPlane(poly_shape poly);
	poly_place CheckPolytoPlane(poly_shape poly, Collision::Plane plane);
	Collision::Plane Splitting_plane(const std::vector<poly_shape>& polygons);
	void Poly_Split(poly_shape& poly, Collision::Plane plane, poly_shape& frontPoly, poly_shape& backPoly);
	BSPNode* bsp_build(const std::vector<poly_shape>& polygons, int depth);
}
