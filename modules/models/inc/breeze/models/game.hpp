/**
 * @file game.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 比赛相关信息 数据模型
 * @version 0.1
 * @date 2026-10-25
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "id.hpp"
#include "unit.hpp"
#include "utils.hpp"
#include <array>
#include <cstdint>

namespace breeze::models
{

/**
 * @brief 比赛类型(比赛模式)
 *
 */
enum class GameMode : std::uint8_t
{
	None,
	UC, ///< 超级对抗赛
	UL, ///< 联盟塞
};

/**
 * @brief 比赛阶段
 *
 */
enum class GameStage : std::uint8_t
{
	None,
	NotStarted,  ///< 未开始
	Preparation, ///< 准备阶段 (你最喜欢的3分钟准备)
	RefereeInit, ///< 裁判系统自检
	Countdown,   ///< 比赛开始前的5秒倒计时
	InMatch,     ///< 比赛中
	Settleing    ///< 结算中 (比赛结束)
};

/**
 * @brief 判罚等级
 *
 */
enum class PenaltyLevel : std::uint8_t
{
	DualYellowCard, ///< 双方黄牌
	YellowCard,     ///< 黄牌
	RedCard,        ///< 红牌
	Forfeiture      ///< 判负
};

/**
 * @brief 血量数据类型 (HP)
 *
 */
struct Health
{
	std::uint16_t Value;
};
/**
 * @brief 热量数据类型
 *
 */
struct Heat
{
	std::uint16_t Value;
};

/**
 * @brief 发弹量
 *
 */
struct AmmoCount
{
	std::uint16_t Value;
};

/**
 * @brief 弹药种类
 *
 */
enum AmmoKind : std::uint8_t
{
	Small, ///< 小弹丸(14mm)
	Large, ///< 大弹丸(42mm)
};

/**
 * @brief 弹速 (m/s)
 * @note TODO: 后续改用物理model单位
 *
 */
struct BulletSpeed : Meter<float>
{};

/**
 * @brief 机器人等级
 */
struct Level
{
	std::uint8_t Value;
};

/**
 * @brief 机器人位姿信息 (2D)
 * @note 场地围挡在红方补给站附近的交点为坐标原点, 沿场地长边向蓝方为 X 轴正方向, 沿场地短边
向红方停机坪为 Y 轴正方向.
 *
 */
struct Pose
{
	Vec2<float> Pos;           ///< 平面坐标
	Degree<float> Orientation; ///< 朝向. 正北(地图y+)为0度, 正向为顺时针, 范围 0~360
};

/**
 * @brief 增益/减益
 * @note 这个不是符(能量机关).
 *
 */
struct Buffs
{
	Percentage Recovery; ///< 每秒回复血量上限百分比
	Heat Cooling;        ///< 每秒热量冷却的量
	Percentage Defence;  ///< 防御增益/减益百分比. 正为增益, 负为减益.
	Percentage Attack;   ///< 攻击增益/减益百分比. 正为增益, 负为减益.
};

enum class MajorFocusType : std::uint8_t
{
	Unspecified,
	Melee,
	LongRange,
};

constexpr auto MAJOR_FOCUS_TYPE_VALUES = [] {
	using enum MajorFocusType;
	return std::array{Unspecified, Melee, LongRange};
}();

enum class ChassisFocusType : std::uint8_t
{
	Unspecified,
	Power,
	Health,
};

constexpr auto CHASSIS_FOCUS_TYPE_VALUES = [] {
	using enum ChassisFocusType;
	return std::array{Unspecified, Power, Health};
}();

enum class LaunchingMechanismFocusType : std::uint8_t
{
	Unspecified,
	Burst,
	Cooling,
};

constexpr auto LAUNCHING_MECHANISM_FOCUS_TYPE_VALUES = [] {
	using enum LaunchingMechanismFocusType;
	return std::array{Unspecified, Burst, Cooling};
}();

/**
 * @brief 伤害源(血量变化类型)
 *
 */
enum class DamageSource : std::uint8_t
{
	Projectile,    ///< 装加班被弹丸击打
	ModuleOffline, ///< 模块离线
	Collision,     ///< 装加班被肘击
	OverSpeed,     ///< 超弹速
	OverHeat,      ///< 超热量
	PowerOverload  ///< 超功率
};

/**
 * @brief 经济(金币)单位
 *
 */
struct EconomyPoint
{
	std::uint16_t Value;
};

