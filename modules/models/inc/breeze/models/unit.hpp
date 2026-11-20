/**
 * @file unit.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 对数值 "单位" 的语义建模
 * @version 0.1
 * @date 2026-10-25
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <cstdint>

namespace breeze::models
{

template <typename T>
struct ValueType
{
	T Value;
	bool operator==(const ValueType&) const = default;
};

/**
 * @brief 整数百分比
 *
 */
struct Percentage
{
	std::int8_t Value; ///< 整数值, 可正可负, 绝对值 <= 100
};

/**
 * @brief 时间, 单位秒
 */
template <typename T>
struct Seconds : ValueType<T>
{};

/**
 * @brief 能量，单位焦耳
 */
template <typename T>
struct Joule
{
	T Value;
};

/**
 * @brief 功率, 单位瓦特
 */
template <typename T>
struct Watt
{
	T Value;
};

/**
 * @brief 角度. 不加说明则规定为顺时针转动, 范围 0~360度.
 *
 * @tparam T
 */
template <typename T>
struct Degree
{
	T Value;
};

/**
 * @brief 弧度. 不佳说明则规定为逆时针转动, 单位radian.
 *
 * @tparam T
 */
template <typename T>
struct Radian
{
	T Value;
};

/**
 * @brief 赫兹. 频率单位 (次/秒).
 *
 */
template <typename T>
struct Hertz
{
	T Value;
};

/**
 * @brief 米. 距离单位
 *
 * @tparam T
 */
template <typename T>
struct Meter
{
	T Value;
};

/**
 * @brief 2D史亮
 *
 */
template <typename T>
struct Vec2
{
	T X, Y;
};

/**
 * @brief 3D史亮
 *
 */
template <typename T>
struct Vec3
{
	T X, Y, Z;
};

/**
 * @brief Unix时间戳
 *
 */
struct TimeStamp
{
	std::uint64_t Value;
};

/**
 * @brief 姿态
 *
 */
struct Posture
{
	/**
	 * @brief 三个轴方向的角度. 单位: 度
	 * @par 参数极性的定义
	 *
	 * Pitch: 抬头为正, 水平线是0.
	 * Roll: 视线平面顺时针(头往右歪)为正, 不歪是0.
	 * Yaw: 头往左扭为正. 正前方为0.
	 */
	Degree<float> Pitch, Roll, Yaw;
};

/**
 * @brief 转速 (转每分钟)
 *
 * @tparam T
 */
template <typename T>
struct Rpm : ValueType<T>
{};

/**
 * @brief 温度单位 (摄氏度)
 *
 * @tparam T
 */
template <typename T>
struct Celsius
{};

template <typename T>
using Temperature = Celsius<T>;

/**
 * @brief 扭矩单位 (牛米)
 *
 * @tparam T
 */
template <typename T>
struct NewtonMeter : ValueType<T>
{};

// TODO: 常量转换函数

} // namespace breeze::models
