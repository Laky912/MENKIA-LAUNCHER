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

## 3. Local profiles (Alt Manager)

- [ ] Open Accounts and create two local profiles with different valid names.
- [ ] Confirm each profile appears in the account list and can be selected independently.
- [ ] Set each profile as the default in turn and confirm the default indicator follows the selection.
- [ ] Close MENKIA completely, reopen it, and confirm both profiles and the selected default remain saved.
- [ ] Launch a disposable offline-mode instance with each profile and verify the in-game player name.
- [ ] Confirm local profiles do not start Microsoft OAuth or request token refresh.
- [ ] Confirm Microsoft accounts can still be added and used normally.
- [ ] Rename a local profile and confirm the updated nickname is saved after restarting MENKIA.
- [ ] Confirm the rename warning explains that offline-mode servers may assign a different player identity.
- [ ] Remove one local profile with Sign Out and confirm the other profile remains.

Local profiles are offline identities only. They do not grant Minecraft ownership, Microsoft authentication, official skins/capes, or access to online-mode servers. Keep this distinction when interpreting test results.

## 4. Modrinth manager

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

## 5. Compatibility matrix

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

## 6. Known limitations — do not mark as passed by assumption

- The current quick buttons are Modrinth searches; they are not built-in HUD overlays.
- Required mod dependencies are not installed automatically yet.
- Mod conflicts are not resolved automatically.
- Sodium and VulkanMod are alternative rendering stacks and must not be combined.
- A client-only launcher cannot report authoritative server TPS without a compatible mod or server-side data.
- Microsoft sign-in and signed self-updates require MENKIA-owned application registration and signing configuration before production release.

## Result

Record the Windows version, Minecraft version, loader, mod project/version, and any crash log for each failure. Do not publish a stable release until the runtime checks and the supported compatibility matrix have been exercised on a real Windows machine.
