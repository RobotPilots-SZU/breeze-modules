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
	ramen::Pushable<std::span<std::uint8_t>> In;
	/**
	 * @brief protobuf编码数据输出
	 *
	 */
	ramen::Pusher<std::span<std::uint8_t>> Out;

	etl::span<std::uint8_t> LinearBuffer;
	etl::icircular_buffer<std::uint8_t> RingBuffer;
};

struct RawEncoder
{
	/**
	 * @brief protobuf数据输入
	 *
	 */
	ramen::Pushable<std::span<std::uint8_t>> In;
	/**
	 * @brief 原始数据输出
	 * @note 我觉得应该能减少拷贝??
	 *
	 */
	ramen::Pusher<std::span<std::uint8_t>> Out;
};
RawDecoder MkRawDecoder();
RawEncoder MkRawEncoder();
} // namespace breeze::communicator::internal
