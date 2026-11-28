/**
 * @file shooter.cpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief Shooter 唤醒准备、命令执行与摩擦轮清堵协调实现
 * @version 0.1
 * @date 2026-11-17
 *
 * @copyright Copyright (c)  2026 RobotPilots-SZU.
 * @license SPDX-License-Identifier: Apache-2.0
 *
 */

#include "shooter.hpp"

#include <algorithm>

namespace breeze::shooter
{
template <std::size_t FricCount>
Shooter<FricCount>::Shooter(const ShooterConfig<FricCount>& conf)
    : Shooter(conf, std::make_index_sequence<FricCount>{})
{}

template <std::size_t FricCount>
template <std::size_t... Indices>
Shooter<FricCount>::Shooter(const ShooterConfig<FricCount>& conf, std::index_sequence<Indices...>)
    : _fricReverseSpeed(conf.FricReverseSpeed),
      _fric{{Fric(conf.FricConf[Indices])...}},
      _feed(conf.FeedConf)
{}

template <std::size_t FricCount>
void Shooter<FricCount>::Update(const ShooterInput<FricCount>& input, float dtMs)
{
	if (input.Command == breeze::models::FiringIntent::Sleep)
	{
		Reset();
		return;
	}
	const bool singleRequested =
	    input.Command == breeze::models::FiringIntent::Once && _controlInfo.LastCommand != breeze::models::FiringIntent::Once;
	_controlInfo.LastCommand = input.Command;
	if (_controlInfo.State == ShootState::Sleep)
	{
		_controlInfo.State = ShootState::Init;
	}

	UpdateFric(input.FricFeedbacks, input.FricTargetSpeeds, dtMs);
	const bool fricReady =
	    std::all_of(_fric.begin(), _fric.end(), [](const Fric& fric) { return fric.GetStatus().Ready; });
	FeedCmd command{};
	if (_controlInfo.State == ShootState::Running && fricReady)
	{
		if (singleRequested && _feed.GetStatus().State == FeedState::Running)
		{
			command.Command = FeedCommand::Single;
		}
		else if (input.Command == breeze::models::FiringIntent::Continuosly && input.FeedTargetSpeed > 0.0f)
		{
			command.Command = FeedCommand::Repeat;
			command.TargetSpeed = input.FeedTargetSpeed;
		}
	}
	_feed.Update(_controlInfo.State, command, input.Feed, dtMs);
	if (_controlInfo.State == ShootState::Init && fricReady && _feed.GetStatus().State == FeedState::Running &&
	    _feed.IsPositionReached(input.Feed.Angle))
	{
		_controlInfo.State = ShootState::Running;
	}
}

template <std::size_t FricCount>
void Shooter<FricCount>::UpdateFric(
    const etl::array<FricFeedback, FricCount>& feedbacks,
    const etl::array<float, FricCount>& targetSpeeds,
    float dtMs
)
{
	if (_controlInfo.FricReversing)
	{
		_controlInfo.FricReverseElapsedMs += dtMs;
		if (_controlInfo.FricReverseElapsedMs >= 500.0f)
		{
			_controlInfo.FricReversing = false;
			_controlInfo.FricReverseElapsedMs = 0.0f;
		}
	}
	if (!_controlInfo.FricReversing && std::any_of(feedbacks.begin(), feedbacks.end(), [](const FricFeedback& feedback) {
		    return feedback.Stuck;
	    }))
	{
		_controlInfo.FricReversing = true;
		_controlInfo.FricReverseElapsedMs = 0.0f;
	}
	for (std::size_t i = 0; i < FricCount; ++i)
	{
		const FricCmd command{
		    .Mode = _controlInfo.FricReversing ? FricState::Reverse : FricState::Running,
		    .TargetSpeed = _controlInfo.FricReversing ? _fricReverseSpeed : targetSpeeds[i]
		};
		_fric[i].Update(_controlInfo.State, command, feedbacks[i], dtMs);
	}
}

template <std::size_t FricCount>
void Shooter<FricCount>::Reset()
{
	_controlInfo = {};
	_feed.Reset();
	for (auto& fric : _fric)
	{
		fric.Reset();
	}
}

template <std::size_t FricCount>
const FricInfo& Shooter<FricCount>::GetFricStatus(std::size_t index) const
{
	return _fric[index].GetStatus();
}

template <std::size_t FricCount>
float Shooter<FricCount>::GetFricOutput(std::size_t index) const
{
	return _fric[index].GetOutput();
}

template <std::size_t FricCount>
const FeedInfo& Shooter<FricCount>::GetFeedStatus() const
{
	return _feed.GetStatus();
}

template <std::size_t FricCount>
float Shooter<FricCount>::GetFeedOutput() const
{
	return _feed.GetOutput();
}

template class Shooter<2>;
template class Shooter<3>;
template class Shooter<6>;

} // namespace breeze::shooter
