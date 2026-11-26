# ArchipelagoUE

**Bring Archipelago multiworld support to your Unreal Engine games or mods.**

ArchipelagoUE is an Unreal Engine plugin that integrates your game / mod with [Archipelago](https://archipelago.gg), the multi-game randomizer platform.

It provides a C++ and Blueprint API for connecting to Archipelago servers, exchanging items, tracking location checks, synchronizing game state, and communicating with other players in a multiworld session.

Whether you're developing a standalone Unreal Engine game or creating a mod for an existing game, ArchipelagoUE provides the networking and data-handling foundation you need to bring Archipelago support to your project.

## Features

ArchipelagoUE provides the core functionality needed to integrate a game with an Archipelago multiworld session.

- **Server connections:** Connect to Archipelago servers using WebSockets.
- **Room information:** Receive information about the connected room, players, games, and available data.
- **Player connections:** Connect a game instance to a specific player slot using a game name, slot name, and optional password.
- **Item synchronization:** Receive and track items sent to your player, including the complete item history.
- **Location checks:** Send location checks to the server and keep track of checked and unchecked locations.
- **Location scouting:** Request information about locations and the items associated with them.
- **Player and slot information:** Access information about players and their respective games.
- **Chat communication:** Send messages to other players in the multiworld session.
- **Player status:** Update your player's status, such as Ready, Playing, or Goal.
- **PrintJSON support:** Receive and resolve structured Archipelago messages, including item and location names, player names, and message formatting.
- **Configuration management:** Store and load connection settings, including server address, port, slot, and password.
- **Blueprint and C++ API:** Integrate Archipelago functionality using either C++ or Blueprint.

## Requirements

- Unreal Engine with C++ support.
- The [WebSocketsSubsystem](https://github.com/ArvurRobin/WebSocketsSubsystem) plugin.
- An Archipelago server. Most users will connect to the [official Archipelago server](https://archipelago.gg).

## Dependencies

ArchipelagoUE requires the following plugin:

- **[WebSocketsSubsystem](https://github.com/ArvurRobin/WebSocketsSubsystem)** – Provides the WebSocket connection and communication functionality used by ArchipelagoUE.

Both plugins must be installed and enabled for ArchipelagoUE to work.

## Installation

1. Download or clone the [WebSocketsSubsystem repository](https://github.com/ArvurRobin/WebSocketsSubsystem).
2. Download or clone this repository.
3. Place both plugin folders inside your Unreal Engine project's `Plugins` directory.

   Your directory structure should look similar to this:

   ```text
   YourProject/
   └── Plugins/
       ├── WebSocketsSubsystem/
       │   ├── WebSocketsSubsystem.uplugin
       │   └── Source/
       │
       └── ArchipelagoUE/
           ├── ArchipelagoUE.uplugin
           └── Source/
   ```

4. Open your Unreal Engine project.
5. Enable **WebSockets Subsystem** and **ArchipelagoUE** in the Plugins window if they are not already enabled.
6. Regenerate project files if necessary and build your project.

After installation, the Archipelago subsystem is available through the GameInstance subsystem.

## Getting Started

ArchipelagoUE is designed to handle the communication with the Archipelago server while leaving game-specific logic in your project.

A typical integration follows this workflow:

1. **Initialize the integration.** Provide your game's name and message color palette using `SetupArchipelago()`.
2. **Configure the connection.** Set the server address, port, player slot, and optional password.
3. **Connect to the server.** Establish the WebSocket connection and wait for the room information.
4. **Connect to your player slot.** Once room information has been received, ArchipelagoUE sends the player connection request.
5. **Handle incoming events.** React to received items, location updates, chat messages, and other server messages.
6. **Integrate with your game.** Apply received items and report completed locations through the provided API.

The connection process is asynchronous. A successful WebSocket connection does not necessarily mean that the player has already connected to the Archipelago room. Wait for the appropriate connection response before treating the integration as ready.

### Connection configuration

The connection configuration is represented by `FAPConfig`.

It contains the following settings:

| Setting | Description |
|---|---|
| `ServerUrl` | The Archipelago server address. |
| `ServerPort` | The server port. |
| `Slot` | The player slot name to connect to. |
| `Password` | The optional room password. |
| `AutoConnectOnStartup` | Whether to connect automatically on startup. |

Configuration can be saved and loaded, allowing connection settings to persist between game sessions.

Passwords are Base64-encoded when stored in the configuration file. This is an encoding mechanism, not encryption, so configuration files should still be treated as sensitive.

### C++ integration

The main entry point is `UArchipelagoSubsystem`, which derives from `UGameInstanceSubsystem`.

You can access it from your GameInstance:

```cpp
UArchipelagoSubsystem* Archipelago = GetGameInstance()->GetSubsystem<UArchipelagoSubsystem>();
```

Before initializing the integration, provide the game's Archipelago name and message color palette:

```cpp
Archipelago->SetupArchipelago(GameName, ColorPalette);
```

The subsystem then manages the connection, room information, player data, and communication with the server.

For the complete list of available functions, events, and data structures, refer to the public headers in the `Source` directory.

### Blueprint integration

The plugin exposes its main functionality to Blueprint, allowing you to integrate Archipelago without implementing the networking layer yourself.

You can use the Archipelago subsystem to initialize the integration, configure connections, send location checks, request synchronization, communicate through chat, and react to incoming server events.

This makes it possible to keep game-specific behavior in Blueprint while ArchipelagoUE handles the underlying protocol communication.

## How It Works

ArchipelagoUE communicates with an Archipelago server using the Archipelago client protocol over WebSockets.

The general connection sequence is:

```text
Your Game / Mod
   |
   v
ArchipelagoSubsystem
   |
   v
WebSocketsSubsystem
   |
   v
Archipelago Server
   |
   v
RoomInfo
   |
   v
Connect Request
   |
   v
Connected / ConnectionRefused
```

After successfully connecting, the subsystem can exchange game and room information with the server.

### Item handling

When another player sends an item to your slot, Archipelago delivers a network item containing information such as:

- Item ID
- Location ID
- Sending player slot

ArchipelagoUE keeps track of the received item history and provides resolved item information, including item, location, and player names where available.

Your game is responsible for interpreting received items and applying their effects to the actual game state.

### Location checks

Your game can report completed locations to the Archipelago server.

ArchipelagoUE also tracks checked and unchecked locations and can request scouting information for locations.

This allows your game to associate location IDs with their corresponding item information and display useful details to the player.

### Chat and messages

ArchipelagoUE supports sending chat messages and receiving server messages.

It also handles structured `PrintJSON` messages, resolving relevant item, location, and player information so that your game can present meaningful messages to the player.

A message can include information such as who sent an item, which player received it, and where the item originated.

### Synchronization

The subsystem supports requesting an item synchronization from the server.

It maintains the received item history and handles synchronization without unnecessarily broadcasting items that have already been processed.

This helps keep the game state consistent with the Archipelago server.

## Important: Game-Specific Implementation

ArchipelagoUE provides the communication and data-handling layer. It does not automatically modify your game's inventory, progression, or world state.

Your game implementation is responsible for:

- Defining the game's Archipelago item and location data.
- Mapping received item IDs to actual game items.
- Applying received items to the game.
- Determining when locations have been completed.
- Reporting completed locations to ArchipelagoUE.
- Presenting messages and other Archipelago information to the player.

This separation allows ArchipelagoUE to remain reusable across different Unreal Engine games.

## Example Use Cases

ArchipelagoUE can be used to add multiworld randomizer support to:

- Unreal Engine games developed from scratch.
- Existing games with Unreal Engine modding support.
- Games with randomized item placements and progression.
- Multiplayer or cooperative experiences using Archipelago.

## Contributing

Contributions, bug reports, and suggestions are welcome!

If you encounter a problem or have an idea for an improvement, please open an issue in the repository. Pull requests are also welcome.

## Support and Resources

- **Repository:** [ArchipelagoUE](https://github.com/ArvurRobin/ArchipelagoUE)
- **Issues:** [GitHub Issues](https://github.com/ArvurRobin/ArchipelagoUE/issues)
- **WebSocketsSubsystem:** [GitHub Repository](https://github.com/ArvurRobin/WebSocketsSubsystem)
- **Archipelago:** [Official Website](https://archipelago.gg)
- **Archipelago Documentation:** [Archipelago Documentation](https://archipelago.gg/tutorial/)

---

Developed by [Robin Hasenbach](https://github.com/ArvurRobin).
