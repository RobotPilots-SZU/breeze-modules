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
 * @tparam TFeedback 反馈输入
 * @tparam TTarget 控制目标
 * @tparam TOutput 控制输出
 */
template <typename TFeedback, typename TTarget, typename TOutput>
using ControlDelegate =
    etl::delegate<TOutput(const TTarget& target, const TFeedback& feedback, models::MilliSeconds<std::int32_t> delta)>;

/**
 * @brief 控制模块的通用抽象
 * @note 无论是控制协调, 还是原子控制组件, 都可以采用这个范式
 *
 * @tparam TFeedback 反馈输入
 * @tparam TTarget 控制目标
 * @tparam TOutput 控制输出
 */
template <typename TFeedback, typename TTarget, typename TOutput>
class ControlModuleBase
{
public:
	using ControlImpl = ControlDelegate<TFeedback, TTarget, TOutput>;

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
	 * @brief 显式启动方法
	 *
	 */
	void Start();

private:
	static constexpr std::int64_t INVALID_TIME = -1;
	ControlImpl _control;
	std::int64_t _lastTime = INVALID_TIME;
};

#pragma region Implement

template <typename TFeedback, typename TTarget, typename TOutput>
void ControlModuleBase<TFeedback, TTarget, TOutput>::Start()
{
	_lastTime = k_uptime_get();
}

template <typename TFeedback, typename TTarget, typename TOutput>
ControlModuleBase<TFeedback, TTarget, TOutput>::ControlModuleBase(ControlImpl control)
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
