// NOLINTBEGIN(readability-magic-numbers)
#include <zephyr/ztest.h>

#include <breeze/communicator/module.hpp>

#include <pb_decode.h>
#include <pb_encode.h>
#include <ramen.hpp>
#include <robotpilots/proto/toplevel/universal_message.pb.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <vector>

namespace
{

using breeze::communicator::Module;
using breeze::communicator::messages::ChassisReport;
using breeze::communicator::messages::DeployCommand;
using breeze::models::Meter;
using breeze::models::MotionMode;

constexpr std::uint8_t START_OF_FRAME = 0xa7;
constexpr std::size_t FRAME_HEADER_SIZE = 4;

std::vector<std::uint8_t> EncodeMessage(const robotpilots_proto_toplevel_UniversalMessage& message)
{
	std::array<std::uint8_t, 128> buffer{};
	auto stream = pb_ostream_from_buffer(buffer.data(), buffer.size());
	if (!pb_encode(&stream, robotpilots_proto_toplevel_UniversalMessage_fields, &message))
	{
		return {};
	}
	return {buffer.begin(), buffer.begin() + stream.bytes_written};
}

std::vector<std::uint8_t> MakeFrame(const robotpilots_proto_toplevel_UniversalMessage& message)
{
	auto payload = EncodeMessage(message);
	std::vector<std::uint8_t> frame(FRAME_HEADER_SIZE + payload.size());
	frame[0] = START_OF_FRAME;
	frame[1] = static_cast<std::uint8_t>(payload.size() >> 16);
	frame[2] = static_cast<std::uint8_t>(payload.size() >> 8);
	frame[3] = static_cast<std::uint8_t>(payload.size());
	std::copy(payload.begin(), payload.end(), frame.begin() + FRAME_HEADER_SIZE);
	return frame;
}

robotpilots_proto_toplevel_UniversalMessage DecodeOutputFrame(const std::vector<std::uint8_t>& frame)
{
	robotpilots_proto_toplevel_UniversalMessage message = robotpilots_proto_toplevel_UniversalMessage_init_zero;
	if (frame.size() < FRAME_HEADER_SIZE || frame[0] != START_OF_FRAME)
	{
		return message;
	}
	const auto payloadSize = (static_cast<std::size_t>(frame[1]) << 16) | (static_cast<std::size_t>(frame[2]) << 8) |
	                         static_cast<std::size_t>(frame[3]);
	if (frame.size() != FRAME_HEADER_SIZE + payloadSize)
	{
		return message;
	}
	auto stream = pb_istream_from_buffer(frame.data() + FRAME_HEADER_SIZE, payloadSize);
	(void)pb_decode(&stream, robotpilots_proto_toplevel_UniversalMessage_fields, &message);
	return message;
}

} // namespace

ZTEST(communicator, decode_deploy_command_with_push_output)
{
	robotpilots_proto_toplevel_UniversalMessage message = robotpilots_proto_toplevel_UniversalMessage_init_zero;
	message.has_command_request = true;
	message.command_request.which_cmd_id = robotpilots_proto_control_CommandRequest_deploy_cmd_tag;
	message.command_request.cmd_id.deploy_cmd.enter = true;
	const auto frame = MakeFrame(message);

	Module module;
	DeployCommand result{};
	bool received = false;
	ramen::Pushable<DeployCommand> observer = [&](const DeployCommand& command) {
		result = command;
		received = true;
	};
	module.Out.DeployCommand >> observer;
	module.InRaw({frame.data(), frame.size()});

	zassert_true(received, "deploy command was not pushed");
	zassert_true(result.Enter, "deploy command was not decoded");
}

ZTEST(communicator, decode_motion_mode_with_push_output)
{
	robotpilots_proto_toplevel_UniversalMessage message = robotpilots_proto_toplevel_UniversalMessage_init_zero;
	message.has_motion_mode = true;
	message.motion_mode = robotpilots_proto_module_MotionMode_MOTION_MODE_STEP_UP;
	const auto frame = MakeFrame(message);

	Module module;
	MotionMode result{};
	bool received = false;
	ramen::Pushable<MotionMode> observer = [&](const MotionMode& mode) {
		result = mode;
		received = true;
	};
	module.Out.MotionMode >> observer;
	module.InRaw({frame.data(), frame.size()});

	zassert_true(received, "motion mode was not pushed");
	zassert_equal(result, MotionMode::StepUp, "motion mode mismatch");
}

ZTEST(communicator, decode_frame_in_multiple_chunks)
{
	robotpilots_proto_toplevel_UniversalMessage message = robotpilots_proto_toplevel_UniversalMessage_init_zero;
	message.has_motion_mode = true;
	message.motion_mode = robotpilots_proto_module_MotionMode_MOTION_MODE_JUMP;
	const auto frame = MakeFrame(message);

	Module module;
	MotionMode result{};
	bool received = false;
	ramen::Pushable<MotionMode> observer = [&](const MotionMode& mode) {
		result = mode;
		received = true;
	};
	module.Out.MotionMode >> observer;

	const auto firstChunkSize = std::size_t{2};
	const auto secondChunkSize = FRAME_HEADER_SIZE + 1;
	module.InRaw({frame.data(), firstChunkSize});
	zassert_false(received, "message emitted before the frame was complete");
	module.InRaw({frame.data() + firstChunkSize, secondChunkSize - firstChunkSize});
	zassert_false(received, "message emitted before the payload was complete");
	module.InRaw({frame.data() + secondChunkSize, frame.size() - secondChunkSize});

	zassert_true(received, "message was not emitted after the final chunk");
	zassert_equal(result, MotionMode::Jump, "motion mode mismatch");
}

ZTEST(communicator, encode_chassis_report_and_collect_pushed_chunks)
{
	Module module;
	std::vector<std::uint8_t> frame;
	ramen::Pushable<std::span<const std::uint8_t>> collector = [&](const std::span<const std::uint8_t>& chunk) {
		frame.insert(frame.end(), chunk.begin(), chunk.end());
	};
	module.OutRaw >> collector;

	const ChassisReport report{.Speed = Meter<float>{1.25F}};
	module.In.ChassisReport(report);

	const auto message = DecodeOutputFrame(frame);
	zassert_true(message.has_chassis_report, "chassis report was not encoded");
	zassert_true(message.chassis_report.speed == report.Speed.Value, "chassis speed mismatch");
}

ZTEST_SUITE(communicator, NULL, NULL, NULL, NULL, NULL);

// NOLINTEND(readability-magic-numbers)
