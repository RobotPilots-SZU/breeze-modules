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
	ramen::Pusher<models::RobotId> Id;
	ramen::Pusher<messages::ModuleInfos> Modules;
	ramen::Pusher<messages::ChassisReport> ChassisReport;
	ramen::Pusher<messages::LegReport> LegReport;
	ramen::Pusher<models::MotionMode> MotionMode;
	ramen::Pusher<models::RobotInfos> AllyRobotInfos;
	ramen::Pusher<models::RobotInfos> EnemyRobotInfos;
};

class Module
{
public:
	Input In;
	Output Out;

	/// @brief 原始数据源. 注意需要在线程上下文.
	ramen::Pushable<std::span<const std::uint8_t>> InRaw;
	/// @brief 原始(编码后)数据输出. 注意不要阻塞.
	ramen::Pusher<std::span<const std::uint8_t>> OutRaw;

	Module();
	~Module();

public:
	Module(const Module&) = delete;
	Module(Module&&) = delete;
	Module& operator=(const Module&) = delete;
	Module& operator=(Module&&) = delete;

private:
	static constexpr auto IMPL_SIZE = 16384;
	struct Impl;
	static ramen::Pushable<std::span<const std::uint8_t>> MakeRawInput(Module& owner);
	alignas(std::max_align_t) std::byte _storage[IMPL_SIZE];
};
} // namespace breeze::communicator
