#include "SpatialPartitioning.h"
#include <array>
#include <random>
#include "SimpleScene_Quad.h"

#define MAX_DEPTH 8
#define MIN_LEAF_SIZE 30

namespace SpatialPartitioning
{
	TreeNode* SpatialPartitioning::BuildOctTree(glm::vec3 center, float halfWidth, int level, int col, std::vector<GameObject>& objs)
	{
		// Base case
		if (level < 0) //max depth
			return nullptr;
		
		// 1. Create new node Node( center, halfwidth )
		// 2. Assign pChildren = nullptr, pObjects = nullptr
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
		for (auto& obj : objs)
		{
			if (isThereObjInParent)
				break;
			Collision::AABB parentAABB(center + halfWidth * glm::vec3(-1.f, -1.f, -1.f), center + halfWidth * glm::vec3(1.f, 1.f, 1.f));

			if (Collision::AABBAABB(parentAABB, obj.aabbBV))
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
			// 3. Calculate new center and halfwidth
			offset.x = ((i & 1) ? step : -step);
			offset.y = ((i & 2) ? step : -step);
			offset.z = ((i & 4) ? step : -step);
			pNode->pChildren[i] = BuildOctTree(center + offset, step, level - 1, ++col, objs);
			if (pNode->pChildren[i])
				pNode->pChildren[i]->colour = childrenColours;
		}
		return pNode;
	}

