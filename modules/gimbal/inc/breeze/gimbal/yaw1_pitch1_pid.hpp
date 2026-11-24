/**
 * @file yaw1_pitch1_pid.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 云台模块单Yaw单Pitch构型pid实现
 * @version 0.1
 * @date 2026-11-10
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <breeze/models/control.hpp>
#include <breeze/models/device.hpp>
#include <breeze/models/unit.hpp>
#include <cstddef>
#include <ramen.hpp>

namespace breeze::gimbal
{

struct PidParam
{
	/**
	 * @brief 比例参数
	 *
	 */
	float Kp;
	/**
	 * @brief 积分参数
	 *
	 */
	float Ki;
	/**
	 * @brief 微分参数
	 *
	 */
	float Kd;
	/**
	 * @brief 积分累积值限幅, <= 0 表示不限幅
	 *
	 */
	float LimitI;
	/**
	 * @brief 积分分离值
	 *
	 */
	float InteralSeperation;
	/**
	 * @brief 输出绝对值限幅, <= 0 表示不限幅
	 *
	 */
	float MaxOutput;
	/**
	 * @brief 输入误差死区, <= 0 表示不启用; 区内误差归零, 保留积分
	 *
	 */
	float InputDeadband;
};
struct MotorParam
{
	// 角度环: 输入为度, 输出为度/秒; 速度环: 输入为度/秒, 输出为牛米。
	PidParam Angle, Speed;
};
struct ModedMotorParam
{
	MotorParam Gyro, Mechanical;
};

/**
 * @brief Pid实现的云台姿态单Yaw, 单Pitch控制模块
 * @note 用于英雄机器人的头部控制
 *
 */
class Yaw1Pitch1PidModule
{
public:
	/**
	 * @brief 目标姿态 (Roll被忽略), 使用当前控制模式的坐标系
	 *
	 */
	ramen::Pushable<models::Posture> InMachTarget;
	/**
	 * @brief 头的Imu数据
	 *
	 */
	ramen::Puller<models::ImuAngularReport> InHeadImu;
	/**
	 * @brief Yaw电机反馈
	 *
	 */
	ramen::Puller<models::MotorReport> InYawMotor;
	/**
	 * @brief Pitch电机反馈
	 *
	 */
	ramen::Puller<models::MotorReport> InPitchMotor;
	/**
	 * @brief 云台控制模式, None表示失能
	 *
	 */
	ramen::Puller<models::GimbalControlMode> InControlMode;
	/**
	 * @brief Yaw电机输出
	 *
	 */
	ramen::Pusher<models::MotorControl> OutYawMotor;
	/**
	 * @brief Pitch电机输出
	 *
	 */
	ramen::Pusher<models::MotorControl> OutPitchMotor;
	struct Param
	{
		/**
		 * @brief Yaw电机参数 (控制模式 -> 电机参数)
		 *
		 */
		ModedMotorParam Yaw;
		/**
		 * @brief Pitch电机参数 (控制模式 -> 电机参数)
		 *
		 */
		ModedMotorParam Pitch;
	};
	Yaw1Pitch1PidModule(Param param);
	~Yaw1Pitch1PidModule();

	/** @brief 初始化控制周期计时 */
	void Start();
	/** @brief 清除两轴所有PID状态 */
	void Reset();

private:
	struct Impl;
	static constexpr int IMPL_SIZE = 1792;
	alignas(std::max_align_t) std::byte _storage[IMPL_SIZE];
	Impl& _impl;
};

} // namespace breeze::gimbal
