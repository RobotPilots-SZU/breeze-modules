#include "breeze/gimbal/module.hpp"
#include <breeze/gimbal/preset_pid.hpp>
#include <breeze/models/control.hpp>
#include <breeze/models/device.hpp>
#include <breeze/models/unit.hpp>
#include <etl/delegate.h>
#include <limits>
#include <new>
#include <ramen.hpp>
#include <robotpilots/algorithm/math.hpp>
#include <robotpilots/algorithm/pid.hpp>

namespace
{
using namespace breeze::gimbal;
using namespace robotpilots::algorithm;
constexpr float PERIOD = 360;
constexpr Pid<float>::Config MkPidConf(PidParam param)
{
	return {
	    .Kp = param.Kp,
	    .Ki = param.Ki,
	    .Kd = param.Kd,
	    // 当前算法实现总是 clamp 积分, 用最大有限值表示不限幅。
	    .IntegralLimit = param.LimitI > 0.0F ? param.LimitI : std::numeric_limits<float>::max(),
	    .IntegralSeparation = param.InteralSeperation
	};
}
constexpr float Limit(float value, float bound) { return etl::clamp(value, -bound, bound); }
class MotorController
{
public:
	MotorController(const MotorParam& param)
	    : _angleController(MkPidConf(param.Angle)),
	      _speedController(MkPidConf(param.Speed)),
	      _param(param)
	{}
	float Update(float targetAngle, float currentAngle, float currentSpeed, float delta)
	{
		assert(delta > 0);

		auto angleError = targetAngle - currentAngle;
		angleError = WrapError(angleError, PERIOD);
		angleError = Deadband(angleError, _param.Angle.InputDeadband);

		const auto targetSpeed = Limit(_angleController.Update(angleError, delta), _param.Angle.MaxOutput);
		const auto speedError = Deadband(targetSpeed - currentSpeed, _param.Speed.InputDeadband);
		return Limit(_speedController.Update(speedError, delta), _param.Speed.MaxOutput);
	}
	void Reset()
	{
		_angleController.Reset();
		_speedController.Reset();
	}

private:
	Pid<float> _angleController;
	Pid<float> _speedController;
	const MotorParam& _param;
};
struct GimbalModedMotorControllers
{
	GimbalModedMotorControllers(const ModedMotorParam& param)
	    : Gyro(param.Gyro),
	      Mechanical(param.Mechanical)
	{}
	MotorController Gyro, Mechanical;
	void Reset()
	{
		Gyro.Reset();
		Mechanical.Reset();
	}
};
}; // namespace
namespace breeze::gimbal
{
struct PidPoseModule::Impl
{
	PidPoseModule& Self;
	SingleYawModule Yaw{etl::make_delegate<Impl, &Impl::YawControl>(*this)};
	PitchModule Pitch{etl::make_delegate<Impl, &Impl::PitchControl>(*this)};
	GimbalModedMotorControllers YawControllers;
	GimbalModedMotorControllers PitchControllers;

	models::GimbalControlMode ModeCache = models::GimbalControlMode::None;
	ramen::PullNary<SingleYawFeedback, models::ImuAngularReport, models::MotorReport> YawReporter =
	    [](const auto& imu, const auto& motor) -> SingleYawFeedback { return {.HeadImu{imu.Yaw}, .YawMotor{motor}}; };
	ramen::PullNary<PitchFeedback, models::ImuAngularReport, models::MotorReport> PitchReporter =
	    [](const auto& imu, const auto& motor) -> PitchFeedback { return {.HeadImu{imu}, .PitchMotor{motor}}; };
	ramen::PushUnary<models::MotorControl, SingleYawOutput> YawOutputer =
	    [](const SingleYawOutput& in) -> models::MotorControl { return {in.YawMotor}; };
	ramen::PushUnary<models::MotorControl, PitchOutput> PitchOutputer =
	    [](const PitchOutput& in) -> models::MotorControl { return {in.PitchMotor}; };

