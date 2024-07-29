#include "partition.h"
#include <array>
#include <random>
#include "modelscene.h"

#define MAX_DEPTH 8
#define MIN_LEAF_SIZE 30

namespace partition
{
	// Helper functions for random color generation
	glm::vec3 generateRandomColor() {
		std::random_device rd;
		std::default_random_engine eng(rd());
		std::uniform_real_distribution<float> distr(0.f, 1.f);
		return glm::vec3(distr(eng), distr(eng), distr(eng));
	}

	// Helper function to check if a polygon is in the parent AABB
	bool isPolygonInParentAABB(const std::vector<poly_shape>& polys, const glm::vec3& center, float halfWidth) {
		Collision::AABB parentAABB(center + halfWidth * glm::vec3(-1.f, -1.f, -1.f), center + halfWidth * glm::vec3(1.f, 1.f, 1.f));
		for (const auto& poly : polys) {
			Collision::Triangle tri(poly.vertices[0], poly.vertices[1], poly.vertices[2]);
			if (Collision::AABB_tri(tri, parentAABB)) {
				return true;
			}
		}
		return false;
	}

	// Helper function to create a TreeNode
	TreeNode* createTreeNode(glm::vec3 center, float halfWidth, int level, int col) {
		TreeNode* pNode = new TreeNode(center, halfWidth, nullptr);
		pNode->depth = level;
		pNode->col = col;
		pNode->colour = generateRandomColor();
		return pNode;
	}

	TreeNode* BuildOctTree(glm::vec3 center, float halfWidth, int level, int col, std::vector<poly_shape>& polys) {
		// Base case
		if (level < 0) return nullptr;

		// Create a new node
		TreeNode* pNode = createTreeNode(center, halfWidth, level, col);

		// Check if there are polygons in the parent AABB
		if (!isPolygonInParentAABB(polys, center, halfWidth))
			return pNode;

		// Recursive subdivision for child nodes
		glm::vec3 offset;
		float step = halfWidth * 0.5f;
		glm::vec3 childrenColours = generateRandomColor();

		for (size_t i = 0; i < 8; i++) {
			offset.x = ((i & 1) ? step : -step);
			offset.y = ((i & 2) ? step : -step);
			offset.z = ((i & 4) ? step : -step);
			pNode->childp[i] = BuildOctTree(center + offset, step, level - 1, ++col, polys);
			if (pNode->childp[i])
				pNode->childp[i]->colour = childrenColours;
		}
		return pNode;
	}


	bool checkStraddle(TreeNode* pNode, const std::vector<poly_shape>& objpolys, int& index) {
		int numberOfNodesIntersected = 0;
		index = -1;

		for (int i = 0; i < 8; ++i) {
			if (pNode->childp[i] == nullptr) continue;

			for (const auto& poly : objpolys) {
				Collision::Triangle polyTri(poly.vertices[0], poly.vertices[1], poly.vertices[2]);
				Collision::AABB nodeAABB(pNode->childp[i]->center + pNode->childp[i]->mid_width * glm::vec3(-1.f, -1.f, -1.f),
					pNode->childp[i]->center + pNode->childp[i]->mid_width * glm::vec3(1.f, 1.f, 1.f));

				if (Collision::AABB_tri(polyTri, nodeAABB)) {
					if (numberOfNodesIntersected > 1) return true; // straddle
					index = i;
					++numberOfNodesIntersected;
				}
			}
		}
		return false;
	}

	void splitAndInsert(TreeNode* pNode, const std::vector<poly_shape>& objpolys) {
		size_t halfSize = objpolys.size() / 2;
		std::vector<poly_shape> objpolys1(objpolys.begin(), objpolys.begin() + halfSize);
		std::vector<poly_shape> objpolys2(objpolys.begin() + halfSize, objpolys.end());

		InsertIntoOctTree(pNode, objpolys1);
		InsertIntoOctTree(pNode, objpolys2);
	}

	int InsertIntoOctTree(TreeNode* pNode, const std::vector<poly_shape>& objpolys) {
		if (objpolys.size() > minPolyCount) {
			splitAndInsert(pNode, objpolys);
			return 0;
		}

		int index = 0;
		if (checkStraddle(pNode, objpolys, index)) {
			// Handle straddle case
			Model polygonData;
			polygonData.loadBSPPolygons(objpolys);
			pNode->data_g = polygonData;
		}
		else if (index >= 0 && pNode->childp[index]) {
			// Insert into the specific child node
			InsertIntoOctTree(pNode->childp[index], objpolys);
		}
		return index;
	}
	// Classify point p to a plane thickened by a given thickness epsilon
	point_place ClassifyPointToPlane(glm::vec3 p, Collision::Plane plane)
	{
		constexpr float PLANE_THICKNESS_EPSILON = 0.1f;
		// Compute signed distance of point from plane
		float dist = glm::dot(plane.m_Normal, p);
		// Classify p based on the signed distance
		if (dist > plane.m_d + PLANE_THICKNESS_EPSILON)
			return point_place::point_front;
		if (dist < plane.m_d - PLANE_THICKNESS_EPSILON)
			return point_place::point_back;
		return point_place::point_plane;
	}

