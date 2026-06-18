# ZombieGame Roadmap

This project started as a Win32/GDI C++ practice game. The modernization goal is to preserve the pixel-art survival-shooter feel while making the codebase easier to build, port, and extend.

The gameplay goal is to grow the current side-scrolling zombie shooter into a light survival-base game: a warm but fragile safehouse by day, a desperate siege by night.

## Technical Modernization

### Phase 0: Preserve The Baseline

- Build from VS Code or a terminal with CMake.
- Keep the archived Win32/GDI baseline available as a behavioral reference when needed.
- Avoid gameplay rewrites until the current behavior is easy to run and compare.

### Phase 1: Extract Game State

- Move shared structs and constants into a small `src/game` layer.
- Separate update logic from `WM_TIMER` and Win32 messages.
- Replace timer-driven movement with a fixed or semi-fixed timestep.
- Keep rendering as an adapter around the existing state.

### Phase 2: Replace The Renderer

Recommended first target: SDL3 or raylib.

- Load images into GPU textures instead of HDC-backed bitmaps.
- Replace `BitBlt`, `TransparentBlt`, `AlphaBlend`, and `PlgBlt` calls with sprite draw calls.
- Keep a 640x480 logical resolution and scale to the window.
- Batch static background layers and draw dynamic actors separately.
- Add full-screen impact feedback such as screen shake for gunshots, explosions, and heavy zombie hits.
- Move sprite-sheet metadata out of ad hoc render code and into reusable animation descriptors.

### Phase 3: Rebuild Navigation

- Replace per-zombie pixel BFS with a building navigation graph.
- Represent floors, stairs, doors, windows, and drop points as nodes and edges.
- Recompute target paths only when player/girlfriend target nodes change.
- Give zombies different target preferences and movement abilities.

### Phase 4: Grow The Game

- Add defense decisions: barricades, repairs, traps, lights, doors.
- Add resource pressure: ammo, medkits, power, noise, limited wave preparation time.
- Add mission nights: survive, rescue, repair, relocate, defend multiple rooms.
- Make weapons tactically distinct rather than only higher DPS.

### Backlog: Systems To Revisit

- Introduce a shared actor/combat base layer so `Player` and `Zombie` can eventually share damage, impulse, status, and explosion-response code.
- Replace color-coded collision authoring with a more scalable level format for future locations such as school, supermarket, and hospital maps.
- Move loose frame-by-frame animation assets toward sprite sheets or atlases with metadata for frame rects, pivots, timing, and hit regions.
- Add a proper asset build step that can package atlases and metadata into a shipping-friendly bundle format.
- Migrate HUD, menu, and shop text from custom debug/pixel drawing to `SDL3_ttf`.
- Add a reusable UI skin system with 9-slice metadata, content padding, per-state frames, and minimum-size rules so buttons, tabs, sliders, and panels can scale cleanly.
- Add semantic hit regions for enemies beyond simple head/body height checks once zombie variety expands.
- Explore a future art pass using GPT Images for new props, enemy variants, and environment set dressing once the runtime asset pipeline is in place.

## Current Priority Milestone

Now that the SDL3 gameplay path, workbench UI, weapon relight preview, and shelter presentation are in good shape, the next milestone should focus on building the first complete day/night survival loop rather than adding more isolated rendering polish.

Recommended immediate scope:

- Add a minimal resource model:
  - metal
  - wood
  - cloth
  - medicine
  - ammo
- Add a small inventory/storage layer that can feed both the workbench and future shelter interactions.
- Add a simple day/night state model so daytime preparation and nighttime defense become one connected loop.
- Make workbench actions consume real resources instead of being presentation-only.
- Keep the implementation narrow enough that the current tavern/safehouse remains the first playable vertical slice.

## Shelter Module Plan

The safehouse should no longer be treated as one bespoke map. It should become the first instance of a reusable shelter-site model that can support future locations and relocations.

Each future survival site should be able to define:

- a shelter exploration/base scene
- a workbench room or corner
- a radio / black-market communication point
- storage / crafting / repair interaction points
- defense surfaces such as doors, windows, lights, and utilities

This means the engine should gradually move toward a reusable site structure rather than hardcoding one-off tavern logic.

Suggested future data concepts:

- `ShelterSiteDef`
  - site id, display name, background scene, collision/nav data, interaction anchors
- `ShelterRoomDef`
  - room type, floor, bounds, tags, power/light hooks
- `InteractionPointDef`
  - workbench, radio, storage, medical bed, generator, etc.
- `SiteTransitionDef`
  - links from one shelter or exploration map to another

## Planned Scene Transitions

The tavern basement now establishes an important reusable rule: every major survival site may contain both a workbench and a radio/black-market station, and each of those interactions deserves its own authored transition sequence.

We should explicitly plan for two reusable transition families:

### Workbench Transition

Purpose:

- enter weapon modification / crafting focus mode from the live shelter scene

Visual beats:

- fade exploration/base HUD
- narrow the viewer's attention with a focus / zoom move
- reveal workbench props and lighting
- bring in the workbench management UI

### Radio / Black-Market Transition

Purpose:

- enter trade / remote contact / mission-intel mode from the live shelter scene

