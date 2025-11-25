#include "GameplayScene.h"
#include <spdlog/spdlog.h>
#include "ClientComponents.h"
#include "Renderer.h"
#include "domain/EcsComponents.h"
#include "domain/GameAction.h"
#include "serialization/GameActionSerializer.h"

namespace lol_at_home_game {

GameplayScene::GameplayScene(std::shared_ptr<lol_at_home_engine::Camera> camera,
                             lol_at_home_engine::Input& input,
                             lol_at_home_engine::Renderer& renderer)
    : camera_(std::move(camera)), input_(input), renderer_(renderer) {}

GameplayScene::~GameplayScene() {
  if (client_ != nullptr) {
    enet_host_destroy(client_);
  }
  enet_deinitialize();
}

void GameplayScene::OnStart() {
  spdlog::info("GameplayScene started");

  if (enet_initialize() != 0) {
    spdlog::error("Failed to initialize ENet");
    return;
  }

  connectToServer();

  camera_->SetPosition({.x = 100.0, .y = 200.0});
  camera_->SetZoom(1.0F);
}

void GameplayScene::Update(double deltaTime) {
  handleInput();
  handleNetwork();
  updateInterpolation(deltaTime);
}

void GameplayScene::Render(lol_at_home_engine::Renderer& renderer) {
  renderEntities(renderer);
}

void GameplayScene::OnEnd() {
  if (peer_ != nullptr && connected_) {
    enet_peer_disconnect(peer_, 0);

    ENetEvent event;
    while (enet_host_service(client_, &event, 3000) > 0) {
      if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
        break;
      }
    }
  }

  spdlog::info("GameplayScene ended");
}

auto GameplayScene::ShouldContinue() -> bool {
  return connected_;
}

void GameplayScene::connectToServer() {
  client_ = enet_host_create(nullptr, 1, 2, 0, 0);
  if (client_ == nullptr) {
    spdlog::error("Failed to create ENet client");
    return;
  }

  ENetAddress address;
  enet_address_set_host(&address, "127.0.0.1");
  address.port = 12345;

  peer_ = enet_host_connect(client_, &address, 2, 0);
  if (peer_ == nullptr) {
    spdlog::error("No available peers for connection");
    return;
  }

  ENetEvent event;
  if (enet_host_service(client_, &event, 5000) > 0 &&
      event.type == ENET_EVENT_TYPE_CONNECT) {
    spdlog::info("Connected to server!");
    connected_ = true;
  } else {
    spdlog::error("Connection to server failed");
    enet_peer_reset(peer_);
    peer_ = nullptr;
  }
}

void GameplayScene::handleInput() {
  if (!connected_ || myEntityId_ == entt::null) {
    return;
  }

  if (input_.IsMouseButtonPressed(SDL_BUTTON_RIGHT)) {
    lol_at_home_engine::Vector2 screenPos = input_.GetMousePosition();
    lol_at_home_engine::Vector2 worldPos =
        camera_->ScreenToWorld(screenPos, renderer_.GetScreenSize());

    lol_at_home_shared::MoveAction action{
        .source = myEntityId_,
        .targetPosition = {.x = worldPos.x, .y = worldPos.y}};

    // todo serialize, create packet, send
  }
}

void GameplayScene::handleNetwork() {
  if (!connected_) {
    return;
  }

  ENetEvent event;
  while (enet_host_service(client_, &event, 0) > 0) {
    switch (event.type) {
      case ENET_EVENT_TYPE_RECEIVE: {
        std::vector<std::byte> data(
            reinterpret_cast<const std::byte*>(event.packet->data),
            reinterpret_cast<const std::byte*>(event.packet->data) +
                event.packet->dataLength);

        // todo extract the union or smth

        enet_packet_destroy(event.packet);
        break;
      }

      case ENET_EVENT_TYPE_DISCONNECT:
        spdlog::info("Disconnected from server");
        connected_ = false;
        break;

      case ENET_EVENT_TYPE_NONE:
      case ENET_EVENT_TYPE_CONNECT:
        break;
    }
  }
}

void GameplayScene::updateInterpolation(double deltaTime) {
  auto view = registry_.view<lol_at_home_shared::Position>();

  for (auto entity : view) {
    const auto& serverPos = view.get<lol_at_home_shared::Position>(entity);

    auto* visualPos = registry_.try_get<VisualPosition>(entity);
    if (visualPos == nullptr) {
      registry_.emplace<VisualPosition>(entity, serverPos);
      continue;
    }

    double dxToServer = serverPos.x - visualPos->position.x;
    double dyToServer = serverPos.y - visualPos->position.y;
    double distance =
        std::sqrt((dxToServer * dxToServer) + (dyToServer * dyToServer));

    if (distance < 0.5) {
      visualPos->position = serverPos;
      continue;
    }

    double speed = 0;
    if (auto* movable =
            registry_.try_get<lol_at_home_shared::Movable>(entity)) {
      speed = movable->speed;
    } else {
      visualPos->position = serverPos;
      continue;
    }

    double deltaSec = deltaTime / 1000.0;
    double moveDistance = speed * deltaSec;

    if (moveDistance >= distance) {
      visualPos->position = serverPos;
    } else {
      double ratio = moveDistance / distance;
      visualPos->position.x += dxToServer * ratio;
      visualPos->position.y += dyToServer * ratio;
    }
  }
}

void GameplayScene::renderEntities(lol_at_home_engine::Renderer& renderer) {
  // todo ??
}

}  // namespace lol_at_home_game
