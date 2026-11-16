# MENKIA Mod Manager

The instance editor includes a Modrinth browser for individual mods. Search results are restricted to the selected Minecraft version; installation resolves the project's version list against the selected/detected loader and writes the primary JAR into that instance's `minecraft/mods` directory.

## Supported version families

- Minecraft 1.8.9: Forge projects only where Modrinth publishes a compatible build. Legacy PvP HUD mods are not guaranteed to be available on Modrinth.
- Minecraft 1.12.2: Forge-compatible projects.
- Modern Minecraft: Fabric, Quilt, Forge and NeoForge projects as published by Modrinth.

## HUD and optimization discovery

Use the quick searches in the instance page for Performance, Keystrokes, CPS, FPS/Ping and HUD. These are searches, not hard-coded promises: Modrinth compatibility data determines which releases are offered for the selected game version and loader.

Sodium and VulkanMod are alternative renderers and must not be installed together. The manager deliberately does not bundle either renderer automatically. Select a compatible project and review its dependencies before launch.

## Current safety limits

- Mods are installed per instance, never globally.
- Installation is disabled while the game is running.
- Only HTTPS primary JAR files are accepted.
- The manager does not silently install required dependencies or resolve mod conflicts yet. Review the project's dependency metadata and compatibility notes before launching.
- TPS HUD values require server-side data or a mod that can obtain them; a client-only launcher cannot infer authoritative server TPS.
