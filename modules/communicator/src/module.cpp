#include "breeze/communicator/module.hpp"
#include "pb_convert.hpp"
#include "raw_convert.hpp"
#include <etl/circular_buffer.h>
#include <new>
#include <ramenx/logic.hpp>

namespace breeze::communicator
{
struct Module::Impl
{
	static constexpr std::size_t RAW_BUFFER_SIZE = 4096;
	static constexpr std::size_t FRAME_BUFFER_SIZE = 4096;
	Module& Owner;
	internal::InputState State{};
	etl::circular_buffer<std::uint8_t, RAW_BUFFER_SIZE> Raw{};
	std::array<std::uint8_t, FRAME_BUFFER_SIZE> Frame{};
	internal::RawDecoder RawDecoder{.Buffer = Frame, .Raw = Raw};
	internal::RawEncoder RawEncoder{};
	internal::PbDecoder PbDecoder;
	internal::PbEncoder PbEncoder;
	using InputCombine = ramen::Combine<
	    ramen::CombineTrigger::Any,
	    void,
	    models::RobotId,
	    messages::ModuleInfos,
	    messages::ChassisReport,
	    messages::LegReport,
	    models::MotionMode,
	    models::RobotInfos,
	    models::RobotInfos
	>;
	InputCombine Inputs{
	    [this](
	        const auto& id,
	        const auto& modules,
	        const auto& chassis,
	        const auto& leg,
	        const auto& motion,
	        const auto& ally,
	        const auto& enemy
	    ) {
		    State.Id = id;
		    State.Modules = modules;
		    State.ChassisReport = chassis;
		    State.LegReport = leg;
		    State.MotionMode = motion;
		    State.AllyRobotInfos = ally;
		    State.EnemyRobotInfos = enemy;
	    },
	    models::RobotId{},
	    messages::ModuleInfos{},
	    messages::ChassisReport{},
	    messages::LegReport{},
	    models::MotionMode{},
	    models::RobotInfos{},
	    models::RobotInfos{}
	};
	ramen::Pushable<std::span<const std::uint8_t>> RawOutput{[this](const auto& data) {
		Owner.OutRaw(std::span<std::uint8_t>{const_cast<std::uint8_t*>(data.data()), data.size()});
	}};

	explicit Impl(Module& owner)
	    : Owner(owner),
	      RawDecoder{.Buffer = Frame, .Raw = Raw},
	      PbDecoder{owner.Out},
	      PbEncoder{State}
	{
		owner.In.Id >> std::get<0>(Inputs.in);
		owner.In.Modules >> std::get<1>(Inputs.in);
		owner.In.ChassisReport >> std::get<2>(Inputs.in);
		owner.In.LegReport >> std::get<3>(Inputs.in);
		owner.In.MotionMode >> std::get<4>(Inputs.in);
		owner.In.AllyRobotInfos >> std::get<5>(Inputs.in);
		owner.In.EnemyRobotInfos >> std::get<6>(Inputs.in);
		Inputs.out >> PbEncoder.Trigger;
		RawEncoder.Out >> RawOutput;
		PbEncoder.Out >> RawEncoder.In;
		RawDecoder.Out >> PbDecoder.In;
	}
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
Module::Module()
    : InRaw(MakeRawInput(*this))
{
	static_assert(sizeof(Impl) <= IMPL_SIZE);
	new (_storage) Impl{*this};
}

ramen::Pushable<std::span<std::uint8_t>> Module::MakeRawInput(Module& owner)
{
	return ramen::Pushable<std::span<std::uint8_t>>{[&owner](const auto& data) {
		auto* impl = reinterpret_cast<Impl*>(owner._storage);
		impl->RawDecoder.In(std::span<const std::uint8_t>{data.data(), data.size()});
	}};
}

Module::~Module() { reinterpret_cast<Impl*>(_storage)->~Impl(); }

} // namespace breeze::communicator
