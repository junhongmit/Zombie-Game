# Item Catalog Draft

This document is a planning catalog for collectible items, inventory assets, and future item metadata. It is not yet a final gameplay balance sheet.

## Metadata Schema Draft

Each item can eventually carry these fields:

| Field | Meaning |
| --- | --- |
| `id` | Stable internal identifier, for example `food_canned_beef`. |
| `name_zh` | Display name in Chinese. |
| `name_en` | Optional English reference name. |
| `category` | Broad item category. |
| `subcategory` | More specific grouping, such as canned food, ammo, medicine, tool. |
| `description` | Flavor text and worldbuilding. |
| `use_effect` | What happens when used, equipped, crafted, traded, or consumed. |
| `weight_kg` | Weight per item. |
| `stack_size` | Maximum count per inventory slot. |
| `base_value` | Trade value, preferably in shell currency or barter score. |
| `rarity` | Common, uncommon, rare, military, unique, story. |
| `locations` | Likely loot locations. |
| `crafting_tags` | Tags such as food, medical, fuel, weapon_part, fertilizer. |
| `asset_status` | Needed, ready, placeholder, cut. |
| `notes` | Design notes, balancing questions, or art notes. |

## Resource Icons Already Planned

These are base resources or warehouse materials rather than detailed backpack items.

| Item | Category | Primary Use | Suggested Locations | Notes |
| --- | --- | --- | --- | --- |
| 食物 | Base Resource | Feeding survivors, cooking | Supermarket, homes, restaurant | Abstract resource for base consumption. |
| 水 | Base Resource | Drinking, farming, medicine | Homes, supermarket, rain collectors | Can later split clean/dirty water. |
| 木材 | Building Resource | Barricades, beds, farm plots, repairs | Homes, school, hardware store | High-volume construction material. |
| 金属 | Building Resource | Doors, defenses, weapons bench | Factory, garage, police station | May represent scrap metal. |
| 布料 | Building/Medical Resource | Bandages, beds, backpacks | Homes, school, clothing store | Useful across medical and crafting. |
| 玻璃 | Building Resource | Windows, lamps, greenhouse, solar parts | Homes, stores, factory | Best introduced after window/greenhouse systems. |
| 通用零件 | Industrial Resource | Repairs, basic crafting | Garages, factories, offices | Good all-purpose component. |
| 机械零件 | Industrial Resource | Generator, turret, advanced workbench | Factory, garage, police station | Higher-tier than generic parts. |
| 电子元件 | Industrial Resource | Radio, lights, batteries, turrets | Offices, school, electronics store | Core resource for power systems. |
| 塑料零件 | Industrial Resource | Containers, water systems, medicine packaging | Homes, stores, hospital | Optional in MVP. |
| 化学品 | Industrial/Medical Resource | Fertilizer, medicine, explosives | Hospital, factory, school lab | Valuable and risky. |
| 汽油 | Fuel | Generator, travel, molotovs | Gas station, cars, garage | Important strategic resource. |
| 蓄电池 | Power Resource | Store electricity, trade, crafting | Cars, electronics store, solar sites | Can be an item or facility component. |
| 电线/电缆 | Power Resource | Power grid construction | Hardware store, factory, offices | Optional if power build costs need detail. |

## Food Items

| Item | Category | Primary Use | Suggested Locations | Notes |
| --- | --- | --- | --- | --- |
| 牛肉罐头 | Food | Hunger recovery, cooking | Supermarket, homes, restaurant | Durable, good early-game food. |
| 蔬菜罐头 | Food | Hunger recovery, cooking | Supermarket, homes | Slightly lighter, common. |
| 豆子罐头 | Food | Hunger recovery, morale flavor | Supermarket, homes | Good cozy-apocalypse staple. |
| 鱼罐头 | Food | Hunger recovery | Supermarket, homes | Strong smell could be a funny later modifier. |
| 午餐肉罐头 | Food | Hunger recovery | Supermarket, military stash | Higher value canned food. |
| 饼干 | Food | Small hunger recovery | Homes, stores, school | Light snack. |
| 压缩饼干 | Food | Efficient travel ration | Military, police station, emergency kits | Great for exploration cost. |
| 巧克力棒 | Food | Small hunger, morale boost | Stores, school, vending machines | Good barter item. |
| 能量棒 | Food | Stamina recovery | Pharmacy, gym, supermarket | Useful for scouting. |
| 方便面 | Food | Cooking ingredient | Homes, supermarket | Needs water or cooking for best effect. |
| 面包 | Food | Fresh food | Restaurant, homes | Spoilage can be introduced later. |
| 土豆 | Food/Crop | Cooking, planting output | Farm plot, supermarket | First crop candidate. |
| 玉米 | Food/Crop | Cooking, planting output | Farm plot, supermarket | Higher yield, longer grow time. |
| 蘑菇 | Food/Crop | Food, medicine ingredient | Basement, damp areas | Could grow with low light. |
| 苹果 | Food | Fresh food, morale | Homes, supermarket | Optional spoilage. |
| 生肉 | Food | Cooking | Hunting, animals, supermarket | Needs cooking or preservation. |
| 熟肉 | Food | Strong hunger recovery | Kitchen, campfire | Crafted from raw meat. |
| 腐坏食物 | Food/Junk | Emergency food, compost | Anywhere | Can become fertilizer. |
| 军用口粮 | Food | High-value ration | Military, police station | Rare and efficient. |
| 宠物粮 | Food | Emergency food, animal feed | Homes, pet store | Sad but useful apocalypse item. |
| 咖啡 | Food/Drink | Stamina, morale | Homes, office, diner | Can reduce fatigue. |
| 酒 | Food/Trade | Morale, disinfectant, trade | Bar, homes, store | Fits the tavern start. |
| 香烟 | Trade/Morale | Barter, morale | Gas station, store, homes | Not food, but often stored near it. |

