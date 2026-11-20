/**
 * @file proto.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 用于板间通信的定长结构体通信协议. 协议名称为 B2B Cast Proto.
 * @note
 * @version 0.1
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "_proto_details.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <etl/span.h>
namespace breeze::b2b
{

/**
 * @brief 表示定长协议支持的所有消息. 纯编译期标记类型.
 * @note 推荐结合 using 使用, 为消息集合起一个名称, 方便Encoder与Decoder共用.
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
		assert(buffer.size() >= sizeof(TMsg) + sizeof(ID));

		std::memcpy(buffer.data(), &ID, sizeof(ID));
		std::memcpy(buffer.data() + sizeof(ID), &msg, sizeof(TMsg));

		return {reinterpret_cast<std::byte*>(buffer.data()), sizeof(ID) + sizeof(TMsg)};
	}
};

template <typename TAvailable>
class Decoder;

template <typename... TMsgs>
class Decoder<AvailableMessages<TMsgs...>>
{
public:
	/**
	 * @brief 解码一帧并同步调用匹配的处理函数.
	 * @note Handle handle如果有函数 `void Handle(const TMsg&)`, 则认为成功处理了消息 `TMsg`. 目前按顺序根据id查找(相当于手写switch)
	 *
	 * @param data 传输层数据
	 * @return bool 帧有效且调用了匹配的 Handle 时返回 true
	 */
	template <typename THandler>
	bool Decode(etl::span<const std::byte> data, THandler& handler) const
	{
		if (data.empty())
		{
			return false;
		}
		const auto id = std::to_integer<std::uint8_t>(data[0]);
		return (TryDecode<TMsgs>(id, data, handler) || ...);
	}

private:
	template <typename TMsg, typename THandler>
	static bool TryDecode(std::uint8_t id, etl::span<const std::byte> data, THandler& handler)
	{
		if constexpr (!details::HasExactHandle<THandler, TMsg>)
		{
			return false;
		}
		else
		{
			if (id != TMsg::ID || data.size_bytes() != sizeof(id) + sizeof(TMsg))
			{
				return false;
			}
			TMsg msg{};
			std::memcpy(&msg, data.data() + sizeof(id), sizeof(TMsg));
			constexpr auto HANDLE = static_cast<details::HandleMember<THandler, TMsg>>(&THandler::Handle);
			(handler.*HANDLE)(msg);
			return true;
		}
	}
};

} // namespace breeze::b2b
