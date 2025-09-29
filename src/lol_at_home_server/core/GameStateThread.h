#pragma once

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include "actions/GameAction.h"
#include "core/GameState.h"
#include "core/GameStateThreadConfig.h"
#include "networking/SerializedGameState.h"

namespace lol_at_home_server {

class GameStateThread {
 public:
  GameStateThread(GameState gameState, GameStateThreadConfig config);
  void Start(std::function<void(const SerializedGameState&)> broadcastFn);
  void Stop();
  void HandleInput(GameActionVariant input);

 private:
  void runGameLoop();
  auto getAndClearQueuedActions() -> std::vector<GameActionVariant>;
  void broadcastDeltaGameState(const GameStateDelta&);
  void broadcastFullGameState(const entt::registry&);

  std::jthread gameThread_;
  std::atomic<bool> isRunning_ = false;
  std::queue<GameActionVariant> actionQueue_;
  std::mutex actionQueueMutex_;
  GameState gameState_;
  GameStateThreadConfig config_;
  std::function<void(const SerializedGameState&)> broadcastFn_;
};

}  // namespace lol_at_home_server
