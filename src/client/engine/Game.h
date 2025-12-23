#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include "Scene.h"
#include "SceneInfo.h"

namespace lol_at_home_engine {

class Game {
 public:
  explicit Game(const GameConfig& config);

  auto Run(std::unique_ptr<Scene> scene) -> void;
  auto GetSceneInfo() -> SceneInfo& { return info_; }

 private:
  void initSDL();
  void cleanupSDL();
  void sceneLoop();

  // todo scene manager? how to stop scene or switch scene (from a scene)?
  std::unique_ptr<Scene> scene_;
  SceneInfo info_;
};

}  // namespace lol_at_home_engine