Visual beats:

- darken and isolate the active radio area
- add signal / interference treatment
- align the camera and prop framing around the radio desk
- reveal the trade / catalog / communication UI

Implementation note:

- these should become reusable transition controllers, not tavern-only cutscenes
- future sites should be able to plug in their own anchors, timings, and scene art while sharing the same flow

## Near-Term Follow-Up After The First Loop

Once the minimal day/night survival loop is playable, the next most valuable expansion should be:

1. inventory and slot-based storage interactions
2. shelter interaction points beyond the workbench
3. radio / black-market screen
4. resource-driven repairs, barricades, and room utilities
5. additional shelter sites and relocation flow

## 游戏策划草案

暂定方向：末日安全屋生存模拟 + 横版尸潮射击。

当前原型已经有一个很强的基础：楼房侧切图、像素主人公、左右两侧尸潮、昏迷女友、枪械战斗。下一步不是把它做成单纯的波次射击小游戏，而是让这栋楼成为一个玩家想守住、也可能不得不放弃的家。

### 一、游戏定位

- 类型：横版像素末日生存游戏。
- 核心组合：轻度生存经营 + 尸潮防守 + 横版射击 + 剧情推进。
- 氛围：cozy apocalypse，温暖但脆弱，平静与压迫交替。
- 设计原则：低复杂度，高沉浸；拟真但简化；让玩家感受到压力，而不是让玩家学习复杂工程系统。

### 二、核心体验

玩家在末日中经营一座安全屋。

白天：

- 搜集资源。
- 修理门窗和设施。
- 种植、做饭、制作弹药。
- 外出探索医院、超市、学校、警局等地点。
- 通过无线电与幸存者网络交易。
- 照顾昏迷或半清醒的女友。

黄昏：

- 封窗、加固门。
- 布置陷阱和防守点。
- 决定今晚是否开灯、是否启动发电机、是否启用炮塔。
- 分配 NPC 工作和防守位置。

夜晚：

- 尸潮来袭。
- 横版射击防守。
- 应对停电、突破、火灾、特殊感染者等突发事件。
- 在保护自己、保护女友、保护资源之间做取舍。

清晨：

- 清理尸体。
- 修复损坏。
- 处理伤病、感染和士气。
- 推进剧情，进入下一天。

玩家可以选择留在基地里养老，也可以不断迁移据点，推进主线，寻找撤离城市的办法。

### 三、世界观

未知病毒爆发，城市沦陷，电网断裂，政府撤离，大部分人死亡或感染。

玩家与昏迷女友被困在城市中。女友被感染但未尸变，需要持续药物抑制。她体内携带特殊病毒株，会不断吸引尸群，类似蜂后效应。

玩家坚守不是为了刷分，而是为了：

- 活下去。
- 守住暂时的家。
- 维持女友生命。
- 等待她醒来。
- 寻找离开城市的方法。

### 四、商店合理化

不要使用传统的“波次结束后打开商店买枪”。

改成幸存者无线电交易网络：

- 城市里仍有地下黑市、幸存者营地、无人机投送、地铁商人和以物易物网络。
- 玩家白天通过无线电联系交易对象，用物资、药品、燃料、零件交换枪械、弹药、图纸和情报。
- 夜晚尸潮越大，无线电越容易收到求救或警告，也可能暴露玩家位置。

这样升级系统会成为世界的一部分，而不是突兀的游戏 UI。

### 五、核心玩法循环

```text
白天经营
  -> 外出探索
  -> 获得资源
  -> 升级基地
  -> 黄昏布防
  -> 夜晚尸潮
  -> 修复损失
  -> 剧情推进
  -> 进入下一天
```

长期循环：

```text
小据点求生
  -> 解锁无线电
  -> 收到新地点情报
  -> 探索并迁移据点
  -> 获得更大空间和更多资源
  -> 承担更高防守风险
  -> 最终前往撤离区
```

### 六、据点迁移与章节

据点迁移是长期成长曲线的核心。玩家会对“家”产生情感，搬家必须有成本，而不是瞬移。

搬家成本：

- 放弃部分资源。
- 运送家具、武器、种子、燃料。
- 运输途中可能被尸潮袭击。
- 新据点更大、更有资源，但也更难守。

#### 第一章：小酒馆

定位：新手保护区。

特点：

- 两层小酒馆。
- 空间小，易守难发展。
- 食物储藏间可以短期生存。
- 店主留下 Glock 手枪、棒球棍和少量食物。
- 可利用地下室工作台进行简易升级。
- 可以在原地建立基础耕地。

解锁内容：

- 基础射击。
- 基础工作台。
- 基础种植。
- 基础尸潮。
- 初级无线电系统。

#### 第二章：学校避难所

特点：

- 空间更大，房间更多，窗户更多。
- 有宿舍、医务室、教室、食堂。
- 更适合容纳 NPC，但更难防守。

剧情：

- 玩家通过无线电收到学校幸存者基地的消息。
- 前往后发现避难所已经沦陷。
- 玩家可以选择清理、重建、救出幸存者，或只搜刮后离开。

#### 第三章：超市据点

特点：

