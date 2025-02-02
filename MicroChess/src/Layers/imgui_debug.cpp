#include "imgui_debug.h"
#include "FlexEngine/Core/imguiwrapper.h"
#include "FlexEngine/Renderer/DebugRenderer/debugrenderer.h"

#include "Components/physics.h"

namespace ChronoShift 
{
  void DebugLayer::OnAttach()
  {
    FLX_FLOW_BEGINSCOPE();
  }

  void DebugLayer::OnDetach()
  {
    FLX_FLOW_ENDSCOPE();

  }

  void DebugLayer::Update()
  {
    ImGui::ShowDemoWindow();

    int i = 0;
    for (auto& entity : FlexECS::Scene::GetActiveScene()->View<BoundingBox2D>())
    {

      const Vector3& max = entity.GetComponent<BoundingBox2D>()->max;
      const Vector3& min = entity.GetComponent<BoundingBox2D>()->min;
      //construct lines for AABB
      Vector3 topleft = min;
      Vector3 topright = { max.x, min.y };
      Vector3 botleft = { min.x, max.y };
      Vector3 botright = max;
      FlexEngine::DebugRenderer::DrawLine2D(topleft, topright);
      FlexEngine::DebugRenderer::DrawLine2D(topright, botright);
      FlexEngine::DebugRenderer::DrawLine2D(botright, botleft);
      FlexEngine::DebugRenderer::DrawLine2D(botleft, topleft);
    }
  }

}