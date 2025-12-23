#include "Components.h"
#include "Game.h"
#include "MyAwesomeSystem.h"

auto main() -> int {
  lol_at_home_engine::Game game({
      .windowTitle = "Lol at Home Client",
      .windowSize = {.x = 1280.0F, .y = 720.0F},
      .targetFPS = 60,
  });

  lol_at_home_engine::Scene scene;

  scene.AddSystem(std::make_unique<lol_at_home_game::MyAwesomeSystem>());

  auto joeBillyBob = scene.CreateEntity();
  joeBillyBob.Add<lol_at_home_game::Transform>(lol_at_home_game::Transform{});
  joeBillyBob.Add<lol_at_home_game::RenderableCircle>(
      lol_at_home_game::RenderableCircle{50.0F});

  game.Run(std::move(scene));
  return 0;
}
