#pragma once

#include <enet/enet.h>
#include <entt/entt.hpp>
#include <memory>
#include "Camera.h"
#include "Input.h"
#include "Scene.h"

namespace lol_at_home_game {

class GameplayScene : public lol_at_home_engine::Scene {
 public:
  GameplayScene(std::shared_ptr<lol_at_home_engine::Camera> camera,
                lol_at_home_engine::Input& input,
                lol_at_home_engine::Renderer& renderer);
  ~GameplayScene() override;

  GameplayScene(const GameplayScene&) = delete;
  auto operator=(const GameplayScene&) -> GameplayScene& = delete;
  GameplayScene(GameplayScene&&) = delete;
  auto operator=(GameplayScene&&) -> GameplayScene& = delete;

  void OnStart() override;
  void Update(double deltaTime) override;
  void Render(lol_at_home_engine::Renderer& renderer) override;
  void OnEnd() override;
  [[nodiscard]] auto ShouldContinue() -> bool override;

 private:
  void connectToServer();
  void handleInput();
  void handleNetwork();
  void handleReceive(const ENetEvent& event);
  void renderEntities(lol_at_home_engine::Renderer& renderer);
  void updateInterpolation(double deltaTime);

  std::shared_ptr<lol_at_home_engine::Camera> camera_;
  lol_at_home_engine::Input& input_;
  lol_at_home_engine::Renderer& renderer_;

  entt::registry registry_;
  ENetHost* client_{nullptr};
  ENetPeer* peer_{nullptr};
  bool connected_{false};
  entt::entity myEntityId_{entt::null};
};

}  // namespace lol_at_home_game
