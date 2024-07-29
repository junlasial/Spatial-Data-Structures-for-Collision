#include <glm/vec3.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "modelscene.h"
#include <../Global/shader.hpp>
#include "transCalculate.h"
#include "BoundingVolume.h"
#include <random>




int minPolyCount = 30;




float model_scale = 1.0f;


Model_Scene::~Model_Scene()
{
	initMembers();
}


Model_Scene::Model_Scene(int windowWidth, int windowHeight) : Scene(windowWidth, windowHeight),
programID(0), angleOfRotation(0.0f)
{
	initMembers();
}


void Model_Scene::initMembers()
{
	programID = 0;
	angleOfRotation = 0.0f;
}

void Model_Scene::CleanUp()
{
	// Cleanup models' resources
	for (auto& model : models)
	{
		model.second.CleanUp();
	}

	// Delete shader program
	if (programID)
	{
		glDeleteProgram(programID);
		programID = 0; // Reset programID to 0 after deletion
	}

	// Free the spatial partition tree if it exists
	if (spatialPartitionTree)
	{
		FreeOctTree(spatialPartitionTree);
		spatialPartitionTree = nullptr; // Set to nullptr after freeing
	}

	// Free the BSP tree if it exists
	if (BSPTree)
	{
		FreeBSPTree(BSPTree);
		BSPTree = nullptr; // Set to nullptr after freeing
	}
}



int Model_Scene::Init()
{
	// Load models
	Model cube;
	cube.loadModel("Global/models/cube.obj");
	models.emplace("Cube", cube);
	intModelID.emplace(5, "Cube");

	Model object1;
	object1.loadModel("Global/models/1/part_a/g0.obj");
	models.emplace("object1", object1);
	intModelID.emplace(2, "object1");

	Model object2;
	object2.loadModel("Global/models/1/part_a/g1.obj");
	models.emplace("object2", object2);
	intModelID.emplace(3, "object2");

	Model object3;
	object3.loadModel("Global/models/1/part_b/g0.obj");
	models.emplace("object3", object3);
	intModelID.emplace(4, "object3");

	Model object4;
	object4.loadModel("Global/models/1/part_b/g1.obj");
	models.emplace("object4", object4);
	intModelID.emplace(1, "object4");

	Model object5;
	object5.loadModel("Global/models/1/part_a/g2.obj");
	models.emplace("object5", object5);
	intModelID.emplace(5, "object5");

	VisualEntity entityA;
	entityA.UpdateTransform(Transform(glm::vec3{ 0.f, 1.0f, -0.65f }, model_scale)); // Default position and scale for entity A
	entityA.AssignModelIdentifier("object1");
	gameObjList.push_back(entityA);
	entityA.entityID = 1;

	VisualEntity entityB;
	entityB.UpdateTransform(Transform(glm::vec3{ 0.f, 1.4f, -0.65f }, model_scale)); // Default position and scale for entity B
	entityB.AssignModelIdentifier("object2");
	gameObjList.push_back(entityB);
	entityB.entityID = 2;

	VisualEntity entityC;
	entityC.UpdateTransform(Transform(glm::vec3{ 0.f, 0.f, 0.f }, model_scale)); // Default position and scale for entity C
	entityC.AssignModelIdentifier("object3");
	gameObjList.push_back(entityC);
	entityC.entityID = 3;

	VisualEntity entityD;
	entityD.UpdateTransform(Transform(glm::vec3{ 0.f, 0.f, 0.f }, model_scale)); // Default position and scale for entity D
	entityD.AssignModelIdentifier("object4");
	gameObjList.push_back(entityD);
	entityD.entityID = 4;

	VisualEntity entityE;
	entityE.UpdateTransform(Transform(glm::vec3{ 0.f, 1.6f, -0.65f }, model_scale)); // Default position and scale for entity E
	entityE.AssignModelIdentifier("object5");
	gameObjList.push_back(entityE);
	entityE.entityID = 5;

	// Create and compile our GLSL program from the shaders
	programID = load_Shader("Global/shaders/Shader.vert",
							"Global/shaders/Shader.frag");

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	miniMapCam.Position = { 0.f, 0.f, 4.f };
	miniMapCam.Zoom = 60.f;
	// Get polygons from models
	auto modelIter = models.begin();
	while (modelIter != models.end())
	{
		modelPolys.insert({ modelIter->first, partition::getPolygonsFromModel(modelIter->second) });
		++modelIter;
	}

	// Get polygons for every game object, put in a single vector
	auto objIter = gameObjList.begin();
	while (objIter != gameObjList.end())
	{
		std::vector<partition::poly_shape> objPolys = partition::getPolygonsOfObj(modelPolys[objIter->FetchModelIdentifier()], *objIter);
		totalObjPolygons.insert(totalObjPolygons.end(), objPolys.begin(), objPolys.end());
		++objIter;
	}


	return Scene::Init();
}

