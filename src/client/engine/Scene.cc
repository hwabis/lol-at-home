#include "Scene.h"
#include "renderables/IWorldRenderable.h"

namespace lol_at_home_engine {

Scene::Scene(SDL_Renderer* renderer, int width, int height)
    : renderer_(renderer), camera_(width, height) {}

void Scene::Render() {
  std::vector<std::unique_ptr<IWorldRenderable>> renderables;
  for (const auto& obj : objects_) {
    obj->PushRender(renderables);
  }

  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
  SDL_RenderClear(renderer_);

  for (const auto& renderable : renderables) {
    renderable->Render(renderer_, camera_);
  }

  SDL_RenderPresent(renderer_);
}

void Scene::Update(std::chrono::duration<double, std::milli> deltaTime) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_EVENT_QUIT:
        continue_ = false;
        break;
      case SDL_EVENT_WINDOW_RESIZED:
        GetCamera().RecalculateView(event.window.data1, event.window.data2);
        break;
      default:
        break;
    }
  }

  input_.Update();

  for (auto& obj : objects_) {
    obj->Update(deltaTime, input_);
  }
}

}  // namespace lol_at_home_engine
