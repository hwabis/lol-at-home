#include "Components.h"
#include "Game.h"
#include "InputSystem.h"
#include "RenderSystem.h"

namespace {

auto getScene() -> lol_at_home_engine::Scene {
  lol_at_home_engine::Scene scene;

  scene.AddSystem(std::make_unique<lol_at_home_game::RenderSystem>());
  scene.AddSystem(std::make_unique<lol_at_home_game::InputSystem>());

  auto joeBillyBob = scene.GetRegistry().create();
  scene.GetRegistry().emplace<lol_at_home_game::Transform>(joeBillyBob);
  scene.GetRegistry().emplace<lol_at_home_game::RenderableCircle>(joeBillyBob,
                                                                  50.0F);

  return scene;
}

}  // namespace

auto main() -> int {
  lol_at_home_engine::Game game({
      .windowTitle = "LoL at Home Client",
      .windowType = lol_at_home_engine::GameConfig::WindowType::Windowed,
      .windowSize = {.x = 1280.0F, .y = 720.0F},
      .targetFPS = 60,
  });

  lol_at_home_engine::Scene scene = getScene();

  game.Run(std::move(scene));
  return 0;
}