/**
 * @brief 能量机关激活状态
 * @note 冷知识, "Rune"取自RM英文手册"PowerRune", 即能量机关, 别尬黑我造词
 */
enum class RuneState : std::uint8_t
{
	Inactive,   ///< 未激活
	Activated,  ///< 已激活
	Activating, ///< 激活中
};

/**
 * @brief 场地占领状态
 */
enum class FieldOccupation : std::uint8_t
{
	None,  ///< 无占领
	Ally,  ///< 我方占领
	Enemy, ///< 敌方占领
	Both,  ///< 双方占领
};
/**
 * @brief 场地类型
 * @note 这会随赛季变更, 记得维护
 *
 */
enum class FieldKind : std::uint8_t
{
	Base,                  ///< 基地增益点
	CentralHighland,       ///< 中央高地增益点
	TrapezoidHighland,     ///< 梯形高地增益点
	LaunchRampBefore,      ///< 飞坡前地形跨越增益点
	LaunchRampAfter,       ///< 飞坡后地形跨越增益点
	CentralHighlandBelow,  ///< 中央高地下方地形跨越增益点
	CentralHighlandAbove,  ///< 中央高地上方地形跨越增益点
	RoadBelow,             ///< 公路下方地形跨越增益点
	RoadAbove,             ///< 公路上方地形跨越增益点
	Fortress,              ///< 堡垒增益点
	Outpost,               ///< 前哨站增益点
	Resupply,              ///< 与资源区不重叠的补给区/RMUL 补给区
	ResourceResupply,      ///< 与资源区重叠的补给区
	Assembly,              ///< 装配增益点
	Center,                ///< 中心增益点（仅 RMUL 适用）
	TunnelRoadBelow,       ///< 公路区下方隧道地形跨越增益点
	TunnelRoadMiddle,      ///< 公路区中间隧道地形跨越增益点
	TunnelRoadAbove,       ///< 公路区上方隧道地形跨越增益点
	TunnelTrapezoidBelow,  ///< 梯形高地较低处隧道地形跨越增益点
	TunnelTrapezoidMiddle, ///< 梯形高地中间隧道地形跨越增益点
	TunnelTrapezoidAbove,  ///< 梯形高地较高处隧道地形跨越增益点
};

/**
 * @brief 飞镖目标
 */
enum class DartTarget : std::uint8_t
{
	None,             ///< 无目标
	Outpost,          ///< 前哨站
	BaseFixed,        ///< 基地固定
	BaseRandomFixed,  ///< 基地随机固定
	BaseRandomMoving, ///< 基地随机移动
	BaseEndMoving,    ///< 基地末端移动
};

/**
 * @brief 烧饼姿态
 *
 */
enum class SentryStance : std::uint8_t
{
	Offensive, ///< 进攻姿态
	Defensive, ///< 防御姿态
	Mobile     ///< 移动姿态
};
/**
 * @brief 烧饼姿态枚举集合
 *
 */
constexpr auto SENTRY_STANCES = []() {
	using enum SentryStance;
	return std::array{Offensive, Defensive, Mobile};
}();

/**
 * @brief 有血量的机器人索引表
 *
 */
constexpr auto HEALTH_ROBOT_IDS = []() constexpr {
	using enum RobotShortId;
	return std::array{Hero1, Juggernaut1, Engineer2, Infantry3, Infantry4, Sentry6};
}();

/**
 * @brief 机器人的血量集合
 *
 */
struct RobotHealths : IndexedEnumArray<RobotShortId, Health, HEALTH_ROBOT_IDS, ROBOT_SHORT_ID_VALUES>
{};

/**
 * @brief 有发单量的机器人索引表
 *
 */
constexpr auto AMMO_ROBOT_IDS = []() constexpr {
	using enum RobotShortId;
	return std::array{Hero1, Juggernaut1, Infantry3, Infantry4, Sentry6, Drone7};
}();

/**
 * @brief 机器人的发单量集合
 *
 */
struct RobotAmmos : IndexedEnumArray<RobotShortId, AmmoCount, AMMO_ROBOT_IDS, ROBOT_SHORT_ID_VALUES>
{};

constexpr auto LEVEL_ROBOT_IDS = []() constexpr {
	using enum RobotShortId;
	return std::array{Hero1, Juggernaut1, Engineer2, Infantry3, Infantry4, Sentry6, Drone7};
}();
/**
 * @brief 机器人的等级集合
 *
 */
