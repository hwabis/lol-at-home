#include "Game.h"
#include "GameplayScene.h"

class LoLAtHomeGame : public lol_at_home_engine::Game {
 public:
  LoLAtHomeGame()
      : Game({.windowTitle = "League of Legends at Home",
              .windowWidth = 1280,
              .windowHeight = 720,
              .targetFPS = 60}) {}
};

auto main() -> int {
  LoLAtHomeGame game;
  game.Run();
  return 0;
}
