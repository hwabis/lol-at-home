#include "GameplayScene.h"
#include <memory>
#include "MyAwesomeGameObject.h"

namespace lol_at_home_game {

GameplayScene::GameplayScene(SDL_Renderer* renderer) : Scene(renderer) {
  AddObject(std::make_unique<MyAwesomeGameObject>());
}

}  // namespace lol_at_home_game
