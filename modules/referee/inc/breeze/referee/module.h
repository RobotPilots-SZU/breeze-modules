/**
 * @file module.h
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 裁判系统数据流模块, 将原始字节流转换为裁判业务模型
 * @version 0.1
 * @date 2026-10-18
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <breeze/messages/game.hpp>
#include <breeze/messages/system.hpp>
#include <breeze/models/id.hpp>
#include <cstddef>
#include <etl/span.h>
#include <ramen.hpp>
#include <ramenx/logic.hpp>
#include <zephyr/autoconf.h>

namespace breeze::referee
{

struct Outputs
{
	ramen::Pullable<models::RobotId> SelfId;
	ramen::Pusher<models::messages::PowerManagementInfo> PowerManagementEvent{};
	ramen::Pusher<models::messages::RegionDectectionInfo> RegionEvent{};
	ramen::Pullable<models::messages::RobotPowers> AllyPowers;
	ramen::Pullable<models::messages::GameState> GameState;
	ramen::Pullable<models::messages::LogisticsInfo> LogisticsInfo;
	ramen::Pusher<models::messages::DamageEvent> DamageEvent{};
	ramen::Pusher<models::messages::OccupationEvent> OccupationEvent{};
	ramen::Pusher<models::messages::RuneEvent> RuneEvent{};
	ramen::Property<models::messages::DartHitEvent> DartHitEvent;
	ramen::Pullable<models::messages::DartInfo> DartInfo;
	ramen::Pullable<models::messages::RobotHealths> AllyHealths;
	ramen::Pullable<models::messages::RobotLevels> AllyLevels;
	ramen::Pullable<models::messages::RobotHeats> AllyHeats;
	ramen::Pullable<models::messages::RobotAmmos> AllyAmmos;
	ramen::Pullable<models::messages::RobotBuffs> AllyBuffs;
	ramen::Pullable<models::messages::RobotPoses> AllyPoses;
};

/**
 * @brief 裁判系统模块, 提供RAMEN输出端口与原始输入端口.
 * @par 输出接口设计原则
 * - 低频数据使用触发式 (Pusher), 在调用输入端口的上下文中处理
 * - 高频数据使用拉取式 (Pullable)
 */
struct Module
{
	Outputs Out;
	ramen::Pushable<etl::span<const uint8_t>> InReferee;
	ramen::Pushable<etl::span<const uint8_t>> InVT;

	Module();
	~Module();
	Module(const Module&) = delete;
	Module(Module&&) = delete;
	Module& operator=(const Module&) = delete;
	Module& operator=(Module&&) = delete;

private:
	struct Impl;
	static constexpr std::size_t IMPL_SIZE =
	    512 + 2 * (CONFIG_BREEZE_MODULE_REFEREE_RING_BUF_SIZE + CONFIG_BREEZE_MODULE_REFEREE_LINEAR_BUF_SIZE);
	alignas(std::max_align_t) std::byte _storage[IMPL_SIZE];
	Impl* _impl;
	Outputs MakeOutputs();
};

} // namespace breeze::referee
