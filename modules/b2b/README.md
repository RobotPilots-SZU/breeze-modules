# Breeze B2B

吊射英雄板间通信模块试行. 目前处于乱写状态, 后续会整理出框架到 `breeze-modules`.

## 通信结构和协议定义

```mermaid
graph LR
subgraph TP[ISO-TP传输层]
  CAN_ID
  TP_PAYLOAD[表现层数据]
end

subgraph B2B[板通表现层解析]
  B2B_ID[ID]
  B2B_CAst_PAYLOAD[结构体强转(B2B Cast Proto)]
end

subgraph Com["Communicator (RP Proto) 解析"]

end

TP_PAYLOAD --> B2B & Com
```

## 抽象层次和结构

设计两层, 一层是板通协议基础设施, 一层是具体协议.

板通协议基础设施有两层:

- 传输层: 采用CAN的 `ISO-TP` 进行传输.
- 表现层: 提供 `B2B Cast Proto`协议, 将结构体与静态ID强转为字节流来传输. 兼容其他表现层, 比如 `RP Proto`.

## ISO-TP 地址与接收

`Transmitter::Config` 分别配置 `RxDataId`、`TxFcId`、`TxDataId`、`RxFcId`。
对端的 `TxDataId` 应与本端的 `RxDataId` 一致，对端的 `RxFcId` 应与本端的
`TxFcId` 一致。数据与 FC 使用独立 CAN ID，避免普通接收和 FC 接收过滤器重叠。

传输使用标准 11 位 CAN ID、ISO-TP 扩展寻址（地址字节为 0）、`bs=8` 和
`stmin=5`。接收通过 `isotp_recv_net()` 收齐整条报文后输出，最大 512 字节；
超出大小的报文会被读完并丢弃。`OutReceiveWait` 返回的 span 在下一次接收前有效。
