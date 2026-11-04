#pragma once

#include <breeze/communicator/module.hpp>
#include <cstdint>
#include <ramen.hpp>
#include <span>

namespace breeze::communicator::internal
{
struct PbDecoder
{
	/**
	 * @brief pb数据输入
	 *
	 */
	ramen::Pushable<std::span<std::uint8_t>> In;
	/**
	 * @brief model数据输出
	 *
	 */
	Output& Out;
};

struct PbEncoder
{
	/**
	 * @brief model数据输入
	 *
	 */
	Input& In;
	/**
	 * @brief pb编码数据输出
	 *
	 */
	ramen::Pusher<std::span<std::uint8_t>> Out;
};
PbDecoder MkPbDecoder(Output& out);
PbEncoder MkPbEncoder(Input& in);
} // namespace breeze::communicator::internal
