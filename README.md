# ArchipelagoUE

**ArchipelagoUE** is an Unreal Engine plugin that integrates your game with [Archipelago](https://archipelago.gg), the multi-game randomizer platform. It enables your game to connect to Archipelago servers, exchange items and location checks, synchronize player and room information, and communicate with other players in a multiworld session. The plugin provides an accessible C++ and Blueprint API, making it easier to bring Archipelago support to Unreal Engine projects or Mods.

## Dependencies

ArchipelagoUE requires the following plugin:

- **[WebSocketsSubsystem](https://github.com/ArvurRobin/WebSocketsSubsystem)** – Provides the WebSocket connection and communication functionality used by ArchipelagoUE.

### Installation

1. Download or clone the [WebSocketsSubsystem repository](https://github.com/ArvurRobin/WebSocketsSubsystem).
2. Install it into your Unreal Engine project's `Plugins` directory.
3. Download or clone this repository into the same `Plugins` directory.
4. Regenerate project files if necessary and build your project.

Both plugins must be installed and enabled for ArchipelagoUE to work.
