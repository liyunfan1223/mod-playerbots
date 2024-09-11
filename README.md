 [English](README.md) | [Español](README_ES.md) | [中文](README_CN.md)

# Playerbots Module

Welcome to the Playerbots Module for AzerothCore, a work in progress project based on the IKE3 Playerbots. These Playerbots utilize actual player data, allowing you to interact with your own alts, form parties, level up characters, and much more.

If you encounter any errors or experience crashes, we kindly request that you report them as GitHub issues. Your valuable feedback will help us improve and enhance this project collaboratively.

You can also get more information in our [discord](https://discord.gg/NQm5QShwf9).

## Installation

Please note that this module requires specific custom changes to AzerothCore. To ensure compatibility, you must compile it with a custom branch from my fork, which can be found here: [liyunfan1223/azerothcore-wotlk/tree/Playerbot](https://github.com/liyunfan1223/azerothcore-wotlk/tree/Playerbot).

To install this module, please refer to the AzerothCore Wiki for detailed instructions: [AzerothCore Installation Guide](https://www.azerothcore.org/wiki/installation).

We've provided a simple method to clone the module:

```bash
git clone https://github.com/liyunfan1223/azerothcore-wotlk.git --branch=Playerbot
cd azerothcore-wotlk/modules
git clone https://github.com/liyunfan1223/mod-playerbots.git --branch=master
```

## Quick Start & Documentation

For a quick start and an extensive set of commands, please refer to documentation here: [Playerbots Documentation](https://github.com/liyunfan1223/mod-playerbots/wiki/Playerbot-Documentation).

Please be aware that documentation for some newly added commands is currently lacking as the project is still under development.

## Progress

The module primarily emphasizes the following key features, and we have implemented improvements in these areas:

- **Bots in World (Random bot):** We have enhanced the behavior of random bots to make them mimic real players more closely, creating a more authentic player server environment.

- **Bots in Raid:** We've empowered bots to conquer challenging raid content by implementing specific strategies for various bosses, making raid encounters more engaging. Additionally, we have enhanced bots' capabilities in various roles such as DPS, healing, and tanking, ensuring they contribute effectively to the success of raid groups.

- **Bots in Battleground:** Bots are now capable of actively participating in battlegrounds alongside real players, adding depth and excitement to these PvP scenarios.

- **Interation with Bots:** We have improved the interaction between real players and bots, enabling players to complete quests and level up with multiple characters while collaborating with the bot companions.

- **Player Progression Path:** We have designed an improved progression path for players, complemented by bots, to offer an alternative and engaging gameplay experience.

- **Stability:** Our efforts have focused on enhancing the overall stability of AzerothCore when using the Playerbots module. These improvements aim to prevent server crashes and ensure a smoother experience for all users.

- **Configuration:** We have introduced a range of configurable options to cater to players with varying requirements, allowing for a more personalized experience.

It's essential to note that there is still a significant amount of work to be done as we continue to enhance the project. We welcome everyone to contribute in various ways.

## Addon

For enhanced control over the bots and to simplify command usage, you can also make use of our addon:
- Chinese version: [Unbot Addon (zh)](https://github.com/liyunfan1223/unbot-addon). 
- English version (maintained by @Revision): [Unbot Addon (en)](https://github.com/noisiver/unbot-addon/tree/english).

## Frequently Asked Questions

**Bots can't cast spells**

- Please make sure that the necessary English DBC file (enUS) is present.

**Compilation error**

- We support for Ubuntu, Windows, and macOS.

- Continuous integration workflows have been established. You can review the build status in [GitHub Actions](https://github.com/liyunfan1223/mod-playerbots/actions).

- If the latest build status fails, please revert to the previous commit. We will address the issue ASAP.

## Acknowledgements

The code for this module is ported from [ZhengPeiRu21/mod-playerbots](https://github.com/ZhengPeiRu21/mod-playerbots) and [celguar/mangosbot-bots](https://github.com/celguar/mangosbot-bots). We extend our gratitude to @ZhengPeiRu21 and @celguar for the continued efforts in maintaining the module.

We also want to express our sincere appreciation to all individuals who have contributed to playerbot development. Your dedication and efforts have been instrumental in shaping this project, and we are thankful for your contributions.


