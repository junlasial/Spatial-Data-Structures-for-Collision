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

void Model_Scene::SetupNanoGUI(GLFWwindow* pWindow)
{
	pWindow = nullptr;
}

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
	// Cleanup VBO
	for (auto model : models)
		model.second.CleanUp();
	glDeleteProgram(programID);
	
	if (spatialPartitionTree != nullptr)
	{
		FreeOctTree(spatialPartitionTree);
	}
	if (BSPTree != nullptr)
	{
		FreeBSPTree(BSPTree);
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

	VisualEntity one;
	one.UpdateTransform(Transform(glm::vec3{ 0.f, 1.0f, -0.65f }, model_scale)); // Default position and scale for object1
	one.AssignModelIdentifier("object1");
	gameObjList.push_back(one);
	one.entityID = 1;

	VisualEntity two;
	two.UpdateTransform(Transform(glm::vec3{ 0.f, 1.4f, -0.65f }, model_scale)); // Default position and scale for object2
	two.AssignModelIdentifier("object2");
	gameObjList.push_back(two);
	two.entityID = 2;

	VisualEntity three;
	three.UpdateTransform(Transform(glm::vec3{ 0.f, 0.f, 0.f }, model_scale)); // Default position and scale for object3
	three.AssignModelIdentifier("object3");
	gameObjList.push_back(three);
	three.entityID = 3;

	VisualEntity four;
	four.UpdateTransform(Transform(glm::vec3{ 0.f, 0.f, 0.f }, model_scale)); // Default position and scale for object4
	four.AssignModelIdentifier("object4");
	gameObjList.push_back(four);
	four.entityID = 4;

	VisualEntity five;
	five.UpdateTransform(Transform(glm::vec3{ 0.f, 1.6f, -0.65f }, model_scale)); // Default position and scale for object5
	five.AssignModelIdentifier("object5");
	gameObjList.push_back(five);
	five.entityID = 5;

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("Global/shaders/Shader.vert", "Global/shaders/Shader.frag");

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	miniMapCam.Position = { 0.f, 0.f, 4.f };
	miniMapCam.Zoom = 60.f;

	// Get polygons from models
	for (auto& model : models)
	{
		modelPolys.insert({ model.first, SpatialPartitioning::getPolygonsFromModel(model.second) });
	}

	// Get polygons for every game object, put in a single vector
	for (auto& obj : gameObjList)
	{
		std::vector<SpatialPartitioning::poly_shape> objPolys = SpatialPartitioning::getPolygonsOfObj(modelPolys[obj.FetchModelIdentifier()], obj);
		totalObjPolygons.insert(totalObjPolygons.end(), objPolys.begin(), objPolys.end());
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

	//Update gameObject attribues (Transform, BV)
	for (int i = 0; i < gameObjList.size(); ++i)
	{
		Transform firstTrans(gameObjList[i].AccessTransform().Position, gameObjList[i].AccessTransform().scale, gameObjList[i].AccessTransform().scale2, gameObjList[i].AccessTransform().rotation, gameObjList[i].AccessTransform().rayDirection);
		firstTrans.triangleVertices = gameObjList[i].AccessTransform().triangleVertices;
		gameObjList[i].UpdateTransform(firstTrans);
		if (gameObjList[i].ShapeModified)
		{
			gameObjList[i].boundingVolume = BoundingVolume::createAABB(models[gameObjList[i].FetchModelIdentifier()].combinedVertices);
			gameObjList[i].boundingVolume.m_Min = gameObjList[i].entityTransform.Position + gameObjList[i].boundingVolume.m_Min;
			gameObjList[i].boundingVolume.m_Max = gameObjList[i].entityTransform.Position + gameObjList[i].boundingVolume.m_Max;

		

			BVHObjs[i] = &gameObjList[i];
			gameObjList[i].ShapeModified = false;
		}
		BVHObjs[i] = &gameObjList[i];

	}



	// ---------- Rendering ----------

	// Camera transform
	glViewport(0, 0, (GLsizei)(camera.width), (GLsizei)(camera.height));
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), camera.width / camera.height, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();

	for (auto& gameObjs : gameObjList)
	{
		Transform gameObjTrans = gameObjs.AccessTransform();

		glm::mat4 objTrans;
		if (gameObjs.FetchModelIdentifier() == "Cube")
			objTrans = projection * view * gameObjTrans.model_matrix3f_getter();
		else
			objTrans = projection * view * gameObjTrans.model_matrix_getter();
		// Uniform transformation (vertex shader)
		GLint vTransformLoc = glGetUniformLocation(programID, "vertexTransform");
		glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &objTrans[0][0]);
		//glm::vec3 colour{ 0.f, 1.f, 0.f };

		GLint fCamPosLoc = glGetUniformLocation(programID, "cameraPos");
		glUniform3f(fCamPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
		glUniform1i(glGetUniformLocation(programID, "renderBoundingVolume"), false);
		//Render the model of the gameObjs
		if (!BSPTreeEnabled)
			models[gameObjs.FetchModelIdentifier()].Draw();

		if (renderBV && !BSPTreeEnabled && !enable_Oct)
		{
			//Render the model of the Bounding Volume
			if (gameObjs.currentShape == "AABB")
			{
				//AABB Update (for now only translation and scale)
				//Set transform for the cube based on the AABB size
				//half extents are the scale
				float scaleX = (gameObjs.boundingVolume.m_Max.x - gameObjs.boundingVolume.m_Min.x) * 0.5f;
				float scaleY = (gameObjs.boundingVolume.m_Max.y - gameObjs.boundingVolume.m_Min.y) * 0.5f;
				float scaleZ = (gameObjs.boundingVolume.m_Max.z - gameObjs.boundingVolume.m_Min.z) * 0.5f;

				glm::vec3 aabbScale{ scaleX, scaleY, scaleZ };
				glm::vec3 aabbCentre = (gameObjs.boundingVolume.m_Max + gameObjs.boundingVolume.m_Min) / 2.f;
				Transform aabbTrans(aabbCentre, gameObjs.AccessTransform().scale, aabbScale, { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f });

				objTrans = projection * view * aabbTrans.model_matrix3f_getter();
				GLint vTransformLoc = glGetUniformLocation(programID, "vertexTransform");
				glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &objTrans[0][0]);
				glUniform1i(glGetUniformLocation(programID, "renderBoundingVolume"), true);
				glm::vec3 colour = glm::vec3(0.f, 1.f, 0.f);
				if (enable_Oct && spatialPartitionTree)
				{
					colour = glm::vec3(0.f, 1.f, 0.f);
		
				}
				glUniform3f(glGetUniformLocation(programID, "renderColour"), colour.x, colour.y, colour.z);
				//Draw
				models["Cube"].BV_draw();
			}
			
		}
		glDisableVertexAttribArray(0);
	}

	
	 if (enable_Oct)
	{
		if (spatialPartitionTree == nullptr)
		{
			glm::vec3 Min = gameObjList[0].boundingVolume.m_Min;
			glm::vec3 Max = gameObjList[0].boundingVolume.m_Max;
			for (auto& obj : gameObjList)
			{
				if (Min.x > obj.boundingVolume.m_Min.x)
					Min.x = obj.boundingVolume.m_Min.x;
				if (Max.x < obj.boundingVolume.m_Max.x)
					Max.x = obj.boundingVolume.m_Max.x;
				if (Min.y > obj.boundingVolume.m_Min.y)
					Min.y = obj.boundingVolume.m_Min.y;
				if (Max.y < obj.boundingVolume.m_Max.y)
					Max.y = obj.boundingVolume.m_Max.y;
				if (Min.z > obj.boundingVolume.m_Min.z)
					Min.z = obj.boundingVolume.m_Min.z;
				if (Max.z < obj.boundingVolume.m_Max.z)
					Max.z = obj.boundingVolume.m_Max.z;
			}
			glm::vec3 center = (Min + Max) / 2.f;
			float halfWidth = std::max(Max.x - Min.x, Max.y - Min.y);
			halfWidth = std::max(Max.z - Min.z, halfWidth);
			halfWidth *= 0.5f;
			spatialPartitionTree = SpatialPartitioning::BuildOctTree(center, halfWidth, 3, 0, totalObjPolygons);

			//Insert all the game Objs into the list
			for (auto& obj : gameObjList)
			{
				std::vector<SpatialPartitioning::poly_shape> objPolys = SpatialPartitioning::getPolygonsOfObj(modelPolys[obj.FetchModelIdentifier()], obj);
				SpatialPartitioning::InsertIntoOctTree(spatialPartitionTree, objPolys);
			}

		}
		else
		{
			if (oct_render)
				RenderOctTree(spatialPartitionTree, projection, view, 0);
		}
	}
	else if (BSPTreeEnabled)
	{
		if (BSPTree == nullptr)
		{
			BSPTree = SpatialPartitioning::bsp_build(totalObjPolygons, 0);
		}
		else
		{
			if (renderBSPTree)
				RenderBSPTree(BSPTree, projection, view);
		}
	}
	


	//Render IMGUI controls after rendering the scene
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
				//runOnce = false;
				if (ImGui::Button("Update BV (After moving)"))
				{
					for (auto& obj : gameObjList) //Render gameObj controls
					{
						obj.ShapeModified = true;
					}
				}

				ImGui::Text("Render Bounding Volumes");
				ImGui::Checkbox("##RenderBV", &renderBV);

				for (size_t i = 0; i < gameObjList.size(); i++) //Render gameObj controls
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
				ImGui::ListBox("", &spatPartTree, items, IM_ARRAYSIZE
				(items), 2);
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





void Model_Scene::RenderOctTree(SpatialPartitioning::TreeNode* tree, const glm::mat4& projection, const glm::mat4& view, int col)
{
	SpatialPartitioning::TreeNode* node = tree;
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

void Model_Scene::RenderBSPTree(SpatialPartitioning::BSPNode* tree, const glm::mat4& projection, const glm::mat4& view)
{
	SpatialPartitioning::BSPNode* node = tree;
	if (node == nullptr)
		return;

	if (node->currType == SpatialPartitioning::BSPNode::Type::INTERNAL)
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

void Model_Scene::FreeOctTree(SpatialPartitioning::TreeNode* node)
{
	if (node == nullptr)
		return;
	for (size_t i = 0; i < 8; i++)
	{
		FreeOctTree(node->childp[i]); //Free all children
	}
	delete node;
}

void Model_Scene::FreeBSPTree(SpatialPartitioning::BSPNode* node)
{
	if (node == nullptr)
		return;
	if (node->tree_front)
		FreeBSPTree(node->tree_front);
	if (node->tree_back)
		FreeBSPTree(node->tree_back);

	delete node;
}