## Water And Drink Items

| Item | Category | Primary Use | Suggested Locations | Notes |
| --- | --- | --- | --- | --- |
| 瓶装水 | Drink | Thirst recovery, travel supply | Supermarket, homes, vending machines | Essential inventory item. |
| 水壶 | Container | Carry water | Outdoor stores, homes | Reusable. |
| 脏水 | Drink/Material | Purify into clean water | Rivers, rain barrels, basements | Risk if consumed directly. |
| 净水片 | Medical/Utility | Purify water | Pharmacy, camping store, military | Useful exploration loot. |
| 水桶 | Container | Carry water, farming | Homes, hardware store | Bulky but practical. |
| 滤水器 | Utility | Base water improvement | Camping store, hardware store | Facility component. |
| 雨水收集瓶 | Utility | Water collection | Crafted, homes | Early survival flavor. |
| 运动饮料 | Drink | Thirst, stamina | Supermarket, gym, vending machines | Better than plain water for stamina. |
| 酒精饮料 | Drink/Trade | Morale, trade, disinfectant | Bar, homes, store | Can overlap with 酒 if simplified. |

## Medical Items

| Item | Category | Primary Use | Suggested Locations | Notes |
| --- | --- | --- | --- | --- |
| 绷带 | Medical | Stop bleeding, basic healing | Homes, hospital, crafted | Core medical item. |
| 消毒酒精 | Medical/Crafting | Infection prevention, crafting | Hospital, pharmacy, bar | Also used for molotovs. |
| 止痛药 | Medical | Pain reduction, short-term recovery | Pharmacy, hospital, homes | Great common medicine. |
| 抗生素 | Medical | Infection treatment | Hospital, pharmacy | Valuable. |
| 医疗箱 | Medical | Strong healing bundle | Hospital, ambulance, police station | Larger, rarer. |
| 急救包 | Medical | Medium healing bundle | Homes, school, cars | Backpack-friendly. |
| 注射器 | Medical/Story | Drug delivery, crafting | Hospital, clinic | Can support girlfriend care. |
| 镇静剂 | Medical/Story | Reduce panic, stabilize girlfriend | Hospital, pharmacy | Links to girlfriend system. |
| 肾上腺素 | Medical | Emergency stamina/revive | Hospital, ambulance | Combat clutch item. |
| 抗病毒药 | Medical/Story | Suppress infection | Hospital, lab, black market | Major story resource. |
| 退烧药 | Medical/Story | Fever control | Pharmacy, homes | Good for girlfriend care. |
| 草药包 | Medical/Crafted | Light healing | Farm, crafted | Ties farming to medicine. |
| 医用胶带 | Medical/Crafting | Bandage crafting, repairs | Hospital, homes | Small utility item. |
| 手术工具 | Medical/Facility | Medical room upgrade | Hospital, clinic | Rare facility component. |
| 血袋 | Medical/Story | Emergency treatment | Hospital | High spoilage if added. |
| 感染抑制剂 | Medical/Story | Delay infection | Hospital, lab | Stronger than antibiotics. |
| 女友专用药剂 | Story Medical | Main quest care | Story, hospital, lab | Unique or rare. |
| 病毒样本 | Story/Trade | Research, endings | Hospital, lab, special infected | Dangerous story item. |

## Ammo And Explosives

