// NOLINTBEGIN(readability-magic-numbers)
#include <breeze/gimbal/yaw1_pitch1_pid.hpp>
#include <zephyr/ztest.h>

namespace
{
using namespace breeze;

gimbal::Yaw1Pitch1PidModule::Param MakeParam()
{
	const gimbal::MotorParam motor{.Angle = {.Kp = 1.0F}, .Speed = {.Kp = 1.0F}};
	return {.Yaw = {motor, motor}, .Pitch = {motor, motor}};
}

struct Rig
{
	models::GimbalControlMode Mode = models::GimbalControlMode::Gyro;
	models::ImuAngularReport Imu{};
	models::MotorReport Yaw{}, Pitch{};
	models::MotorControl YawOutput{}, PitchOutput{};
	int ModeReads = 0, ImuReads = 0, YawReads = 0, PitchReads = 0;
	int YawWrites = 0, PitchWrites = 0;
	bool ChangeFeedbackOnOutput = false;
	ramen::Pullable<models::GimbalControlMode> ModeSource{[this](auto& value) {
		value = Mode;
		++ModeReads;
	}};
	ramen::Pullable<models::ImuAngularReport> ImuSource{[this](auto& value) {
		value = Imu;
		++ImuReads;
	}};
	ramen::Pullable<models::MotorReport> YawSource{[this](auto& value) {
		value = Yaw;
		++YawReads;
	}};
	ramen::Pullable<models::MotorReport> PitchSource{[this](auto& value) {
		value = Pitch;
		++PitchReads;
	}};
	ramen::Pushable<models::MotorControl> YawSink{[this](const auto& value) {
		YawOutput = value;
		++YawWrites;
	}};
	ramen::Pushable<models::MotorControl> PitchSink{[this](const auto& value) {
		PitchOutput = value;
		++PitchWrites;
		if (ChangeFeedbackOnOutput)
		{
			Mode = models::GimbalControlMode::None;
			Imu.Yaw.Value = 100.0F;
		}
	}};
    gimbal::Yaw1Pitch1PidModule Module;

    explicit Rig(gimbal::Yaw1Pitch1PidModule::Param param = MakeParam())
	    : Module(param)
	{
		Module.InControlMode >> ModeSource;
		Module.InHeadImu >> ImuSource;
		Module.InYawMotor >> YawSource;
		Module.InPitchMotor >> PitchSource;
		Module.OutYawMotor >> YawSink;
		Module.OutPitchMotor >> PitchSink;
		Module.Start();
	}

	void Step(float yaw, float pitch)
	{
		k_sleep(K_MSEC(10));
		Module.InMachTarget({.Pitch = {pitch}, .Roll = {123.0F}, .Yaw = {yaw}});
	}

	void Expect(float yaw, float pitch) const
	{
		zassert_within(
		    YawOutput.Torque.Value, yaw, 0.001F, "Yaw torque mismatch: got %f expected %f",
		    static_cast<double>(YawOutput.Torque.Value), static_cast<double>(yaw)
		);
		zassert_within(
		    PitchOutput.Torque.Value, pitch, 0.001F, "Pitch torque mismatch: got %f expected %f",
		    static_cast<double>(PitchOutput.Torque.Value), static_cast<double>(pitch)
		);
	}
};

ZTEST(gimbal, gyro_flow_uses_one_snapshot)
{
	Rig rig;
	rig.Imu.Yaw.Value = 350.0F;
	rig.Imu.YawRate.Value = 2.0F;
	rig.Imu.Pitch.Value = 10.0F;
	rig.Imu.PitchRate.Value = -3.0F;
	rig.ChangeFeedbackOnOutput = true;
	rig.Step(10.0F, -10.0F);
	rig.Expect(18.0F, -17.0F);
	zassert_equal(rig.ModeReads, 1);
	zassert_equal(rig.ImuReads, 1);
	zassert_equal(rig.YawReads, 0);
	zassert_equal(rig.PitchReads, 0);
	zassert_equal(rig.YawWrites, 1);
	zassert_equal(rig.PitchWrites, 1);
}

ZTEST(gimbal, mechanical_feedback_and_rpm_conversion)
{
	Rig rig;
	rig.Mode = models::GimbalControlMode::Mechanical;
	rig.Yaw.Angle.Value = 10.0F;
	rig.Yaw.Speed.Value = 2.0F;
	rig.Pitch.Angle.Value = 350.0F;
	rig.Pitch.Speed.Value = -2.0F;
	rig.Step(350.0F, 10.0F);
	rig.Expect(-32.0F, 32.0F);
	zassert_equal(rig.ImuReads, 0);
	zassert_equal(rig.YawReads, 1);
	zassert_equal(rig.PitchReads, 1);
}

ZTEST(gimbal, both_deadbands_and_output_limits)
{
	auto param = MakeParam();
	param.Yaw.Gyro.Angle.InputDeadband = 2.0F;
	param.Pitch.Gyro.Speed.InputDeadband = 2.0F;
	param.Yaw.Gyro.Angle.MaxOutput = 3.0F;
	param.Yaw.Gyro.Speed.Kp = 2.0F;
	param.Pitch.Gyro.Speed.MaxOutput = 4.0F;
	Rig rig(param);
	rig.Step(1.0F, 1.0F);
	rig.Expect(0.0F, 0.0F);
	rig.Step(20.0F, 20.0F);
	rig.Expect(6.0F, 4.0F);
	rig.Step(-20.0F, -20.0F);
	rig.Expect(-6.0F, -4.0F);
	// 角度进入死区时速度环仍需制动, 不应直接将扭矩归零。
	rig.Imu.YawRate.Value = 1.0F;
	rig.Step(0.0F, 0.0F);
	rig.Expect(-2.0F, 0.0F);
}

ZTEST(gimbal, integral_uses_seconds_and_survives_deadband)
{
	auto param = MakeParam();
	param.Yaw.Gyro.Angle = {.Ki = 1.0F, .InputDeadband = 2.0F};
	param.Pitch.Gyro.Speed = {.Ki = 1.0F};
	Rig rig(param);
	const auto start = k_uptime_get();
	rig.Step(10.0F, 10.0F);
	const auto expected = static_cast<float>(k_uptime_get() - start) / 100.0F;
	zassert_true(expected > 0.0F);
	rig.Expect(expected, expected);
	rig.Step(1.0F, 0.0F);
	rig.Expect(expected, expected);
}

ZTEST(gimbal, integral_limit_and_separation)
{
	auto param = MakeParam();
	param.Yaw.Gyro.Angle = {.Ki = 1.0F, .LimitI = 0.05F};
	param.Pitch.Gyro.Angle = {.Ki = 1.0F, .InteralSeperation = 5.0F};
	Rig rig(param);
	rig.Step(10.0F, 1.0F);
	zassert_true(rig.PitchOutput.Torque.Value > 0.0F);
	rig.Step(10.0F, 10.0F);
	rig.Expect(0.05F, 0.0F);
}

ZTEST(gimbal, successive_targets_update_output)
{
	Rig rig;
	rig.Step(3.0F, 4.0F);
	rig.Step(30.0F, 40.0F);
	rig.Expect(30.0F, 40.0F);
}

} // namespace

ZTEST_SUITE(gimbal, NULL, NULL, NULL, NULL, NULL);
// NOLINTEND(readability-magic-numbers)
