#pragma once

//#include "Reflection/base.h"
#include "gameobject.h"

namespace FlexEngine
{

  class Scene
  { //FLX_REFL_SERIALIZABLE
  public:
    GameObject* AddGameObject(std::string name);
    GameObject* GetGameObject(std::string name) const;

    GameObjectMap gameobjects;
  };

}