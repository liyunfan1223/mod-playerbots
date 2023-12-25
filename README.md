# Playerbots Module

Welcome to the Playerbots Module for AzerothCore, a work in progress project based on the IKE3 Playerbots. These Playerbots utilize actual player data, allowing you to interact with your own alts, form parties, level up characters, and much more.

If you encounter any errors or experience crashes, we kindly request that you report them as GitHub issues. Your valuable feedback will help us improve and enhance this project collaboratively.

## Installation

Please note that this module requires specific custom changes to AzerothCore. To ensure compatibility, you must compile it with a custom branch from my fork, which can be found here: [liyunfan1223/azerothcore-wotlk/tree/Playerbot](https://github.com/liyunfan1223/azerothcore-wotlk/tree/Playerbot).

To install this module, please refer to the AzerothCore Wiki for detailed instructions: [AzerothCore Installation Guide](https://www.azerothcore.org/wiki/installation).

We've provided a simple method to clone the module:

```bash
git clone https://github.com/liyunfan1223/azerothcore-wotlk.git --branch=Playerbot
cd azerothcore-wotlk
cd module
git clone https://github.com/liyunfan1223/mod-playerbots.git --branch=master
```

## Quick Start & Documentation

For a quick start and an extensive set of commands, you can refer to the manual for the original version of playerbots by ike3. Most of the basic commands are available for this module. You can find the documentation here: [IKE3 Playerbots Documentation](https://ike3.github.io/mangosbot-docs/). Please note that in our module, you need to replace all occurrences of `.bot` with `.playerbot bot` in the documentation.

Please be aware that documentation for newly added commands is currently lacking as the project is still under development. 

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

For enhanced control over the bots and to simplify command usage, you can also make use of our addon: [Unbot Addon](https://github.com/liyunfan1223/unbot-addon). Currently, this addon offers better support only for Simplified Chinese client.

## Frequently Asked Questions

**Why won't my bot cast spells?**

- Ensure the presence of the required English DBC file (enUS).

**Compilation failed on Windows?**

- Developed primarily on Linux, I may miss cross-platform compilation issues. Feel free to report them.

- Future plans include implementing an auto build workflow to prevent such issues.

## Acknowledgements

The code for this module is ported from [ZhengPeiRu21/mod-playerbots](https://github.com/ZhengPeiRu21/mod-playerbots) and [celguar/mangosbot-bots](https://github.com/celguar/mangosbot-bots). We extend our gratitude to @ZhengPeiRu21 and @celguar for the continued efforts in maintaining the module. 

We also want to express our sincere appreciation to all individuals who have contributed to playerbot development, both in the past and the present. Your dedication and efforts have been instrumental in shaping this project, and we are thankful for your contributions.
 