	// Return value specifying whether the polygon ‘poly’ lies in front of,
	// behind of, on, or straddles the plane ‘plane’
	poly_place CheckPolytoPlane(poly_shape poly, Collision::Plane plane)
	{
		// Loop over all polygon vertices and count how many vertices
		// lie in front of and how many lie behind of the thickened plane
		int numInFront = 0, numBehind = 0;
		int numVerts = poly.vertices.size();
		for (int i = 0; i < numVerts; i++) {
			glm::vec3 p = poly.vertices[i];
			switch (ClassifyPointToPlane(p, plane)) {
			case point_place::point_front:
				numInFront++;
				break;
			case point_place::point_back:
				numBehind++;
				break;
			}
		}
		// If vertices on both sides of the plane, the polygon is straddling
		if (numBehind != 0 && numInFront != 0)
			return poly_place::onplane;
		// If one or more vertices in front of the plane and no vertices behind
		// the plane, the polygon lies in front of the plane
		if (numInFront != 0)
			return poly_place::planefront;
		// Ditto, the polygon lies behind the plane if no vertices in front of
		// the plane, and one or more vertices behind the plane
		if (numBehind != 0)
			return poly_place::planeback;
		// All vertices lie on the plane so the polygon is coplanar with the plane
		return poly_place::coplane;
	}

	Collision::Plane polygon_GetPlane(poly_shape poly)
	{
		//Assuming that all points in poly lie on a single plane
		glm::vec3 dir1 = poly.vertices[0] - poly.vertices[1];
		glm::vec3 dir2 = poly.vertices[0] - poly.vertices[2];
		glm::vec3 normal = glm::normalize(glm::cross(dir1, dir2));
		Collision::Plane plane(normal, glm::dot(normal, poly.vertices[0]));
		return plane;
	}

	// Given a vector of polygons, attempts to compute a good splitting plane
	Collision::Plane Splitting_plane(const std::vector<poly_shape>& polygons)
	{
		// Blend factor for optimizing for balance or splits (should be tweaked)
		const float K = 0.8f;
		// Variables for tracking best splitting plane seen so far
		Collision::Plane bestPlane;
		float bestScore = FLT_MAX;
		// Try the plane of each polygon as a dividing plane
		//Incremental
		int increment = (polygons.size() / 10); //speed things up (NON-OPTIMAL though)
		for (int i = 0; i < polygons.size(); i += increment) {
			int numInFront = 0, numBehind = 0, numStraddling = 0;
			//if (i == 10)
			Collision::Plane plane = polygon_GetPlane(polygons[i]);
			// Test against all other polygons
			for (int j = 0; j < polygons.size(); j++) {
				// Ignore testing against self
				if (i == j) continue;
				// Keep standing count of the various poly-plane relationships
				switch (CheckPolytoPlane(polygons[j], plane)) {
				case poly_place::coplane:
					/* Coplanar polygons treated as being in front of plane */
				case poly_place::planefront:
					numInFront++;
					break;
				case poly_place::planeback:
					numBehind++;
					break;
				case poly_place::onplane:
					numStraddling++;
					break;
				}
			}
			// Compute score as a weighted combination (based on K, with K in range
			// 0..1) between balance and splits (lower score is better)
			float score = K * numStraddling + (1.0f - K) * abs(numInFront - numBehind);
			if (score < bestScore) {
				bestScore = score;
				bestPlane = plane;
			}
		}
		return bestPlane;
	}

	int IntersectSegmentPlane(glm::vec3 a, glm::vec3 b, Collision::Plane p, float& t, glm::vec3& q)
	{
		// Compute the t value for the directed line ab intersecting the plane
		glm::vec3 ab = b - a;
		t = (p.m_d - glm::dot(p.m_Normal, a)) / glm::dot(p.m_Normal, ab);
		// If t in [0..1] compute and return intersection point
		if (t >= 0.0f && t <= 1.0f) {
			q = a + t * ab;
			return 1;
		}
		// Else no intersection
		return 0;
	}

	point_place classifyAndStoreVertex(const glm::vec3& vertex, point_place aSide, Collision::Plane plane,
		std::vector<glm::vec3>& frontVerts, std::vector<glm::vec3>& backVerts) {
		point_place bSide = ClassifyPointToPlane(vertex, plane);

		if (bSide == point_place::point_front) {
			frontVerts.push_back(vertex);
		}
		else if (bSide == point_place::point_back) {
			backVerts.push_back(vertex);
		}
		else {
			frontVerts.push_back(vertex);
			if (aSide == point_place::point_back) {
				backVerts.push_back(vertex);
			}
		}

		return bSide;
	}