- 资源丰富，大型仓储。
- 多入口，极难防守。
- 可引入食物保鲜、电力需求、自动炮塔、噪音系统。

#### 第四章：警察局

特点：

- 高级武器、铁门、防御工事。
- 可引入机枪塔、狙击位、监控室和特殊感染者。
- 高风险区域，可能聚集更危险的尸群。

#### 第五章：撤离区

目标：

- 修复交通工具。
- 带幸存者撤离。
- 面对最终选择。

结局分支：

- 放弃女友。
- 带她撤离。
- 留守基地。
- 全员死亡。

### 七、基地经营系统

房间功能化，让玩家经营的是一座具体的建筑，而不是抽象菜单。

可建设或升级内容：

- 工作台：改枪、制弹、制作陷阱。
- 发电机：供电、噪音、燃料消耗。
- 炮塔：强力防守，但耗电且制造噪音。
- 农田：生产土豆、玉米、药草等作物。
- 医疗床：治疗伤口、感染抑制、女友护理。
- 储物箱：提高资源上限。
- 厨房：提升食物效率和士气。
- 观察站：预警尸群、天气、探索风险。
- 无线电塔：交易、情报、剧情推进。

### 八、轻量电力系统

不要做电压、电流、线路损耗、负载均衡等硬核模拟。

电力只做状态化管理：

- 发电机：燃料、状态、噪音。
- 房间：已供电或未供电。
- 设备：开启或关闭。

玩家真正要做的决策：

- 今晚是否开灯？
- 是否启动发电机？
- 是否启用炮塔？
- 是否牺牲安静来换取视野和火力？

风险：

- 灯光吸引尸群。
- 发电机产生噪音。
- 停电会让房间变黑、炮塔失效、冰箱停机、无线电关闭。

核心原则：玩家感受到“看得见但更危险”的权衡，而不是管理复杂线路。

### 九、噪音与灯光系统

噪音可以成为比金币更重要的隐形资源。

噪音来源：

- 开枪。
- 爆炸。
- 发电机。
- 炮塔。
- 夜间修理。
- 无线电广播。

灯光来源：

- 房间灯。
- 探照灯。
- 手电。
- 火把或应急灯。

影响：

- 噪音越高，附近尸群越容易聚集。
- 灯光越强，夜间视野越好，但更容易吸引尸潮。
- 特殊感染者可能对噪音或灯光有不同反应。

### 十、种植系统

轻量化，不做农业模拟。

玩家操作：

- 放置耕地。
- 选择种子。
- 指定 NPC 自动照顾。

NPC 自动处理：

- 浇水。
- 收获。
- 简单维护。

作物类型：

- 土豆：稳定主食。
- 玉米：产量高但周期长。
- 药草：制作绷带和抗病毒药。

天气可以提供简单修正，例如下雨加速成长，寒冷降低效率。

### 十一、女友系统

女友是 emotional anchor，也是主线推进器。

她不应该只是一个躺着的血条。她可以：

- 梦话。
- 短暂清醒。
- 病情恶化。
- 突然抽搐。
- 恢复部分记忆。
- 提供剧情线索。
- 因病毒波动吸引特殊尸潮。
- 后期出现半感染化倾向。

状态示例：

- 体温。
- 感染指数。
- 镇静剂需求。
- 清醒度。
- 病毒波动。

最终抉择可以围绕她展开：

- 她是否是病毒源？
- 她是否持续吸引尸潮？
- 救她是否会害死更多幸存者？
- 玩家是否愿意放弃她？

### 十二、NPC 幸存者系统

NPC 能把游戏从射击防守提升为末日安全屋模拟。

职业：

- 医生。
- 工程师。
- 军人。
- 厨师。
- 小偷。
- 孩子。

状态：

- 饥饿。
- 疲劳。
- 恐慌。
- 感染。
- 士气。

行为：

- 工作。
- 修理。
- 防守。
- 争吵。
- 偷东西。
- 崩溃。
- 死亡。

设计原则：

- 半自动经营，玩家像据点领导者，而不是流水线工人。
- NPC 永久死亡可以增加情感重量，但早期版本应控制复杂度。

### 十三、尸潮系统

不要只是固定波次刷怪，而是让尸潮像世界的一部分。

尸潮来源：

- 女友病毒波动。
- 城市尸群迁徙。
- 玩家制造噪音。
- 灯光暴露。
- 剧情事件。

夜晚节奏分层：

- 普通夜晚：只有零星僵尸随机生成，数量主要受噪音、灯光、女友病毒波动和当前区域危险度影响。
- 紧张夜晚：附近尸群活动增强，入口压力更高，可能出现小规模突破事件。
- 惊魂夜：大型尸潮经过或主动攻击据点，是主要高压防守夜。

普通夜晚设计目标：

- 不让玩家每天晚上都进入高强度战斗疲劳。
- 让低噪音、低暴露的玩家获得相对平静的夜晚。
- 让白天经营、修理、种植和角色互动有喘息空间。

惊魂夜预警：

- 无线电可以提前收到尸潮消息。
- 地图上标注尸潮行经方向。
- 显示预计到达时间，例如“尸潮将在 2 天后经过东城区”。
- 玩家可以选择加固据点、关闭高噪音设备、撤离、诱导尸潮改道或冒险搜集防御资源。