| Item | Category | Primary Use | Suggested Locations | Notes |
| --- | --- | --- | --- | --- |
| 手枪弹 | Ammo | Pistol ammo | Police station, homes, black market | MVP ammo type. |
| 步枪弹 | Ammo | Rifle ammo | Police station, military, black market | Mid-game. |
| 霰弹 | Ammo | Shotgun ammo | Police station, hunting store | Distinct combat feel. |
| 冲锋枪弹 | Ammo | SMG ammo | Police station, black market | Could merge with pistol ammo early. |
| 狙击弹 | Ammo | Sniper ammo | Military, police station | Rare. |
| 机枪弹链 | Ammo | Machine gun/turret ammo | Military, police station | Heavy and valuable. |
| 火药 | Ammo Material | Craft ammo/explosives | Police station, factory, crafted | Crafting ingredient. |
| 空弹壳 | Currency/Ammo Material | Currency, ammo crafting | Combat cleanup, police station | Recommended daily currency. |
| 弹头 | Ammo Material | Craft ammo | Police station, factory | Optional if ammo crafting is detailed. |
| 弹匣 | Weapon Part | Reload speed/capacity | Police station, military | Can be attachment or consumable. |
| 燃烧瓶 | Throwable | Area denial, fire damage | Crafted, bar, gas station | Great tavern starter item. |
| 手雷 | Throwable | Explosion | Military, police station | Rare. |
| 土制炸弹 | Throwable/Trap | Explosion, trap | Crafted | Requires chemicals. |
| 遥控炸药 | Explosive | Planned demolition/trap | Military, factory | Late-game. |
| 地雷 | Trap | Defense | Military, police station | High risk/reward. |
| 闪光弹 | Throwable | Stun enemies | Police station, military | Useful for specials. |
| 信号弹 | Utility | Light, signal, distraction | Cars, emergency kits | Can attract help or zombies. |
| 噪音诱饵 | Utility | Lure zombies | Crafted, electronics | Strong tactical item. |
| 烟雾弹 | Utility | Escape, concealment | Police station, crafted | Optional. |

## Weapons And Attachments

| Item | Category | Primary Use | Suggested Locations | Notes |
| --- | --- | --- | --- | --- |
| Glock 手枪 | Weapon | Starter firearm | Tavern, police station | Existing core weapon. |
| 左轮手枪 | Weapon | Reliable pistol | Homes, police station | Slow reload, high damage. |
| 霰弹枪 | Weapon | Close defense | Police station, hunting store | Great for door breaches. |
| 猎枪 | Weapon | Long-range rifle | Homes, hunting store | Early rifle. |
| AK 系列步枪 | Weapon | Assault rifle | Black market, military | Mid-game. |
| M16/M4 | Weapon | Assault rifle | Military, police station | Higher-tier. |
| UZI/冲锋枪 | Weapon | Fast fire | Black market, police station | High noise. |
| 狙击枪 | Weapon | Precision | Police station, military | Rooftop/observation gameplay. |
| 机枪 | Weapon | Heavy defense | Military, police station | Possibly turret weapon. |
| 棒球棍 | Melee | Starter melee | Tavern, homes | Existing flavor. |
| 撬棍 | Tool/Melee | Melee, opening doors | Homes, hardware store | Very useful dual-purpose item. |
| 消防斧 | Tool/Melee | Melee, chopping | Fire station, homes | Strong but heavy. |
| 砍刀 | Melee | Fast melee | Homes, store | Quiet combat. |
| 小刀 | Melee/Tool | Last resort, crafting | Homes, backpacks | Light. |
| 铁管 | Melee/Junk | Basic melee | Factory, street | Common. |
| 消音器 | Attachment | Lower gun noise | Police station, black market | Very valuable in noise system. |
| 长枪管 | Attachment | Range/accuracy | Workbench, police station | Upgrade part. |
| 红点瞄具 | Attachment | Accuracy | Police, military | Common optic. |
| 战术手电 | Attachment | Weapon light | Police, military, electronics | Light helps but exposes. |
| 扩容弹匣 | Attachment | Ammo capacity | Police, military | Familiar upgrade. |
| 快拔握把 | Attachment | Reload/handling | Police, military | Workbench upgrade. |
| 枪托 | Attachment | Stability | Police, military | Rifle/SMG upgrade. |
| 激光瞄准器 | Attachment | Aim speed/accuracy | Military, black market | Draws power? Optional. |
| 清洁工具包 | Weapon Tool | Maintain weapons | Police, homes | Could prevent jams. |
| 武器零件 | Weapon Part | Repairs/upgrades | Police, factory, workbench | Abstract part resource. |

## Tools And Utility Items

