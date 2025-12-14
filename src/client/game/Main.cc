#include "Game.h"
#include "GameplayScene.h"

class LoLAtHomeGame : public lol_at_home_engine::Game {
 public:
  LoLAtHomeGame()
      : Game({.windowTitle = "lololol",
              .windowWidth = 1280,
              .windowHeight = 720,
              .targetFPS = 60}) {}
};

auto main() -> int {
  LoLAtHomeGame game;
  game.Run(
      [](SDL_Renderer* renderer) -> std::unique_ptr<lol_at_home_engine::Scene> {
        return std::make_unique<lol_at_home_game::GameplayScene>(renderer);
      });
  return 0;
}
