#pragma once

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include "GameAction.h"
#include "GameState.h"
#include "GameStateThreadConfig.h"

namespace lol_at_home_server {

class GameStateThread {
 public:
  explicit GameStateThread(GameState gameState, GameStateThreadConfig config);
  void Start();
  void Stop();
  void HandleInput(GameActionVariant input);

 private:
  void runAndBlockGameLoop();
  auto getAndClearQueuedActions() -> std::vector<GameActionVariant>;
  static void broadcastDeltaGameState(const GameStateDelta&);
  static void broadcastFullGameState(
      const std::unordered_map<EntityId, std::unique_ptr<Entity>>&);

  std::jthread gameThread_;
  std::atomic<bool> isRunning_ = false;
  std::queue<GameActionVariant> actionQueue_;
  std::mutex actionQueueMutex_;
  GameState gameState_;
  GameStateThreadConfig config_;
};

}  // namespace lol_at_home_server