| Item | Category | Primary Use | Suggested Locations | Notes |
| --- | --- | --- | --- | --- |
| 扳手 | Tool | Repairs, build mode icon | Hardware, garage | Iconically important. |
| 螺丝刀 | Tool | Electronics, repairs | Homes, garage | Common tool. |
| 锤子 | Tool | Build/repair | Homes, hardware | Basic builder item. |
| 钳子 | Tool | Wire, traps | Hardware, garage | Power/trap support. |
| 锯子 | Tool | Wood crafting | Hardware, homes | Construction support. |
| 斧头 | Tool/Melee | Wood, melee | Homes, hardware | Similar to fire axe. |
| 铲子 | Tool | Farming, burial, digging | Homes, hardware | Strong survival flavor. |
| 焊枪 | Tool | Metal repairs | Garage, factory | Needs fuel maybe. |
| 电钻 | Tool | Build speed | Hardware, factory | Needs power/battery. |
| 胶带 | Tool Material | Repairs, crafting | Homes, hardware | Universal crafting joke, but useful. |
| 绳子 | Tool Material | Traps, hauling | Homes, hardware | Multi-use. |
| 手电筒 | Utility | Light | Homes, cars, police | Requires battery. |
| 打火机 | Utility | Fire, molotov | Homes, store | Small common item. |
| 火柴 | Utility | Fire | Homes, camping | Low-tech backup. |
| 收音机 | Utility/Story | Broadcast, parts | Homes, offices | Can scrap or repair. |
| 对讲机 | Utility | Survivor coordination | Police, school | Useful for NPC missions. |
| 指南针 | Utility | Exploration | Camping, school | Could reduce travel risk. |
| 望远镜 | Utility | Scouting | Homes, school, rooftop | Observation post upgrade. |
| 地图碎片 | Intel | Unlock locations | Homes, school, offices | Exploration progression. |
| 钥匙 | Key | Unlock rooms | Anywhere | Specific keys can be story items. |
| 锁具 | Tool Material | Door upgrades | Hardware, homes | Also lockpicking. |
| 工具箱 | Tool Bundle | Repair bonus | Garage, factory | Heavy but valuable. |

## Electrical And Mechanical Items

| Item | Category | Primary Use | Suggested Locations | Notes |
| --- | --- | --- | --- | --- |
| 小型蓄电池 | Power | Store power, portable power | Cars, electronics | Smaller than base battery. |
| 汽车电瓶 | Power | Battery, vehicle, generator start | Cars, garage | Heavy and useful. |
| 发电机零件 | Mechanical | Repair generator | Garage, hardware, factory | Great quest item. |
| 太阳能板 | Power Facility | Solar power | Rooftops, electronics, school | Big item/facility component. |
| 逆变器 | Electrical | Solar/battery upgrade | Electronics, hardware | Advanced component. |
| 保险丝 | Electrical | Repair circuits | Homes, hardware, offices | Small stackable item. |
| 电线卷 | Electrical | Power grid construction | Hardware, factory | Your cable icon can cover this. |
| 电路板 | Electrical | Radio/turret/electronics | Offices, electronics store | Similar to electronic components. |
| 继电器 | Electrical | Control devices | Factory, electronics | Optional advanced part. |
| 开关盒 | Electrical | Room power control | Hardware, factory | Good build item. |
| 灯泡 | Electrical | Lamps | Homes, hardware | Fragile, common. |
| 探照灯灯泡 | Electrical | Searchlights | Police, factory, hardware | Higher-tier lighting. |
| 马达 | Mechanical | Turret, doors, pumps | Factory, garage | Heavy component. |
| 齿轮组 | Mechanical | Machines, traps | Factory, garage | Mechanical component flavor. |
| 轴承 | Mechanical | Machines | Factory, garage | Optional detail. |
| 火花塞 | Mechanical | Generator, vehicle | Garage, gas station | Good gas station loot. |
| 油管 | Mechanical | Fuel systems | Garage, gas station | Optional. |
| 备用轮胎 | Vehicle | Vehicle repair | Cars, garage | For evacuation/migration. |

## Building And Defense Items

| Item | Category | Primary Use | Suggested Locations | Notes |
| --- | --- | --- | --- | --- |
| 木板 | Building | Barricades, repairs | Homes, school, hardware | Can be item and resource. |
| 金属板 | Building | Reinforced doors, armor | Factory, garage | Higher-tier defense. |
| 钉子 | Building | Wood construction | Hardware, homes | Small stackable. |
| 螺丝 | Building | Metal/electronics construction | Hardware, factory | Can merge with generic parts. |
| 铁丝网 | Defense | Slow zombies | Police, military, hardware | Strong visual asset. |
| 沙袋 | Defense | Cover, barricade | Police, military | Good base defense. |
| 混凝土袋 | Building | Heavy fortification | Construction site, hardware | Heavy. |
| 玻璃板 | Building | Windows, greenhouse | Hardware, homes | Uses glass resource. |
| 防弹玻璃 | Building | Advanced windows | Police, bank | Rare. |
| 门锁 | Building | Door security | Homes, hardware | Upgrade component. |
| 门铰链 | Building | Door repair | Hardware, homes | Optional. |
| 铁门组件 | Building | Reinforced entrance | Factory, police | Large build component. |
| 路障套件 | Defense | Quick barricade | Crafted, hardware | Convenient item form. |
| 炮塔底座 | Defense | Turret construction | Factory, military | Advanced. |
| 炮塔枪管 | Defense | Turret construction | Military, police | Advanced. |
| 陷阱零件 | Defense | Traps | Hardware, factory | Abstract trap part. |
| 尖刺陷阱 | Defense | Damage zombies | Crafted | Could be placed object. |
| 捕兽夹 | Defense | Immobilize enemies | Hunting store, homes | Good low-tech trap. |
| 警报器 | Defense/Utility | Alert, lure | Electronics, police | Noise risk. |
| 铃铛陷阱 | Defense/Utility | Quiet alert | Crafted | Low-tech warning system. |

