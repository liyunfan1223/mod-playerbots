<p align="center">
    <a href="https://github.com/liyunfan1223/mod-playerbots/blob/master/README.md">English</a>
    |
    <a href="https://github.com/liyunfan1223/mod-playerbots/blob/master/README_CN.md">中文</a>
</p>


<div align="center">
  <img src="icon.png" alt="Playerbots Icon" width="700px">
</div>

<div align="center">
    <img src="https://github.com/liyunfan1223/mod-playerbots/actions/workflows/macos_build.yml/badge.svg">
    <img src="https://github.com/liyunfan1223/mod-playerbots/actions/workflows/core_build.yml/badge.svg">
    <img src="https://github.com/liyunfan1223/mod-playerbots/actions/workflows/windows_build.yml/badge.svg">
</div>

# Playerbots Module
`mod-playerbots` is an [AzerothCore](https://www.azerothcore.org/) module that adds player-like bots to a server. The project is based off [IKE3's Playerbots](https://github.com/ike3/mangosbot). Features include:

- Bots that utilize real player data, allowing players to interact with their other characters, form parties, level up, and more;
- Random bots that wander through the world and behave like players, simulating the MMO experience;
- Bots capable of running raids and battlegrounds;
- Highly configurable settings to define how bots behave;
- Excellent performance, even when running thousands of bots.

**This project is still under development**. If you encounter any errors or experience crashes, we kindly request that you [report them as GitHub issues](https://github.com/liyunfan1223/mod-playerbots/issues/new?template=bug_report.md). Your valuable feedback will help us improve this project collaboratively.

**Playerbots Module** has a **[Discord server](https://discord.gg/NQm5QShwf9)** where you can discuss the project.

## Installation

### Classic Installation

`mod-playerbots` requires a custom branch of AzerothCore to work: [liyunfan1223/azerothcore-wotlk/tree/Playerbot](https://github.com/liyunfan1223/azerothcore-wotlk/tree/Playerbot). To install the module, simply run:

```bash
git clone https://github.com/liyunfan1223/azerothcore-wotlk.git --branch=Playerbot
cd azerothcore-wotlk/modules
git clone https://github.com/liyunfan1223/mod-playerbots.git --branch=master
```

For more information, refer to the [AzerothCore Installation Guide](https://www.azerothcore.org/wiki/installation) and [Installing a Module](https://www.azerothcore.org/wiki/installing-a-module) pages.

### Docker Installation

**Docker installation is considered experimental.** To install the module on a Docker installation, run:

```bash
git clone https://github.com/liyunfan1223/azerothcore-wotlk.git --branch=Playerbot
cd azerothcore-wotlk/modules
git clone https://github.com/liyunfan1223/mod-playerbots.git --branch=master
```

Afterwards, create a `docker-compose.override.yml` file in the `azerothcore-wotlk` directory. This override file allows for mounting the modules directory to the `ac-worldserver` service which is required for it to run. Put the following inside and save:

```yml
services:
  ac-worldserver:
    volumes:
      - ./modules:/azerothcore/modules:ro
```

Additionally, this override file can be used to set custom configuration settings for `ac-worldserver` and any modules you install as environment variables:

```yml
services:
  ac-worldserver:
    environment:
      AC_RATE_XP_KILL: "1"
      AC_AI_PLAYERBOT_RANDOM_BOT_AUTOLOGIN: "1"
    volumes:
      - ./modules:/azerothcore/modules:ro
```

For example, to double the experience gain rate per kill, take the setting `Rate.XP.Kill = 1` from [woldserver.conf](https://github.com/liyunfan1223/azerothcore-wotlk/blob/Playerbot/src/server/apps/worldserver/worldserver.conf.dist), convert it to an environment variable, and change it to the desired setting in the override file to get `AC_RATE_XP_KILL: "2"`. If you wanted to disable random bots from logging in automatically, take the `AiPlayerbot.RandomBotAutologin = 1` setting from [playerbots.conf](https://github.com/liyunfan1223/mod-playerbots/blob/master/conf/playerbots.conf.dist) and do the same to get `AC_AI_PLAYERBOT_RANDOM_BOT_AUTOLOGIN: "0"`. For more information on how to configure Azerothcore, Playerbots, and other module settings as environment variables in Docker Compose, see the "Configuring AzerothCore in Containers" section in the [Install With Docker](https://www.azerothcore.org/wiki/install-with-docker) guide.

Before building, consider setting the database password. One way to do this is to create a `.env` file in the root `azerothcore-wotlk` directory using the [template](https://github.com/liyunfan1223/azerothcore-wotlk/blob/Playerbot/conf/dist/env.docker). This file also allows you to set the user and group Docker uses for the services in case you run into any permissions issues, which are the most common cause for Docker installation problems.

Use `docker compose up -d --build` to build and run the server. For more information, including how to create an account and taking backups, refer to the [Install With Docker](https://www.azerothcore.org/wiki/install-with-docker) page.

## Documentation

The [Playerbots Wiki](https://github.com/liyunfan1223/mod-playerbots/wiki) contains an extensive overview of addons, commands, and recommended configurations. Please note that documentation may be incomplete or out-of-date in some sections. Contributions are welcome.

## Frequently Asked Questions

- **Why aren't my bots casting spells?** Please make sure that the necessary English DBC file (enUS) is present.
- **What platforms are supported?** We support Ubuntu, Windows, and macOS. Other Linux distros may work, but will not receive support.
- **Why isn't my source compiling?** Please [check the build status of our CI](https://github.com/liyunfan1223/mod-playerbots/actions). If the latest build is failing, rever to the last successful commit until we address the issue.

## Addons

Typically, bots are controlled via chat commands. For larger bot groups, this can be unwieldy. As an alternative, community members have developed client Add-Ons to allow controlling bots through the in-game UI. We recommend you check out their projects:

- [Multibot](https://github.com/Macx-Lio/MultiBot) (by Macx-Lio)
- [Unbot Addon (zh)](https://github.com/liyunfan1223/unbot-addon) (Chinese version by Liyunfan)
- [Unbot Addon (en)](https://github.com/noisiver/unbot-addon/tree/english) (English version translated by @Revision)

## Acknowledgements

`mod-playerbots` is is based off [ZhengPeiRu21/mod-playerbots](https://github.com/ZhengPeiRu21/mod-playerbots) and [celguar/mangosbot-bots](https://github.com/celguar/mangosbot-bots). We extend our gratitude to [@ZhengPeiRu21](https://github.com/ZhengPeiRu21) and [@celguar](https://github.com/celguar) for the continued efforts in maintaining the module.

Also, a thank you to the many contributors who've helped build this project:

<a href="https://github.com/liyunfan1223/mod-playerbots/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=liyunfan1223/mod-playerbots" />
</a>
