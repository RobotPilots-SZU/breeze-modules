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
};
} // namespace breeze::models