## Farming And Animal Items

| Item | Category | Primary Use | Suggested Locations | Notes |
| --- | --- | --- | --- | --- |
| 土豆种子 | Farming | Grow potatoes | Supermarket, homes, farm | MVP seed. |
| 玉米种子 | Farming | Grow corn | Supermarket, homes, farm | Later crop. |
| 药草种子 | Farming | Grow herbs | Hospital, homes, garden | Medical loop. |
| 蘑菇菌包 | Farming | Grow mushrooms | Basement, supermarket | Low-light crop. |
| 花盆 | Farming | Indoor planting | Homes, school | Small plot item. |
| 肥料袋 | Farming | Growth boost | Garden store, hardware | Generic fertilizer. |
| 农家肥 | Farming | Long mild fertilizer | Farm, animal pen, compost | Slow but steady. |
| 化学肥 | Farming/Chemical | Short strong fertilizer | Factory, school lab, garden store | Strong but limited. |
| 喷壶 | Farming Tool | Water plants | Homes, garden store | Optional if farming has tools. |
| 农具 | Farming Tool | Farming speed | Homes, garden store | Could be abstract. |
| 饲料 | Animal | Feed animals | Farm, pet store | For later animal system. |
| 鸡蛋 | Food/Animal | Food, cooking | Animal pen, homes | Later. |
| 奶 | Food/Animal | Food, cooking | Animal pen | Later. |
| 肉干 | Food | Preserved food | Crafted, homes | Good travel food. |
| 动物笼 | Animal | Capture/keep animals | Pet store, farm | Later. |
| 简易温室材料 | Farming Building | Greenhouse | Hardware, school | Uses glass/plastic. |

## Trade, Intel, And Story Items

| Item | Category | Primary Use | Suggested Locations | Notes |
| --- | --- | --- | --- | --- |
| 弹壳货币 | Currency | Daily trade, ammo crafting | Combat cleanup, police | Main hard currency candidate. |
| 无线电信用卡/记账本 | Currency/Intel | Display radio credit | Radio contacts, quests | UI artifact more than inventory. |
| 黑市代币 | Currency | Special vendors | Black market, quests | Optional. |
| 旧钞票 | Junk/Story | Scrap, firestarter, flavor | Bank, homes, stores | Not real currency. |
| 金表 | Trade | Barter valuable | Homes, offices | Old-world luxury. |
| 珠宝 | Trade | Barter valuable | Homes, stores | High value, no survival use. |
| 罕见药品 | Trade/Medical | Barter or treatment | Hospital, black market | Can overlap with antivirals. |
| 机密文件 | Intel/Story | Quest, blackmail, lore | Police, hospital, offices | Story hooks. |
| 军用通行证 | Key/Story | Unlock military areas | Military, police | Progression item. |
| 医院门禁卡 | Key/Story | Unlock hospital rooms | Hospital | Good exploration gate. |
| 警局钥匙卡 | Key/Story | Unlock armory | Police station | Great weapon progression. |
| 学校避难所地图 | Intel/Story | Unlock school | Radio, school, survivor | Main chapter hook. |
| 照片 | Story | Character memory | Homes, girlfriend room | Emotional collectible. |
| 女友病历 | Story | Main quest clue | Hospital, clinic | Links to infection. |
| 录音带 | Story | Lore, quest | Homes, offices | Can play at base. |
| 日记本 | Story | Lore, hints | Homes, school | Good worldbuilding. |
| 求救信号纸条 | Story/Intel | Unlock rescue event | Anywhere | Small event hook. |

## Scavenged Junk And Scrap Items

