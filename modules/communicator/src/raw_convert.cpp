#include "raw_convert.hpp"
#include <cstddef>
#include <etl/algorithm.h>
#include <etl/circular_buffer.h>
#include <etl/endianness.h>
#include <etl/span.h>

namespace
{

constexpr std::uint8_t SOF = 0xa7;
constexpr std::size_t LEN_BYTES = 3;
constexpr std::size_t HEADER_SIZE = sizeof(SOF) + LEN_BYTES;

struct DecodeResult
{
	/**
	 * @brief 应该前进的步数(丢弃缓冲区的大小)
	 *
	 */
	std::size_t Step;
	/**
	 * @brief 解析出的数据. {}表示无结果. 无结果时应该直接中断循环.
	 *
	 */
	etl::span<const std::uint8_t> Data;
};

DecodeResult DecoderStep(etl::span<std::uint8_t> buffer, const etl::icircular_buffer<std::uint8_t>& raw)
{
	const auto itSof = etl::find(raw.begin(), raw.end(), SOF);

	// 无数据或数据不足够帧头, 不丢数据, 继续等.
	if (itSof == raw.end() || static_cast<std::size_t>(raw.end() - itSof) < HEADER_SIZE)
	{
		return {};
	}

	const auto itHeader = itSof + 1;
	// clang-format off
	const auto len =
		  (static_cast<uint32_t>(itHeader[0]) << 16)
		| (static_cast<uint32_t>(itHeader[1]) << 8)
		| static_cast<uint32_t>(itHeader[2]);
	// clang-format on
	const auto itData = itSof + sizeof(SOF) + LEN_BYTES;

	// 再次查长度
	if (const auto avail = static_cast<std::size_t>(raw.end() - itData); avail < len)
	{
		return {};
	}

	// 没毛病, 直接让下游同步解. TODO: 目前用拷贝大法, 后续可以考虑用nanopb神秘特性减少拷贝, 别急.
	etl::copy(itData, itData + len, buffer.begin());
	return {.Step = static_cast<std::size_t>(itData + len - raw.begin()), .Data = buffer.subspan(HEADER_SIZE, len)};
}

} // namespace

namespace breeze::communicator::internal
{
void RawDecoder::Accept(std::span<const std::uint8_t> data)
{
	if (auto overflow = static_cast<int>(data.size()) - static_cast<int>(Raw.available()); overflow > 0)
	{
		Raw.pop(etl::min(static_cast<std::size_t>(overflow), Raw.size()));
	}

	Raw.push(data.begin(), data.end());
	auto itNext = Raw.begin();
	while (itNext != Raw.end())
	{
		auto result = DecoderStep(Buffer, Raw);
		if (!result.Data.empty())
		{
			Out(result.Data);
			itNext += result.Step;
		}
		else
		{
			break;
		}
	}
}
void RawEncoder::Emit(std::span<const std::uint8_t> in)
{
	const auto len = in.size();
	// clang-format off
	std::uint8_t header[HEADER_SIZE] =
	{
	    SOF,
		static_cast<std::uint8_t>(len >> 16),
		static_cast<std::uint8_t>(len >> 8),
	    static_cast<std::uint8_t>(len)
	};
	// clang-format on
	Out(std::span{header});
	Out(in);
}
} // namespace breeze::communicator::internal