基础生成关系：

```text
普通夜晚僵尸数量 = 区域基础危险 + 噪音暴露 * 噪音系数 + 光照暴露 * 光照系数 + 女友病毒波动
惊魂夜尸潮规模 = 尸群规模 + 路径接近度 + 据点暴露度 + 剧情修正
```

敌人类型：

- 普通僵尸：走最近入口。
- 嗅探者：优先追踪女友所在房间。
- 攀爬者：从阳台、窗户或屋顶进入。
- 撞击者：专门破门和路障。
- 尖叫者：提高全局噪音，引来更多尸群。
- 特殊感染者：章节中逐步引入。

防守失败不一定立刻 Game Over。可以允许玩家放弃一楼、封死楼梯、撤到二楼、牺牲储藏室，形成战线后撤的戏剧感。

### 十四、探索系统

白天外出探索让玩家不得不冒险。

可探索地点：

- 医院：药品、感染风险、剧情线索。
- 地铁：商人、捷径、黑暗区域。
- 警局：武器、弹药、高危感染者。
- 超市：食物、燃料、多人争夺。
- 民居：家具、种子、随机事件。

风险：

- 黑暗区域。
- 特殊感染者。
- 资源争夺。
- NPC 受伤或失踪。
- 夜晚前无法及时回家。

### 十五、美术与氛围

美术方向：

- 高细节像素风。
- 暖光 vs 冷夜。
- 末日工业感。
- cozy apocalypse。

灵感来源：

- This War of Mine。
- Project Zomboid。
- Fallout Shelter。
- Darkwood。
- State of Decay。

核心情绪：

- 温暖但脆弱。
- 平静与压迫交替。
- 安全感与绝望感循环。

场景氛围：

- 雨夜。
- 停电。
- 无线电杂音。
- 月光。
- 昏暗暖灯。
- 做饭、修枪、听广播、种植物。

### 十六、模块化系统设计

所有系统都优先按“数据状态 + 系统规则 + UI 表现”三层拆开。底层不绑定具体语言或库，先保证对象边界清晰，后续用 C++/SDL3 实现时可以逐步替换或扩展。

核心原则：

- 一格一物：基地内的大部分可交互对象都放在格子上。
- 房间承载功能：格子属于房间，房间决定探索、供电、光照、防守和居住价值。
- 系统弱耦合：种植、电力、光照、噪音、探索、尸潮通过状态值交互，而不是互相直接控制。
- 先做可运行闭环，再增加复杂模拟。

#### 1. 房间与格子系统

职责：

- 表示据点结构、楼层、房间、格子、门窗、楼梯和入口。
- 记录每个房间是否已探索、是否可用、是否被破坏。
- 记录每个格子的占用物、供电、光照、噪音、可建造类型。

核心对象：

- `BaseSite`：一个据点，例如小酒馆、学校、超市、警察局。
- `Room`：一个房间，例如酒吧、地下室、储藏间、卧室、屋顶。
- `GridCell`：房间内的单个格子。
- `PlacedObject`：放置物的基类概念，例如耕地、灯、发电机、电池、工作台、炮塔。
- `Entrance`：门、窗、屋顶入口、地下通道等可被突破的位置。

关键状态：

- `explored`：是否已探索。
- `powered`：是否有电。
- `lightLevel`：光照强度。
- `noiseLevel`：当前噪音。
- `integrity`：房间、门窗或设施完整度。
- `occupant`：格子上的放置物。

#### 2. 建造与放置系统

职责：

- 管理一格一物放置规则。
- 校验资源、空间、房间类型和前置科技。
- 处理建造、拆除、维修、升级。

核心对象：

- `BuildCatalog`：所有可建造物定义。
- `BuildRecipe`：资源需求、时间需求、工具等级。
- `BuildSystem`：校验和执行建造行为。
- `UpgradePath`：设施升级路线。

可放置对象：

- 耕地。
- 作物架或花盆。
- 动物栏，后期加入。
- 灯。
- 探照灯。
- 汽油发电机。
- 太阳能板。
- 电池。
- 工作台。
- 储物箱。
- 路障。
- 炮塔。
- 床或医疗床。

#### 3. 电力系统

职责：

- 计算发电、储电和耗电。
- 决定房间和设施是否供电。
- 根据负载生成噪音。

核心对象：

- `PowerSource`：电源，例如汽油发电机、太阳能板。
- `Battery`：储电设施。
- `PowerConsumer`：耗电设施，例如灯、炮塔、冰箱、无线电、医疗设备。
- `PowerGrid`：据点内的电力网络。

电力来源：

- 汽油发电机：功率大，夜晚可用，耗油，噪音高。
- 太阳能板：白天发电，安静，需要材料搭建。
- 电池：储存白天电量，夜晚释放。

关键规则：

```text
可用电力 = 发电量 + 电池放电量
负载比例 = 当前耗电 / 最大可供电
发电机噪音 = 基础噪音 + 负载比例 * 额外噪音
```

设计重点：

