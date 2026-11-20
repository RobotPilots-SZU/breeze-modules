#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>

namespace breeze::b2b::details
{
/**
 * @brief 协议支持的消息. 需要持有uint8类型的`ID`编译常量.
 */
template <typename T>
concept Message = requires {
	{ T::ID } -> std::same_as<const std::uint8_t&>;
};

/**
 * @brief 协议消息Id的独立性校验. 拦截重复Id.
 */
template <typename... Ts>
concept UniqueIds = []() consteval -> bool {
	constexpr std::uint8_t IDS[] = {Ts::ID...};
	for (std::size_t i = 0; i < sizeof...(Ts); ++i)
	{
		for (std::size_t j = i + 1; j < sizeof...(Ts); ++j)
		{
			if (IDS[i] == IDS[j])
			{
				return false;
			}
		}
	}
	return true;
}();

template <typename T, typename... Ts>
concept MessageOf = (std::same_as<T, Ts> || ...);
template <typename... TMsgs>
concept ValidMessages = (Message<TMsgs> && ...) && UniqueIds<TMsgs...>;

template <typename THandler, typename TMsg>
using HandleMember = void (THandler::*)(const TMsg&);

template <typename THandler, typename TMsg>
concept HasExactHandle = requires {
	static_cast<HandleMember<THandler, TMsg>>(&THandler::Handle);
};

} // namespace breeze::b2b::details
