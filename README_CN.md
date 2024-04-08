 [English](README.md) | [Español](README_ES.md) | [中文](README_CN.md)

# 玩家机器人模块

欢迎使用AzerothCore的玩家机器人模块，这是一个基于IKE3玩家机器人的正在进行中的项目。这些玩家机器人利用实际的玩家数据，使您能够与您自己的替身进行交互，组建队伍，升级角色等等。

如果您遇到任何错误或出现崩溃，请您将它们报告为GitHub问题。您宝贵的反馈将帮助我们协作改进和增强这个项目。

## 安装

请注意，此模块需要对AzerothCore进行特定的自定义更改。为了确保兼容性，您必须使用我fork的自定义分支来编译它，可以在这里找到：[liyunfan1223/azerothcore-wotlk/tree/Playerbot](https://github.com/liyunfan1223/azerothcore-wotlk/tree/Playerbot)。

要安装此模块，请参考AzerothCore Wiki的详细说明：[AzerothCore安装指南](https://www.azerothcore.org/wiki/installation)。

我们提供了一个简单的方法来克隆该模块：

```bash
git clone https://github.com/liyunfan1223/azerothcore-wotlk.git --branch=Playerbot
cd azerothcore-wotlk/modules
git clone https://github.com/liyunfan1223/mod-playerbots.git --branch=master
```

## 快速开始与文档

要快速开始并了解一系列命令，您可以参考ike3原版playerbots的手册。该模块提供了大部分基本命令。您可以在此找到文档：[IKE3 Playerbots 文档](https://ike3.github.io/mangosbot-docs/)。请注意，在我们的模块中，您需要将文档中所有的 `.bot` 替换为 `.playerbot bot`。

请注意，由于项目仍在开发中，新添加的命令的文档目前尚不完善。

## 进展

该模块主要强调以下关键功能，并在这些领域实施了改进：

- **世界中的机器人（随机机器人）：** 我们增强了随机机器人的行为，使它们更接近真实玩家的表现，从而创建了更真实的玩家服务器环境。

- **团队副本中的机器人：** 我们赋予机器人征服具有挑战性的团队副本内容的能力，通过为各种Boss实施特定策略，使团队副本更加吸引人。此外，我们增强了机器人在DPS、治疗和坦克等各种角色中的能力，确保它们有效地为团队的成功做出贡献。

- **战场中的机器人：** 机器人现在能够与真实玩家一起积极参与战场，为这些PvP场景增添了深度和刺激。

- **与机器人的交互：** 我们改进了真实玩家和机器人之间的交互，使玩家能够在与机器人伙伴合作时完成任务并升级多个角色。

- **玩家进阶路径：** 我们设计了一个改进的玩家进阶路径，辅以机器人，为玩家提供了一种替代且引人入胜的游戏体验。

- **稳定性：** 我们的努力主要集中在增强使用Playerbots模块时AzerothCore的整体稳定性。这些改进旨在防止服务器崩溃，并确保所有用户都能获得更流畅的体验。

- **配置：** 我们引入了一系列可配置的选项，以满足不同需求的玩家，从而提供更个性化的体验。

值得注意的是，随着我们继续改进项目，还有大量工作需要完成。我们欢迎每个人以不同的方式做出贡献。

## 插件

为了更好地控制机器人并简化命令的使用，您还可以使用我们的插件：[Unbot Addon](https://github.com/liyunfan1223/unbot-addon)。目前，该插件仅对简体中文客户端提供更好的支持。

## 常见问题

**机器人无法释放技能**

- 请确保必要的英文DBC文件（enUS）存在。

**编译错误**

- 我们支持Ubuntu、Windows和macOS。

- 我们建立了持续集成工作流。您可以在[GitHub Actions](https://github.com/liyunfan1223/mod-playerbots/actions)中查看构建状态。

- 如果最新的构建状态失败，请恢复到上一个提交。我们将尽快解决此问题。

## 致谢

该模块的代码来自[ZhengPeiRu21/mod-playerbots](https://github.com/ZhengPeiRu21/mod-playerbots)和[celguar/mangosbot-bots](https://github.com/celguar/mangosbot-bots)。我们衷心感谢@ZhengPeiRu21和@celguar对维护该模块的持续努力。

我们还要向所有为playerbot开发做出贡献的个人表示诚挚的感谢。您的奉献和努力对塑造这个项目至关重要，我们对您的贡献表示感谢。