- 不模拟电压、电流和线路损耗。
- 玩家只需要理解“电够不够”“哪里有电”“发电机吵不吵”。
- 电网负载越高，发电机越响，越容易吸引尸潮。

#### 4. 光照系统

职责：

- 计算房间和格子的光照强度。
- 影响作物生长、玩家视野和僵尸能力。
- 与电力系统和尸潮系统联动。

核心对象：

- `LightEmitter`：灯、探照灯、火把、应急灯。
- `LightMap`：房间或据点的光照分布。
- `VisibilityState`：可见、昏暗、黑暗、未探索。

关键规则：

- 黑暗区域：僵尸更活跃，移动和攻击更强。
- 微光区域：普通状态。
- 强光区域：僵尸减速，攻击力降低。
- 夜间植物灯：帮助作物生长，但耗电并可能暴露基地。

#### 5. 噪音与威胁系统

职责：

- 汇总枪声、发电机、炮塔、修理、无线电等噪音。
- 按距离衰减噪音。
- 计算尸潮吸引值和区域危险变化。

核心对象：

- `NoiseEmitter`：噪音源。
- `NoiseMap`：据点周边噪音分布。
- `ThreatMeter`：今晚威胁值。
- `HordeDirector`：根据威胁值安排尸潮。

关键规则：

```text
格子噪音 = 声源噪音 / (1 + 距离 * 衰减系数)
今晚尸潮 = 基础威胁 + 女友病毒波动 + 总噪音暴露 + 光照暴露 + 剧情修正
```

设计重点：

- 噪音是隐形资源。
- 枪越强不一定越好，因为它可能让下一波更危险。
- 发电机位置、负载和房间隔离都能成为玩家决策。

#### 6. 种植与养殖系统

职责：

- 管理作物种植、生长、收获和肥料效果。
- 后期扩展动物养殖。
- 与光照、电力、肥料、天气和 NPC 工作联动。

核心对象：

- `CropDefinition`：作物定义。
- `CropInstance`：具体种下的一株作物。
- `Fertilizer`：肥料。
- `FarmPlot`：耕地或种植格。
- `AnimalPen`：动物栏，后期加入。

作物属性：

- 生长时间。
- 需光等级。
- 需肥等级。
- 基础产量。
- 食用效果。
- 可加工产物。

肥料类型：

- 农家肥：持续时间长，效果温和，来源稳定但慢。
- 化学肥：效果强，持续短，需要化学品。
- 特殊肥料：后期扩展，可带来高产或感染风险。

食物效果：

- 增加饱食度。
- 提高体力恢复。
- 提高血量恢复。
- 提升士气。
- 药草可制作绷带、镇静剂或抗病毒药。

#### 7. 探索与迷雾系统

职责：

- 管理基地内部未探索房间。
- 管理城市外部探索地点。
- 生成资源、事件、风险和新地点。

核心对象：

- `ExplorationSite`：外部地点，例如加油站、医院、超市、警局。
- `ExplorationMission`：一次派遣任务。
- `LootTable`：资源表。
- `EventTable`：随机事件表。
- `ScoutReport`：探索结果报告。

内部探索：

- 未探索房间显示为黑色或迷雾。
- 探索后房间变为可使用。
- 某些房间需要钥匙、工具、电力或剧情条件。

外部探索消耗：

- 食物。
- 弹药。
- 药品。
- 时间。
- 幸存者体力。

地点保底资源：

- 加油站：保底 1 桶汽油，概率额外 1 到 2 桶。
- 医院：保底药品，概率抗病毒药。
- 超市：保底食物，概率种子或电池。
- 警局：保底弹药，概率枪械或配件。
- 废弃工厂：保底金属和零件，概率化学品。

#### 8. 幸存者派遣系统

职责：

- 指挥幸存者去不同地区探索。
- 根据角色能力、装备和地点风险计算结果。
- 生成受伤、感染、失踪、发现新地点、遇见商人等事件。

核心对象：

- `Survivor`：幸存者。
- `SkillSet`：战斗、搜索、医疗、工程、胆量、负重等能力。
- `Assignment`：工作或探索任务。
- `MissionRisk`：任务风险。
- `MissionOutcome`：任务结果。

幸存者属性：

- 体力。
- 战斗能力。
- 搜索能力。
- 医疗能力。
- 工程能力。
- 胆量或恐慌抗性。
- 背包容量。

派遣决策：

- 去哪里。
- 带什么装备。
- 主要目标资源。
- 是否冒险深入。
- 是否当天必须返回。

#### 9. 资源与库存系统

职责：

- 管理基础资源、高级资源、武器、配件、药品、燃料和食物。
- 支持建造、制作、交易、探索消耗和 NPC 消耗。

核心对象：

- `ResourceStack`：一种资源及数量。
- `Inventory`：库存。
- `StorageObject`：储物设施。
- `ItemDefinition`：物品定义。
- `CraftRecipe`：制作配方。

资源类型：

- 食物。
- 水。
- 木板。
- 金属零件。
- 机械零件。
- 塑料零件。
- 布料。
- 化学品。
- 药品。
- 弹药。
- 汽油。
- 电池。
- 枪械和配件。

#### 10. 尸潮与战斗导演系统

职责：