	int InsertIntoOctTree(TreeNode* pNode, GameObject* newObject)
	{
		// Calculate which child cell the center of newObject is in
		// Check if newObject straddles multiple child nodes
		std::pair<int, bool> result = getChildIndex(pNode, newObject);
		if (!result.second && pNode->pChildren[result.first]) //if doesnt straddle
		{
			// Does not straddle the child nodes
			// Push the object into the cell that contains its center 
			InsertIntoOctTree(pNode->pChildren[result.first], newObject);
		}
		else
		{
			// Node straddles the child cells, so store it at parent level 
			// Insert into this node’s list of objects
			//newObject->depth = pNode->depth; //assign depth used for rendering later
			newObject->octTreeNode = pNode; //assign depth used for rendering later (diff child diff colour)
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
			glm::vec3 centreOfObj = (newObject->aabbBV.m_Max + newObject->aabbBV.m_Min) * 0.5f;
			float d = (centreOfObj[axis] - pNode->center[axis]);
			// Check if d is within bounds of the BV
			float objHalfExtents = (newObject->aabbBV.m_Max[axis] - newObject->aabbBV.m_Min[axis]) * 0.5f;
 			if (abs(d) <= objHalfExtents)
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

	// Classify point p to a plane thickened by a given thickness epsilon
	POINT_ATTRIB ClassifyPointToPlane(glm::vec3 p, Collision::Plane plane)
	{
		constexpr float PLANE_THICKNESS_EPSILON = 0.1f;
		// Compute signed distance of point from plane
		float dist = glm::dot(plane.m_Normal, p);
		// Classify p based on the signed distance
		if (dist > plane.m_d + PLANE_THICKNESS_EPSILON)
			return POINT_ATTRIB::POINT_IN_FRONT_OF_PLANE;
		if (dist < plane.m_d  - PLANE_THICKNESS_EPSILON)
			return POINT_ATTRIB::POINT_BEHIND_PLANE;
		return POINT_ATTRIB::POINT_ON_PLANE;
	}

	// Return value specifying whether the polygon ‘poly’ lies in front of,
	// behind of, on, or straddles the plane ‘plane’
	POLYGON_ATTRIB ClassifyPolygonToPlane(Polygon poly, Collision::Plane plane)
	{
		// Loop over all polygon vertices and count how many vertices
		// lie in front of and how many lie behind of the thickened plane
		int numInFront = 0, numBehind = 0;
		int numVerts = poly.vertices.size();
		for (int i = 0; i < numVerts; i++) {
			glm::vec3 p = poly.vertices[i];
			switch (ClassifyPointToPlane(p, plane)) {
				case POINT_ATTRIB::POINT_IN_FRONT_OF_PLANE:
					numInFront++;
					break;
				case POINT_ATTRIB::POINT_BEHIND_PLANE:
					numBehind++;
					break;
			}
		}
		// If vertices on both sides of the plane, the polygon is straddling
		if (numBehind != 0 && numInFront != 0)
			return POLYGON_ATTRIB::POLYGON_STRADDLING_PLANE;
		// If one or more vertices in front of the plane and no vertices behind
		// the plane, the polygon lies in front of the plane
		if (numInFront != 0)
			return POLYGON_ATTRIB::POLYGON_IN_FRONT_OF_PLANE;
		// Ditto, the polygon lies behind the plane if no vertices in front of
		// the plane, and one or more vertices behind the plane
		if (numBehind != 0)
			return POLYGON_ATTRIB::POLYGON_BEHIND_PLANE;
		// All vertices lie on the plane so the polygon is coplanar with the plane
		return POLYGON_ATTRIB::POLYGON_COPLANAR_WITH_PLANE;
	}

	Collision::Plane GetPlaneFromPolygon(Polygon poly)
	{
		//Assuming that all points in poly lie on a single plane
		glm::vec3 dir1 = poly.vertices[0] - poly.vertices[1];
		glm::vec3 dir2 = poly.vertices[0] - poly.vertices[2];
		glm::vec3 normal = glm::normalize(glm::cross(dir1, dir2));
		Collision::Plane plane(normal, glm::dot(normal, poly.vertices[0]));
		return plane;
	}

	// Given a vector of polygons, attempts to compute a good splitting plane
	Collision::Plane PickSplittingPlane(const std::vector<Polygon>& polygons)
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
			Collision::Plane plane = GetPlaneFromPolygon(polygons[i]);
			// Test against all other polygons
			for (int j = 0; j < polygons.size(); j++) {
				// Ignore testing against self
				if (i == j) continue;
				// Keep standing count of the various poly-plane relationships
				switch (ClassifyPolygonToPlane(polygons[j], plane)) {
					case POLYGON_ATTRIB::POLYGON_COPLANAR_WITH_PLANE:
						/* Coplanar polygons treated as being in front of plane */
					case POLYGON_ATTRIB::POLYGON_IN_FRONT_OF_PLANE:
						numInFront++;
						break;
					case POLYGON_ATTRIB::POLYGON_BEHIND_PLANE:
						numBehind++;
						break;
					case POLYGON_ATTRIB::POLYGON_STRADDLING_PLANE:
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

	void SplitPolygon(Polygon& poly, Collision::Plane plane, Polygon& frontPoly, Polygon& backPoly)
	{
		std::vector<glm::vec3> frontVerts;
		std::vector<glm::vec3> backVerts;
		// Test all edges (a, b) starting with edge from last to first vertex
		glm::vec3 a = poly.vertices[poly.vertices.size() - 1];
		POINT_ATTRIB aSide = ClassifyPointToPlane(a, plane);
		// Loop over all edges given by vertex pair (n - 1, n)
		for (int n = 0; n < poly.vertices.size(); n++) {
			glm::vec3 b = poly.vertices[n];
			POINT_ATTRIB bSide = ClassifyPointToPlane(b, plane);
			if (bSide == POINT_ATTRIB::POINT_IN_FRONT_OF_PLANE) {
				if (aSide == POINT_ATTRIB::POINT_BEHIND_PLANE) {
					// Edge (a, b) straddles, output intersection point to both sides
					glm::vec3 i;
					float t; //not used
					//Outputs intersectiion point i between edge and plane
					IntersectSegmentPlane(a, b, plane, t, i);
					assert(ClassifyPointToPlane(i, plane) == POINT_ATTRIB::POINT_ON_PLANE);
					frontVerts.push_back(i);
					backVerts.push_back(i);
				}
				// In all three cases, output b to the front side
				frontVerts.push_back(b);
			}
			else if (bSide == POINT_ATTRIB::POINT_BEHIND_PLANE) {
				if (aSide == POINT_ATTRIB::POINT_IN_FRONT_OF_PLANE) {
					// Edge (a, b) straddles plane, output intersection point
					glm::vec3 i;
					float t; //not used
					//Outputs intersectiion point i between edge and plane
					IntersectSegmentPlane(a, b, plane, t, i); 
					assert(ClassifyPointToPlane(i, plane) == POINT_ATTRIB::POINT_ON_PLANE);
					frontVerts.push_back(i);
					backVerts.push_back(i);
				}
				else if (aSide == POINT_ATTRIB::POINT_ON_PLANE) {
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
				if (aSide == POINT_ATTRIB::POINT_BEHIND_PLANE)
				{
					backVerts.push_back(b);
				}
			}
			// Keep b as the starting point of the next edge
			a = b;
			aSide = bSide;
		}
		// Create (and return) two new polygons from the two vertex lists
		Polygon frontP (frontVerts);
		Polygon backP (backVerts);

		frontPoly = frontP;
		backPoly = backP;
	}

	std::vector<Polygon> getPolygonsFromModel(const Model& model)
	{
		std::vector<Polygon> polygons;

		for (auto& mesh : model.meshes)
		{
			const auto& idx = mesh.indices;
			const auto& v = mesh.vertices;

			for (size_t i = 0; i < idx.size(); i += 3)
			{
				std::vector<glm::vec3> vertices{ v[idx[i]].Position , v[idx[i+1]].Position , v[idx[i+2]].Position };
				Polygon poly(vertices); //1 triangle 1 polygon
				polygons.push_back(poly);
			}
		}

		//Form polygons
		return polygons;
	}

	std::vector<Polygon> getPolygonsOfObj(const std::vector<Polygon>& modelPolys, GameObject& obj)
	{
		std::vector<Polygon> polygons;
		for (auto& modelPoly : modelPolys)
		{
			Polygon poly(modelPoly);
			for (auto& x : poly.vertices)
			{
				x += obj.transform.Position; //account for offset of game OBJECT position in world space (assumes no scaling, rotation)
			}
			polygons.push_back(poly);
		}
		return polygons;
	}

	BSPNode* BuildBSPTree(const std::vector<Polygon>& polygons, int depth)
	{
		// Return NULL tree if there are no polygons
		if (polygons.empty()) return NULL;
		// Get number of polygons in the input vector

		// If criterion for a leaf is matched, create a leaf node from remaining polygons
		if (polygons.size() <= minPolyCount) //|| ...etc...)
		return new BSPNode(polygons);
		// Select best possible partitioning plane based on the input geometry
		Collision::Plane splitPlane = PickSplittingPlane(polygons);
		std::vector<Polygon> frontList, backList;
		// Test each polygon against the dividing plane, adding them
		// to the front list, back list, or both, as appropriate
		for (int i = 0; i < polygons.size(); i++) {
			Polygon poly = polygons[i], frontPart, backPart;
			switch (ClassifyPolygonToPlane(poly, splitPlane)) {
				case POLYGON_ATTRIB::POLYGON_COPLANAR_WITH_PLANE:
					// What’s done in this case depends on what type of tree is being
					// built. For a node-storing tree, the polygon is stored inside
					// the node at this level (along with all other polygons coplanar
					// with the plane). Here, for a leaf-storing tree, coplanar polygons
					// are sent to either side of the plane. In this case, to the front
					// side, by falling through to the next case
				case POLYGON_ATTRIB::POLYGON_IN_FRONT_OF_PLANE:
					frontList.push_back(poly);
					break;
				case POLYGON_ATTRIB::POLYGON_BEHIND_PLANE:
					backList.push_back(poly);
					break;
				case POLYGON_ATTRIB::POLYGON_STRADDLING_PLANE:
					// Split polygon to plane and send a part to each side of the plane
					SplitPolygon(poly, splitPlane, frontPart, backPart);
					frontList.push_back(frontPart);
					backList.push_back(backPart);
					break;
			}
		}

		// Not even being split case
		if (backList.size() == polygons.size() || frontList.size() == polygons.size())
			return new BSPNode(polygons); //leaf case

		// Recursively build child subtrees and return new tree root combining them
		BSPNode* frontTree = BuildBSPTree(frontList, depth + 1);
		BSPNode* backTree = BuildBSPTree(backList, depth + 1);

		return new BSPNode(frontTree, backTree);
	}

	BSPNode::BSPNode(const std::vector<Polygon>& polygons) //Leaf nodes
	{
		Model polygonData;
		polygonData.loadBSPPolygons(polygons);
		this->currType = Type::LEAF;
		this->geometry = polygonData;

		std::random_device rd;
		std::default_random_engine eng(rd());
		std::uniform_real_distribution<float> distr(0.1f, 1.f);
		this->colour = glm::vec3(distr(eng), distr(eng), distr(eng));
	}

}

