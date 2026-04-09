# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

- **IDE**: Visual Studio
- **Target**: x64 Release
- **Build**: `生成 → x64 → Release → 运行`
- **Graphics Library**: EasyX (`graphics.h`)

## Architecture

### Core Systems (all in `main.cpp`)

| System | Functions | Description |
|--------|-----------|-------------|
| Plant | `drawPlant()`, `updatePlant()` | Renders and animates plants on the 3x9 grid |
| Zombie | `createZombie()`, `updateZombie()`, `drawZombie()` | Wave-based spawning, movement, eating |
| Sunshine | `createSunshine()`, `updateSunshine()`, `collectSunshine()` | Natural drop + Sunflower production, Bezier curve flight |
| Bullet | `shoot()`, `updateBullets()`, `drawBullets()` | Peas fired by Peashooters |
| Collision | `checkBullet_to_ZM()`, `checkZombie_to_Plant()` | Hit detection |
| Wave | `updateWaveSystem()` | 5 waves, escalating zombie count |
| Lawn Mower | `updateMowers()` | Last-line defense, activates when zombie reaches left edge |

### Key Classes

- **`Plant`** (Plant.h) - Base class with `blood`, `type`, `frameIndex`, `row`, `col`
  - `Peashooter` - Shoots peas (80 frames fire interval), `shootTime` tracks fire rate
  - `Sunflower` - Produces sunshine (every 600 frames) via `produceSun()`
  - `PotatoMine` - State machine: `POTATO_HIDDEN → RISING → READY → EXPLODING`, deals 150 damage on zombie contact
- **`Zombie`** (Zombie.h) - `x`, `y`, `row`, `blood`, `speed`, `dead`, `eating`, `eatingTime`
- **`MapCell`** (Map.h) - Grid cell holding `Plant*`, `type`, `frameIndex`
- **`SunshineBall`** (sunshineBall.h) - State machine: `SUNSHINE_DOWN → GROUND → COLLECT → PRODUCT`, uses Bezier curves
- **`vector2`** (vector2.h) - 2D math struct with Bezier curve support
- **`ExplosionEffect`** (Plant.h) - Potato mine explosion, max 10 concurrent, 8 animation frames

### Game Loop

Fixed timestep at 60 FPS (`TIMESTEP = 1.0/60.0`), accumulator pattern prevents physics drift on lag spikes.

```
while (gameIsRunning) {
    double deltaTime = currentTime - lastTime;
    accumulator += deltaTime;
    while (accumulator >= TIMESTEP) {
        updateGame(TIMESTEP);  // All game logic
        accumulator -= TIMESTEP;
    }
    updateWindow();  // Render
    userClick();     // Input
}
```

### Project Structure

```
PLANT_VS_ZOMBIE/
├── main.cpp            # 游戏主循环、输入处理、所有系统更新逻辑
├── Plant.h/cpp         # 植物基类 + 爆炸效果
├── Peashooter.h/cpp    # 豌豆射手
├── Sunflower.h/cpp     # 向日葵
├── PotatoMine.h/cpp    # 土豆地雷状态机
├── Zombie.h/cpp        # 僵尸
├── PlantFactory.h      # 植物工厂（创建植物实例）
├── Map.h               # 网格单元格定义
├── sunshineBall.h      # 阳光球状态机
├── vector2.h/cpp       # 二维向量 + 贝塞尔曲线
├── tool.h/cpp          # PNG透明贴图 + 帧率计算
├── config.h            # 枚举、宏常量、UI尺寸
└── res/                # 资源目录
```

### Grid & Coordinate System

- Screen: 900x600 (`WIN_WIDTH`, `WIN_HEIGHT`)
- Map area: x=256~1000, y=179~489
- Grid: 3 rows × 9 cols (`MAP_ROW`, `MAP_COL`)
- Cell size: 81×102 pixels
- Plant placement: `x = 256 + col * 81`, `y = 179 + row * 102 + 14`
- Zombie spawn: right edge, `y = 172 + (1 + row) * 100`

### Enums (config.h)

```cpp
enum { SUNSHINE_DOWN, SUNSHINE_GROUND, SUNSHINE_COLLECT, SUNSHINE_PRODUCT };
enum { WAN_DOU, XIANG_RI_KUI, TU_DAO, ZHI_WU_COUNT };  // Plant types: Peashooter, Sunflower, PotatoMine
enum { GOING, WIN, FAIL };  // Game status
enum { WALKING, EATING, DEAD };  // Zombie state
enum { POTATO_HIDDEN, POTATO_RISING, POTATO_READY, POTATO_EXPLODING };
```

### User Interaction

- **Left-click card**: Select plant (costs sunshine), follows mouse cursor
- **Left-click map**: Place selected plant (if cell empty)
- **Right-click**: Cancel selection, refund sunshine; 在铲子模式下右键取消铲子模式
- **Shovel mode**: Left-click plant to remove it
- **Pause button**: Top-right corner (PAUSE_BTN_X=920)

### Bullet System

- Max 30 bullets (`bullets[30]`), speed=4 px/frame
- Bullet- zombie collision: `row` match + x in [zombie.x+80, zombie.x+110]
- Peashooter fires when zombie in danger zone (x < WIN_WIDTH-60) on same row

### Resource Paths

- Backgrounds/UI: `res/background.jpg`, `res/bar5.png`
- Cards: `res/Cards/card_%d.png`
- Plants: `res/plants/%d/%d.png` (type/frame)
- Zombies: `res/zombies_walk/%d.png` (walk), `res/zombies_eat/%d.png` (eat), `res/zombies_dead/%d.png` (death), `res/zombies_stand/%d.png` (stand)
- Bullets: `res/bullets/bullet_normal.png`, `res/bullets/bullet_blast.png`
- Sunshine: `res/sunshine/%d.png`
- Potato Mine: `res/potato_mine_dir/` (ready/underground), `res/explosion/%d.png`
- Menus: `res/menu.png`, `res/menu1.png`, `res/menu2.png`
- Win/Fail: `res/win.png`, `res/fail.png`
- Shovel: `res/shovel_slot.png`, `res/shovel.png`
- Mower: `res/mower.png`
