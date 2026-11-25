/**
 * @file id.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 机器人id相关的数据模型与映射
 * @version 0.1
 * @date 2026-10-25
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <array>
#include <cstdint>

namespace breeze::models
{

/**
 * @brief 机器人类型
 *
 */
enum class RobotType : std::uint8_t
{
	Unknown,    ///< 无效值
	Hero,       ///< 英雄
	Engineer,   ///< 工程
	Juggernaut, ///< 重装
	Infantry,   ///< 步兵
	Sentry,     ///< 哨兵
	Drone,      ///< 无人机
};

/**
 * @brief 机器人短id, 为机器人的具体编号表示.
 * @note 设计这个东西看起来虽然暴力, 但确实能解决magic number硬编码编号的问题, 而且提供一定的强类型检查.
 *
 */
enum class RobotShortId : std::uint8_t
{
	Unknown0,    ///< 无效值
	Hero1,       ///< 1号英雄
	Engineer2,   ///< 2号工程
	Juggernaut1, ///< 1号重装
	Infantry3,   ///< 3号步兵
	Infantry4,   ///< 4号步兵
	Infantry5,   ///< 5号步兵
	Sentry6,     ///< 6号哨兵
	Drone7,      ///< 7号无人机
};

/**
 * @brief 机器人短Id所蕴含的信息
 * @note 这是一个辅助结构体, 用来简化信息表示
 *
 */
struct RobotShortIdInfo
{
	RobotType Type;
	std::uint8_t Digit;
	bool operator==(const RobotShortIdInfo&) const = default;
};

/**
 * @brief 阵营信息(红蓝方)
 *
 */
enum class TeamKind : std::uint8_t
{
	Neutral, ///< 中立
	Red,     ///< 红方
	Blue     ///< 蓝方
};

struct RobotId
{
	RobotShortId ShortId;
	TeamKind Team;
	bool operator==(const RobotId&) const = default;
};

/**
 * @brief 机器人短id的全部元信息
 *
 */
constexpr RobotShortIdInfo ToInfo(const RobotShortId sid)
{
	switch (sid)
	{
	case RobotShortId::Hero1:
		return {RobotType::Hero, 1};

	case RobotShortId::Engineer2:
		return {RobotType::Engineer, 2};

	case RobotShortId::Juggernaut1:
		return {RobotType::Juggernaut, 1};

	case RobotShortId::Infantry3:
		return {RobotType::Infantry, 3};

	case RobotShortId::Infantry4:
		return {RobotType::Infantry, 4};

	case RobotShortId::Infantry5:
		return {RobotType::Infantry, 5};

	case RobotShortId::Drone7:
		return {RobotType::Drone, 7};

	case RobotShortId::Sentry6:
		return {RobotType::Sentry, 6};

	default:
		return {RobotType::Unknown, 0};
	}
}

/**
 * @brief 从机器人短id推断计算机器人类型
 *
 * @param sid
 * @version 26.2.0
 * @return RobotType
 */
constexpr RobotType ToType(const RobotShortId sid)
{
	const auto info = ToInfo(sid);
	return info.Type;
}

/**
 * @brief 从机器人完整id推断机器人类型
 *
 * @param id
 * @return constexpr RobotType
 */
constexpr RobotType ToType(const RobotId id) { return ToType(id.ShortId); }

/**
 * @brief 从机器人短id推断机器人数字
 *
 * @param sid
 * @return constexpr uint8_t
 */
constexpr std::uint8_t ToDigit(const RobotShortId sid)
{
	const auto info = ToInfo(sid);
	return info.Digit;
}

/**
 * @brief 从机器人完整id推断3位数编号
 *
 * @param id
 * @return constexpr std::uint8_t
 */
constexpr std::uint8_t ToNum(const RobotId id)
{
	constexpr std::uint8_t TEAM_MOD = 100;
	std::uint8_t teamDigit = TeamKind() == TeamKind::Blue ? TEAM_MOD : 0;
	return ToDigit(id.ShortId) + teamDigit;
}

/**
 * @brief RobotType的元数据
 * @warning 务必与 @ref RobotType 保持同步!
 *
 */
constexpr auto ROBOT_TYPE_VALUES = []() {
	using enum RobotType;
	return std::array{Unknown, Hero, Engineer, Juggernaut, Infantry, Sentry, Drone};
}();

/**
 * @brief RobotShortId的元数据
 * @warning 务必与 @ref RobotShortId 保持同步!
 *
 */
constexpr auto ROBOT_SHORT_ID_VALUES = []() {
	using enum RobotShortId;
	return std::array{
	    Unknown0, Hero1, Engineer2, Juggernaut1, Infantry3, Infantry4, Infantry5, Sentry6, Drone7,
	};
}();

} // namespace breeze::models
