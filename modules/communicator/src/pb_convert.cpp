#include "pb_convert.hpp"
#include "conversion/direct.hpp"
#include <pb_decode.h>
#include <pb_encode.h>

namespace breeze::communicator::internal
{

void PbDecoder::Accept(std::span<const std::uint8_t> data)
{
	robotpilots_proto_toplevel_UniversalMessage message = robotpilots_proto_toplevel_UniversalMessage_init_zero;
	auto stream = pb_istream_from_buffer(data.data(), data.size());
	if (!pb_decode(&stream, robotpilots_proto_toplevel_UniversalMessage_fields, &message))
	{
		return;
	}
	if (message.has_command_request &&
	    message.command_request.which_cmd_id == robotpilots_proto_control_CommandRequest_deploy_cmd_tag)
	{
		Out.DeployCommand(direct::FromProto(message.command_request.cmd_id.deploy_cmd));
	}
	if (message.has_motion_mode)
	{
		Out.MotionMode(direct::FromProto(message.motion_mode));
	}
}

void PbEncoder::Emit()
{
	robotpilots_proto_toplevel_UniversalMessage message = robotpilots_proto_toplevel_UniversalMessage_init_zero;
	if (In.Id)
	{
		message.has_id = true;
		message.id = direct::ToProto(*In.Id);
	}
	if (In.Modules)
	{
		for (const auto type : messages::MODULE_TYPE_VALUES)
		{
			const auto* info = In.Modules->At(type);
			if (!info || !*info || message.modules_count >= std::size(message.modules))
			{
				continue;
			}
			message.modules[message.modules_count++] = direct::ToProto(type, **info);
		}
	}
	if (In.ChassisReport)
	{
		message.has_chassis_report = true;
		message.chassis_report = direct::ToProto(*In.ChassisReport);
	}
	if (In.LegReport)
	{
		message.has_leg_report = true;
		message.leg_report = direct::ToProto(*In.LegReport);
	}
	if (In.MotionMode)
	{
		message.has_motion_mode = true;
		message.motion_mode = direct::ToProto(*In.MotionMode);
	}
	const auto allyTeam = In.Id ? In.Id->Team : models::TeamKind::Neutral;
	const auto enemyTeam = allyTeam == models::TeamKind::Red    ? models::TeamKind::Blue
	                       : allyTeam == models::TeamKind::Blue ? models::TeamKind::Red
	                                                            : models::TeamKind::Neutral;
	if (In.AllyRobotInfos)
	{
		direct::ToProto(
		    *In.AllyRobotInfos, allyTeam, message.robot_infos, message.robot_infos_count, std::size(message.robot_infos)
		);
	}
	if (In.EnemyRobotInfos)
	{
		direct::ToProto(
		    *In.EnemyRobotInfos, enemyTeam, message.robot_infos, message.robot_infos_count,
		    std::size(message.robot_infos)
		);
	}
	auto stream = pb_ostream_from_buffer(Buffer.data(), Buffer.size());
	if (!pb_encode(&stream, robotpilots_proto_toplevel_UniversalMessage_fields, &message))
	{
		return;
	}
	Out(std::span<const std::uint8_t>{Buffer.data(), stream.bytes_written});
}

} // namespace breeze::communicator::internal
