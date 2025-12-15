#include "GameplayScene.h"
#include <memory>
#include "MyAwesomeGameObject.h"

namespace lol_at_home_game {

GameplayScene::GameplayScene(SDL_Renderer* renderer, int width, int height)
    : Scene(renderer, width, height) {
  AddObject(std::make_unique<MyAwesomeGameObject>());
}

}  // namespace lol_at_home_game