| Item | Category | Primary Use | Suggested Locations | Notes |
| --- | --- | --- | --- | --- |
| 破旧手机 | Junk/Electronics | Scrap into electronics | Homes, offices | Common modern loot. |
| 收音机零件 | Junk/Electronics | Radio repair | Homes, offices | Quest/crafting item. |
| 手表 | Junk/Trade | Barter, scrap | Homes, offices | Could contain small parts. |
| 相机 | Junk/Trade | Barter, parts | Homes, offices | Flavor. |
| 玩具 | Junk/Morale | Morale, trade | Homes, school | Especially for child NPCs. |
| 书 | Junk/Morale | Morale, skill learning | Homes, school | Useful cozy item. |
| 旧衣服 | Junk/Cloth | Scrap into cloth | Homes, stores | Good cloth source. |
| 背包 | Equipment | Increase carry capacity | Homes, school, camping | Useful item and UI category. |
| 塑料瓶 | Junk/Plastic | Scrap, water storage | Homes, street | Common. |
| 空罐头 | Junk/Metal | Scrap, trap, alarm | Homes, supermarket | Great for noisemakers. |
| 废铁 | Junk/Metal | Scrap into metal | Factory, street | Common metal loot. |
| 破电池 | Junk/Power | Scrap, repair into battery | Cars, electronics | Useful in power loop. |
| 车钥匙 | Key/Vehicle | Vehicle events | Homes, cars | Story/exploration hook. |
| 车载工具包 | Tool Bundle | Repairs | Cars, garage | Great car loot. |
| 满油桶 | Fuel Container | Carry gasoline | Gas station, garage | Inventory item for fuel trips. |
| 空油桶 | Fuel Container | Bring fuel back | Gas station, garage | Enables gas station loop. |

## First Asset Priority

If art time is limited, prepare these backpack item icons first:

| Priority | Items |
| --- | --- |
| 1 | 手枪弹, 步枪弹, 霰弹, 空弹壳, 牛肉罐头, 蔬菜罐头, 瓶装水, 绷带, 止痛药, 医疗箱 |
| 2 | 扳手, 手电筒, 满油桶, 空油桶, 土豆种子, 肥料袋, 消毒酒精, 镇静剂, 抗病毒药, 无线电零件 |
| 3 | 燃烧瓶, 噪音诱饵, 消音器, 扩容弹匣, 汽车电瓶, 保险丝, 灯泡, 钉子, 铁丝网, 路障套件 |
| 4 | 金表, 珠宝, 旧钞票, 医院门禁卡, 警局钥匙卡, 女友病历, 录音带, 日记本, 照片 |

## Asset ID Index

Use these snake_case IDs as the preferred asset filenames and stable item identifiers. Suggested icon filename format: `<asset_id>.png`.

### Base Resources

| Asset ID | Item |
| --- | --- |
| `resource_food` | 食物 |
| `resource_water` | 水 |
| `resource_wood` | 木材 |
| `resource_metal` | 金属 |
| `resource_cloth` | 布料 |
| `resource_glass` | 玻璃 |
| `resource_general_parts` | 通用零件 |
| `resource_mechanical_parts` | 机械零件 |
| `resource_electronic_parts` | 电子元件 |
| `resource_plastic_parts` | 塑料零件 |
| `resource_chemicals` | 化学品 |
| `resource_gasoline` | 汽油 |
| `resource_battery` | 蓄电池 |
| `resource_cable` | 电线/电缆 |

### Food And Drinks

| Asset ID | Item |
| --- | --- |
| `food_canned_beef` | 牛肉罐头 |
| `food_canned_vegetables` | 蔬菜罐头 |
| `food_canned_beans` | 豆子罐头 |
| `food_canned_fish` | 鱼罐头 |
| `food_canned_luncheon_meat` | 午餐肉罐头 |
| `food_crackers` | 饼干 |
| `food_hard_tack` | 压缩饼干 |
| `food_chocolate_bar` | 巧克力棒 |
| `food_energy_bar` | 能量棒 |
| `food_instant_noodles` | 方便面 |
| `food_bread` | 面包 |
| `food_potato` | 土豆 |
| `food_corn` | 玉米 |
| `food_mushroom` | 蘑菇 |
| `food_apple` | 苹果 |
| `food_raw_meat` | 生肉 |
| `food_cooked_meat` | 熟肉 |
| `food_rotten` | 腐坏食物 |
| `food_military_ration` | 军用口粮 |
| `food_pet_food` | 宠物粮 |
| `drink_coffee` | 咖啡 |
| `trade_alcohol` | 酒 |
| `trade_cigarettes` | 香烟 |
| `drink_bottled_water` | 瓶装水 |
| `container_canteen` | 水壶 |
| `water_dirty` | 脏水 |
| `medical_water_purification_tablets` | 净水片 |
| `container_water_bucket` | 水桶 |
| `utility_water_filter` | 滤水器 |
| `utility_rain_collector_bottle` | 雨水收集瓶 |
| `drink_sports_drink` | 运动饮料 |
| `drink_alcoholic` | 酒精饮料 |

### Medical

