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


}
