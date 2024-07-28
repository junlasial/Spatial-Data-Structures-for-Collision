#include "GameObject.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

void GameObject::SetTransform(Transform trans)
{
	transform = trans;
}
void GameObject::SetModelID(const char* id)
{
	model_id = id;
}
Transform& GameObject::GetTransform()
{
	return transform;
}
const char* GameObject::GetModelID()
{
	return model_id;
}

void GameObject::DrawImGuiControls()
{
	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, 85);
	ImGui::Text("Position");
	ImGui::NextColumn();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() / 5.0f);
	ImGui::Button("X");
	ImGui::SameLine();
	ImGui::DragFloat("##PositionX", &transform.Position.x, 0.05f, 0.0f, 0.0f, "%.2f");
	ImGui::SameLine();
	ImGui::Text(" ");
	ImGui::SameLine();
	ImGui::Button("Y");
	ImGui::SameLine();
	ImGui::DragFloat("##PositionY", &transform.Position.y, 0.05f, 0.0f, 0.0f, "%.2f");
	ImGui::SameLine();
	ImGui::Text(" ");
	ImGui::SameLine();
	ImGui::Button("Z");
	ImGui::SameLine();
	ImGui::DragFloat("##PositionZ", &transform.Position.z, 0.05f, 0.0f, 0.0f, "%.2f");
	ImGui::NextColumn();


	
	ImGui::Columns(1);



}

