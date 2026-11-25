/**
 * @file system.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 系统消息, 无关比赛与应用
 * @version 0.1
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "breeze/models/unit.hpp"
namespace breeze::models::messages
{

/**
 * @brief 时间戳同步消息
 *
 */
struct TimeStampSync : TimeStamp
{
	bool operator==(const TimeStampSync&) const = default;
};

/**
 * @brief 电源管理消息
 * @note TODO: 这一块暂时没想好放哪个层次, 暂时放在system
 *
 */
struct PowerManagementInfo
{
	bool Gimbal;  ///< 云台是否供电
	bool Chassis; ///< 底盘是否供电
	bool Shooter; ///< 发射机构是否供电
	bool operator==(const PowerManagementInfo&) const = default;
};

} // namespace breeze::models::messages
