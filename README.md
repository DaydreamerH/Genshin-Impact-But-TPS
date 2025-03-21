# GenshinImpact: ModernWarfare

你说的对，但是 GenshinImpact: ModernWarfare 是一款基于 Unreal Engine 开发的多人在线对战游戏。

# 所有素材均来自互联网，仅用于学习交流，不以任何方式盈利或商业化使用，在没有作者允许的情况下不能进行二次开发。

## 1. 游戏简介

GenshinImpact: ModernWarfare 以团队对抗为核心玩法，结合夺旗玩法，鼓励玩家在战斗中制定灵活策略。主要规则包括：

- **击杀对手**：每成功击杀一名敌方玩家获得 1 分。
- **炸弹传递**：将炸弹送达指定区域获得 20 分；若炸弹被移出指定区域，则扣除 20 分。
- **胜负判定**：倒计时结束时，累计得分较高的队伍获胜。

更多的游戏介绍参考：[项目介绍博客](https://daydreamerh.github.io/2025/03/21/Genshin-Impact-But-TPS%E9%A1%B9%E7%9B%AE/)  

## 2. 项目核心内容

- 基于 OnlineSubsystem 实现了 Steam 平台的联机功能。
- 完成了射弹类枪械（Projectile-based）和射线类枪械（Hitscan-based）的实现。
- 持枪时较为跟手的动画效果。
- 通过 Server-Rewind 技术，优化了处于较高延迟下的玩家射击体验。

## 3. 系统需求

- **操作系统**：Windows 10 或以上版本
- **开发引擎**：Unreal Engine 5.4.4
  - 使用插件
    - OnlineSubsystem
    - VRM4U [https://github.com/ruyo/VRM4U](https://github.com/ruyo/VRM4U)  
- **其他依赖**：Steam 客户端

## 4. 安装与运行

### 4.1 克隆仓库

```bash
git clone https://github.com/DaydreamerH/Genshin-Impact-But-TPS
```
### 4.2 环境配置

- 使用UE5.4或更高版本。
- 本项目基于Steam平台进行联机，需要安装Steam。

### 4.3 构建项目

- 使用 Unreal Engine 打开项目文件。
- 按照 Unreal Engine 官方文档进行项目构建与编译，使用 Development Configuration 进行打包，
需注意本项目使用了测试用的 SteamDevAppId=480 ，不能基于 Shipping Configuration 打包。

## 5. 部分素材来源
- 人物模型：可莉/模之屋。
- 炸弹模型：可莉的炸弹/模之屋。
- 人物动作动画：Mixmao 或 虚幻引擎入门素材 重定向后得到。

如有疑问、侵权或建议，请及时联系！
