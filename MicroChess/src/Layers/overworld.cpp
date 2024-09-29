#include "overworld.h"

#include "States.h"
#include "Layers.h"

#include "Components/battlecomponents.h"
#include "Components/charactercomponents.h"
#include "Components/physics.h"
#include "Components/rendering.h"
#include "Components/characterinput.h"

#include "BattleSystems/physicssystem.h"

#include "Renderer/sprite2d.h"


namespace ChronoShift {


  void OverworldLayer::SetupWorld()
  {
    auto scene = FlexECS::Scene::GetActiveScene();

    #pragma region Creating Entities 
    FlexECS::Entity player1 = FlexECS::Scene::CreateEntity("White Queen");
    player1.AddComponent<CharacterInput>({ });
    player1.AddComponent<Rigidbody>({ {}, false });
    player1.AddComponent<BoundingBox2D>({ });
    player1.AddComponent<IsActive>({ true });
    player1.AddComponent<Position>({ {200, 600} });
    player1.AddComponent<Rotation>({ });
    player1.AddComponent<Scale>({ { 100,100 } });
    player1.AddComponent<Transform>({});
    player1.AddComponent<ZIndex>({ 10 });
    player1.AddComponent<Sprite>({
      scene->Internal_StringStorage_New(R"(\images\chess_queen.png)"),
      //scene->Internal_StringStorage_New(R"()"),
      Vector3::One,
      Vector3::Zero,
      Vector3::One,
      Renderer2DProps::Alignment_Center
     });
    player1.AddComponent<Shader>({ scene->Internal_StringStorage_New(R"(\shaders\texture)") });

  
    FlexECS::Entity house = FlexECS::Scene::CreateEntity("House");
    house.AddComponent<Rigidbody>({ {}, true });
    house.AddComponent<BoundingBox2D>({ });
    house.AddComponent<IsActive>({ true });
    house.AddComponent<Position>({ {800, 500 } });
    house.AddComponent<Rotation>({ });
    house.AddComponent<Scale>({ { 250,250 } });
    house.AddComponent<Transform>({});
    house.AddComponent<ZIndex>({ 10 });
    house.AddComponent<Sprite>({
        scene->Internal_StringStorage_New(R"()"),
        { 0.45f, 0.58f, 0.32f },
        Vector3::Zero,
        Vector3::One,
        Renderer2DProps::Alignment_Center
       });
    house.AddComponent<Shader>({ scene->Internal_StringStorage_New(R"(\shaders\texture)") });

    FlexECS::Entity box = FlexECS::Scene::CreateEntity("Movable Box");
    box.AddComponent<Rigidbody>({ {}, false });
    box.AddComponent<BoundingBox2D>({ });
    box.AddComponent<IsActive>({ true });
    box.AddComponent<Position>({ {350, 500 } });
    box.AddComponent<Rotation>({ });
    box.AddComponent<Scale>({ { 150,150 } });
    box.AddComponent<Transform>({});
    box.AddComponent<ZIndex>({ 10 });
    box.AddComponent<Sprite>({
        scene->Internal_StringStorage_New(R"()"),
        { 0.35f, 0.58f, 0.80f },
        Vector3::Zero,
        Vector3::One,
        Renderer2DProps::Alignment_Center
       });
    box.AddComponent<Shader>({ scene->Internal_StringStorage_New(R"(\shaders\texture)") });
    #pragma endregion


  }



  void OverworldLayer::OnAttach()
  {
    FLX_FLOW_BEGINSCOPE();

    // ECS Setup
    auto scene = FlexECS::Scene::CreateScene();
    FlexECS::Scene::SetActiveScene(scene);

    SetupWorld();
  }

  void OverworldLayer::OnDetach()
  {
    FLX_FLOW_ENDSCOPE();
  }

  void OverworldLayer::Update()
  {
    for (auto& entity : FlexECS::Scene::GetActiveScene()->View<CharacterInput>())
    {
      entity.GetComponent<CharacterInput>()->up = Input::GetKey(GLFW_KEY_W);
      entity.GetComponent<CharacterInput>()->down = Input::GetKey(GLFW_KEY_S);
      entity.GetComponent<CharacterInput>()->left = Input::GetKey(GLFW_KEY_A);
      entity.GetComponent<CharacterInput>()->right = Input::GetKey(GLFW_KEY_D);
    }

    for (auto& entity : FlexECS::Scene::GetActiveScene()->View<CharacterInput, Rigidbody>())
    {
      auto& velocity = entity.GetComponent<Rigidbody>()->velocity;
      velocity.x = 0.0f;
      velocity.y = 0.0f;

      if (entity.GetComponent<CharacterInput>()->up)
      {
        velocity.y = -300.0f;
      }

      if (entity.GetComponent<CharacterInput>()->down)
      {
        velocity.y = 300.0f;
      }

      if (entity.GetComponent<CharacterInput>()->left)
      {
        velocity.x = -300.0f;
      }

      if (entity.GetComponent<CharacterInput>()->right)
      {
        velocity.x = 300.0f;
      }
    }

    //For testing 2500 objects
    //Create one, then clone the rest
    if (Input::GetKeyDown(GLFW_KEY_0))
    {
      auto scene = FlexECS::Scene::GetActiveScene();
      for (auto& entity : FlexECS::Scene::GetActiveScene()->View<EntityName>()) 
      {
        scene->DestroyEntity(entity);
      }

      FlexECS::Entity thing = FlexECS::Scene::CreateEntity("White Queen");
      thing.AddComponent<IsActive>({ true });
      thing.AddComponent<Position>({ {0,0} });
      thing.AddComponent<Rotation>({ });
      thing.AddComponent<Scale>({ { 15,15 } });
      thing.AddComponent<ZIndex>({ 10 });
      thing.AddComponent<Sprite>({
        scene->Internal_StringStorage_New(R"(\images\chess_queen.png)"),
        Vector3::One,
        Vector3::Zero,
        Vector3::One,
        Renderer2DProps::Alignment_Center
       });
      thing.AddComponent<Shader>({ scene->Internal_StringStorage_New(R"(\shaders\texture)") });

      for (size_t x = 0; x < 50; x++)
      {
        for (size_t y = 0; y < 50; y++)
        {
          FlexECS::Entity cloned_thing = scene->CloneEntity(thing);
          auto& position = cloned_thing.GetComponent<Position>()->position;
          //cloned_thing.GetComponent<IsActive>()->is_active = false;
          position.x = 15 * (x + 1);
          position.y = 15 * (y + 1);
        }
      }
    }

    UpdatePhysicsSystem();
    RendererSprite2D();
  }
}
