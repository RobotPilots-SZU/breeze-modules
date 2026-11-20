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
