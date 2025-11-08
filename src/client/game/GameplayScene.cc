#include "GameplayScene.h"
#include <spdlog/spdlog.h>
#include "ClientComponents.h"
#include "EcsComponents.h"
#include "GameAction.h"
#include "GameActionSerializer.h"
#include "GameStateSerializer.h"
#include "PlayerAssignment.h"
#include "PlayerAssignmentSerializer.h"
#include "Renderer.h"

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

  camera_->SetPosition({.X = 100.0, .Y = 200.0});
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
        .Source = myEntityId_,
        .TargetPosition = {.X = worldPos.X, .Y = worldPos.Y}};

    auto bytes = lol_at_home_shared::GameActionSerializer::Serialize(action);
    ENetPacket* packet = enet_packet_create(bytes.data(), bytes.size(),
                                            ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer_, 0, packet);

    spdlog::info("Sent move to (" + std::to_string(worldPos.X) + ", " +
                 std::to_string(worldPos.Y) + ")");
  }
}

void GameplayScene::handleNetwork() {
  if (!connected_) {
    return;
  }

  ENetEvent event;
  while (enet_host_service(client_, &event, 0) > 0) {
    /*
    switch (event.type) {
      case ENET_EVENT_TYPE_RECEIVE: {
        std::vector<std::byte> data(
            reinterpret_cast<const std::byte*>(event.packet->data),
            reinterpret_cast<const std::byte*>(event.packet->data) +
                event.packet->dataLength);

        try {
          switch (event.channelID) {
            case lol_at_home_shared::NetworkChannels::Control: {
              auto assignment =
                  lol_at_home_shared::PlayerAssignmentSerializer::Deserialize(
                      data);
              myEntityId_ = assignment.AssignedEntity;
              spdlog::info("Assigned to entity: " +
                           std::to_string(static_cast<uint32_t>(myEntityId_)));
              break;
            }
            case lol_at_home_shared::NetworkChannels::GameState: {
              lol_at_home_shared::GameStateSerializer::Deserialize(registry_,
                                                                   data);
              break;
            }
            default: {
              spdlog::error(std::string("Received unknown channelID: ") +
                            std::to_string(event.channelID));
            }
          }
        } catch (const std::exception& e) {
          spdlog::error(std::string("Failed to deserialize: ") + e.what());
        }

        enet_packet_destroy(event.packet);
        break;
      }

      case ENET_EVENT_TYPE_DISCONNECT:
        spdlog::info("Disconnected from server");
        connected_ = false;
        break;

      default:
        break;
    }
        */ // todo all the client code is broken rn but i just care abt server right now so just comment stuff out so server can build lolololol
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

    double dxToServer = serverPos.X - visualPos->Position.X;
    double dyToServer = serverPos.Y - visualPos->Position.Y;
    double distance =
        std::sqrt(dxToServer * dxToServer + dyToServer * dyToServer);

    if (distance < 0.5) {
      visualPos->Position = serverPos;
      continue;
    }

    double speed = 0;
    if (auto* movable =
            registry_.try_get<lol_at_home_shared::Movable>(entity)) {
      speed = movable->Speed;
    } else {
      visualPos->Position = serverPos;
      continue;
    }

    double deltaSec = deltaTime / 1000.0;
    double moveDistance = speed * deltaSec;

    if (moveDistance >= distance) {
      visualPos->Position = serverPos;
    } else {
      double ratio = moveDistance / distance;
      visualPos->Position.X += dxToServer * ratio;
      visualPos->Position.Y += dyToServer * ratio;
    }
  }
}

void GameplayScene::renderEntities(lol_at_home_engine::Renderer& renderer) {
  auto view = registry_.view<lol_at_home_shared::Position>();
  for (auto entity : view) {
    lol_at_home_shared::Position renderPos;
    if (auto* visualPos = registry_.try_get<VisualPosition>(entity)) {
      renderPos = visualPos->Position;
    } else {
      renderPos = view.get<lol_at_home_shared::Position>(entity);
    }

    lol_at_home_engine::Color color{.r = 100, .g = 200, .b = 255, .a = 255};
    if (auto* team = registry_.try_get<lol_at_home_shared::Team>(entity)) {
      color = team->TeamColorFB == lol_at_home_shared::Team::Color::Blue
                  ? lol_at_home_engine::Color{.r = 100,
                                              .g = 150,
                                              .b = 255,
                                              .a = 255}
                  : lol_at_home_engine::Color{
                        .r = 255, .g = 100, .b = 100, .a = 255};
    }

    renderer.DrawCircle({.X = renderPos.X, .Y = renderPos.Y}, 50.0, color);

    if (auto* health = registry_.try_get<lol_at_home_shared::Health>(entity)) {
      double healthPercent = health->CurrentHealth / health->MaxHealth;
      lol_at_home_engine::Vector2 barPos{.X = renderPos.X - 30.0,
                                         .Y = renderPos.Y - 70.0};
      lol_at_home_engine::Vector2 barSize{.X = 60.0 * healthPercent, .Y = 5.0};

      renderer.DrawRect(barPos, barSize, {.r = 0, .g = 255, .b = 0, .a = 255});
    }

    if (auto* moving = registry_.try_get<lol_at_home_shared::Moving>(entity)) {
      renderer.DrawCircle(
          {.X = moving->TargetPosition.X, .Y = moving->TargetPosition.Y}, 10.0,
          {.r = 255, .g = 255, .b = 0, .a = 255});
      renderer.DrawLine(
          {.X = renderPos.X, .Y = renderPos.Y},
          {.X = moving->TargetPosition.X, .Y = moving->TargetPosition.Y},
          {.r = 255, .g = 255, .b = 0, .a = 128});
    }
  }
}

}  // namespace lol_at_home_game
