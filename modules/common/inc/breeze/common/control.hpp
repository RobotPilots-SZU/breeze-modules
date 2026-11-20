/**
 * @file control.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 为控制模块设计的通用可复用框架
 * @version 0.1
 * @date 2026-11-14
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <breeze/models/unit.hpp>
#include <etl/delegate.h>
#include <ramen.hpp>
#include <zephyr/kernel.h>

namespace breeze::common
{

/**
 * @brief 控制算法委托
 * @tparam TTarget 控制目标
 * @tparam TFeedback 反馈输入
 * @tparam TOutput 控制输出
 */
template <typename TTarget, typename TFeedback, typename TOutput>
using ControlDelegate =
    etl::delegate<TOutput(const TTarget& target, const TFeedback& feedback, models::MilliSeconds<std::int32_t> delta)>;

/**
 * @brief 控制模块的通用抽象
 * @note 无论是控制协调, 还是原子控制组件, 都可以采用这个范式
 *
 * @tparam TTarget 控制目标
 * @tparam TFeedback 反馈输入
 * @tparam TOutput 控制输出
 */
template <typename TTarget, typename TFeedback, typename TOutput>
class ControlModuleBase
{
public:
	using ControlImpl = ControlDelegate<TTarget, TFeedback, TOutput>;

	/**
	 * @brief 目标值触发输入. 控制算法, 输出等等都在该推送线程执行.
	 *
	 */
	ramen::Pushable<TTarget> InTarget;
	/**
	 * @brief 反馈输入. 按需抓取
	 *
	 */
	ramen::Puller<TFeedback> InFeedback;
	/**
	 * @brief 控制输出
	 *
	 */
	ramen::Pusher<TOutput> Out;
	ControlModuleBase(ControlImpl control);
	/**
	 * @brief 启动
	 *
	 */
	void Start();
	/**
	 * @brief 重置
	 *
	 */
	void Reset();

private:
	ControlImpl _control;
	std::int64_t _lastTime = 0;
};

#pragma region Implement

template <typename TTarget, typename TFeedback, typename TOutput>
void ControlModuleBase<TTarget, TFeedback, TOutput>::Start()
{
	_lastTime = k_uptime_get();
}

template <typename TTarget, typename TFeedback, typename TOutput>
void ControlModuleBase<TTarget, TFeedback, TOutput>::Reset()
{
	_lastTime = k_uptime_get();
}

template <typename TTarget, typename TFeedback, typename TOutput>
ControlModuleBase<TTarget, TFeedback, TOutput>::ControlModuleBase(ControlImpl control)
    : InTarget([this](const TTarget& target) {
	      const auto& report = *InFeedback;

	      const auto rawTime = static_cast<std::int32_t>(k_uptime_delta(&_lastTime));
	      const models::MilliSeconds<std::int32_t> time{.Value = rawTime};

	      auto out = _control(target, report, time);

	      Out(out);
      }),
      _control(control)
{}

#pragma endregion
} // namespace breeze::common
