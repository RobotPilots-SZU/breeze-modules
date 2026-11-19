// NOLINTBEGIN(readability-magic-numbers)
#include <breeze/b2b/proto.hpp>
#include <zephyr/ztest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <tuple>

namespace
{
struct MessageA
{
	static constexpr std::uint8_t ID = 0x31;
	std::uint32_t Value;
};

struct MessageB
{
	static constexpr std::uint8_t ID = 0x42;
	std::uint16_t Value;
};

using Messages = breeze::b2b::AvailableMessages<MessageA, MessageB>;

ZTEST(b2b, encode_and_decode_round_trip)
{
	breeze::b2b::Encoder<Messages> encoder;
	std::array<std::byte, sizeof(MessageA) + 1> buffer{};
	const MessageA input{.Value = 0x12345678};
	const auto encoded = encoder.Encode(input, buffer);

	zassert_equal(encoded.size(), sizeof(MessageA) + 1);
	zassert_equal(std::to_integer<std::uint8_t>(encoded[0]), MessageA::ID);

	MessageA decoded{};
	auto callback = [&decoded](const MessageA& message) { decoded = message; };
	breeze::b2b::Decoder<MessageA> decoder(std::tuple{etl::make_delegate(callback)});
	zassert_true(decoder.Decode(encoded));
	zassert_equal(decoded.Value, input.Value);
}

ZTEST(b2b, decode_rejects_wrong_length_and_unknown_id)
{
	int calls = 0;
	auto callback = [&calls](const MessageA&) { ++calls; };
	breeze::b2b::Decoder<MessageA> decoder(std::tuple{etl::make_delegate(callback)});
	std::array<std::byte, sizeof(MessageA) + 2> data{};
	data[0] = static_cast<std::byte>(MessageA::ID);

	zassert_false(decoder.Decode({data.data(), 1}));
	zassert_false(decoder.Decode(data));
	data[0] = static_cast<std::byte>(0x7F);
	zassert_false(decoder.Decode({data.data(), sizeof(MessageA) + 1}));
	zassert_equal(calls, 0);
}
} // namespace

ZTEST_SUITE(b2b, NULL, NULL, NULL, NULL, NULL);

// NOLINTEND(readability-magic-numbers)
