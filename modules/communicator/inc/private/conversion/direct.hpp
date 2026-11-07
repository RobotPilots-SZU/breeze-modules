#pragma once

#include "breeze/communicator/messages.hpp"
#include <breeze/models/control.hpp>
#include <breeze/models/game.hpp>
#include <breeze/models/id.hpp>
#include <cstdint>
#include <optional>
#include <robotpilots/proto/common/id.pb.h>
#include <robotpilots/proto/common/module.pb.h>
#include <robotpilots/proto/control/command.pb.h>
#include <robotpilots/proto/game/robot.pb.h>
#include <robotpilots/proto/module/chassis.pb.h>
#include <robotpilots/proto/module/leg.pb.h>
#include <robotpilots/proto/module/motion.pb.h>

namespace breeze::communicator::internal::direct
{
inline models::RobotShortId ToShortId(robotpilots_proto_common_RobotType type)
{
	switch (type)
	{
	case robotpilots_proto_common_RobotType_ROBOT_TYPE_1:
		return models::RobotShortId::Hero1;
	case robotpilots_proto_common_RobotType_ROBOT_TYPE_2:
		return models::RobotShortId::Engineer2;
	case robotpilots_proto_common_RobotType_ROBOT_TYPE_3:
		return models::RobotShortId::Infantry3;
	case robotpilots_proto_common_RobotType_ROBOT_TYPE_4:
		return models::RobotShortId::Infantry4;
	case robotpilots_proto_common_RobotType_ROBOT_TYPE_6:
		return models::RobotShortId::Drone7;
	case robotpilots_proto_common_RobotType_ROBOT_TYPE_7:
		return models::RobotShortId::Sentry6;
	default:
		return models::RobotShortId::Unknown0;
	}
}

inline robotpilots_proto_common_RobotType ToRobotType(models::RobotShortId id)
{
	switch (id)
	{
	case models::RobotShortId::Hero1:
		return robotpilots_proto_common_RobotType_ROBOT_TYPE_1;
	case models::RobotShortId::Engineer2:
		return robotpilots_proto_common_RobotType_ROBOT_TYPE_2;
	case models::RobotShortId::Infantry3:
		return robotpilots_proto_common_RobotType_ROBOT_TYPE_3;
	case models::RobotShortId::Infantry4:
		return robotpilots_proto_common_RobotType_ROBOT_TYPE_4;
	case models::RobotShortId::Drone7:
		return robotpilots_proto_common_RobotType_ROBOT_TYPE_6;
	case models::RobotShortId::Sentry6:
		return robotpilots_proto_common_RobotType_ROBOT_TYPE_7;
	default:
		return robotpilots_proto_common_RobotType_ROBOT_TYPE_UNSPECIFIED;
	}
}

inline robotpilots_proto_common_Team ToTeam(models::TeamKind team)
{
	switch (team)
	{
	case models::TeamKind::Red:
		return robotpilots_proto_common_Team_TEAM_RED;
	case models::TeamKind::Blue:
		return robotpilots_proto_common_Team_TEAM_BLUE;
	default:
		return robotpilots_proto_common_Team_TEAM_UNSPECIFIED;
	}
}

inline models::RobotId FromProto(const robotpilots_proto_common_RobotId& value)
{
	const auto team = value.team == robotpilots_proto_common_Team_TEAM_RED    ? models::TeamKind::Red
	                  : value.team == robotpilots_proto_common_Team_TEAM_BLUE ? models::TeamKind::Blue
	                                                                          : models::TeamKind::Neutral;
	return {ToShortId(value.type), team};
}

inline robotpilots_proto_common_RobotId ToProto(models::RobotId value)
{
	return {.team = ToTeam(value.Team), .type = ToRobotType(value.ShortId)};
}

inline messages::ModuleInfo FromProto(const robotpilots_proto_common_ModuleInfo& value)
{
	return {
	    .Index = value.has_idx ? std::optional<std::uint32_t>{value.idx} : std::nullopt,
	    .State = value.state == robotpilots_proto_common_ModuleState_MODULE_STATE_ONLINE
		             ? messages::ModuleState::Online
		             : messages::ModuleState::Offline
	};
}

inline robotpilots_proto_common_ModuleInfo ToProto(messages::ModuleType, const messages::ModuleInfo& value)
{
	robotpilots_proto_common_ModuleInfo result{};
	result.type = robotpilots_proto_common_ModuleType_MODULE_TYPE_AUTO_AIM;
	result.has_idx = value.Index.has_value();
	result.idx = value.Index.value_or(0);
	result.state = value.State == messages::ModuleState::Online
	                   ? robotpilots_proto_common_ModuleState_MODULE_STATE_ONLINE
	                   : robotpilots_proto_common_ModuleState_MODULE_STATE_OFFLINE;
	return result;
}

inline messages::DeployCommand FromProto(const robotpilots_proto_control_DeployCommand& value)
{
	return {.Enter = value.enter};
}

inline robotpilots_proto_control_DeployCommand ToProto(const messages::DeployCommand& value)
{
	return {.enter = value.Enter};
}

inline messages::ChassisReport FromProto(const robotpilots_proto_module_ChassisReport& value)
{
	return {.Speed = {value.speed}};
}

inline robotpilots_proto_module_ChassisReport ToProto(const messages::ChassisReport& value)
{
	return {.speed = value.Speed.Value};
}

inline messages::LegReport FromProto(const robotpilots_proto_module_LegReport& value)
{
	return {.Height = value.height, .SmallTheta = {value.small_thera}, .LargeTheta = {value.large_thera}};
}

inline robotpilots_proto_module_LegReport ToProto(const messages::LegReport& value)
{
	return {.height = value.Height, .small_thera = value.SmallTheta.Value, .large_thera = value.LargeTheta.Value};
}

inline models::MotionMode FromProto(robotpilots_proto_module_MotionMode value)
{
	switch (value)
	{
	case robotpilots_proto_module_MotionMode_MOTION_MODE_SPIN:
		return models::MotionMode::Spin;
	case robotpilots_proto_module_MotionMode_MOTION_MODE_HYPER_SPIN:
		return models::MotionMode::HyperSpin;
	case robotpilots_proto_module_MotionMode_MOTION_MODE_STEP_UP:
		return models::MotionMode::StepUp;
	case robotpilots_proto_module_MotionMode_MOTION_MODE_STEP_DOWN:
		return models::MotionMode::StepDown;
	case robotpilots_proto_module_MotionMode_MOTION_MODE_LAUNCH:
		return models::MotionMode::Launch;
	case robotpilots_proto_module_MotionMode_MOTION_MODE_JUMP:
		return models::MotionMode::Jump;
	default:
		return models::MotionMode::None;
	}
}

inline robotpilots_proto_module_MotionMode ToProto(models::MotionMode value)
{
	switch (value)
	{
	case models::MotionMode::Spin:
		return robotpilots_proto_module_MotionMode_MOTION_MODE_SPIN;
	case models::MotionMode::HyperSpin:
		return robotpilots_proto_module_MotionMode_MOTION_MODE_HYPER_SPIN;
	case models::MotionMode::StepUp:
		return robotpilots_proto_module_MotionMode_MOTION_MODE_STEP_UP;
	case models::MotionMode::StepDown:
		return robotpilots_proto_module_MotionMode_MOTION_MODE_STEP_DOWN;
	case models::MotionMode::Launch:
		return robotpilots_proto_module_MotionMode_MOTION_MODE_LAUNCH;
	case models::MotionMode::Jump:
		return robotpilots_proto_module_MotionMode_MOTION_MODE_JUMP;
	default:
		return robotpilots_proto_module_MotionMode_MOTION_MODE_UNSPECIFIED;
	}
}

inline void ToProto(
    const models::RobotInfos& value,
    models::TeamKind team,
    robotpilots_proto_game_RobotStatusInfo* output,
    pb_size_t& count,
    pb_size_t capacity
)
{
	const auto protoTeam = ToTeam(team);
	for (const auto id : models::ROBOT_SHORT_ID_VALUES)
	{
		const auto type = ToRobotType(id);
		if (type == robotpilots_proto_common_RobotType_ROBOT_TYPE_UNSPECIFIED || count >= capacity)
		{
			continue;
		}

		auto& robot = output[count++];
		robot.has_id = true;
		robot.id = {.team = protoTeam, .type = type};
		if (const auto* health = value.Healths.At(id))
		{
			robot.has_health = true;
			robot.health = health->Value;
		}
		if (const auto* ammo = value.Ammos.At(id))
		{
			robot.has_ammo = true;
			robot.ammo = ammo->Value;
		}
		if (const auto* heat = value.Heats.At(id))
		{
			robot.has_heat = true;
			robot.heat = heat->Value;
		}
		if (const auto* cooling = value.CoolingRates.At(id))
		{
			robot.has_cooling_rate = true;
			robot.cooling_rate = cooling->Value;
		}
		if (const auto* buffs = value.Buffs.At(id))
		{
			robot.buffs_count = 4;
			robot.buffs[0] = {
			    .type = robotpilots_proto_game_BuffType_BUFF_TYPE_RECOVERY,
			    .strength = static_cast<std::uint32_t>(static_cast<std::int32_t>(buffs->Recovery.Value))
			};
			robot.buffs[1] = {
			    .type = robotpilots_proto_game_BuffType_BUFF_TYPE_COOLING, .strength = buffs->Cooling.Value
			};
			robot.buffs[2] = {
			    .type = robotpilots_proto_game_BuffType_BUFF_TYPE_DEFENSE,
			    .strength = static_cast<std::uint32_t>(static_cast<std::int32_t>(buffs->Defence.Value))
			};
			robot.buffs[3] = {
			    .type = robotpilots_proto_game_BuffType_BUFF_TYPE_ATTACK,
			    .strength = static_cast<std::uint32_t>(static_cast<std::int32_t>(buffs->Attack.Value))
			};
		}
		if (const auto* focus = value.MajorFocuses.At(id))
		{
			robot.has_major_focus = true;
			robot.major_focus = static_cast<robotpilots_proto_game_MajorFocusType>(*focus);
		}
		if (const auto* focus = value.ChassisFocuses.At(id))
		{
			robot.has_chassis_focus = true;
			robot.chassis_focus = static_cast<robotpilots_proto_game_ChassisFocusType>(*focus);
		}
		if (const auto* focus = value.LaunchingMechanismFocuses.At(id))
		{
			robot.has_launching_mechanism_focus = true;
			robot.launching_mechanism_focus = static_cast<robotpilots_proto_game_LaunchingMechanismFocusType>(*focus);
		}
	}
}
} // namespace breeze::communicator::internal::direct
