/**
 * @file module.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 云台模块的原子模块, 提供常见云台构型的算法无关声明
 * @note 完整功能请使用preset.
 * @version 0.1
 * @date 2026-11-09
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <breeze/common/control.hpp>
#include <breeze/models/device.hpp>
#include <breeze/models/unit.hpp>
#include <etl/array.h>
#include <etl/delegate.h>
#include <ramen.hpp>

namespace breeze::gimbal
{

// Pitch 控制
#pragma region Pitch

struct PitchFeedback
{
	/**
	 * @brief 头上的Imu反馈值. 通常只读Pitch
	 *
	 */
	models::ImuAngularReport HeadImu;
	/**
	 * @brief Pitch电机反馈值
	 *
	 */
	models::MotorReport PitchMotor;
};

struct PitchTarget
{
	models::Degree<float> Pitch;
};

struct PitchOutput
{
	models::MotorControl PitchMotor;
};

using PitchModule = common::ControlModuleBase<PitchFeedback, PitchTarget, PitchOutput>;

#pragma endregion

// 单Yaw
#pragma region Single Yaw
/**
 * @brief 单Yaw反馈
 *
 */
struct SingleYawFeedback
{
	/**
	 * @brief 头上的Imu反馈值. 通常只读Yaw
	 *
	 */
	models::ImuAngularReport HeadImu;
	/**
	 * @brief Yaw电机反馈值
	 *
	 */
	models::MotorReport YawMotor;
};
struct SingleYawTarget
{
	models::Degree<float> Yaw;
};
struct SingleYawOutput
{
	models::MotorControl YawMotor;
};

/**
 * @brief 单Yaw控制模块
 * @note (Yaw目标, 反馈) -> Yaw 电机控制
 *
 */
using SingleYawModule = common::ControlModuleBase<SingleYawFeedback, SingleYawTarget, SingleYawOutput>;

#pragma endregion

#pragma region Double Yaw

struct DoubleYawFeedback
{
	/**
	 * @brief 上方Imu反馈
	 *
	 */
	models::ImuAngularReport UpImu;

	/**
	 * @brief 上方 Yaw 电机反馈
	 *
	 */
	models::MotorReport UpMotor;
	/**
	 * @brief 下方Imu反馈
	 *
	 */
	models::ImuAngularReport DownImu;
	/**
	 * @brief 下方 Yaw 电机反馈
	 *
	 */
	models::ImuAngularReport DownMotor;
};
struct DoubleYawTarget
{
	models::Degree<float> Yaw;
};
struct DoubleYawOutput
{
	models::MotorControl UpYawMotor;
	models::MotorControl DownYawMotor;
};

/**
 * @brief 双Yaw控制模块
 * @note (Yaw目标, 反馈) -> 两个Yaw 电机控制
 *
 */
using DoubleYawModule = common::ControlModuleBase<DoubleYawFeedback, DoubleYawTarget, DoubleYawOutput>;
#pragma endregion

} // namespace breeze::gimbal
