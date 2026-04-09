# 土豆地雷机制文档

## 状态流程
```
HIDDEN（地下） → RISING（升起中） → READY（就绪） → EXPLODING（爆炸中） → 消失
```

## 各状态行为

| 状态 | 描述 | 可被啃食 | 触爆条件 |
|------|------|---------|----------|
| **HIDDEN** | 地下隐藏，约180帧后转变 | 是 | 转变时检测到僵尸立即爆炸 |
| **RISING** | 升起中，持续120帧 | 否 | 持续检测，范围内僵尸触发爆炸 |
| **READY** | 就绪待发 | 否 | 检测到僵尸立即爆炸 |
| **EXPLODING** | 爆炸中，持续60帧 | 否 | - |

## 爆炸检测
- **检测范围**：以地雷为中心 ±81 像素（整格宽度）
- **伤害范围**：±80 像素内的同列僵尸
- **伤害值**：150 点伤害

## 关键逻辑
1. **HIDDEN → RISING 转变**：如果附近有僵尸（±81像素），立即爆炸
2. **RISING 期间**：不可啃食，僵尸靠近任何时候都可能爆炸
3. **RISING → READY 转变**：再次检测，附近有僵尸则爆炸
4. **READY 状态**：不可啃食，僵尸靠近立即爆炸
5. **HIDDEN 被啃死**：不触发爆炸，直接消失

## 代码位置
- 状态定义：`PotatoMineState` 枚举 (`PotatoMine.h`)
- 状态转换：`PotatoMine::update()` (`PotatoMine.cpp`)
- 爆炸检测：`PotatoMine::checkZombieInRange()` (`PotatoMine.cpp`)
- 爆炸触发：`PotatoMine::triggerExplosion()` (`PotatoMine.cpp`)
- 啃食逻辑：`main.cpp` 的 `checkZombie_to_Plant()` - 检测到 POTATO_MINE 类型时，在 READY 或 RISING 状态触发爆炸
