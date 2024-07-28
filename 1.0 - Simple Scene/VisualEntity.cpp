#include "VisualEntity.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

void VisualEntity::UpdateTransform(Transform newTransform)
{
    entityTransform = newTransform;
}
void VisualEntity::AssignModelIdentifier(const char* identifier)
{
    modelIdentifier = identifier;
}
Transform& VisualEntity::AccessTransform()
{
    return entityTransform;
}
const char* VisualEntity::FetchModelIdentifier()
{
    return modelIdentifier;
}

void VisualEntity::RenderImGuiControls()
{
    ImGui::Text("Transform Controls");
    ImGui::Separator();

    ImGui::Text("Position");
    ImGui::SliderFloat("X", &entityTransform.Position.x, -10.0f, 10.0f, "%.2f");
    ImGui::SliderFloat("Y", &entityTransform.Position.y, -10.0f, 10.0f, "%.2f");
    ImGui::SliderFloat("Z", &entityTransform.Position.z, -10.0f, 10.0f, "%.2f");
}
