/**
 * @file io.hpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief 发射模块输入输出定义
 * @version 0.1
 * @date 2026-11-15
 *
 * @note 这个文件定义了发射模块的输入输出接口。
 *
 * @copyright Copyright (c)  2026 RobotPilots-SZU.
 * @license SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

namespace breeze::shooter
{

/**
 * @brief 单个摩擦轮的设备反馈
 * @note 原样填写驱动返回的量, 不做方向换算:
 *       摩擦轮转速单位为 rpm, 极性与电机正转一致。
 */
struct FricFeedback
{
	float Speed{}; ///< 转速(rpm), 电机正转为正
	bool Stuck{};  ///< 当前堵转状态, 由设备层更新与解除
};

/**
 * @brief 拨盘的设备反馈
 * @note 应用层将驱动速度转换为输出轴角速度(°/s), 保留电机原始极性:
 *       速度目标和相对位移按 FeedConfig::Direction 换算, 反馈直接参与 PID.
 *       绝对弹位表与反馈使用同一输出轴坐标, 表内目标角度不做极性转换.
 *       角度为拨盘轴单圈位置 [0, 360), 保留标定零点, 不在唤醒时置零.
 *
 */
struct FeedFeedback
{
	float Speed{};  ///< 输出轴角速度(°/s), 电机正转为正
	float Angle{};  ///< 拨盘轴角度 [0, 360), 电机正转方向递增, 单位为度
	bool Stuck{};   ///< 当前堵转状态, 由设备层更新与解除
};

} // namespace breeze::shooter
