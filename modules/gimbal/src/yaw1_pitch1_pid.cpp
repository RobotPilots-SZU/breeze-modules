#include <breeze/gimbal/yaw1_pitch1_pid.hpp>
#include <etl/algorithm.h>
#include <limits>
#include <new>
#include <robotpilots/algorithm/math.hpp>
#include <robotpilots/algorithm/pid.hpp>
#include <zephyr/kernel.h>

namespace
{
using namespace breeze::gimbal;
using namespace robotpilots::algorithm;
constexpr float PERIOD = 360.0F;

constexpr Pid<float>::Config MkPidConf(PidParam param)
{
	return {
	    .Kp = param.Kp,
	    .Ki = param.Ki,
	    .Kd = param.Kd,
	    .IntegralLimit = param.LimitI > 0.0F ? param.LimitI : std::numeric_limits<float>::max(),
	    .IntegralSeparation = param.InteralSeperation
	};
}

constexpr float Limit(float value, float bound) { return bound > 0.0F ? etl::clamp(value, -bound, bound) : value; }

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
		assert(delta > 0.0F);
		auto angleError = Deadband(WrapError(targetAngle - currentAngle, PERIOD), _param.Angle.InputDeadband);
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

struct ModedMotorControllers
{
	ModedMotorControllers(const ModedMotorParam& param)
	    : Gyro(param.Gyro),
	      Mechanical(param.Mechanical)
	{}
	MotorController Gyro;
	MotorController Mechanical;
	void Reset()
	{
		Gyro.Reset();
		Mechanical.Reset();
	}
};
} // namespace

namespace breeze::gimbal
{
struct Yaw1Pitch1PidModule::Impl
{
	Yaw1Pitch1PidModule& Self;
	Param Parameters;
	ModedMotorControllers YawControllers;
	ModedMotorControllers PitchControllers;
	models::GimbalControlMode ModeCache = models::GimbalControlMode::None;
	std::int64_t LastTime = 0;

	Impl(Yaw1Pitch1PidModule& self, const Param& param)
	    : Self(self),
	      Parameters(param),
	      YawControllers(Parameters.Yaw),
	      PitchControllers(Parameters.Pitch)
	{}

	void CheckMode(models::GimbalControlMode mode)
	{
		if (mode != ModeCache)
		{
			YawControllers.Reset();
			PitchControllers.Reset();
			ModeCache = mode;
		}
	}

	void UpdateGyro(const models::Posture& target, const models::ImuAngularReport& feedback)
	{
		CheckMode(models::GimbalControlMode::Gyro);
		const auto delta = static_cast<float>(k_uptime_delta(&LastTime)) / 1000.0F;
		const auto yaw =
		    YawControllers.Gyro.Update(target.Yaw.Value, feedback.Yaw.Value, feedback.YawRate.Value, delta);
		const auto pitch =
		    PitchControllers.Gyro.Update(target.Pitch.Value, feedback.Pitch.Value, feedback.PitchRate.Value, delta);
		Self.OutYawMotor({.Torque = {yaw}});
		Self.OutPitchMotor({.Torque = {pitch}});
	}

	void UpdateMechanical(
	    const models::Posture& target,
	    const models::MotorReport& yawFeedback,
	    const models::MotorReport& pitchFeedback
	)
	{
		CheckMode(models::GimbalControlMode::Mechanical);
		const auto delta = static_cast<float>(k_uptime_delta(&LastTime)) / 1000.0F;
		const auto yaw = YawControllers.Mechanical.Update(
		    target.Yaw.Value, yawFeedback.Angle.Value, yawFeedback.Speed.Value * 6.0F, delta
		);
		const auto pitch = PitchControllers.Mechanical.Update(
		    target.Pitch.Value, pitchFeedback.Angle.Value, pitchFeedback.Speed.Value * 6.0F, delta
		);
		Self.OutYawMotor({.Torque = {yaw}});
		Self.OutPitchMotor({.Torque = {pitch}});
	}

	void Start() { LastTime = k_uptime_get(); }
	void Reset()
	{
		YawControllers.Reset();
		PitchControllers.Reset();
		LastTime = k_uptime_get();
	}
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
Yaw1Pitch1PidModule::Yaw1Pitch1PidModule(Param param)
    : InMachTarget([this](const models::Posture& target) {
	      const auto mode = *InControlMode;
	      if (mode == models::GimbalControlMode::Gyro)
	      {
		      const auto imu = *InHeadImu;
		      _impl.UpdateGyro(target, imu);
	      }
	      else if (mode == models::GimbalControlMode::Mechanical)
	      {
		      _impl.UpdateMechanical(target, *InYawMotor, *InPitchMotor);
	      }
	      else
	      {
		      _impl.CheckMode(models::GimbalControlMode::None);
		      OutYawMotor({.Torque = {0}});
		      OutPitchMotor({.Torque = {0}});
	      }
      }),
      _impl(*new (_storage) Impl{*this, param})
{
	static_assert(sizeof(Impl) <= IMPL_SIZE, "Impl storage size is smaller than Impl type");
	static_assert(alignof(Impl) <= alignof(std::max_align_t), "Impl storage alignment is insufficient");
}

Yaw1Pitch1PidModule::~Yaw1Pitch1PidModule() { _impl.~Impl(); }
void Yaw1Pitch1PidModule::Start() { _impl.Start(); }
void Yaw1Pitch1PidModule::Reset() { _impl.Reset(); }
} // namespace breeze::gimbal
