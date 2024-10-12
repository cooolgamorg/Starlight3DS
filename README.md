# Starlight3DS
*Nintendo 3DS "Custom Firmware"*

![GitHub Downloads (all assets, all releases)](https://img.shields.io/github/downloads/cooolgamorg/Starlight3DS/total)
![License](https://img.shields.io/badge/License-GPLv3-blue.svg)

*Nintendo 3DS "Custom Firmware"*

**Please check the [Official Luma3DS readme](https://github.com/LumaTeam/Luma3DS) for more info!**

## What it is
**Starlight3DS** is an unofficial fork of [Luma3DS](https://github.com/LumaTeam/Luma3DS) with more features, this is a continuation of [DullPointer's luma fork](https://github.com/DullPointer/Luma3DS) where most features are taken from it and made compatible for the latest Luma3DS version at the time of writing this.

There's also some features taken from other luma forks as well and some that I added myself. Please check the [credits](https://github.com/cooolgamorg/Starlight3DS?tab=readme-ov-file#credits) to see people who made this fork possible!

Starlight3DS requires a full-system persisent exploit such as [boot9strap](https://github.com/SciresM/boot9strap) to run.

## Changes with the official build
Note: The config file is different from the original luma3DS, it's "configplus.ini" instead of "config.ini" to avoid conflicts if you want to switch between this modified version and the official one (like for playing CTGP-7 for example).

- Changed auto-copy to ctrnand and essential backup checks to be less annoying
- Restored UNITINFO and enable rosalina on safe_firm options on the luma config menu (TWL patch option is now with "enable external firms and modules")
- Removed the need of having both "Game Patching" and "External Firms and Modules" enabled for patching sysmodules, just "External Firms and Modules" need to be enabled
- Added shortcuts:
  - Press start + select to toggle bottom screen (nice when you watch videos) inspired by [This](https://github.com/DullPointer/Luma3DS/commit/9fea831656446cbaa2b5b4f6364407bb1b35dee7)
  - Press A + B + X + Y + Start to instantly reboot the console. Useful in case of freeze, but can corrupt your sdcard. This is why this option is now hidden in the `config.ini` file
  - Press Start on Rosalina menu to toggle wifi -> [Original](https://github.com/DullPointer/Luma3DS/commit/c1a20558bed3d792d54069719a898006af20ba85)
  - Press Select on Rosalina menu to toggle LEDs -> [Original](https://github.com/DullPointer/Luma3DS/commit/fa70d374c00e39dee8b9ef54f60deb1da35a0c51) (and press Y to force blue led as a workaround when the battery is low)
- Added [Extra Config menu](https://github.com/DullPointer/Luma3DS/commit/174ed486ab59bd249488c9035682fa7d058d1e80) including:
  - Automatically suppress LEDs
  - Cut power to TWL Flashcards allowing to save battery
  - Home button opens Rosalina
  - start + select shortcut to toggle top/bottom screen (enabled on bottom screen by default)
- Added Day and Night mode (lite):
  - You can choose the screen brightness
  - You can choose to turn on or off LEDs
  - Those will be applied at a desired time
- Added n3ds clock + L2 status in rosalina menu -> [Original](https://github.com/DullPointer/Luma3DS/commit/2dbfa8b5c9b719b7f3056691f54332f42da6de8d)
- Added Software Volume Control -> (now merged with the official luma3ds one)
- Added extended brightness presets and per-screen brightness -> [Original](https://github.com/DullPointer/Luma3DS/commit/83e8d471a377bc6960fae00d6694f5fe86dcca42) WARNING: a little broken and can exceed the limits!
- Added permanent brightness calibration -> [Original](https://github.com/DullPointer/Luma3DS/commit/0e67a667077f601680f74ddc10ef88a799a5a7ad)
- Added a option to cut wifi while in sleep mode allowing to save battery -> [Original](https://github.com/DullPointer/Luma3DS/commit/174ed486ab59bd249488c9035682fa7d058d1e80)
- Added the ability to redirect layeredFS path -> [Original](https://github.com/DeathChaos25/Luma3DS/commit/8f68d0a19d2ed80fb41bbe8499cb2b7b027e8a8c)
- Added loading of custom logo while launching a app from luma/logo.bin -> [Original](https://github.com/Pixel-Pop/Luma3DS/commit/d225d9fa507dcccce3a6c86d0a38f7998f39b7a2)
- Added loading of external decrypted otp from luma/otp.bin -> [Original](https://github.com/truedread/Luma3DS/commit/aa395a5910113b991e6ad7edc3415152be9bbb03#diff-ef6e6ba78a0250ac4b86068018f17d423fe816bb00fb3b550725f1cb6f983d29)
- Changed colors on config menu because why not
- Continue running after a errdisp error happens, this is the same option as instant reboot because they kinda go together (you decide when to reboot after an error occur)
- Added play coin maximize to rosalina menu -> [Original](https://github.com/Gruetzig/Luma3DS/commit/1e329b55dade61ba74a0bb1cc6e59d2504d0bde1)
- Plugin Selector -> [Original](https://github.com/Tekito-256/Luma3DS)

## Maintainers
**Original Luma3DS mainteners**:

* **[@TuxSH](https://github.com/TuxSH)**: lead developer, created and maintains most features of the project. Joined in 2016
* **[@AuroraWright](https://github.com/AuroraWright)**: author of the project, implemented the core features (most of the baremetal boot settings menu and firmware loading code) with successful design decisions that made the project popular. Created the project in 2015, currently inactive
* **[@PabloMK7](https://github.com/PabloMK7)**: maintainer of the plugin loader feature merged for the v13.0 release. Joined in 2023

**Starlight3DS mainteners**:

* **[@cooolgamer](https://github.com/cooolgamer)**
* [Everyone who added features by forking the project](#credits)

## Known issues

**Official Luma3DS issues:**

* **Cheat engine crashes with some applications, in particular Pokémon games**: there is a race condition in Nintendo's `Kernel11` pertaining to attaching a new `KDebugThread` to a `KThread` on thread creation, and another thread null-dereferencing `thread->debugThread`. This causes the cheat engine to crashes games that create and destroy many threads all the time (like Pokémon).
    * For these games, having a **dedicated "game plugin"** is the only alternative until `Kernel11` is reimplemented.
* **Applications reacting to Rosalina menu button combo**: Rosalina merely polls button input at an interval to know when to show the menu. This means that the Rosalina menu combo can sometimes be processed by the game/process that is going to be paused.
    * You can **change the menu combo** in the "Miscellaneous options" submenu (then save it with "Save settings" in the main menu) to work around this.

**Starlight3DS issues:**

* **Some games such as Donkey Kong Country Returns 3D doesn't work**: I geniunely have no clue why this happen, I can't test it in my side because I don't have the game. Even if I could, I don't know how to investigate this issue. Help is welcomed! Discuss about this issue [In this issue](https://github.com/cooolgamorg/Starlight3DS/issues/17)
* **Low extended values for backlight are broken**: If you hold L or R to use the extended values below the original limits, it will glitch and gets extremely high. I checked the code for this and it seems good to me, maybe I need to investigate a little more.
    * DO NOT USE THESE VALUES IF THIS HAPPEN! This *may* DAMAGE the screen!

## Building from source

To build StarLight3DS, the following is needed:
* git
* [makerom](https://github.com/jakcron/Project_CTR) in `$PATH`
* [firmtool](https://github.com/TuxSH/firmtool) installed
* up-to-date devkitARM and libctru:
    * install `dkp-pacman` (or, for distributions that already provide pacman, add repositories): https://devkitpro.org/wiki/devkitPro_pacman
    * install packages from `3ds-dev` metapackage: `sudo dkp-pacman -S 3ds-dev --needed`
    * while libctru and StarLight3DS releases are kept in sync, you may have to build libctru from source for non-release StarLight3DS commits

While StarLight3DS releases are bundled with `3ds-hbmenu`, StarLight3DS actually compiles into one single file: `boot.firm`. Just copy it over to the root of your SD card ([ftpd](https://github.com/mtheall/ftpd) is the easiest way to do so), and you're done.

## Setup / Usage / Features
See https://github.com/LumaTeam/Luma3DS/wiki (needs rework)

## Licensing
This software is licensed under the terms of the GPLv3. You can find a copy of the license in the LICENSE.txt file.

Files in the GDB stub are instead triple-licensed as MIT or "GPLv2 or any later version", in which case it's specified in the file header. PM, SM, PXI reimplementations are also licensed under MIT.

## Credits
See https://github.com/LumaTeam/Luma3DS/wiki/Credits for original Luma3DS credits

People who made this fork possible:
- [DullPointer](https://github.com/DullPointer): For making the original fork where a lot of features come from,
- Sono: For contributing in DullPointer's Luma3DS Fork,
- Nutez: For contributing in DullPointer's Luma3DS Fork,
- [Pixel-Pop](https://github.com/Pixel-Pop): For adding the custom logo.bin feature,
- [DeathChaos25](https://github.com/DeathChaos25): For the ability to redirect the layeredFS paths,
- [truedread](https://github.com/truedread): For the external otp loading feature,
- [Gruetzig](https://github.com/Gruetzig): For the 300 Play Coins in the rosalina menu feature,
- [Tekito-256](https://github.com/Tekito-256): For the Plugin Selector, please check [their fork](https://github.com/Tekito-256/Luma3DS) for more plugin related features!
- [exalented](https://github.com/exalented): For adding quick top screen toggle feature,
- [Alexyo21](https://github.com/Alexyo21): For helping me!