- 根据威胁值、剧情和地图状态生成夜晚尸潮。
- 选择敌人类型、入口、目标和突破事件。
- 区分普通夜晚、紧张夜晚和惊魂夜。
- 根据无线电情报和世界地图生成尸潮预报。
- 让防守失败有层次，而不是立刻结束。

核心对象：

- `ZombieDefinition`：僵尸类型定义。
- `ZombieSpawnPlan`：生成计划。
- `HordeDirector`：尸潮导演。
- `HordeForecast`：尸潮预报，包括预计到达时间、方向、规模和可信度。
- `HordeRoute`：尸群在城市地图上的行进路线。
- `DefenseState`：基地防守状态。
- `BreachEvent`：突破事件。
- `NightThreatProfile`：当晚威胁类型，普通夜晚、紧张夜晚或惊魂夜。

夜晚类型：

- `QuietNight`：零星僵尸，主要由噪音和灯光触发。
- `TenseNight`：小规模压力，可能有局部突破。
- `HordeNight`：尸潮经过或攻击据点，需要提前准备。

尸潮预报：

- 由无线电、观察站、幸存者情报或地图事件提供。
- 可在地图上显示方向箭头、路径、预计到达倒计时和危险等级。
- 情报可以有可信度，低等级无线电可能给出模糊或延迟的预警。

敌人目标偏好：

- 普通僵尸：走最近入口。
- 嗅探者：追踪女友所在房间。
- 攀爬者：从阳台、窗户或屋顶进入。
- 撞击者：优先破坏门和路障。
- 尖叫者：提高全局噪音，引来更多尸群。

失败层次：

- 门窗受损。
- 房间沦陷。
- 一楼放弃。
- 楼梯封死。
- 储藏室损失。
- NPC 受伤或死亡。
- 女友受到威胁。

#### 11. 无线电交易与剧情系统

职责：

- 替代传统商店。
- 管理黑市商人、幸存者营地、求救信号和剧情广播。
- 提供尸潮预警、天气消息和区域危险变化。
- 让枪械、药品、情报和新地点来源合理化。

核心对象：

- `RadioContact`：无线电联系人。
- `TradeOffer`：交易条目。
- `Reputation`：信任等级。
- `BroadcastEvent`：广播事件。
- `QuestState`：任务状态。
- `HordeWarning`：尸潮预警消息。

规则：

- 信号越好，可交易对象越多。
- 信任等级越高，商品和情报越可靠。
- 长时间通讯可能制造风险或暴露位置。
- 某些主线地点通过无线电解锁。
- 高等级无线电能更早收到尸潮方向、规模和到达时间。
- 低等级无线电可能只收到模糊警告，例如“东边有大批感染者移动”。

#### 12. 女友与主线推进系统

职责：

- 管理女友状态、病毒波动、剧情触发和最终抉择。
- 让她成为尸潮吸引、情感目标和主线线索的交汇点。

核心对象：

- `CompanionPatient`：女友角色状态。
- `InfectionState`：感染和病毒波动。
- `CareAction`：照顾行为。
- `StoryFlag`：剧情标记。
- `EndingState`：结局状态。

关键状态：

- 体温。
- 感染指数。
- 镇静剂需求。
- 清醒度。
- 病毒波动。
- 记忆碎片。

### 十七、交互设计草案

交互目标：玩家感觉自己在经营一座具体的安全屋，而不是在操作一张表格；同时避免每个小动作都必须让角色跑到格子旁边，导致节奏拖沓。

推荐采用混合交互：

- 鼠标负责规划、选择、下达命令。
- 角色负责执行需要临场感和风险感的动作。
- 白天偏策略管理，夜晚偏近身应急和横版战斗。

#### 1. 交互模式

##### 基地经营模式

适用阶段：白天和黄昏。

玩家可以：

- 鼠标悬浮房间、格子、设施查看状态。
- 点击格子或设施打开上下文菜单。
- 放置建筑蓝图。
- 分配 NPC 工作。
- 调整电力、灯光、发电机、炮塔开关。
- 查看房间状态、噪音、光照和供电。

时间处理：

- 白天可以暂停或慢速规划。
- 下达的动作进入任务队列，由玩家或 NPC 执行。
- 建造、修理、收获、探索房间等动作消耗时间或行动点。

设计重点：

- 玩家可以像管理者一样规划。
- 执行仍然消耗时间，不是瞬间完成。
- 避免“每个耕地都要亲自走过去点一下”的重复劳动。

##### 建造模式

入口：

- 在主 HUD 下方物品栏正上方放置一个扳手图标按钮。
- 点击扳手进入建造模式。
- 也可以用 `B` 快捷键打开。

可用阶段：

- 白天可完整使用。
- 黄昏可使用，但应强调防御设施、路障、灯光和修理。
- 战斗状态下禁用，按钮变灰并显示原因，例如“尸潮来袭时无法建造”。

建造模式 UI：

- 左侧或底部显示建造分类：基础、种植、电力、防御、储物、生活、医疗。
- 鼠标悬浮格子显示可建造范围、资源需求、供电需求、噪音或光照影响。
- 放置后先生成蓝图，由主角或 NPC 执行建造。
- 支持批量放置耕地、灯、路障等重复对象。

