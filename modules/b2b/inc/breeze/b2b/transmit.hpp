/**
 * @file transmit.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief can 板通的传输实现
 * @version 0.1
 * @date 2026-11-18
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <cstdint>
#include <etl/span.h>
#include <ramen.hpp>
#include <zephyr/device.h>

namespace breeze::b2b
{
/**
 * @brief 经典CAN协议的一对一通信. 使用ISO-TP进行表现层数据传输.
 * @note 一对Transmitter服务于一种协议. 对于 B2B的结构体协议和RP-Proto, 通常是分别创建Transmitter
 *
 */
class Transmitter
{
public:
	/**
	 * @brief 发送触发点.
	 *
	 */
	ramen::Pushable<etl::span<const std::byte>> InSend;
	/**
	 * @brief 阻塞接收处理.
	 * @warning 只应该有一个线程处理, 不要连接多次本数据流
	 *
	 */
	ramen::Pullable<etl::span<const std::byte>> OutReceiveWait;

	void Start();

	struct Config
	{
		std::uint16_t RxDataId; ///< 接收普通数据的CAN ID
		std::uint16_t TxFcId;   ///< 为接收的数据回复FC的CAN ID
		std::uint16_t TxDataId; ///< 发送普通数据的CAN ID
		std::uint16_t RxFcId;   ///< 为发送的数据接收FC的CAN ID, 不得与RxDataId相同
		etl::span<std::byte> Buffer;
	};

	/**
	 * @brief Transmitter 构造
	 *
	 * @param dev can总线设备
	 * @param conf 两个方向的数据与FC地址配置
	 */
	explicit Transmitter(const device* dev, Config conf);
	~Transmitter();

	Transmitter(const Transmitter&) = delete;
	Transmitter(Transmitter&&) = delete;
	Transmitter& operator=(const Transmitter&) = delete;
	Transmitter& operator=(Transmitter&&) = delete;

private:
	struct Impl;
	static constexpr std::size_t IMPL_SIZE = 1024;
	alignas(std::max_align_t) std::byte _storage[IMPL_SIZE];
	Impl* P();
};
} // namespace breeze::b2b
