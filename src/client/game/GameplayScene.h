#pragma once

#include <enet/enet.h>
#include <entt/entt.hpp>
#include "Scene.h"

namespace lol_at_home_game {

class GameplayScene : public lol_at_home_engine::Scene {
 public:
  GameplayScene();
  ~GameplayScene() override;

  GameplayScene(const GameplayScene&) = delete;
  auto operator=(const GameplayScene&) -> GameplayScene& = delete;
  GameplayScene(GameplayScene&&) = delete;
  auto operator=(GameplayScene&&) -> GameplayScene& = delete;

  [[nodiscard]] auto ShouldContinueImpl() const -> bool override;

 private:
  void connectToServer();
  void handleInput();
  void handleNetwork();
  void handleReceive(const ENetEvent& event);
  void renderEntities(lol_at_home_engine::Renderer& renderer);
  void updateInterpolation(double deltaTime);

  entt::registry lolRegistry_;
  ENetHost* client_{nullptr};
  ENetPeer* peer_{nullptr};
  bool connected_{false};
  entt::entity myEntityId_{entt::null};
};

}  // namespace lol_at_home_game