| Asset ID | Item |
| --- | --- |
| `medical_bandage` | 绷带 |
| `medical_disinfectant_alcohol` | 消毒酒精 |
| `medical_painkillers` | 止痛药 |
| `medical_antibiotics` | 抗生素 |
| `medical_medkit` | 医疗箱 |
| `medical_first_aid_kit` | 急救包 |
| `medical_syringe` | 注射器 |
| `medical_sedative` | 镇静剂 |
| `medical_adrenaline` | 肾上腺素 |
| `medical_antiviral` | 抗病毒药 |
| `medical_fever_reducer` | 退烧药 |
| `medical_herbal_pack` | 草药包 |
| `medical_tape` | 医用胶带 |
| `medical_surgical_tools` | 手术工具 |
| `medical_blood_bag` | 血袋 |
| `medical_infection_suppressant` | 感染抑制剂 |
| `story_girlfriend_medicine` | 女友专用药剂 |
| `story_virus_sample` | 病毒样本 |

### Ammo And Explosives

| Asset ID | Item |
| --- | --- |
| `ammo_pistol` | 手枪弹 |
| `ammo_rifle` | 步枪弹 |
| `ammo_shotgun_shell` | 霰弹 |
| `ammo_smg` | 冲锋枪弹 |
| `ammo_sniper` | 狙击弹 |
| `ammo_machine_gun_belt` | 机枪弹链 |
| `material_gunpowder` | 火药 |
| `currency_spent_shells` | 空弹壳 |
| `material_bullet_tips` | 弹头 |
| `weapon_magazine` | 弹匣 |
| `throwable_molotov` | 燃烧瓶 |
| `throwable_grenade` | 手雷 |
| `explosive_pipe_bomb` | 土制炸弹 |
| `explosive_remote_charge` | 遥控炸药 |
| `trap_landmine` | 地雷 |
| `throwable_flashbang` | 闪光弹 |
| `utility_flare` | 信号弹 |
| `utility_noise_lure` | 噪音诱饵 |
| `throwable_smoke_grenade` | 烟雾弹 |

### Weapons And Attachments

| Asset ID | Item |
| --- | --- |
| `weapon_glock_pistol` | Glock 手枪 |
| `weapon_revolver` | 左轮手枪 |
| `weapon_shotgun` | 霰弹枪 |
| `weapon_hunting_rifle` | 猎枪 |
| `weapon_ak_rifle` | AK 系列步枪 |
| `weapon_m16_rifle` | M16/M4 |
| `weapon_uzi_smg` | UZI/冲锋枪 |
| `weapon_sniper_rifle` | 狙击枪 |
| `weapon_machine_gun` | 机枪 |
| `melee_baseball_bat` | 棒球棍 |
| `tool_crowbar` | 撬棍 |
| `tool_fire_axe` | 消防斧 |
| `melee_machete` | 砍刀 |
| `melee_knife` | 小刀 |
| `melee_iron_pipe` | 铁管 |
| `attachment_suppressor` | 消音器 |
| `attachment_long_barrel` | 长枪管 |
| `attachment_red_dot_sight` | 红点瞄具 |
| `attachment_tactical_flashlight` | 战术手电 |
| `attachment_extended_magazine` | 扩容弹匣 |
| `attachment_quickdraw_grip` | 快拔握把 |
| `attachment_stock` | 枪托 |
| `attachment_laser_sight` | 激光瞄准器 |
| `tool_weapon_cleaning_kit` | 清洁工具包 |
| `material_weapon_parts` | 武器零件 |

### Tools And Utility

| Asset ID | Item |
| --- | --- |
| `tool_wrench` | 扳手 |
| `tool_screwdriver` | 螺丝刀 |
| `tool_hammer` | 锤子 |
| `tool_pliers` | 钳子 |
| `tool_saw` | 锯子 |
| `tool_axe` | 斧头 |
| `tool_shovel` | 铲子 |
| `tool_welding_torch` | 焊枪 |
| `tool_power_drill` | 电钻 |
| `material_duct_tape` | 胶带 |
| `material_rope` | 绳子 |
| `utility_flashlight` | 手电筒 |
| `utility_lighter` | 打火机 |
| `utility_matches` | 火柴 |
| `utility_radio` | 收音机 |
| `utility_walkie_talkie` | 对讲机 |
| `utility_compass` | 指南针 |
| `utility_binoculars` | 望远镜 |
| `intel_map_fragment` | 地图碎片 |
| `key_generic` | 钥匙 |
| `material_lock` | 锁具 |
| `tool_toolbox` | 工具箱 |

### Electrical And Mechanical

