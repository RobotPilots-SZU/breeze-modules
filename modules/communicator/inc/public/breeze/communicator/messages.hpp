/**
 * @file messages.hpp
 * @brief communicator 模块的协议适配消息模型.
 */
#pragma once

#include <array>
#include <breeze/models/unit.hpp>
#include <breeze/models/utils.hpp>
#include <cstdint>
#include <optional>

namespace breeze::communicator::messages
{

enum class ModuleState : std::uint8_t
{
	Offline,
	Online,
};

enum class ModuleType : std::uint8_t
{
	AutoAim,
};

constexpr auto MODULE_TYPE_VALUES = [] {
	using enum ModuleType;
	return std::array{AutoAim};
}();

struct ModuleInfo
{
	std::optional<std::uint32_t> Index;
	ModuleState State{};
};

using ModuleInfos = models::IndexedEnumArray<ModuleType, std::optional<ModuleInfo>, MODULE_TYPE_VALUES>;

/**
 * @brief 吊射模式切换命令.
 */
struct DeployCommand
{
	bool Enter;
};

struct ChassisReport
{
	models::Meter<float> Speed;
};

struct LegReport
{
	std::uint32_t Height;
	models::Degree<float> SmallTheta;
	models::Degree<float> LargeTheta;
};

} // namespace breeze::communicator::messages
