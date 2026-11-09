/**
 * @file raw_convert.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 流式解析原始数据为pb字节流, 以及流式编码数据
 * @version 0.1
 * @date 2026-11-04
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <cstdint>
#include <etl/circular_buffer.h>
#include <etl/span.h>
#include <ramen.hpp>

namespace breeze::communicator::internal
{

struct RawDecoder
{
	/**
	 * @brief 原始数据输入
	 *
	 */
	ramen::Pushable<std::span<const std::uint8_t>> In = [this](const auto& in) { Accept(in); };
	/**
	 * @brief protobuf编码数据输出
	 *
	 */
	ramen::Pusher<std::span<const std::uint8_t>> Out;
	etl::span<std::uint8_t> Buffer;
	etl::icircular_buffer<std::uint8_t>& Raw;

private:
	void Accept(std::span<const std::uint8_t> data);
};

struct RawEncoder
{
	/**
	 * @brief protobuf数据输入
	 *
	 */
	ramen::Pushable<std::span<const std::uint8_t>> In = [this](const std::span<const std::uint8_t>& in) { Emit(in); };
	/**
	 * @brief 原始数据输出(流式, 不是一次性输出)
	 * @note 因为是流式输出, 所以这里仅作数据流动, 不能作为整包触发发送的参考.
	 *
	 */
	ramen::Pusher<std::span<const std::uint8_t>> Out;

private:
	void Emit(std::span<const std::uint8_t> in);
};
} // namespace breeze::communicator::internal
