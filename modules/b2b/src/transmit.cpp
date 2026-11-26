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
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(breeze_b2b_transmit);

namespace breeze::b2b
{

struct Transmitter::Impl
{
	const device* Dev;
	isotp_send_ctx SendCtx{};
	isotp_recv_ctx RecvCtx{};
	isotp_msg_id RxDataAddr;
	isotp_msg_id TxFcAddr;
	isotp_msg_id TxDataAddr;
	isotp_msg_id RxFcAddr;
	isotp_fc_opts FcOpts{.bs = 8, .stmin = 5};
	etl::array<std::byte, 512> RxBuffer;

	static isotp_msg_id Address(std::uint16_t id)
	{
		isotp_msg_id address{.flags = ISOTP_MSG_EXT_ADDR};
		address.std_id = id;
		return address;
	}

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
	Impl(const device* dev, Config conf)
	    : Dev(dev),
	      RxDataAddr(Address(conf.RxDataId)),
	      TxFcAddr(Address(conf.TxFcId)),
	      TxDataAddr(Address(conf.TxDataId)),
	      RxFcAddr(Address(conf.RxFcId))
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
			auto ret = isotp_bind(&RecvCtx, Dev, &RxDataAddr, &TxFcAddr, &FcOpts, K_FOREVER);
			assert(ret == 0);
		}
	}

	void Send(etl::span<const std::byte> payload)
	{
		auto data = reinterpret_cast<const std::uint8_t*>(payload.data());
		const auto ret = isotp_send(&SendCtx, Dev, data, payload.size(), &TxDataAddr, &RxFcAddr, nullptr, nullptr);
		if (ret != ISOTP_N_OK)
		{
			LOG_ERR("ISO-TP send failed: %d", ret);
		}
	}

	etl::span<const std::byte> Receive()
	{
		auto& buffer = RxBuffer;
		std::size_t total = 0;
		bool oversized = false;
		// NOLINTNEXTLINE(cppcoreguidelines-init-variables)
		int remaining;
		do
		{
			net_buf* chunk = nullptr;
			remaining = isotp_recv_net(&RecvCtx, &chunk, K_FOREVER);
			if (remaining < 0)
			{
				LOG_ERR("ISO-TP receive failed: %d", remaining);
				return {};
			}
			const auto length = net_buf_frags_len(chunk);
			oversized = oversized || length > buffer.size() - total;
			if (!oversized)
			{
				total += net_buf_linearize(buffer.data() + total, buffer.size() - total, chunk, 0, length);
			}
			net_buf_unref(chunk);
		} while (remaining != 0);

		if (oversized)
		{
			LOG_ERR("ISO-TP message exceeds receive buffer");
			return {};
		}
		return {buffer.data(), total};
	}
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
Transmitter::Transmitter(const device* dev, Config conf)
    : InSend([this](const etl::span<const std::byte>& info) { P()->Send(info); }),
      OutReceiveWait([this](auto& out) { out = P()->Receive(); })
{
	static_assert(sizeof(Impl) <= IMPL_SIZE);
	std::construct_at(P(), dev, conf);
}

Transmitter::Impl* Transmitter::P() { return std::launder(reinterpret_cast<Impl*>(_storage)); }
void Transmitter::Start() { P()->Start(); }

} // namespace breeze::b2b
