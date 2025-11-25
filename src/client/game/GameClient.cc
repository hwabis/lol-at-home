#include <memory>
#include "Game.h"
#include "GameplayScene.h"

class LoLAtHomeGame : public lol_at_home_engine::Game {
 public:
  LoLAtHomeGame()
      : Game({.WindowTitle = "League of Legends at Home",
              .windowWidth = 1280,
              .windowHeight = 720,
              .targetFPS = 60}) {}

 protected:
  auto CreateRootScene()
      -> std::unique_ptr<lol_at_home_engine::Scene> override {
    return std::make_unique<lol_at_home_game::GameplayScene>(
        GetCamera(), GetInput(), GetRenderer());
  }
};

auto main() -> int {
  LoLAtHomeGame game;
  game.Run();
  return 0;
}
