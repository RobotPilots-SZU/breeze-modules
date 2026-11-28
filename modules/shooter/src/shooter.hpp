/**
 * @file shooter.hpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief 发射命令、目标速度与机械动作协调接口
 * @version 0.1
 * @date 2026-11-02
 * @note 根据应用层命令和目标速度协调唤醒准备、单发、连发与整组摩擦轮清堵.
 * @copyright Copyright (c)  2026 RobotPilots-SZU.
 * @license SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "feed.hpp"
#include "fric.hpp"
#include "state.hpp"
#include <breeze/models/control.hpp>
#include <breeze/shooter/config.hpp>
#include <breeze/shooter/io.hpp>
#include <cstddef>
#include <etl/array.h>
#include <utility>

namespace breeze::shooter
{

/**
 * @brief 本周期命令、目标速度与设备反馈, Update 不修改或保存其副本
 *
 */
template <std::size_t FricCount>
struct ShooterInput
{
	breeze::models::FiringIntent Command{breeze::models::FiringIntent::Sleep}; ///< Sleep 卸力, 其它命令唤醒并准备拨弹
	etl::array<float, FricCount> FricTargetSpeeds{}; ///< 各轮目标转速大小(rpm), 非负
	float FeedTargetSpeed{}; ///< 连发目标角速度大小(°/s), 非负, 零值暂停连发
	FeedFeedback Feed{}; ///< 拨盘轴原始反馈与设备层堵转状态
	etl::array<FricFeedback, FricCount> FricFeedbacks{}; ///< 各摩擦轮原始反馈与设备层堵转状态
};

/**
 * @brief 发射模块跨周期控制状态
 *
 */
struct ShooterControlInfo
{
	ShootState State{ShootState::Sleep}; ///< 整机准备阶段
	breeze::models::FiringIntent LastCommand{breeze::models::FiringIntent::Sleep}; ///< 上一周期命令, 用于单发边沿检测
	bool FricReversing{}; ///< 整组摩擦轮是否正在反转清堵
	float FricReverseElapsedMs{}; ///< 本次摩擦轮反转已持续时间(ms)
};

/**
 * @brief 集成摩擦轮与拨盘的发射控制器
 * @note 目标速度和发射决策由应用层提供, 本类只协调机械动作.
 *       仅实例化 2、3、6 路, 模板实现在 shooter.cpp 中独立编译.
 * @tparam FricCount 摩擦轮数量
 */
template <std::size_t FricCount>
class Shooter
{
public:
	/**
	 * @brief 构造函数
	 *
	 * @param conf 摩擦轮、拨盘及整组清堵配置
	 */
	explicit Shooter(const ShooterConfig<FricCount>& conf);

	/**
	 * @brief 更新整机生命周期、摩擦轮与拨盘控制
	 *
	 * @param input 本周期完整输入
	 * @param dtMs 控制周期(ms)
	 * @note Sleep 立即中止动作并卸力; 其它命令唤醒, 准备完成后才接收拨弹请求.
	 *       Init、忙碌或未到速时收到的单发边沿均不缓存; 保持 Once 不重复发射.
	 *       Continuosly 且拨盘目标速度大于零时连发; Idle、未到速或摩擦轮清堵时下发 Hold.
	 *       Hold 下已开始的单发或拨盘清堵继续完成; 拨盘目标速度为零不禁止单发.
	 */
	void Update(const ShooterInput<FricCount>& input, float dtMs);

	/**
	 * @brief 读取拨盘状态
	 *
	 */
	[[nodiscard]] const FeedInfo& GetFeedStatus() const;

	/**
	 * @brief 读取拨盘输出力矩(N·m), 电机原始极性
	 *
	 */
	[[nodiscard]] float GetFeedOutput() const;

	/**
	 * @brief 读取整机内部状态
	 *
	 */
	[[nodiscard]] ShootState GetState() const { return _controlInfo.State; }

	/**
	 * @brief 读取指定摩擦轮状态
	 * @param index 摩擦轮索引, 调用方保证 index < FricCount
	 */
	[[nodiscard]] const FricInfo& GetFricStatus(std::size_t index) const;

	/**
	 * @brief 读取指定摩擦轮本周期电机力矩指令
	 * @param index 摩擦轮索引, 调用方保证 index < FricCount
	 * @return 电机正转极性下的力矩(N·m)
	 */
	[[nodiscard]] float GetFricOutput(std::size_t index) const;

	/**
	 * @brief 清除整机控制状态及 Fric/Feed 的 PID、动作计时和输出, 进入 Sleep
	 *
	 */
	void Reset();

	Shooter(const Shooter&) = delete;
	Shooter(Shooter&&) = delete;
	Shooter& operator=(const Shooter&) = delete;
	Shooter& operator=(Shooter&&) = delete;

private:
	/**
	 * @brief 按配置索引直接构造不可复制、不可移动的 Fric 数组
	 *
	 */
	template <std::size_t... Indices>
	Shooter(const ShooterConfig<FricCount>& conf, std::index_sequence<Indices...>);

	/**
	 * @brief 根据逐轮堵转反馈协调清堵, 任一轮堵转后整组以配置速度反转 500ms
	 *
	 * @note 堵转标志由设备层更新和解除; 反转期间忽略, 到时仍堵转则再次反转.
	 *       Sleep 命令中止清堵; 正常运行使用本周期传入的逐轮目标速度.
	 */
	void UpdateFric(
	    const etl::array<FricFeedback, FricCount>& feedbacks,
	    const etl::array<float, FricCount>& targetSpeeds,
	    float dtMs
	);

	const float _fricReverseSpeed; ///< 整组清堵反转速度大小(rpm)
	ShooterControlInfo _controlInfo{};

	etl::array<Fric, FricCount> _fric; ///< 摩擦轮对象
	Feed _feed;                        ///< 拨盘对象
};

} // namespace breeze::shooter
