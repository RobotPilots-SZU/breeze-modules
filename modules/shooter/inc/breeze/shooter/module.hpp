/**
 * @file module.hpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief   发射模块接口
 * @version 0.1
 * @date 2026-11-08
 *
 * @copyright Copyright (c)  2026 RobotPilots-SZU.
 * @license SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <breeze/models/control.hpp>
#include "config.hpp"
#include "io.hpp"
#include <cstddef>
#include <etl/array.h>
#include <ramen.hpp>

namespace breeze::shooter
{

/**
 * @brief 常驻的发射模块, 通过外部 Tick 输入驱动 Shooter, 不持有线程
 * @tparam FricCount 摩擦轮数量, 最多 6 路, 当前实例化 2、3、6 路
 * @note 应用中只构造一个实例, 构造后处于 Sleep; 输入使用各自的最新值.
 *       连接端口后由单一周期源串行推送 Tick, 控制在推送者上下文执行.
 */
template <std::size_t FricCount>
class Module
{
	static_assert(FricCount >= 1 && FricCount <= 6);

public:
	/**
	 * @brief 发射模块输入
	 * @note 用于存储发射模块的输入数据。
	 */
	struct Input
	{
		ramen::Pushable<float> Tick;             ///< 每次推送执行一个控制周期, 输入为实际间隔(ms)
		ramen::Pushable<breeze::models::FiringIntent> Command; ///< 默认为 Sleep; 由应用层决定唤醒、待命或发射
		ramen::Pushable<etl::array<float, FricCount>> FricTargetSpeeds; ///< 摩擦轮目标转速(rpm)，正值
		ramen::Pushable<float> FeedTargetSpeed;                         ///< 连发目标角速度(°/s), 正值，为零时暂停连发；
		etl::array<ramen::Puller<breeze::shooter::FricFeedback>, FricCount> FricFeedback{}; ///< 摩擦轮反馈
		ramen::Puller<breeze::shooter::FeedFeedback> FeedFeedback{};                        ///< 拨盘反馈
	};

	/**
	 * @brief 发射模块输出
	 * @note 用于存储发射模块的输出数据。
	 */
	struct Output
	{
		etl::array<ramen::Pusher<float>, FricCount> FricOutput{}; ///< 摩擦轮力矩输出(N·m)
		ramen::Pusher<float> FeedOutput{};                        ///< 拨盘力矩输出(N·m)
	};

	Input In;     ///< 输入接口
	Output Out{}; ///< 输出接口

	/**
	 * @brief 构造常驻内部实现并绑定数据端口
	 *
	 * @param config 发射控制配置
	 */
	explicit Module(const ShooterConfig<FricCount>& config);

	Module(const Module&) = delete;
	Module(Module&&) = delete;
	Module& operator=(const Module&) = delete;
	Module& operator=(Module&&) = delete;

private:
	struct Impl;
	Impl* _impl{}; ///< 指向常驻的静态实现
};

} // namespace breeze::shooter
