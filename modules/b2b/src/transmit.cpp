#include "zephyr/kernel.h"
#include <breeze/b2b/transmit.hpp>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <etl/delegate.h>
#include <etl/span.h>
#include <memory>
#include <new>
#include <ramenx/async.hpp>
#include <zephyr/canbus/isotp.h>
#include <zephyr/device.h>

namespace breeze::b2b
{

struct Transmitter::Impl
{
	const device* Dev;
	isotp_send_ctx SendCtx{};
	isotp_recv_ctx RecvCtx{};
	isotp_msg_id TxAddr;
	isotp_msg_id RxAddr;
	isotp_fc_opts FcOpts{.bs = 8, .stmin = 5};
	etl::array<std::byte, 512> RxBuffer;

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
	Impl(const device* dev, std::uint16_t selfId, std::uint16_t targetId)
	    : Dev(dev),
	      TxAddr({.std_id = targetId & static_cast<std::uint16_t>(0x7FFu), .flags = ISOTP_MSG_EXT_ADDR}),
	      RxAddr({.std_id = selfId & static_cast<std::uint16_t>(0x7FFu), .flags = ISOTP_MSG_EXT_ADDR})
	{
		assert(dev != nullptr);
		assert(device_is_ready(dev));
	}

	void Start()
	{
		{
			auto ret = can_start(Dev);
			assert(ret == 0 || ret == -EALREADY);
		}
		{
			auto ret = isotp_bind(&RecvCtx, Dev, &RxAddr, &TxAddr, &FcOpts, K_FOREVER);
			assert(ret == 0);
		}
	}

	void Send(etl::span<const std::byte> payload)
	{
		auto data = reinterpret_cast<const std::uint8_t*>(payload.data());
		isotp_send(&SendCtx, Dev, data, payload.size(), &TxAddr, &RxAddr, nullptr, nullptr);
	}

	etl::span<const std::byte> Receive()
	{
		auto& buffer = RxBuffer;
		auto received = isotp_recv(&RecvCtx, reinterpret_cast<std::uint8_t*>(buffer.data()), buffer.size(), K_FOREVER);
		if (received > ISOTP_N_OK)
		{
			return {buffer.data(), static_cast<std::size_t>(received)};
		}

		return {};
	}
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
Transmitter::Transmitter(const device* dev, Config conf)
    : InSend([this](const etl::span<const std::byte>& info) { P()->Send(info); }),
      OutReceiveWait([this](auto& out) { out = P()->Receive(); })
{
	static_assert(sizeof(Impl) <= IMPL_SIZE);
	std::construct_at(P(), dev, conf.SelfId, conf.TargetId);
}

Transmitter::Impl* Transmitter::P() { return std::launder(reinterpret_cast<Impl*>(_storage)); }

} // namespace breeze::b2b
