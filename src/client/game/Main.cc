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
      // todo this lambda is kinda ugly...
      // i think gameconfig should be passed to scene not game?
      [](SDL_Renderer* renderer, int width,
         int height) -> std::unique_ptr<lol_at_home_engine::Scene> {
        return std::make_unique<lol_at_home_game::GameplayScene>(renderer,
                                                                 width, height);
      });
  return 0;
}