	Impl(PidPoseModule& self, const Param& param)
	    : Self(self),
	      YawControllers(param.Yaw),
	      PitchControllers(param.Pitch)
	{
		Yaw.InFeedback >> YawReporter.out;
		Pitch.InFeedback >> PitchReporter.out;

		Yaw.Out >> YawOutputer.in;
		YawOutputer.out >> self.OutYawMotor;

		Pitch.Out >> PitchOutputer.in;
		PitchOutputer.out >> self.OutPitchMotor;
	}

	void Update(const models::Posture& posture)
	{
		const auto mode = *Self.InControlMode;
		if (mode != ModeCache)
		{
			YawControllers.Reset();
			PitchControllers.Reset();
			ModeCache = mode;
		}

		Pitch.InTarget({posture.Pitch});
		Yaw.InTarget({posture.Yaw});
	}

	SingleYawOutput YawControl(
	    const SingleYawTarget& target,
	    const SingleYawFeedback& feedback,
	    models::MilliSeconds<std::int32_t> delta
	)
	{
		auto& [targetAngle] = target.Yaw;
		const auto convertedDelta = static_cast<float>(delta.Value) / 1000.0f;
		switch (ModeCache)
		{
			using enum models::GimbalControlMode;
		case Gyro: {
			auto& controller = YawControllers.Gyro;
			auto& [currentAngle] = feedback.HeadImu.Yaw;
			auto& [currentSpeed] = feedback.HeadImu.YawRate;
			auto result = controller.Update(targetAngle, currentAngle, currentSpeed, convertedDelta);
			return {result};
		}
		case Mechanical: {
			auto& controller = YawControllers.Mechanical;
			auto& [currentAngle] = feedback.YawMotor.Angle;
			auto& [currentSpeed] = feedback.YawMotor.Speed;
			auto result = controller.Update(targetAngle, currentAngle, currentSpeed * (360.0f / 6.0f), convertedDelta);
			return {result};
		}
		default:
			return {0};
		}
	}
	PitchOutput PitchControl(
	    const PitchTarget& target,
	    const PitchFeedback& feedback,
	    models::MilliSeconds<std::int32_t> delta
	)
	{
		auto& [targetAngle] = target.Pitch;
		const auto convertedDelta = static_cast<float>(delta.Value) / 1000.0f;
		switch (ModeCache)
		{
			using enum models::GimbalControlMode;
		case Gyro: {
			auto& controller = PitchControllers.Gyro;
			auto& [currentAngle] = feedback.HeadImu.Pitch;
			auto& [currentSpeed] = feedback.HeadImu.PitchRate;
			auto result = controller.Update(targetAngle, currentAngle, currentSpeed, convertedDelta);
			return {result};
		}
		case Mechanical: {
			auto& controller = PitchControllers.Mechanical;
			auto& [currentAngle] = feedback.PitchMotor.Angle;
			auto& [currentSpeed] = feedback.PitchMotor.Speed;
			auto result = controller.Update(targetAngle, currentAngle, currentSpeed * (360.0f / 6.0f), convertedDelta);
			return {result};
		}
		default:
			return {0};
		}
	}
	void Start()
	{
		Yaw.Start();
		Pitch.Start();
	}
	void Reset()
	{
		YawControllers.Reset();
		PitchControllers.Reset();
		Yaw.Reset();
		Pitch.Reset();
	}
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
PidPoseModule::PidPoseModule(PidPoseModule::Param param)
    : InMachTarget([this](const models::Posture& posture) { _impl.Update(posture); }),
      _impl(*new (_storage) Impl{*this, param})
{
	static_assert(sizeof(Impl) <= IMPL_SIZE, "Impl storage size is smaller than Impl type");
	static_assert(alignof(Impl) <= alignof(std::max_align_t), "Impl storage alignment is insufficient");
}
PidPoseModule::~PidPoseModule() { _impl.~Impl(); }
void PidPoseModule::Start() { _impl.Start(); }
void PidPoseModule::Reset() { _impl.Reset(); }
} // namespace breeze::gimbal
