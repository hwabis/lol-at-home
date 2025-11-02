#pragma once

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include "GameAction.h"
#include "core/GameState.h"
#include "core/GameStateThreadConfig.h"

namespace lol_at_home_server {

class GameStateThread {
 public:
  GameStateThread(GameState gameState, GameStateThreadConfig config);
  void Start(std::function<void(const entt::registry&,
                                const std::vector<entt::entity>&)> broadcastFn);
  void Stop();
  void HandleInput(lol_at_home_shared::GameActionVariant input);

  auto CreatePlayerEntity() -> entt::entity {
    return gameState_.CreatePlayerEntity();
  }

 private:
  void runGameLoop();
  auto getAndClearQueuedActions()
      -> std::vector<lol_at_home_shared::GameActionVariant>;

  std::jthread gameThread_;
  std::atomic<bool> isRunning_ = false;
  std::queue<lol_at_home_shared::GameActionVariant> actionQueue_;
  std::mutex actionQueueMutex_;
  GameState gameState_;
  GameStateThreadConfig config_;
  std::function<void(const entt::registry&, const std::vector<entt::entity>&)>
      broadcastFn_;
};

}  // namespace lol_at_home_server