退出规则：

- 点击扳手按钮再次退出。
- 按 `Esc` 退出。
- 夜晚进入战斗状态时自动退出并锁定。

设计重点：

- 建造是明确模式，不混在普通点击里，减少误操作。
- 战斗中禁用建造，让夜晚防守保留压力。
- 黄昏是“最后布防窗口”，玩家能感到准备时间正在结束。

##### 角色控制模式

适用阶段：探索房间、夜晚防守、突发事件。

玩家直接控制主角移动和战斗。

可以执行：

- 开门。
- 搜索柜子。
- 捡起物品。
- 修理附近设施。
- 手动装填发电机燃油。
- 手动治疗 NPC 或女友。
- 搬运关键物资。
- 近距离建造或拆除路障。

设计重点：

- 有危险、有空间位置、有时间压力的动作需要角色靠近。
- 这类动作保留横版游戏的身体感。

##### 战斗应急模式

适用阶段：夜晚尸潮。

玩家仍然可以通过 UI 快速切换少量关键系统，但不能像白天一样自由建设。

允许快速操作：

- 开关某个房间的灯。
- 切换发电机运行模式。
- 启用或关闭炮塔。
- 给 NPC 下达简单命令，例如“守这里”“撤退”“修门”。
- 使用快捷物品，例如医疗包、燃烧瓶、弹药包。

限制：

- 夜晚不能随意新建大型设施。
- 复杂制作和交易不可用。
- 修理、加固、救人需要角色或 NPC 到现场。

设计重点：

- 夜晚操作要快。
- 不能让玩家在尸潮中打开复杂菜单慢慢算资源。
- 关键决策是“救哪里、关哪里、撤哪里”。

#### 2. 格子交互规则

鼠标悬浮格子时显示轻量信息：

- 格子类型。
- 当前占用物。
- 是否有电。
- 光照强度。
- 噪音强度。
- 可执行动作。

左键：

- 选择格子或设施。
- 打开简短操作面板。

右键：

- 快速下达默认动作。
- 对空格子：移动到此处或打开建造快捷菜单。
- 对作物：收获或查看。
- 对设施：使用或切换状态。
- 对损坏物：修理。

长按或双击：

- 进入详细面板。
- 查看完整属性、升级、拆除、优先级、任务队列。

拖拽：

- 批量选择格子。
- 批量指定耕地、照明区域、修理区域。
- 批量取消蓝图。

#### 3. 操作距离规则

并不是所有操作都需要角色站在旁边。

远程可规划：

- 放置蓝图。
- 分配 NPC。
- 设置房间用途。
- 开关已有电网设备。
- 设定作物种植计划。
- 查看状态。

近身才可执行：

- 建造蓝图。
- 修理损坏物。
- 搜索未探索容器。
- 清理堵塞物。
- 添加汽油。
- 收获作物，除非有 NPC 被分配为农夫。
- 抢救女友或 NPC。
- 搬运重物。

例外：

- 如果设施接入电网并有远程控制模块，可以远程切换。
- 如果有对应 NPC 职责，玩家可以远程下命令，由 NPC 去执行。
- 夜晚高风险动作会要求角色或 NPC 实际到场。

#### 4. 任务队列系统

鼠标点击不一定立即完成动作，而是创建任务。

核心对象：

- `InteractionCommand`：玩家下达的一次交互命令。
- `TaskQueue`：房间、角色或据点的任务队列。
- `TaskRequirement`：执行任务所需资源、工具、距离、时间和技能。
- `TaskExecutor`：执行者，可以是主角或 NPC。

任务示例：

- 建造耕地。
- 播种土豆。
- 给发电机加油。
- 修理一楼门。
- 打开地下室灯。
- 收获药草。
- 搜索未探索房间。
- 搬运汽油到发电机房。

任务优先级：

- 普通。
- 重要。
- 紧急。
- 禁止 NPC 自动处理。

设计重点：

- 白天玩家更像在安排工作。
- 夜晚任务队列会被打断，形成突发压力。

#### 5. UI 层级

交互 UI 分三层，避免所有信息同时出现。

第一层：悬浮信息。

- 简短状态。
- 可用动作图标。
- 危险提示。

第二层：快捷操作面板。

- 建造。
- 修理。
- 开关。
- 收获。
- 分配。
- 升级。
- 拆除。

第三层：详细管理面板。

- 完整属性。
- 资源需求。
- 电力消耗。
- 噪音输出。
- 光照范围。
- 任务队列。
- 升级路线。

#### 6. 输入建议

鼠标：

- 悬浮查看。
- 左键选择。
- 右键默认动作。
- 拖拽批量选择。
- 滚轮缩放或切换楼层视野，视最终镜头设计决定。

键盘：

- `WASD` 或方向键移动角色。
- `E` 与附近对象交互。
- `R` 换弹。
- 数字键切换武器或快捷物品。
- `Space` 暂停或确认，具体功能按模式决定。
- `Tab` 切换管理面板。
- `B` 建造菜单。
- `M` 地图。
- `J` 任务日志。

手柄支持可后置，但交互设计应尽量不依赖过小 UI 热区。