struct RobotLevels : IndexedEnumArray<RobotShortId, Level, LEVEL_ROBOT_IDS, ROBOT_SHORT_ID_VALUES>
{};

/**
 * @brief 有热量的机器人索引表
 *
 */
constexpr auto HEAT_ROBOT_IDS = AMMO_ROBOT_IDS;

/**
 * @brief 机器人的热量集合
 *
 */
struct RobotHeats : IndexedEnumArray<RobotShortId, Heat, HEAT_ROBOT_IDS, ROBOT_SHORT_ID_VALUES>
{};

/**
 * @brief 有弹速的机器人索引表
 *
 */
constexpr auto BULLET_SPEED_ROBOT_IDS = AMMO_ROBOT_IDS;
/**
 * @brief 机器人弹速集合
 *
 */
struct RobotBulletSpeeds : IndexedEnumArray<RobotShortId, BulletSpeed, BULLET_SPEED_ROBOT_IDS, ROBOT_SHORT_ID_VALUES>
{};

/**
 * @brief 地面机器人集合
 *
 */
constexpr auto GROUND_ROBOT_IDS = []() {
	using enum RobotShortId;
	return std::array{Hero1, Juggernaut1, Engineer2, Infantry3, Infantry4, Sentry6};
}();
/**
 * @brief 有功率限制的机器人集合
 *
 */
constexpr auto POWER_ROBOT_IDS = GROUND_ROBOT_IDS;
/**
 * @brief 机器人功率集合
 *
 */
struct RobotPowers : IndexedEnumArray<RobotShortId, Watt<std::uint8_t>, POWER_ROBOT_IDS, ROBOT_SHORT_ID_VALUES>
{};

/**
 * @brief 有坐标的机器人索引表
 * @note 目前假设是所有机器人, 后续会改成根据比赛中实际存在的兵种调整
 *
 */
constexpr auto POSE_ROBOT_IDS = ROBOT_SHORT_ID_VALUES;

struct RobotPoses : IndexedEnumArray<RobotShortId, Pose, POSE_ROBOT_IDS, ROBOT_SHORT_ID_VALUES>
{};

/**
 * @brief 有Buff的机器人索引表
 * @note 目前假设是所有机器人, 后续会改成根据比赛中实际存在的兵种调整
 *
 */
constexpr auto BUFF_ROBOT_IDS = ROBOT_SHORT_ID_VALUES;

/**
 * @brief 机器人Buff集合
 *
 */
struct RobotBuffs : IndexedEnumArray<RobotShortId, Buffs, BUFF_ROBOT_IDS, ROBOT_SHORT_ID_VALUES>
{};

/** @brief 机器人整车性能体系集合. */
struct RobotMajorFocuses : IndexedEnumArray<RobotShortId, MajorFocusType, ROBOT_SHORT_ID_VALUES>
{};

/** @brief 机器人底盘性能体系集合. */
struct RobotChassisFocuses : IndexedEnumArray<RobotShortId, ChassisFocusType, ROBOT_SHORT_ID_VALUES>
{};

/** @brief 机器人发射机构性能体系集合. */
struct RobotLaunchingMechanismFocuses
    : IndexedEnumArray<RobotShortId, LaunchingMechanismFocusType, ROBOT_SHORT_ID_VALUES>
{};

/**
 * @brief 单侧机器人的比赛数据, 按领域组件分别索引机器人.
 */
struct RobotInfos
{
	RobotHealths Healths;
	RobotAmmos Ammos;
	RobotHeats Heats;
	RobotHeats CoolingRates;
	RobotBuffs Buffs;
	RobotMajorFocuses MajorFocuses;
	RobotChassisFocuses ChassisFocuses;
	RobotLaunchingMechanismFocuses LaunchingMechanismFocuses;
};

/**
 * @brief 机器人脱战状态集合 (true表示脱战)
 *
 */
struct RobotOutofCombats : IndexedEnumArray<RobotShortId, bool, BUFF_ROBOT_IDS, ROBOT_SHORT_ID_VALUES>
{};

/**
 * @brief 烧饼姿态持续时间集合
 *
 */
struct SentryStanceDurations : IndexedEnumArray<SentryStance, Seconds<std::uint8_t>, SENTRY_STANCES, SENTRY_STANCES>
{};

} // namespace breeze::models
