#include <enet/enet.h>
#include <spdlog/spdlog.h>
#include <entt/entt.hpp>
#include <iostream>
#include <thread>
#include "EcsComponents.h"
#include "GameStateSerializer.h"

auto main() -> int {
  if (enet_initialize() != 0) {
    spdlog::error("Failed to initialize ENet");
    return 1;
  }

  ENetHost* client = enet_host_create(nullptr, 1, 2, 0, 0);
  if (client == nullptr) {
    spdlog::error("Failed to create ENet client");
    enet_deinitialize();
    return 1;
  }

  ENetAddress address;
  enet_address_set_host(&address, "127.0.0.1");
  address.port = 12345;

  ENetPeer* peer = enet_host_connect(client, &address, 2, 0);
  if (peer == nullptr) {
    spdlog::error("No available peers for connection");
    enet_host_destroy(client);
    enet_deinitialize();
    return 1;
  }

  ENetEvent event;
  if (enet_host_service(client, &event, 5000) > 0 &&
      event.type == ENET_EVENT_TYPE_CONNECT) {
    spdlog::info("Connected to server!");
  } else {
    spdlog::error("Connection to server failed");
    enet_peer_reset(peer);
    enet_host_destroy(client);
    enet_deinitialize();
    return 1;
  }

  entt::registry registry;

  spdlog::info("Receiving game state updates. Press Enter to quit.");

  bool running = true;
  std::thread inputThread([&running]() {
    std::string input;
    std::getline(std::cin, input);
    running = false;
  });

  while (running) {
    while (enet_host_service(client, &event, 10) > 0) {
      switch (event.type) {
        case ENET_EVENT_TYPE_RECEIVE: {
          spdlog::info("Received " + std::to_string(event.packet->dataLength) +
                       " bytes from server");

          try {
            std::vector<std::byte> data(
                reinterpret_cast<const std::byte*>(event.packet->data),
                reinterpret_cast<const std::byte*>(event.packet->data) +
                    event.packet->dataLength);

            lol_at_home_shared::GameStateSerializer::Deserialize(registry,
                                                                 data);

            auto posView = registry.view<lol_at_home_shared::Position>();
            spdlog::info("Registry now contains " +
                         std::to_string(posView.size()) + " entities");

            for (auto entity : posView) {
              const auto& pos =
                  posView.get<lol_at_home_shared::Position>(entity);
              spdlog::info("  Entity " +
                           std::to_string(static_cast<uint32_t>(entity)) +
                           ": Position (" + std::to_string(pos.X) + ", " +
                           std::to_string(pos.Y) + ")");

              if (auto* health =
                      registry.try_get<lol_at_home_shared::Health>(entity)) {
                spdlog::info(
                    "    Health: " + std::to_string(health->CurrentHealth) +
                    "/" + std::to_string(health->MaxHealth));
              }

              if (auto* moving =
                      registry.try_get<lol_at_home_shared::Moving>(entity)) {
                spdlog::info("    Moving to: (" +
                             std::to_string(moving->TargetPosition.X) + ", " +
                             std::to_string(moving->TargetPosition.Y) + ")");
              }
            }
          } catch (const std::exception& e) {
            spdlog::error(std::string("Failed to deserialize game state: ") +
                          e.what());
          }

          enet_packet_destroy(event.packet);
          break;
        }

        case ENET_EVENT_TYPE_DISCONNECT:
          spdlog::info("Disconnected from server");
          running = false;
          break;

        default:
          break;
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  inputThread.join();

  spdlog::info("Shutting down client...");
  enet_peer_disconnect(peer, 0);

  while (enet_host_service(client, &event, 3000) > 0) {
    if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
      break;
    }
  }

  enet_host_destroy(client);
  enet_deinitialize();
  spdlog::info("Client shut down");

  return 0;
}
