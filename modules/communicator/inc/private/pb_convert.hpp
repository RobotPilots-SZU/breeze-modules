#pragma once

#include <array>
#include <breeze/communicator/module.hpp>
#include <cstdint>
#include <ramen.hpp>
#include <robotpilots/proto/toplevel/universal_message.pb.h>
#include <span>
#include <optional>

namespace breeze::communicator::internal
{
struct InputState
{
	std::optional<models::RobotId> Id;
	std::optional<messages::ModuleInfos> Modules;
	std::optional<messages::ChassisReport> ChassisReport;
	std::optional<messages::LegReport> LegReport;
	std::optional<models::MotionMode> MotionMode;
	std::optional<models::RobotInfos> AllyRobotInfos;
	std::optional<models::RobotInfos> EnemyRobotInfos;
};

struct PbDecoder
{
	/**
	 * @brief pb数据输入
	 *
	 */
	ramen::Pushable<std::span<const std::uint8_t>> In = [this](const auto& data) { Accept(data); };
	/**
	 * @brief model数据输出
	 *
	 */
	Output& Out;
	explicit PbDecoder(Output& out)
	    : Out(out)
	{}

	void Accept(std::span<const std::uint8_t> data);
};

struct PbEncoder
{
	/**
	 * @brief model数据输入
	 *
	 */
	InputState& In;
	/**
	 * @brief pb编码数据输出
	 *
	 */
	ramen::Pusher<std::span<const std::uint8_t>> Out;
	ramen::Pushable<> Trigger = [this] { Emit(); };
	std::array<std::uint8_t, 2048> Buffer{};
	explicit PbEncoder(InputState& in)
	    : In(in)
	{}
	void Emit();
};
} // namespace breeze::communicator::internal
