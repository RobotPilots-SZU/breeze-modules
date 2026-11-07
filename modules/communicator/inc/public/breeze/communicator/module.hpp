/**
 * @file module.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 通信模块顶层封装
 * @version 0.1
 * @date 2026-11-04
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "messages.hpp"
#include <breeze/models/control.hpp>
#include <breeze/models/game.hpp>
#include <breeze/models/id.hpp>
#include <cstdint>
#include <ramen.hpp>
#include <span>

namespace breeze::communicator
{
/**
 * @brief Model类型的解析后数据.
 *
 */
struct Output
{
	ramen::Pusher<messages::DeployCommand> DeployCommand;
	ramen::Pusher<models::MotionMode> MotionMode;
};
/**
 * @brief Model类型的编码前数据.
 *
 */
struct Input
{
	ramen::Pushable<models::RobotId> Id;
	ramen::Pushable<messages::ModuleInfos> Modules;
	ramen::Pushable<messages::ChassisReport> ChassisReport;
	ramen::Pushable<messages::LegReport> LegReport;
	ramen::Pushable<models::MotionMode> MotionMode;
	ramen::Pushable<models::RobotInfos> AllyRobotInfos;
	ramen::Pushable<models::RobotInfos> EnemyRobotInfos;
};

class Module
{
public:
	Input In;
	Output Out;

	/// @brief 原始数据源. 注意需要在线程上下文.
	ramen::Pushable<std::span<std::uint8_t>> InRaw;
	/// @brief 原始(编码后)数据输出. 注意不要阻塞.
	ramen::Pusher<std::span<std::uint8_t>> OutRaw;

	Module();

public:
	Module(const Module&) = delete;
	Module(Module&&) = delete;
	Module& operator=(const Module&) = delete;
	Module& operator=(Module&&) = delete;

private:
	static constexpr auto IMPL_SIZE = 256;
	struct Impl;
	alignas(std::max_align_t) std::byte _storage[IMPL_SIZE];
};
} // namespace breeze::communicator
