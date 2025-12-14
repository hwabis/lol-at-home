#include "Scene.h"
#include "IWorldRenderable.h"

namespace lol_at_home_engine {

Scene::Scene(SDL_Renderer* renderer) : renderer_(renderer) {}

void Scene::Render() {
  std::vector<std::unique_ptr<IWorldRenderable>> renderables;
  for (const auto& obj : objects_) {
    obj->PushRender(renderables);
  }

  SDL_SetRenderDrawColor(renderer_, 100, 255, 255, 255);
  SDL_RenderClear(renderer_);

  // todo visit renderables... for now draw smth default
  // todo probably pass in the camera here somehow
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
  SDL_RenderLine(renderer_, 0, 0, 100, 100);
  SDL_RenderPresent(renderer_);
}

void Scene::Update(std::chrono::duration<double, std::milli> deltaTime) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      continue_ = false;
    }
  }

  input_.Update();

  for (auto& obj : objects_) {
    obj->Update(deltaTime, input_);
  }
}

}  // namespace lol_at_home_engine
