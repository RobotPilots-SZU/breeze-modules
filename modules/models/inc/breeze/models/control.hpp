/**
 * @file control.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 机器人控制相关 数据模型 与 指令
 * @note 这个 model 旨在统一上位机, 板间通信等等的机器人控制逻辑表示, 以及用于对控制的抽象, 联系modules中各种控车板块.
 * @version 0.1
 * @date 2026-10-26
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "unit.hpp"
#include <cstdint>

namespace breeze::models
{

/**
 * @brief 运动模式类别
 *
 */
enum class MotionMode : std::uint8_t
{
	None,
	Spin,      ///< 小陀螺
	HyperSpin, ///< 大陀螺
	StepUp,    ///< 上台阶
	StepDown,  ///< 下台阶
	Launch,    ///< 飞坡
	Jump       ///< 跳跃
};

/**
 * @brief 机器人控制命令的Tag类型, 无内容.
 *
 */
struct ControlCmd
{};

/**
 * @brief 云台控制模式
 *
 */
enum class GimbalControlMode : std::uint8_t
{
	None,
	Gyro,       ///< 陀螺仪模式
	Mechanical, ///< 机械模式
	Speed       ///< 速度模式
};

/**
 * @brief 控云台命令
 *
 */
struct GimbalCmd : ControlCmd
{
	GimbalControlMode Mode; ///< 云台控制模式
	Degree<float> Pitch;    ///< Pitch. 正值表示向上.
};

/**
 * @brief 底盘控制模式
 *
 */
enum class ChassisControlMode : std::uint8_t
{
	None,
	Speed,   ///< 速度Pid闭环
	Position ///< 位置Pid闭环
};

/**
 * @brief 底盘控制命令
 * @note TODO: 目前建模仅为临时表示, 后续根据底盘模块设计再做改动
 *
 */
struct ChassisCmd : ControlCmd
{
	Vec2<std::int16_t> Speed;           ///< (右移速度, 前进速度) 的归一化矢量 max(int16) 表示满速
	Degree<std::int16_t> RotationSpeed; ///< 旋转速度 的归一化标量. max(int16) 表示顺时针转满速.
};

/**
 * @brief 开火模式(实际上是弹速控制)
 *
 */
enum class FiringMode : std::uint8_t
{
	Normal, ///< 普通模式
	Lob     ///< 吊射模式
};

/**
 * @brief 开火意图
 *
 */
enum class FiringIntent : std::uint8_t
{
	Idle,        ///< 停止射击
	Once,        ///< 单发
	Continuosly, ///< 连发
};

/**
 * @brief 射击命令.
 * @note 目前计划是点完就自动处理拨盘和摩擦轮.
 *
 */
struct ShooterCmd : ControlCmd
{
	FiringMode Mode;
	FiringIntent Intent;
};

} // namespace breeze::models
