#include "partition.h"
#include <array>
#include <random>
#include "modelscene.h"

#define MAX_DEPTH 8
#define MIN_LEAF_SIZE 30

namespace partition
{
	TreeNode* partition::BuildOctTree(glm::vec3 center, float halfWidth, int level, int col, std::vector<poly_shape>& polys)
	{
		// Base case
		if (level < 0) //max depth
			return nullptr;

		// 1. Create new node Node( center, mid_width )
		// 2. Assign childp = nullptr, pObjects = nullptr
		TreeNode* pNode = new TreeNode(center, halfWidth, nullptr);
		pNode->depth = level;
		pNode->col = col; //render diff children diff col
		std::random_device rd;
		std::default_random_engine eng(rd());
		std::uniform_real_distribution<float> distr(0.f, 1.f);
		pNode->colour = glm::vec3(distr(eng), distr(eng), distr(eng));
		glm::vec3 offset;
		float step = halfWidth * 0.5f;

		bool isThereObjInParent = false;
		for (auto& poly : polys)
		{
			if (isThereObjInParent)
				break;
			Collision::AABB parentAABB(center + halfWidth * glm::vec3(-1.f, -1.f, -1.f), center + halfWidth * glm::vec3(1.f, 1.f, 1.f));

			Collision::Triangle tri(poly.vertices[0], poly.vertices[1], poly.vertices[2]);
			if (Collision::AABB_tri(tri, parentAABB))
			{
				isThereObjInParent = true;
				break;
			}
		}

		if (isThereObjInParent == false)
			return pNode; //stop creating 8 subtrees

		glm::vec3 childrenColours = glm::vec3(distr(eng), distr(eng), distr(eng)); //same depth
		for (size_t i = 0; i < 8; i++)
		{
			// 3. Calculate new center and mid_width
			offset.x = ((i & 1) ? step : -step);
			offset.y = ((i & 2) ? step : -step);
			offset.z = ((i & 4) ? step : -step);
			pNode->childp[i] = BuildOctTree(center + offset, step, level - 1, ++col, polys);
			if (pNode->childp[i])
				pNode->childp[i]->colour = childrenColours;
		}
		return pNode;
	}

