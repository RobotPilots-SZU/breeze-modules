/**
 * @file module.cpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief 发射模块静态 Impl、数据缓存与外部 Tick 控制接线
 * @version 0.1
 * @date 2026-11-17
 *
 * @copyright Copyright (c)  2026 RobotPilots-SZU.
 * @license SPDX-License-Identifier: Apache-2.0
 *
 */

#include <breeze/shooter/module.hpp>

#include "shooter.hpp"

#include <atomic>

namespace breeze::shooter
{
namespace
{
static_assert(
	std::atomic<breeze::models::FiringIntent>::is_always_lock_free && std::atomic<float>::is_always_lock_free
);
} // namespace

template <std::size_t FricCount>
struct Module<FricCount>::Impl
{
	Shooter<FricCount> Controller;
	std::atomic<breeze::models::FiringIntent> Command{breeze::models::FiringIntent::Sleep};
	etl::array<std::atomic<float>, FricCount> FricTargetSpeeds{};
	std::atomic<float> FeedTargetSpeed{};
	explicit Impl(const ShooterConfig<FricCount>& config)
	    : Controller(config)
	{}

	/**
	 * @brief 按外部时间间隔读取反馈、更新控制并推送力矩
	 *
	 */
	void Update(Module& module, float dtMs)
	{
		ShooterInput<FricCount> input{
		    .Command = Command.load(std::memory_order_relaxed),
		    .FeedTargetSpeed = FeedTargetSpeed.load(std::memory_order_relaxed)
		};
		for (std::size_t i = 0; i < FricCount; ++i)
		{
			input.FricTargetSpeeds[i] = FricTargetSpeeds[i].load(std::memory_order_relaxed);
			module.In.FricFeedback[i](input.FricFeedbacks[i]);
		}
		module.In.FeedFeedback(input.Feed);
		Controller.Update(input, dtMs);
		for (std::size_t i = 0; i < FricCount; ++i)
		{
			module.Out.FricOutput[i](Controller.GetFricOutput(i));
		}
		module.Out.FeedOutput(Controller.GetFeedOutput());
	}
};

template <std::size_t FricCount>
Module<FricCount>::Module(const ShooterConfig<FricCount>& config)
    : In{
          .Tick = [this](const float& dtMs) { _impl->Update(*this, dtMs); },
          .Command = [this](const breeze::models::FiringIntent& command) { _impl->Command.store(command, std::memory_order_relaxed); },
          .FricTargetSpeeds = [this](const etl::array<float, FricCount>& speeds) {
	          for (std::size_t i = 0; i < FricCount; ++i)
	          {
		          _impl->FricTargetSpeeds[i].store(speeds[i], std::memory_order_relaxed);
	          }
          },
          .FeedTargetSpeed =
              [this](const float& speed) { _impl->FeedTargetSpeed.store(speed, std::memory_order_relaxed); }
      }
{
	static Impl impl(config);
	_impl = &impl;
}

template class Module<2>;
template class Module<3>;
template class Module<6>;

} // namespace breeze::shooter
