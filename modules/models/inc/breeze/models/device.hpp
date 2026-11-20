/**
 * @file device.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 对各种设备的数据建模, 常用于算法与模块的解耦, 以及方便通信协议的转换.
 * @version 0.1
 * @date 2026-11-09
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include "breeze/models/game.hpp"
#include "unit.hpp"
#include <etl/variant.h>

namespace breeze::models
{

#pragma region IMU

/**
 * @brief IMU处理后的数据 (角度相关)
 *
 */
struct ImuAngularReport
{
	/**
	 * @brief 姿态的角度
	 *
	 */
	Pose Angle;
	/**
	 * @brief 姿态的变化速率(角速度)
	 *
	 */
	Pose Rate;
};

/**
 * @brief IMU处理后的数据 (机体)
 *
 */
struct ImuBodyReport
{
	/**
	 * @brief 三个轴方向的加速度. 单位: m/s^2
	 * @par 参数极性的定义
	 *
	 * X, Y. 目前暂时未定义极性, 由对应模块自行定义.
	 * Z 默认规定: 向上为正.
	 */
	Meter<float> XAcc, YAcc, ZAcc;
};

#pragma endregion

#pragma region Encoder
/**
 * @brief 编码器(电机)值. 编码区间为 [0,Bound]
 */
struct EncoderReport
{
	/**
	 * @brief 当前编码值
	 *
	 */
	std::uint16_t Encode;
};

/**
 * @brief 编码器参数
 *
 */
struct EncoderParam
{
	/**
	 * @brief 编码区间. 表示编码区间为 [0, Bound]
	 *
	 */
	std::uint16_t Bound;
};

#pragma endregion

#pragma region Motor

/**
 * @brief 电机的反馈
 *
 */
struct MotorReport
{
	/**
	 * @brief 编码值
	 *
	 */
	EncoderReport Encoder;
	/**
	 * @brief 转速
	 *
	 */
	Rpm<float> Speed;
};

/**
 * @brief 电机控制指令
 *
 */
struct MotorControl
{
	/**
	 * @brief 输出扭矩
	 *
	 */
	NewtonMeter<float> Torque;
};

#pragma endregion

} // namespace breeze::models