	int InsertIntoOctTree(TreeNode* pNode, const std::vector<poly_shape>& objpolys)
	{
		int index = 0, straddle = 0;
		int numberOfNodesIntersected = 0;
		// Compute the octant number [0..7] the object sphere center is in
		// If straddling any of the dividing x, y, or z planes, exit directly

		// Calculate which child cell the center of newObject is in
		// Check if newObject straddles multiple child nodes
		if (objpolys.size() > minPolyCount) //Terminating criteria, split into smaller inserts
		{
			size_t halfSize = objpolys.size() / 2;
			std::vector<poly_shape> objpolys1 (objpolys.begin(), objpolys.begin() + halfSize);
			std::vector<poly_shape> objpolys2 (objpolys.begin() + halfSize, objpolys.end());
			InsertIntoOctTree(pNode, objpolys1);
			InsertIntoOctTree(pNode, objpolys2);
		}
		
		for (size_t i = 0; i < 8; i++)
		{
			if (straddle) break;
			if (pNode->childp[i] == nullptr) break;
			for (auto& poly : objpolys) //check if all the polygons are intersecting the child node
			{
				Collision::Triangle polyTri(poly.vertices[0], poly.vertices[1], poly.vertices[2]);

				Collision::AABB nodeAABB(pNode->childp[i]->center + pNode->childp[i]->mid_width * glm::vec3(-1.f, -1.f, -1.f), pNode->childp[i]->center + pNode->childp[i]->mid_width * glm::vec3(1.f, 1.f, 1.f));

				if (Collision::AABB_tri(polyTri, nodeAABB))
				{
					if (numberOfNodesIntersected > 1)
					{
						straddle = true; //intersected more than 1 node
						index = -1; //throw exception later if accessing
						break;
					}
					++numberOfNodesIntersected;
					index = i; //if not it will return the index of the child that contains the polygons directly
				}
			}
		}

		if (!straddle && pNode->childp[index]) //if doesnt straddle
		{
			// Does not straddle the child nodes
			// Push the object into the cell that contains its center 
			InsertIntoOctTree(pNode->childp[index], objpolys);
		}
		else
		{
			// Node straddles the child cells, so store it at parent level  (I  read somewhere we shouldnt
			// add to both child nodes)
			// Insert into this node’s list of objects (polygon level not gameobj)
			Model polygonData;
			polygonData.loadBSPPolygons(objpolys);
			pNode->data_g = polygonData;
			//newObject->depth = pNode->depth; //assign depth used for rendering later
			//newObject->octTreeNode = pNode; //assign depth used for rendering later (diff child diff colour)
			//pNode->pObjects.push_back(newObject);
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

	void Poly_Split(poly_shape& poly, Collision::Plane plane, poly_shape& frontPoly, poly_shape& backPoly)
	{
		std::vector<glm::vec3> frontVerts;
		std::vector<glm::vec3> backVerts;
		// Test all edges (a, b) starting with edge from last to first vertex
		glm::vec3 a = poly.vertices[poly.vertices.size() - 1];
		point_place aSide = ClassifyPointToPlane(a, plane);
		// Loop over all edges given by vertex pair (n - 1, n)
		for (int n = 0; n < poly.vertices.size(); n++) {
			glm::vec3 b = poly.vertices[n];
			point_place bSide = ClassifyPointToPlane(b, plane);
			if (bSide == point_place::point_front) {
				if (aSide == point_place::point_back) {
					// Edge (a, b) straddles, output intersection point to both sides
					glm::vec3 i;
					float t; //not used
					//Outputs intersectiion point i between edge and plane
					IntersectSegmentPlane(a, b, plane, t, i);
					assert(ClassifyPointToPlane(i, plane) == point_place::point_plane);
					frontVerts.push_back(i);
					backVerts.push_back(i);
				}
				// In all three cases, output b to the front side
				frontVerts.push_back(b);
			}
			else if (bSide == point_place::point_back) {
				if (aSide == point_place::point_front) {
					// Edge (a, b) straddles plane, output intersection point
					glm::vec3 i;
					float t; //not used
					//Outputs intersectiion point i between edge and plane
					IntersectSegmentPlane(a, b, plane, t, i);
					assert(ClassifyPointToPlane(i, plane) == point_place::point_plane);
					frontVerts.push_back(i);
					backVerts.push_back(i);
				}
				else if (aSide == point_place::point_plane) {
					// Output a when edge (a, b) goes from ‘on’ to ‘behind’ plane
					backVerts.push_back(a);
				}
				// In all three cases, output b to the back side
				backVerts.push_back(b);
			}
			else {
				// b is on the plane. In all three cases output b to the front side
				frontVerts.push_back(b);
				// In one case, also output b to back side
				if (aSide == point_place::point_back)
				{
					backVerts.push_back(b);
				}
			}
			// Keep b as the starting point of the next edge
			a = b;
			aSide = bSide;
		}
		// Create (and return) two new polygons from the two vertex lists
		poly_shape frontP(frontVerts);
		poly_shape backP(backVerts);

		frontPoly = frontP;
		backPoly = backP;
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

	BSPNode* bsp_build(const std::vector<poly_shape>& polygons, int depth)
	{
		// Return NULL tree if there are no polygons
		if (polygons.empty()) return NULL;
		// Get number of polygons in the input vector

		// If criterion for a leaf is matched, create a leaf node from remaining polygons
		if (polygons.size() <= minPolyCount) //|| ...etc...)
			return new BSPNode(polygons);
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
			return new BSPNode(polygons); //leaf case

		// Recursively build child subtrees and return new tree root combining them
		BSPNode* tree_front = bsp_build(frontList, depth + 1);
		BSPNode* tree_back = bsp_build(backList, depth + 1);

		return new BSPNode(tree_front, tree_back);
	}

	BSPNode::BSPNode(const std::vector<poly_shape>& polygons) //Leaf nodes
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