#### 7. 白天与夜晚的交互差异

白天：

- 鼠标管理权限完整。
- 可以暂停规划。
- 可以批量建造、种植、修理。
- NPC 自动执行大量重复劳动。
- 扳手按钮可进入完整建造模式。

黄昏：

- 重点是布防。
- UI 强调入口、路障、灯、发电机、炮塔。
- 可以一键检查今晚风险：电力、噪音、门窗完整度、弹药、女友状态。
- 建造模式仍可使用，但更偏向防御和修理。

夜晚：

- 主角控制优先。
- 鼠标只保留快速切换和战术命令。
- 建造和复杂管理受限。
- 危险交互需要靠近或派 NPC 到场。
- 战斗状态下扳手按钮禁用。

清晨：

- 显示损失报告。
- 支持一键生成修理任务。
- 支持安排白天工作。

#### 8. 设计取舍

推荐答案不是“纯鼠标点击”或“必须角色走到旁边”，而是：

- 规划类动作可以远程点击。
- 执行类动作需要角色或 NPC 到场。
- 重复劳动交给任务队列和 NPC。
- 夜晚应急保留近身风险。

这样可以同时获得：

- 管理游戏的清晰度。
- 横版动作游戏的沉浸感。
- 末日生存的时间压力。
- 模块化系统的可扩展性。

### 十八、系统优先级

建议先完成“可运行的纵向切片”，再扩展复杂模拟。

第一阶段，核心框架：

1. 房间与格子系统。
2. 建造放置：耕地、灯、发电机、电池。
3. 基础资源与库存。
4. 白天/夜晚时间循环。

第二阶段，系统联动：

1. 电力系统。
2. 光照系统。
3. 噪音与威胁系统。
4. 基础种植。

第三阶段，外部循环：

1. 外部探索派遣。
2. 加油站、超市、医院三个地点。
3. 无线电交易。
4. 探索结果报告。

第四阶段，夜晚压力：

1. 尸潮根据噪音和光照动态变化。
2. 不同入口和房间突破。
3. 防守失败的分层后果。
4. 女友基础状态和病毒波动。

第五阶段，长期扩展：

1. 幸存者技能与风险。
2. 太阳能、化肥和高级作物。
3. 养殖动物。
4. 学校据点和基地迁移。
5. 主线剧情和多结局。

### 十九、开发 Milestones

#### Milestone 1：核心战斗 Prototype，1 到 2 周

内容：

- 横版移动。
- 枪械射击。
- 僵尸 AI。
- 基础波次系统。

目标：

- 验证射击是否爽快。
- 验证尸潮压力是否成立。

#### Milestone 2：酒馆基地 Prototype，2 到 3 周

内容：

- 酒馆侧切地图。
- 房间系统。
- 基础灯光。
- 白天/夜晚切换。

目标：

- 建立“家”的氛围。
- 让玩家理解基地不是背景，而是可失去的空间。

#### Milestone 3：基础经营系统，2 周

内容：

- 工作台。
- 种植。
- 储物箱。
- 简单资源系统。

目标：

- 形成白天经营循环。

#### Milestone 4：夜晚防守系统，2 到 3 周

内容：

- 强化尸潮。
- 路障。
- 修理。
- 电力与照明。
- 噪音和灯光吸引。

目标：

- 形成白天经营 -> 黄昏布防 -> 夜晚防守 -> 清晨修复的核心循环。

#### Milestone 5：外出探索系统，3 周

内容：

- 小地图探索。
- 搜刮。
- 随机事件。
- 医院/超市 prototype。

目标：

- 形成风险与收益。
- 解释资源、枪械和药品来源。

#### Milestone 6：NPC 系统，3 到 4 周

内容：

- NPC 加入。
- 自动工作。
- 情绪系统。
- 死亡/感染。

目标：

- 提升情感投入。
- 让基地从“玩家的楼”变成“幸存者的家”。

#### Milestone 7：基地迁移系统，2 到 3 周

内容：

- 学校地图。
- 搬迁成本。
- 多基地差异。

目标：

- 形成长线成长。
- 让玩家在“养老”和“继续逃出去”之间做选择。

#### Milestone 8：主线剧情，4 周

内容：

- 女友剧情。
- 无线电剧情。
- 撤离区。
- 多结局。

目标：

- 建立长期目标。
- 让玩家知道为什么活下去。

#### Milestone 9：持续扩展

内容：

- 新枪械。
- 新感染者。
- 新地图。
- 新事件。
- Roguelite 模式。
- Mod 支持。

### 二十、最小可玩版本 MVP

建议优先完成一个单地图纵向切片，而不是一开始做大地图开放世界。

MVP 内容：

- 小酒馆地图。
- 白天种地和修理。
- 黄昏布防。
- 夜晚尸潮。
- 工作台。
- 简单探索。
- 无线电交易。
- 女友基础状态。
- 基础剧情。

MVP 目标：

- 证明“在末日里经营一个温暖但脆弱的家”这个体验成立。
- 证明射击、防守、经营和剧情可以形成闭环。
- 让玩家既能留在基地养老，也能选择推进主线逃离城市。