| Asset ID | Item |
| --- | --- |
| `power_small_battery` | 小型蓄电池 |
| `power_car_battery` | 汽车电瓶 |
| `mechanical_generator_parts` | 发电机零件 |
| `power_solar_panel` | 太阳能板 |
| `electrical_inverter` | 逆变器 |
| `electrical_fuse` | 保险丝 |
| `electrical_wire_spool` | 电线卷 |
| `electrical_circuit_board` | 电路板 |
| `electrical_relay` | 继电器 |
| `electrical_switch_box` | 开关盒 |
| `electrical_light_bulb` | 灯泡 |
| `electrical_searchlight_bulb` | 探照灯灯泡 |
| `mechanical_motor` | 马达 |
| `mechanical_gear_set` | 齿轮组 |
| `mechanical_bearing` | 轴承 |
| `mechanical_spark_plug` | 火花塞 |
| `mechanical_fuel_hose` | 油管 |
| `vehicle_spare_tire` | 备用轮胎 |

### Building And Defense

| Asset ID | Item |
| --- | --- |
| `building_wood_plank` | 木板 |
| `building_metal_plate` | 金属板 |
| `building_nails` | 钉子 |
| `building_screws` | 螺丝 |
| `defense_barbed_wire` | 铁丝网 |
| `defense_sandbag` | 沙袋 |
| `building_concrete_bag` | 混凝土袋 |
| `building_glass_panel` | 玻璃板 |
| `building_bulletproof_glass` | 防弹玻璃 |
| `building_door_lock` | 门锁 |
| `building_door_hinge` | 门铰链 |
| `building_iron_door_kit` | 铁门组件 |
| `defense_barricade_kit` | 路障套件 |
| `defense_turret_base` | 炮塔底座 |
| `defense_turret_barrel` | 炮塔枪管 |
| `defense_trap_parts` | 陷阱零件 |
| `defense_spike_trap` | 尖刺陷阱 |
| `defense_bear_trap` | 捕兽夹 |
| `defense_alarm` | 警报器 |
| `defense_bell_trap` | 铃铛陷阱 |

### Farming And Animals

| Asset ID | Item |
| --- | --- |
| `seed_potato` | 土豆种子 |
| `seed_corn` | 玉米种子 |
| `seed_medicinal_herb` | 药草种子 |
| `seed_mushroom_kit` | 蘑菇菌包 |
| `farming_flower_pot` | 花盆 |
| `farming_fertilizer_bag` | 肥料袋 |
| `farming_compost` | 农家肥 |
| `farming_chemical_fertilizer` | 化学肥 |
| `tool_watering_can` | 喷壶 |
| `tool_farming_tools` | 农具 |
| `animal_feed` | 饲料 |
| `food_egg` | 鸡蛋 |
| `food_milk` | 奶 |
| `food_jerky` | 肉干 |
| `animal_cage` | 动物笼 |
| `building_greenhouse_kit` | 简易温室材料 |

### Trade, Intel, Story, And Junk

| Asset ID | Item |
| --- | --- |
| `currency_shells` | 弹壳货币 |
| `currency_radio_credit_ledger` | 无线电信用卡/记账本 |
| `currency_black_market_token` | 黑市代币 |
| `junk_old_cash` | 旧钞票 |
| `trade_gold_watch` | 金表 |
| `trade_jewelry` | 珠宝 |
| `trade_rare_medicine` | 罕见药品 |
| `story_confidential_files` | 机密文件 |
| `key_military_pass` | 军用通行证 |
| `key_hospital_access_card` | 医院门禁卡 |
| `key_police_access_card` | 警局钥匙卡 |
| `intel_school_shelter_map` | 学校避难所地图 |
| `story_photo` | 照片 |
| `story_girlfriend_medical_record` | 女友病历 |
| `story_audio_tape` | 录音带 |
| `story_diary` | 日记本 |
| `story_distress_note` | 求救信号纸条 |
| `junk_old_phone` | 破旧手机 |
| `junk_radio_parts` | 收音机零件 |
| `junk_watch` | 手表 |
| `junk_camera` | 相机 |
| `junk_toy` | 玩具 |
| `junk_book` | 书 |
| `junk_old_clothes` | 旧衣服 |
| `equipment_backpack` | 背包 |
| `junk_plastic_bottle` | 塑料瓶 |
| `junk_empty_can` | 空罐头 |
| `junk_scrap_metal` | 废铁 |
| `junk_broken_battery` | 破电池 |
| `key_car_key` | 车钥匙 |
| `tool_vehicle_toolkit` | 车载工具包 |
| `fuel_can_full` | 满油桶 |
| `fuel_can_empty` | 空油桶 |

## Notes For Future Balancing

- Avoid adding every listed item to the first playable version.
- Keep MVP resources readable and small.
- Use detailed backpack items to create scavenging texture, but convert many of them into base resources when returned to storage.
- Items with strong story flavor should have unique descriptions and limited locations.
- Items that affect combat or survival need weight and stack size early, because they shape exploration choices.