int Model_Scene::Render()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	glUseProgram(programID);
	glEnableVertexAttribArray(0);

	UpdateGameObjectAttributes();

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), camera.width / camera.height, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();

	for (auto& gameObj : gameObjList)
	{
		RenderGameObject(gameObj, projection, view);

		if (renderBV && !BSPTreeEnabled && !enable_Oct)
		{
			RenderBoundingVolume(gameObj, projection, view);
		}
	}

	PrepareSpatialPartitioning(projection, view);

	// Render IMGUI controls after rendering the scene
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	{
		ImGui::Begin("Controls");
		if (ImGui::BeginTabBar("Inspector"))
		{
			if (ImGui::BeginTabItem("Bounding Volumes"))
			{
				BVHenabled = false;
				enable_Oct = false;
				BSPTreeEnabled = false;
				if (ImGui::Button("Update BV (After moving)"))
				{
					for (auto& obj : gameObjList) // Render gameObj controls
					{
						obj.ShapeModified = true;
					}
				}

				ImGui::Text("Render Bounding Volumes");
				ImGui::Checkbox("##RenderBV", &renderBV);

				for (size_t i = 0; i < gameObjList.size(); i++) // Render gameObj controls
				{
					ImGui::Text("Object %d", i + 1);
					ImGui::PushID(i);
					gameObjList[i].RenderImGuiControls();
					ImGui::PopID();
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Spatial Partitioning"))
			{
				ImGui::Text("Click to rebuild tree");
				if (ImGui::Button("Update Tree"))
				{
					if (spatialPartitionTree != nullptr)
					{
						FreeOctTree(spatialPartitionTree);
						spatialPartitionTree = nullptr;
					}
					if (BSPTree != nullptr)
					{
						FreeBSPTree(BSPTree);
						BSPTree = nullptr;
					}
				}

				static const char* items[]{ "OctTree", "BSPTree" };
				ImGui::NewLine();
				ImGui::Text("Trees");
				ImGui::ListBox("", &spatPartTree, items, IM_ARRAYSIZE(items), 2);
				if (spatPartTree == 0)
				{
					enable_Oct = true;
					BSPTreeEnabled = false;
				}
				else
				{
					BSPTreeEnabled = true;
					enable_Oct = false;
				}
				BVHenabled = false;
				ImGui::Text("Render OctTree");
				ImGui::Checkbox("##RenderOctTree", &oct_render);

				ImGui::Text("OctTree Level (Max 3)");
				ImGui::InputInt("##OctTreeDepth", &depth_OctT);
				if (depth_OctT > 3) depth_OctT = 3;
				if (depth_OctT < 0) depth_OctT = 0;

				ImGui::Text("Render BSPTree");
				ImGui::Checkbox("##RenderBSPTree", &renderBSPTree);

				ImGui::Text("Minimum poly_shape Count");
				ImGui::SameLine();
				ImGui::DragInt("##MinPolyCount", &minPolyCount, 1, 30, 300);

				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	return 0;
}


void Model_Scene::UpdateGameObjectAttributes()
{
	for (size_t i = 0; i < gameObjList.size(); ++i)
	{
		auto& gameObj = gameObjList[i];
		auto& currentTransform = gameObj.AccessTransform();
		Transform updatedTrans(currentTransform.Position, currentTransform.scale, currentTransform.scale2, currentTransform.rotation, currentTransform.rayDirection);
		updatedTrans.triangleVertices = currentTransform.triangleVertices;

		gameObj.UpdateTransform(updatedTrans);

		if (gameObj.ShapeModified)
		{
			auto& modelVertices = models[gameObj.FetchModelIdentifier()].combined_v;
			gameObj.boundingVolume = BoundingVolume::makeAABB(modelVertices);
			gameObj.boundingVolume.m_Min += gameObj.entityTransform.Position;
			gameObj.boundingVolume.m_Max += gameObj.entityTransform.Position;

			BVHObjs[i] = &gameObj;
			gameObj.ShapeModified = false;
		}
		BVHObjs[i] = &gameObj;
	}
}

void Model_Scene::RenderGameObject(VisualEntity& gameObj, const glm::mat4& projection, const glm::mat4& view)
{
	Transform gameObjTrans = gameObj.AccessTransform();
	glm::mat4 objTrans = projection * view * gameObjTrans.model_matrix_getter();

	GLint vTransformLoc = glGetUniformLocation(programID, "vertexTransform");
	glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &objTrans[0][0]);

	GLint fCamPosLoc = glGetUniformLocation(programID, "cameraPos");
	glUniform3f(fCamPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
	glUniform1i(glGetUniformLocation(programID, "renderBoundingVolume"), false);

	if (!BSPTreeEnabled)
		models[gameObj.FetchModelIdentifier()].Draw();
}

void Model_Scene::RenderBoundingVolume(VisualEntity& gameObj, const glm::mat4& projection, const glm::mat4& view)
{
	if (gameObj.currentShape == "AABB")
	{
		float scaleX = (gameObj.boundingVolume.m_Max.x - gameObj.boundingVolume.m_Min.x) * 0.5f;
		float scaleY = (gameObj.boundingVolume.m_Max.y - gameObj.boundingVolume.m_Min.y) * 0.5f;
		float scaleZ = (gameObj.boundingVolume.m_Max.z - gameObj.boundingVolume.m_Min.z) * 0.5f;

		glm::vec3 aabbScale{ scaleX, scaleY, scaleZ };
		glm::vec3 aabbCentre = (gameObj.boundingVolume.m_Max + gameObj.boundingVolume.m_Min) / 2.f;
		Transform aabbTrans(aabbCentre, gameObj.AccessTransform().scale, aabbScale, { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f });

		glm::mat4 objTrans = projection * view * aabbTrans.model_matrix_getter();
		GLint vTransformLoc = glGetUniformLocation(programID, "vertexTransform");
		glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &objTrans[0][0]);
		glUniform1i(glGetUniformLocation(programID, "renderBoundingVolume"), true);

		glm::vec3 colour = glm::vec3(0.f, 1.f, 0.f);
		if (enable_Oct && spatialPartitionTree)
		{
			colour = glm::vec3(0.f, 1.f, 0.f);
		}
		glUniform3f(glGetUniformLocation(programID, "renderColour"), colour.x, colour.y, colour.z);
		models["Cube"].BV_draw();
	}
}

void Model_Scene::PrepareSpatialPartitioning(const glm::mat4& projection, const glm::mat4& view)
{
	if (enable_Oct)
	{
		if (spatialPartitionTree == nullptr)
		{
			glm::vec3 Min = gameObjList[0].boundingVolume.m_Min;
			glm::vec3 Max = gameObjList[0].boundingVolume.m_Max;
			for (auto& obj : gameObjList)
			{
				Min = glm::min(Min, obj.boundingVolume.m_Min);
				Max = glm::max(Max, obj.boundingVolume.m_Max);
			}
			glm::vec3 center = (Min + Max) / 2.f;
			float halfWidth = std::max({ Max.x - Min.x, Max.y - Min.y, Max.z - Min.z }) * 0.5f;
			spatialPartitionTree = partition::BuildOctTree(center, halfWidth, 3, 0, totalObjPolygons);

			for (auto& obj : gameObjList)
			{
				auto objPolys = partition::getPolygonsOfObj(modelPolys[obj.FetchModelIdentifier()], obj);
				partition::InsertIntoOctTree(spatialPartitionTree, objPolys);
			}
		}
		else if (oct_render)
		{
			RenderOctTree(spatialPartitionTree, projection, view, 0);
		}
	}
	else if (BSPTreeEnabled)
	{
		if (BSPTree == nullptr)
		{
			BSPTree = partition::build_bsp(totalObjPolygons, 0);
		}
		else if (renderBSPTree)
		{
			RenderBSPTree(BSPTree, projection, view);
		}
	}
}






void Model_Scene::RenderOctTree(partition::TreeNode* tree, const glm::mat4& projection, const glm::mat4& view, int col)
{
	partition::TreeNode* node = tree;
	if (node == nullptr)
		return;

	if (node->depth < depth_OctT)
		return; //dont render the deeper nodes
	Transform aabbTrans;


	glm::vec3 octTreeNodeCentre = node->center;
	Transform temp(octTreeNodeCentre, 1.f, { node->mid_width, node->mid_width, node->mid_width }, { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f });
	aabbTrans = temp;

	//gameObjs.UpdateTransform(aabbTrans);
	glm::mat4 objTrans;
	objTrans = projection * view * aabbTrans.model_matrix3f_getter();
	GLint vTransformLoc = glGetUniformLocation(programID, "vertexTransform");
	glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &objTrans[0][0]);
	glUniform1i(glGetUniformLocation(programID, "renderBoundingVolume"), true);

	////TreeDepth 0 Root Node
	glm::vec3 colour = glm::vec3(1.f, 0.f, 0.f); //Red

	//Get nodes' colours
	if (col > 0)
	{
		colour = node->colour;
	}

	glUniform3f(glGetUniformLocation(programID, "renderColour"), colour.x, colour.y, colour.z);
	//Draw
	models["Cube"].BV_draw(); //Render the OctTree nodes
	
	for (size_t i = 0; i < 8; i++)
	{
		RenderOctTree(node->childp[i], projection, view, ++col); //Recursive call for all the children
	}
}


void Model_Scene::RenderBSPTree(partition::node_bsp* tree, const glm::mat4& projection, const glm::mat4& view)
{
	partition::node_bsp* node = tree;
	if (node == nullptr)
		return;

	if (node->currType == partition::node_bsp::Type::INTERNAL)
	{
		RenderBSPTree(node->tree_front, projection, view);
		RenderBSPTree(node->tree_back, projection, view);
		return;
	}
	else
	{
		glm::mat4 objTrans = projection * view * glm::mat4(1.0f);
		// Uniform transformation (vertex shader)
		GLint vTransformLoc = glGetUniformLocation(programID, "vertexTransform");
		glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &objTrans[0][0]);


		GLint fCamPosLoc = glGetUniformLocation(programID, "cameraPos");
		glUniform3f(fCamPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
		glUniform1i(glGetUniformLocation(programID, "renderBoundingVolume"), true);

		glm::vec3 colour{ 0.f, 1.f, 0.f };

		colour = node->colour;
		glUniform3f(glGetUniformLocation(programID, "renderColour"), colour.x, colour.y, colour.z);
		node->data_g.tri_draw();
	}

	

}

void Model_Scene::FreeOctTree(partition::TreeNode* node)
{
	if (node == nullptr)
		return;
	for (size_t i = 0; i < 8; i++)
	{
		FreeOctTree(node->childp[i]); //Free all children
	}
	delete node;
}

void Model_Scene::FreeBSPTree(partition::node_bsp* node)
{
	if (node == nullptr)
		return;
	if (node->tree_front)
		FreeBSPTree(node->tree_front);
	if (node->tree_back)
		FreeBSPTree(node->tree_back);

	delete node;
}
