#include "breeze/referee/module.h"
#include "conversion/processor.hpp"
#include "protocol/parser.h"

#include <etl/array.h>
#include <etl/circular_buffer.h>

#include <new>

namespace breeze::referee
{
namespace
{
struct ParserState
{
	etl::circular_buffer<uint8_t, CONFIG_BREEZE_MODULE_REFEREE_RING_BUF_SIZE> Ring{};
	etl::array<uint8_t, CONFIG_BREEZE_MODULE_REFEREE_LINEAR_BUF_SIZE> Linear{};
};

} // namespace

struct Module::Impl
{
	ParserState RefereeParser;
	ParserState VtParser;
	internal::State State;
	internal::Processor Processor;

	explicit Impl(Outputs& outputs)
	    : Processor(outputs, State)
	{}

	void OnRawPackage(RawPackage package)
	{
		protocol::rx::Receiver::Dispatch(package, [this](const auto& value) { Processor.Receive(value); });
	}

	void Feed(ParserState& parser, etl::span<const uint8_t> data)
	{
		StreamParser::Feed(
		    parser.Ring, {parser.Linear.begin(), parser.Linear.end()}, data,
		    StreamParser::Observer::create<Impl, &Impl::OnRawPackage>(*this)
		);
	}
};

Outputs Module::MakeOutputs()
{
	return Outputs{
	    .SelfId = {[this](auto& value) { value = _impl->State.SelfId; }},
	    .AllyPowers = {[this](auto& value) { value = _impl->State.AllyPowers; }},
	    .GameState = {[this](auto& value) { value = _impl->State.GameState; }},
	    .LogisticsInfo = {[this](auto& value) { value = _impl->State.LogisticsInfo; }},
	    .DartHitEvent = {{}},
	    .DartInfo = {[this](auto& value) { value = _impl->State.DartInfo; }},
	    .AllyHealths = {[this](auto& value) { value = _impl->State.AllyHealths; }},
	    .AllyLevels = {[this](auto& value) { value = _impl->State.AllyLevels; }},
	    .AllyHeats = {[this](auto& value) { value = _impl->State.AllyHeats; }},
	    .AllyAmmos = {[this](auto& value) { value = _impl->State.AllyAmmos; }},
	    .AllyBuffs = {[this](auto& value) { value = _impl->State.AllyBuffs; }},
	    .AllyPoses = {[this](auto& value) { value = _impl->State.AllyPoses; }},
	};
}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
Module::Module()
    : Out(MakeOutputs()),
      InReferee([this](const etl::span<const uint8_t>& data) { _impl->Feed(_impl->RefereeParser, data); }),
      InVT([this](const etl::span<const uint8_t>& data) { _impl->Feed(_impl->VtParser, data); }),
      _impl(new (_storage) Impl{Out})
{
	static_assert(sizeof(Impl) <= IMPL_SIZE);
	static_assert(alignof(Impl) <= alignof(std::max_align_t));
}

Module::~Module() { _impl->~Impl(); }

} // namespace breeze::referee
