#include "Scene.h"
#include "IWorldRenderable.h"

namespace lol_at_home_engine {

void Scene::Render() {
  std::vector<std::unique_ptr<IWorldRenderable>> renderables;
  for (const auto& obj : objects_) {
    auto objRenderables = obj->Render();
    for (auto& objRenderable : objRenderables) {
      renderables.push_back(std::move(objRenderable));
    }
  }

  // todo render them LOL... pass SDL_Renderer* to here ??
  /*
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 0);

    // todo visit renderables... for now draw smth default
    SDL_RenderLine(sdlRenderer, 0, 0, 100, 100);

    SDL_RenderClear(sdlRenderer);
    SDL_RenderPresent(sdlRenderer);
  */
}

void Scene::Update(std::chrono::duration<double, std::milli> deltaTime) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      continue_ = false;
    }
  }

  for (auto& obj : objects_) {
    obj->Update(deltaTime);
  }
}

}  // namespace lol_at_home_engine
