/**
 * @file game.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 比赛相关的消息类型. 通常来自裁判系统模块, rp-proto模块.
 * @note 要求: 编写类型时应该无关红蓝/敌我方, 具体是哪一边应该由channel的语义决定.
 * @version 0.1
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <breeze/models/game.hpp>
#include <breeze/models/unit.hpp>
#include <cstdint>

namespace breeze::models::messages
{

// == 全局比赛信息
#pragma region Game Globals

/**
 * @brief 全局比赛状态信息
 *
 */
struct GameState
{
	GameMode Mode;                 ///< 比赛类型
	GameStage Stage;               ///< 比赛阶段
	Seconds<std::uint16_t> Remain; ///< 当前阶段剩余时间
	bool operator==(const GameState&) const = default;
};

#pragma endregion

// == 机器人单位信息
#pragma region Unit Status

/**
 * @brief 机器人血量消息
 * @note 当前血量, 最大血量使用
 *
 */
using models::RobotHealths;
/**
 * @brief 机器人等级消息
 * @note 当前等级, 最大等级使用
 *
 */
using models::RobotLevels;
/**
 * @brief 机器人热量消息
 * @note 当前热量, 冷却速率使用
 *
 */
using models::RobotHeats;
/**
 * @brief 机器人功率消息
 * @note 当前功率, 最大功率使用
 *
 */
using models::RobotPowers;
/**
 * @brief 机器人弹量消息
 *
 */
using models::RobotAmmos;
/**
 * @brief 机器人弹速消息
 * @note 当前弹速, 最大弹速
 *
 */
using models::RobotBulletSpeeds;

/**
 * @brief 机器人位置&姿态消息
 *
 */
using models::RobotPoses;

/**
 * @brief 机器人 Buff 消息
 *
 */
using models::RobotBuffs;

#pragma endregion

// === 比赛相关杂项
#pragma region Misc

/**
 * @brief 后勤信息 (队伍的经济, 加密等级等等队伍级别属性)
 *
 */
struct LogisticsInfo
{
	EconomyPoint Economy; ///< 金币
	Level CryptionLevel;  ///< 加密等级(对方干扰波难度等级)
	bool operator==(const LogisticsInfo&) const = default;
};

/**
 * @brief 飞镖发射相关数据
 *
 */
struct DartInfo
{
	Seconds<std::uint8_t> RemainingTime; ///< 飞镖发射剩余时间
	DartTarget TargetSelected;           ///< 当前选中目标
	std::uint8_t HitCount;               ///< 击中总次数
	bool operator==(const DartInfo&) const = default;
};

/**
 * @brief 雷达相关数据
 *
 */
struct LidarInfo
{
	std::uint8_t HyperVulnerabilityLeft; ///< 剩余发动双倍易伤的次数
	bool HyperVulnerabilityEffective;    ///< 双倍易伤生效中 (对方是否被易伤)
	bool KeyModifiable;                  ///< 是否可以修改密钥
	bool operator==(const LidarInfo&) const = default;
};

// TODO: 烧饼相关Model在后续有需求时再写. 大概率是直接走bool类型触发式通知而非记录数量.

/**
 * @brief 区域检测状态 (RFID)
 *
 */
#pragma pack(push, 1)
struct RegionDectectionInfo
{
	bool Resupply,         ///< 补给区
	    CenterHighland,    ///< 中高
	    TrapezoidHighland, ///< 梯高
	    Fortress,          ///< 堡垒增益点
	    Outpost,           ///< 前哨增益点
	    Base,              ///< 基地增益点
	    Center;            ///< 中心增益点(RMUL)
	bool operator==(const RegionDectectionInfo&) const = default;
};
#pragma pack(pop)

#pragma endregion

// == 比赛事件, 通常是触发式发送
#pragma region Game Events
/**
 * @brief 区域占领事件.
 *
 */
struct OccupationEvent
{
	FieldOccupation Resupply, ///< 补给区
	    CenterHighland,       ///< 中高
	    TrapezoidHighland,    ///< 梯高
	    Fortress,             ///< 堡垒增益点
	    Outpost,              ///< 前哨增益点
	    Base,                 ///< 基地增益点
	    Center;               ///< 中心增益点(RMUL)
	bool operator==(const OccupationEvent&) const = default;
};

/**
 * @brief 能量机关(符)事件
 *
 */
struct RuneEvent
{
	RuneState SmallState; ///< 小符状态
	RuneState LargeState; ///< 大符状态
	bool operator==(const RuneEvent&) const = default;
};

/**
 * @brief 飞镖击中事件
 *
 */
struct DartHitEvent
{
	Seconds<std::uint16_t> LastTime; ///< 上一次击中时间(比赛时间)
	DartTarget LastTarget;           ///< 上一次击中目标

	bool operator==(const DartHitEvent& other) const = default;
};

/**
 * @brief 受伤事件
 * @note TODO: 将装甲模块/测速模块ID用枚举类型化
 *
 */
struct DamageEvent
{
	std::uint8_t ArmorId; ///< 装甲板/测速模块ID
	DamageSource Source;  ///< 伤害源(伤害类型)
	bool operator==(const DamageEvent&) const = default;
};

/**
 * @brief 射击事件. 弹丸发射后发送.
 *
 */
struct ShootEvent
{
	Hertz<std::uint8_t> Freq; ///< 弹速 (单位赫兹)
	BulletSpeed InitialSpeed; ///< 射击初速度 (m/s)
	bool operator==(const ShootEvent&) const = default;
};

#pragma endregion

} // namespace breeze::models::messages
