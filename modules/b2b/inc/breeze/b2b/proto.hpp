/**
 * @file proto.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 板间定长通信协议基本框架
 * @note
 * @version 0.1
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "_proto_details.hpp"
#include <cstddef>
#include <cstring>
#include <etl/delegate.h>
#include <etl/ranges.h>
#include <etl/span.h>
#include <utility>
namespace breeze::b2b
{

/**
 * @brief 表示定长协议支持的所有消息. 纯编译期标记类型.
 *
 * @tparam TMsgs 所有消息类型
 */
template <details::ValidMessages... TMsgs>
    requires(sizeof...(TMsgs) > 0 && details::UniqueIds<TMsgs...>)
struct AvailableMessages
{
	template <typename TMsg>
	static consteval bool Contains()
	{
		return details::MessageOf<TMsg, TMsgs...>;
	}
};

template <details::Message TMsg>
using Callback = etl::delegate<void(const TMsg&)>;

template <details::ValidMessages... TMsgs>
using CallbackTable = std::tuple<Callback<TMsgs>...>;

template <typename TAvailable>
class Encoder;

template <typename... TMsgs>
class Encoder<AvailableMessages<TMsgs...>>
{
public:
	template <details::Message TMsg>
	    requires(details::MessageOf<TMsg, TMsgs...>)
	/**
	 * @brief 编码数据到缓冲区上
	 *
	 * @param msg 要编码的消息
	 * @param buffer 缓冲区. 通常由调用方在栈上创建数组
	 * @return etl::span<const std::byte> 编码后数据. 只要不进断言就会成功.
	 */
	etl::span<const std::byte> Encode(const TMsg& msg, etl::span<std::byte> buffer)
	{
		constexpr std::uint8_t ID = TMsg::ID;
		assert(buffer.size() > sizeof(TMsg) + sizeof(ID));

		std::memcpy(buffer.data(), &ID, sizeof(ID));
		std::memcpy(buffer.data() + sizeof(ID), &msg, sizeof(TMsg));

		return {reinterpret_cast<std::byte*>(buffer.data()), sizeof(ID) + sizeof(TMsg)};
	}
};

template <details::Message... TMsgs>
    requires(sizeof...(TMsgs) > 0 && details::UniqueIds<TMsgs...>)
class Decoder
{
public:
	using Callbacks = CallbackTable<TMsgs...>;

	template <typename TAvailable>
	    requires(details::HandledMessagesAreAvailable<TAvailable, TMsgs...>)
	explicit Decoder(Callbacks callbacks)
	    : _callbacks(callbacks)
	{}

	/**
	 * @brief 根据处理表遍历查找并处理.
	 * @note 代码相当于手写一个大switch, 只是自动化了
	 *
	 * @param transfer 传输层数据
	 * @return bool 该消息是否支持被处理
	 */
	bool Decode(etl::span<const std::byte> data) { return DecodeImpl(data, std::index_sequence_for<TMsgs...>{}); };

private:
	Callbacks _callbacks;

	template <std::size_t... Is>
	bool DecodeImpl(etl::span<const std::byte> transfer, std::index_sequence<Is...>)
	{
		return (TryDecodeSingle<Is, TMsgs>(transfer) || ...);
	}

	template <std::size_t Index, typename TMsg>
	bool TryDecodeSingle(etl::span<const std::byte> transfer)
	{
		constexpr std::uint8_t ID = TMsg::ID;
		// 长度不足直接停止解码
		if (transfer.size_bytes() < sizeof(ID))
		{
			return true;
		}
		if (auto id = *reinterpret_cast<const decltype(ID)*>(transfer.data()); id == ID)
		{
			if (transfer.size_bytes() != sizeof(ID) + sizeof(TMsg))
			{
				const auto& msg = *reinterpret_cast<const TMsg*>(transfer.data() + sizeof(ID));
				std::get<Index>(_callbacks)(msg);
			}
			return true;
		}
		return false;
	};

	/**
	 * @brief 构造 `Decoder`. 充分利用编译器推断.
	 * @code {.cpp}
	 *
	    #pragma pack(push, 1)
	    struct MsgA
	    {
	        static constexpr std::uint8_t ID = 0x10;
	        float MotorTorque;
	    };
	    struct MsgB
	    {
	        static constexpr std::uint8_t ID = 0x11;
	        float Yaw;
	    };

	    struct MsgC
	    {
	        static constexpr std::uint8_t ID = 0x12;
	        float Pitch;
	    };

	    #pragma pack(pop)

	    using Avail = breeze::b2b::AvailableMessages<MsgA, MsgB>;

	    breeze::b2b::Encoder<Avail> encoder;
	    auto cb_a = [](const MsgA&) {};
	    auto cb_b = [](const MsgB&) {};
	    auto decoder = breeze::b2b::MkDecoder<Avail>(std::tuple{
	        etl::make_delegate(cb_a),
	        etl::make_delegate(cb_b),
	    });

	 * @endcode
	 *
	 *
	 * @tparam TAvailable
	 * @tparam THandledMsgs
	 */
	template <typename TAvailable, details::Message... THandledMsgs>
	    requires(details::HandledMessagesAreAvailable<TAvailable, THandledMsgs...>)
	Decoder<THandledMsgs...> MkDecoder(CallbackTable<THandledMsgs...> callbacks)
	{
		return {callbacks};
	}
};

} // namespace breeze::b2b
