// NOLINTBEGIN(readability-magic-numbers)
#include <breeze/b2b/proto.hpp>
#include <zephyr/ztest.h>

#include <array>
#include <cstddef>
#include <cstdint>

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
	operator MessageA() const { return {.Value = Value}; }
};

using Messages = breeze::b2b::AvailableMessages<MessageA, MessageB>;

struct Handler
{
	MessageA LastA{};
	MessageB LastB{};
	void Handle(const MessageA& msg) { LastA = msg; }
	void Handle(const MessageB& msg) { LastB = msg; }
};

struct PartialHandler
{
	int Calls = 0;
	void Handle(const MessageA&) { ++Calls; }
};

ZTEST(b2b, encode_and_decode_round_trip)
{
	breeze::b2b::Encoder<Messages> encoder;
	std::array<std::byte, sizeof(MessageA) + 1> buffer{};
	const MessageA input{.Value = 0x12345678};
	const auto encoded = encoder.Encode(input, buffer);

	zassert_equal(encoded.size(), sizeof(MessageA) + 1);
	zassert_equal(std::to_integer<std::uint8_t>(encoded[0]), MessageA::ID);

	Handler handler;
	const breeze::b2b::Decoder<Messages> decoder;
	zassert_true(decoder.Decode(encoded, handler));
	zassert_equal(handler.LastA.Value, input.Value);

	const MessageB input_b{.Value = 0x5678};
	zassert_true(decoder.Decode(encoder.Encode(input_b, buffer), handler));
	zassert_equal(handler.LastB.Value, input_b.Value);
}

ZTEST(b2b, decode_rejects_wrong_length_and_unknown_id)
{
	PartialHandler handler;
	const breeze::b2b::Decoder<Messages> decoder;
	std::array<std::byte, sizeof(MessageA) + 2> data{};
	data[0] = static_cast<std::byte>(MessageA::ID);

	zassert_false(decoder.Decode({}, handler));
	zassert_false(decoder.Decode({data.data(), 1}, handler));
	zassert_false(decoder.Decode(data, handler));
	data[0] = static_cast<std::byte>(0x7F);
	zassert_false(decoder.Decode({data.data(), sizeof(MessageA) + 1}, handler));
	zassert_equal(handler.Calls, 0);
}

ZTEST(b2b, decode_requires_an_exact_handle)
{
	breeze::b2b::Encoder<Messages> encoder;
	std::array<std::byte, sizeof(MessageB) + 1> buffer{};
	const auto encoded = encoder.Encode(MessageB{.Value = 42}, buffer);
	const breeze::b2b::Decoder<Messages> decoder;

	struct EmptyHandler {} empty_handler;
	zassert_false(decoder.Decode(encoded, empty_handler));

	// MessageB converts to MessageA, but this must not call Handle(const MessageA&).
	PartialHandler handler;
	zassert_false(decoder.Decode(encoded, handler));
	zassert_equal(handler.Calls, 0);
}
} // namespace

ZTEST_SUITE(b2b, NULL, NULL, NULL, NULL, NULL);

// NOLINTEND(readability-magic-numbers)
