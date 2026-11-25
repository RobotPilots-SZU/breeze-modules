/**
 * @file utils.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 工具型通用类型
 * @version 0.1
 * @date 2026-10-26
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <array>
#include <cassert>
#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace breeze::models
{

/**
 * @brief 枚举静态查找表
 *
 * @tparam TEnum 枚举类型
 * @tparam TValue 目标值类型
 * @tparam TIds 表支持的枚举值
 * @tparam TAllValues 枚举所有可能的值
 * @par 参考代码示例
 * @code {.cpp}
 * struct RobotHealths : IndexedEnumArray<RobotShortId, HealthType, HEALTH_ROBOT_IDS, ROBOT_SHORT_ID_VALUES>
 * {};
 * @endcode
 *
 */
template <typename TEnum, typename TValue, auto TIds, auto TAllValues = TIds>
class IndexedEnumArray
{
	static constexpr std::size_t N = TAllValues.size();
	static constexpr std::size_t INVALID = -1;
	static_assert(TIds.size() > 0, "IndexedEnumArray requires at least one indexed ID");
	static_assert(N > 0, "IndexedEnumArray requires at least one enum value");

	static consteval bool ValidIds()
	{
		for (std::size_t i = 0; i < TAllValues.size(); ++i)
		{
			if (static_cast<std::size_t>(TAllValues[i]) >= N)
			{
				return false;
			}
			for (std::size_t j = 0; j < i; ++j)
			{
				if (TAllValues[j] == TAllValues[i])
				{
					return false;
				}
			}
		}
		for (std::size_t i = 0; i < TIds.size(); ++i)
		{
			const auto id = static_cast<std::size_t>(TIds[i]);
			if (id >= N)
			{
				return false;
			}
			for (std::size_t j = 0; j < i; ++j)
			{
				if (TIds[j] == TIds[i])
				{
					return false;
				}
			}
			bool found = false;
			for (const auto allId : TAllValues)
			{
				found |= allId == TIds[i];
			}
			if (!found)
			{
				return false;
			}
		}
		return true;
	}
	static_assert(ValidIds(), "IndexedEnumArray IDs and enum values must be unique, in range, and consistent");

	static constexpr auto MkIndexer()
	{
		std::array<std::size_t, N> table{};
		for (auto& x : table)
		{
			x = INVALID;
		}
		for (std::size_t i = 0; i < TIds.size(); ++i)
		{
			table[static_cast<std::size_t>(TIds[i])] = i;
		}
		return table;
	}

	static constexpr auto INDEXER = MkIndexer();
	std::array<TValue, TIds.size()> _values;

public:
	bool operator==(const IndexedEnumArray&) const = default;

	template <typename TFactory, std::size_t... TIndexes>
	static consteval bool ValidFactory(std::index_sequence<TIndexes...>)
	{
		return (requires(TFactory& factory) {
			{ std::invoke(factory, std::integral_constant<TEnum, TIds[TIndexes]>{}) } -> std::same_as<TValue>;
		} && ...);
	}

	template <typename TFactory, std::size_t... TIndexes>
	static constexpr std::array<TValue, sizeof...(TIndexes)> MkValues(
	    TFactory& factory,
	    std::index_sequence<TIndexes...>
	)
	{
		return {{std::invoke(factory, std::integral_constant<TEnum, TIds[TIndexes]>{})...}};
	}

	IndexedEnumArray()
	    requires std::default_initializable<TValue>
	= default;

	/**
	 * @brief 使用工厂对id进行逐个构造. id是编译期常量. Factory会接收泛型参数 TEnum类型的TId
	 * @code {.cpp}
	 * {[this]<typename TId>(TId) {
	        // ...
	 }}
	 * @endcode
	 *
	 */
	template <typename TFactory>
	    requires(
	        !std::same_as<std::remove_cvref_t<TFactory>, IndexedEnumArray> &&
	        ValidFactory<std::remove_reference_t<TFactory>>(std::make_index_sequence<TIds.size()>{})
	    )
	explicit constexpr IndexedEnumArray(TFactory&& factory)
	    : _values(MkValues(factory, std::make_index_sequence<TIds.size()>{}))
	{}

	/**
	 * @brief 根据枚举索引内容
	 *
	 * @param id 枚举
	 */
	TValue& operator[](TEnum id)
	{
		auto idx = INDEXER[static_cast<std::size_t>(id)];
		assert(idx != INVALID && "enum id not in Ids list");
		return _values[idx];
	}
	const TValue& operator[](TEnum id) const
	{
		auto idx = INDEXER[static_cast<std::size_t>(id)];
		assert(idx != INVALID && "enum id not in Ids list");
		return _values[idx];
	}
	/**
	 * @brief 根据枚举索引内容, 不存在则返回空指针
	 *
	 * @param id 枚举索引
	 * @return TValue* 指向的内存. 不存在则返回nullptr
	 */
	TValue* At(TEnum id)
	{
		auto idx = INDEXER[static_cast<std::size_t>(id)];
		return idx != INVALID ? &_values[idx] : nullptr;
	}
	const TValue* At(TEnum id) const
	{
		auto idx = INDEXER[static_cast<std::size_t>(id)];
		return idx != INVALID ? &_values[idx] : nullptr;
	}
};
} // namespace breeze::models
