#pragma once

#include <SDL3/SDL.h>
#include "Scene.h"
#include "SceneInfo.h"

namespace lol_at_home_engine {

class Game {
 public:
  explicit Game(const GameConfig& config);

  auto Run(Scene&& scene) -> void;

 private:
  void initSDL();
  void cleanupSDL();
  void sceneLoop();

  // todo scene manager? how to stop scene or switch scene (from a scene)?
  // each scene is going to need to "see" this scene mgr
  Scene scene_;
  SceneInfo info_;
};

}  // namespace lol_at_home_engine
