# MENKIA Windows x64 — manual acceptance checklist

This checklist is for validating the packaged Windows build. A successful CI compile and CTest run do not replace these runtime checks.

## 1. Package integrity

- [ ] Extract the complete `MENKIA-Windows-x64.zip` to a writable folder.
- [ ] Keep `MENKIA.exe` beside the shipped Qt and MinGW runtime DLLs and plugin folders.
- [ ] Start `MENKIA.exe` from the extracted folder (do not launch it from inside the ZIP).
- [ ] Confirm the launcher opens without a missing-DLL or Qt-platform-plugin error.
- [ ] Confirm the application can create and write to its data directory.

## 2. Launcher basics

- [ ] Complete the first-run wizard.
- [ ] Create a clean Minecraft instance.
- [ ] Confirm the instance appears in the library and can be reopened after restarting MENKIA.
- [ ] Confirm instance settings, notes, screenshots and logs pages open.
- [ ] Confirm the launcher reports Java/runtime errors clearly instead of silently failing.

## 3. Modrinth manager

Use a disposable instance and back it up before testing installation.

- [ ] Open the instance editor and select **MENKIA Mods & HUD**.
- [ ] Confirm the displayed Minecraft version matches the selected instance.
- [ ] Confirm the detected loader matches the instance profile; the loader selector must remain read-only.
- [ ] Search for a known compatible project and confirm the results are populated.
- [ ] Search an incompatible version/loader combination and confirm it is not offered for installation.
- [ ] Install one small, trusted primary JAR and confirm it appears only in that instance's `mods` folder.
- [ ] Confirm an existing filename requires replacement confirmation.
- [ ] Confirm a release with required dependencies is stopped with a clear explanation.
- [ ] Confirm installation is blocked while the Minecraft instance is running.
- [ ] Launch Minecraft and confirm the selected mod loads successfully.

## 4. Compatibility matrix

Test only combinations for which the selected loader and mod release actually exist.

| Minecraft | Loader | Expected validation |
| --- | --- | --- |
| 1.8.9 | Forge | Detect loader, search compatible Forge mods, install and launch |
| 1.12.2 | Forge | Detect loader, search compatible Forge mods, install and launch |
| Modern | Fabric | Detect loader, search compatible Fabric mods, install and launch |
| Modern | Quilt | Detect loader, search compatible Quilt mods, install and launch |
| Modern | Forge | Detect loader, search compatible Forge mods, install and launch |
| Modern | NeoForge | Detect loader, search compatible NeoForge mods, install and launch |

A project not published for a given Minecraft/loader combination is not a launcher failure.

## 5. Known limitations — do not mark as passed by assumption

- The current quick buttons are Modrinth searches; they are not built-in HUD overlays.
- Required mod dependencies are not installed automatically yet.
- Mod conflicts are not resolved automatically.
- Sodium and VulkanMod are alternative rendering stacks and must not be combined.
- A client-only launcher cannot report authoritative server TPS without a compatible mod or server-side data.
- Microsoft sign-in and signed self-updates require MENKIA-owned application registration and signing configuration before production release.

## Result

Record the Windows version, Minecraft version, loader, mod project/version, and any crash log for each failure. Do not publish a stable release until the runtime checks and the supported compatibility matrix have been exercised on a real Windows machine.