	void handleStraddling(const glm::vec3& a, const glm::vec3& b, Collision::Plane plane,
		std::vector<glm::vec3>& frontVerts, std::vector<glm::vec3>& backVerts) {
		glm::vec3 intersectionPoint;
		float t;
		if (IntersectSegmentPlane(a, b, plane, t, intersectionPoint)) {
			frontVerts.push_back(intersectionPoint);
			backVerts.push_back(intersectionPoint);
		}
	}

	void Poly_Split(poly_shape& poly, Collision::Plane plane, poly_shape& frontPoly, poly_shape& backPoly) {
		std::vector<glm::vec3> frontVerts;
		std::vector<glm::vec3> backVerts;

		glm::vec3 a = poly.vertices.back(); // Last vertex
		point_place aSide = ClassifyPointToPlane(a, plane);



		for (const auto& b : poly.vertices) {
			if (aSide != ClassifyPointToPlane(b, plane)) {
				handleStraddling(a, b, plane, frontVerts, backVerts);


			}
			aSide = classifyAndStoreVertex(b, aSide, plane, frontVerts, backVerts);
			a = b;
		}

		frontPoly = poly_shape(frontVerts);

		backPoly = poly_shape(backVerts);
	}

	std::vector<poly_shape> getPolygonsFromModel(const Model& model)
	{
		std::vector<poly_shape> polygons;

		for (auto& mesh : model.meshes)
		{
			const auto& idx = mesh.indices;
			const auto& v = mesh.vertices;

			for (size_t i = 0; i < idx.size(); i += 3)
			{
				std::vector<glm::vec3> vertices{ v[idx[i]].Position , v[idx[i + 1]].Position , v[idx[i + 2]].Position };
				poly_shape poly(vertices); //1 triangle 1 polygon
				polygons.push_back(poly);
			}
		}

		//Form polygons
		return polygons;
	}

	std::vector<poly_shape> getPolygonsOfObj(const std::vector<poly_shape>& modelPolys, VisualEntity& obj)
	{
		std::vector<poly_shape> polygons;
		for (auto& modelPoly : modelPolys)
		{
			poly_shape poly(modelPoly);
			for (auto& x : poly.vertices)
			{
				x += obj.entityTransform.Position; //account for offset of game OBJECT position in world space (assumes no scaling, rotation)
			}
			polygons.push_back(poly);
		}
		return polygons;
	}

	node_bsp* build_bsp(const std::vector<poly_shape>& polygons, int depth)
	{
		// Return NULL tree if there are no polygons
		if (polygons.empty()) return NULL;
		// Get number of polygons in the input vector

		// If criterion for a leaf is matched, create a leaf node from remaining polygons
		if (polygons.size() <= minPolyCount) //|| ...etc...)
			return new node_bsp(polygons);
		// Select best possible partitioning plane based on the input data_g
		Collision::Plane splitPlane = Splitting_plane(polygons);
		std::vector<poly_shape> frontList, backList;
		// Test each polygon against the dividing plane, adding them
		// to the front list, back list, or both, as appropriate
		for (int i = 0; i < polygons.size(); i++) {
			poly_shape poly = polygons[i], frontPart, backPart;
			switch (CheckPolytoPlane(poly, splitPlane)) {
			case poly_place::coplane:
				// What’s done in this case depends on what type of tree is being
				// built. For a node-storing tree, the polygon is stored inside
				// the node at this level (along with all other polygons coplanar
				// with the plane). Here, for a leaf-storing tree, coplanar polygons
				// are sent to either side of the plane. In this case, to the front
				// side, by falling through to the next case
			case poly_place::planefront:
				frontList.push_back(poly);
				break;
			case poly_place::planeback:
				backList.push_back(poly);
				break;
			case poly_place::onplane:
				// Split polygon to plane and send a part to each side of the plane
				Poly_Split(poly, splitPlane, frontPart, backPart);
				frontList.push_back(frontPart);
				backList.push_back(backPart);
				break;
			}
		}

		// Not even being split case
		if (backList.size() == polygons.size() || frontList.size() == polygons.size())
			return new node_bsp(polygons); //leaf case

		// Recursively build child subtrees and return new tree root combining them
		node_bsp* tree_front = build_bsp(frontList, depth + 1);
		node_bsp* tree_back = build_bsp(backList, depth + 1);

		return new node_bsp(tree_front, tree_back);
	}

	node_bsp::node_bsp(const std::vector<poly_shape>& polygons) //Leaf nodes
	{
		Model polygonData;
		polygonData.loadBSPPolygons(polygons);
		this->currType = Type::LEAF;
		this->data_g = polygonData;

		std::random_device rd;
		std::default_random_engine eng(rd());
		std::uniform_real_distribution<float> distr(0.1f, 1.f);
		this->colour = glm::vec3(distr(eng), distr(eng), distr(eng));
	}

}

