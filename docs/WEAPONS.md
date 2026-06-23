# Weapon Catalog Draft

This document is a planning catalog for weapon assets, weapon metadata, upgrade paths, and future balancing. The current runtime metadata lives in `assets/weapons/weapons.json`; this file is for design planning before assets and JSON entries are finalized.

## Current Runtime Metadata

The current `assets/weapons/weapons.json` stores each weapon with these fields:

| Field | Meaning |
| --- | --- |
| `name` | Display name or current internal name. |
| `image_path` | In-hand weapon sprite path. Current files use names such as `hold glock.png`. |
| `preview_image_path` | Large UI preview image path. |
| `icon_image_path` | Small UI icon path, currently present for some weapons. |
| `ui_card_template` | UI card style, such as `pistol`, `rifle`, `shotgun`, `sniper`. |
| `route_x` | Weapon hold/render offset or aiming route parameter. |
| `route_y` | Weapon hold/render offset or aiming route parameter. |
| `type` | Current weapon behavior type. Existing examples use `101` for bullets and `102` for grenade launcher. |
| `magazine_size` | Rounds per magazine. |
| `reload_duration` | Reload time in seconds. |
| `diameter` | Caliber/diameter value, currently numeric. |
| `up` | Current recoil/upward kick parameter. |
| `damage` | Damage per projectile or shot. |
| `full_auto` | Whether holding fire continues shooting. |
| `speed_rpm` | Fire rate in rounds per minute. |
| `price` | Current price value. Future economy should use shells, radio credit, or barter value. |
| `shoot_sound_path` | Weapon firing sound path. |

## Existing Weapon Entries

| Asset ID | Current Name | Category | Current Metadata Notes | Asset Notes |
| --- | --- | --- | --- | --- |
| `weapon_glock_pistol` | Glock | Pistol | 15 mag, 1.5 reload, 9mm, 20 damage, 600 rpm, semi-auto | Has hold, preview, icon, depth. |
| `weapon_desert_eagle` | Desert Eagle | Pistol | 8 mag, 1.5 reload, 12.7mm, 60 damage, 60 rpm, semi-auto | Has hold, preview, icon, depth. |
| `weapon_ak47` | AK47 | Assault Rifle | 30 mag, 1.5 reload, 7.62mm, 50 damage, 600 rpm, full-auto | Has hold, preview, icon, depth. |
| `weapon_ak47_grenade_launcher` | Grenade on AK47 | Launcher Attachment | 1 mag, 40mm, 100 damage, behavior type 102 | Has hold sprite; preview temporarily falls back to hold sprite until a larger preview is added. |
| `weapon_awm` | AWM | Sniper Rifle | 10 mag, 1.5 reload, 7.62mm, 100 damage, 60 rpm, semi-auto | Has hold, preview, depth. |
| `weapon_aug` | AUG | Assault Rifle | 30 mag, 2.0 reload, 5.56mm, 30 damage, 800 rpm, full-auto | Has hold, preview, icon, depth. |
| `weapon_m249` | M249 | Light Machine Gun | 100 mag, 3.0 reload, 5.56mm, 20 damage, 750 rpm, full-auto | Has hold sprite and sounds; preview temporarily falls back to hold sprite until a larger preview is added. |
| `weapon_p90` | P90 | SMG/PDW | Not yet in `weapons.json` | Preview asset exists. |

## Future Metadata Schema

Future JSON can stay simple at first, but these fields are useful once weapons become collectible and upgradeable.

| Field | Meaning |
| --- | --- |
| `id` | Stable snake_case weapon ID. |
| `display_name` | UI display name. |
| `category` | `pistol`, `smg`, `assault_rifle`, `shotgun`, `sniper`, `lmg`, `special`, `melee`. |
| `tier` | Rough progression tier, for example 1 to 5. |
| `ammo_type` | `ammo_pistol`, `ammo_rifle`, `ammo_shotgun_shell`, `fuel_gasoline`, etc. |
| `damage` | Base damage per projectile. |
| `pellet_count` | For shotguns or special weapons. |
| `fire_rate_rpm` | Fire rate. |
| `magazine_size` | Base magazine size. |
| `reload_duration` | Base reload time. |
| `accuracy` | 0-100 accuracy stat for UI. |
| `stability` | 0-100 recoil control stat for UI. |
| `range` | Effective range stat. |
| `noise` | Noise generated per shot. Important for zombie attraction. |
| `weight_kg` | Inventory/loadout weight. |
| `handling` | Aim speed or movement penalty. |
| `armor_piercing` | Armor penetration modifier. |
| `crit_chance` | Optional crit/headshot helper. |
| `headshot_multiplier` | Optional sniper/high-skill stat. |
| `durability` | Optional weapon condition. |
| `allowed_mod_slots` | Attachment slots available to this weapon. |
| `upgrade_paths` | Named upgrade branches, such as stealth, tactical, brute force. |
| `base_value_shells` | Trade value in shell currency. |
| `unlock_source` | Tavern, police station, black market, military zone, story. |
| `image_path` | In-hand sprite. |
| `preview_image_path` | Large UI sprite. |
| `icon_image_path` | Small inventory icon. |
| `depth_image_path` | Optional depth/lighting helper sprite if used. |
| `shoot_sound_path` | Fire sound. |

## Attachment Slot Types

| Slot ID | Slot Name | Typical Effects |
| --- | --- | --- |
| `muzzle` | 枪口 | Suppressor, brake, compensator, flash hider. |
| `barrel` | 枪管 | Range, accuracy, pellet spread, damage. |
| `optic` | 瞄具 | Accuracy, headshot ease, night vision, thermal. |
| `underbarrel` | 下挂 | Grip, flashlight, laser, grenade launcher. |
| `magazine` | 弹匣 | Capacity, reload speed, weight. |
| `stock` | 枪托 | Stability, handling, recoil. |
| `grip` | 握把 | Handling, reload speed, stability. |
| `ammo_mod` | 弹药改造 | Armor piercing, incendiary, explosive, electric. |
| `receiver` | 机匣/枪机 | Fire rate, reliability, full-auto conversion. |
| `skin` | 涂装 | Cosmetic or morale flavor. |

## Weapon Design Principles

- Do not make each new weapon a pure DPS upgrade.
- Give each weapon a clear fantasy: quiet, reliable, brutal, precise, crowd-control, boss-killer, base-defense.
- Let old weapons remain useful through cheap ammo, low noise, fast handling, or special upgrades.
- Noise matters as much as damage because gunfire attracts zombies.
- Magazine size and reload time should create tension during door breaches.
- Strong late-game weapons should consume rare ammo, fuel, battery charge, or special parts.
- Pixel-art production can reuse 8-10 weapon skeletons with different barrels, stocks, optics, magazines, colors, and attachments.

## Pistols

| Asset ID | Display Name | Role | Base Fantasy | Upgrade Direction | Asset Status |
| --- | --- | --- | --- | --- | --- |
| `weapon_glock17` | Glock 17 | Starter pistol | Cheap, common ammo, easy repair | Extended mag, suppressor, red dot, fast reload | Existing Glock can represent this. |
| `weapon_beretta_m9` | Beretta M9 | Stable pistol | High stability, accurate sidearm | Match trigger, laser sight, lightweight slide | Needed. |
| `weapon_five_seven` | Five-Seven | Armor-piercing pistol | Punches through armored zombies | AP ammo, precision barrel, tactical optic | Needed. |
| `weapon_desert_eagle` | Desert Eagle | Hand cannon | Huge single-shot damage, loud | Heavy barrel, muzzle brake, gold variant | Existing. |
| `weapon_revolver_357` | .357 Revolver | Reliable high-impact sidearm | Slow reload, high stopping power | Speed loader, long barrel, custom grip | Needed. |
| `weapon_silenced_pistol` | Integrally Suppressed Pistol | Stealth sidearm | Low noise, low damage | Subsonic ammo, quiet slide, larger mag | Needed. |

### Pistol Notes

- Pistols should stay relevant because they are light, fast, and often quieter than rifles.
- Starter ammo can be `ammo_pistol`.
- Best use cases: emergency backup, indoor exploration, quiet cleanup.

## SMGs And PDWs

| Asset ID | Display Name | Role | Base Fantasy | Upgrade Direction | Asset Status |
| --- | --- | --- | --- | --- | --- |
| `weapon_ump45` | UMP45 | Heavy SMG | Higher damage, slower SMG | Extended mag, suppressor, tactical grip | Needed. |
| `weapon_mp5` | MP5 | Stable SMG | Smooth, reliable, controllable | Collapsible stock, competition bolt, suppressor | Needed. |
| `weapon_vector` | Vector | Close-range shredder | Very high fire rate, hungry ammo use | Drum mag, electric rounds, recoil kit | Needed. |
| `weapon_p90` | P90 | High-capacity PDW | Large mag, compact, good for hordes | 100-round module, optic, armor-piercing 5.7 | Preview exists, JSON needed. |
| `weapon_uzi` | UZI | Cheap spray weapon | Rough, noisy, high fire rate | Stock, barrel, extended mag | Needed. |
| `weapon_mac10` | MAC-10 | Black market bullet hose | Wild recoil, brutal indoors | Suppressor, wire stock, drum mag | Needed. |

### SMG Notes

- SMGs should dominate close indoor fights but burn ammo quickly.
- They can be noisy if fired full-auto, creating resource tension.

## Assault Rifles

| Asset ID | Display Name | Role | Base Fantasy | Upgrade Direction | Asset Status |
| --- | --- | --- | --- | --- | --- |
| `weapon_ak47` | AK-47 | High-damage rifle | Reliable, loud, heavy hit | Rage path: damage up, stability down; tactical path: stability/accuracy | Existing. |
| `weapon_ak74` | AK-74 | Accurate AK variant | Lower damage, better control | Precision barrel, brake, optic | Needed. |
| `weapon_m4a1` | M4A1 | Modular all-rounder | Most upgrade slots, “LEGO gun” | Muzzle, barrel, grip, stock, optic, mag | Needed. |
| `weapon_aug` | AUG | Optic rifle | Built-in scope, good mid-range | Thermal, night vision, stability kit | Existing. |
| `weapon_scar_h` | SCAR-H | Heavy battle rifle | Mid/late-game powerhouse | AP ammo, explosive rounds, heavy stock | Needed. |
| `weapon_fn_fal` | FN FAL | Semi-auto marksman rifle | High damage, semi-auto precision | Scope, long barrel, AP receiver | Needed. |
| `weapon_g36c` | G36C | Lightweight rifle | Fast handling, lower damage | Reflex optic, light stock, suppressor | Needed. |
| `weapon_hk416` | HK416 | Premium tactical rifle | Accurate, reliable, expensive | High-end tactical upgrade tree | Needed. |

### Assault Rifle Notes

- Rifles are the main mid-game workhorse.
- M4A1 should have the most attachment flexibility.
- AK47 should be stronger early but louder and harder to stabilize.

## Shotguns

| Asset ID | Display Name | Role | Base Fantasy | Upgrade Direction | Asset Status |
| --- | --- | --- | --- | --- | --- |
| `weapon_m870` | Remington M870 | Reliable pump shotgun | Door defense, simple and strong | Long barrel, dragon's breath, slug rounds | Needed. |
| `weapon_spas12` | SPAS-12 | Tactical shotgun | Flexible combat shotgun | Folding stock, dual-mode firing | Needed. |
| `weapon_aa12` | AA-12 | Automatic shotgun | Horde mower, very loud | 32-round drum, recoil kit | Needed. |
| `weapon_ks23` | KS-23 | Monster killer shotgun | Highest single-shot shotgun damage | Heavy slug, breaching rounds | Needed. |
| `weapon_double_barrel_shotgun` | Double-Barrel Shotgun | Early burst weapon | Two huge shots, long reload | Sawed-off, reinforced stock | Needed. |
| `weapon_sawed_off_shotgun` | Sawed-Off Shotgun | Panic sidearm | Close-range burst | Quick reload, wider spread | Needed. |

### Shotgun Notes

- Shotguns should be scary in corridors and doorway breaches.
- Ammo should be heavy and less common than pistol ammo.
- Dragon's breath and slugs can create strong build variety.

## Sniper And Marksman Rifles

| Asset ID | Display Name | Role | Base Fantasy | Upgrade Direction | Asset Status |
| --- | --- | --- | --- | --- | --- |
| `weapon_remington_700` | Remington 700 | Starter sniper | Cheap hunting rifle style | Better scope, long barrel | Needed. |
| `weapon_awm` | AWM | Boss killer | High damage, precision | Penetration, headshot multiplier, stability | Existing. |
| `weapon_m82_barrett` | M82 Barrett | Anti-materiel rifle | Extreme damage, huge noise | AP rounds, explosive rounds | Needed. |
| `weapon_vss` | VSS | Stealth marksman rifle | Integrated suppressor, low noise | Subsonic ammo, optic, larger mag | Needed. |
| `weapon_m14_ebr` | M14 EBR | Semi-auto marksman | Sustained precision fire | Scope, stock, AP ammo | Needed. |
| `weapon_crossbow` | Crossbow | Silent special marksman | Quiet, recoverable bolts | Broadhead, explosive bolt, rope bolt | Optional but very thematic. |

### Sniper Notes

- Snipers are not just more damage; they should enable rooftop defense, boss hunting, and high-value target removal.
- Noise cost should be high except for VSS/crossbow.

## Light Machine Guns

| Asset ID | Display Name | Role | Base Fantasy | Upgrade Direction | Asset Status |
| --- | --- | --- | --- | --- | --- |
| `weapon_rpk` | RPK | AK-family LMG | Reliable base-defense rifle | 75-round drum, heavy barrel | Needed. |
| `weapon_m249` | M249 | Sustained fire LMG | Long fire support, high ammo use | 150-round belt, bipod, cooling | Existing. |
| `weapon_pkm` | PKM | Heavy damage LMG | Highest LMG damage, heavy recoil | Explosive tips, heavy tripod | Needed. |
| `weapon_mg42` | MG42 | Rare antique buzzsaw | Terrifying fire rate, rare parts | Cooling kit, belt feed upgrade | Optional rare weapon. |

### LMG Notes

- LMGs are excellent for horde nights, but heavy, loud, and ammo-hungry.
- They fit NPC defense posts and turret systems.

## Special Weapons

| Asset ID | Display Name | Role | Base Fantasy | Upgrade Direction | Asset Status |
| --- | --- | --- | --- | --- | --- |
| `weapon_flamethrower` | 火焰喷射器 | Area denial | Burns crowds and blocks entrances | Burn time, spray distance, fuel efficiency | Needed. |
| `weapon_railgun` | 电磁枪 | Tech endgame | Charge shot, pierces multiple enemies | Penetration count, charge speed | Needed. |
| `weapon_grenade_launcher` | 榴弹发射器 | Explosive crowd control | Arc shots, blast radius | Explosion radius, fire zone | Needed. |
| `weapon_rpg` | RPG | Heavy explosive | Boss/horde burst | Tracking rocket, cluster warhead | Needed. |
| `weapon_plasma_rifle` | Plasma Rifle | Sci-fi endgame | High tech, battery ammo | Heat control, splash damage | Optional endgame. |
| `weapon_tesla_cannon` | Tesla Cannon | Crowd stun | Chains electricity between zombies | Chain count, stun duration | Optional endgame. |

### Special Weapon Notes

- These should come from late base tech, black market, military sites, or story research.
- They should consume fuel, battery charge, special ammo, or rare parts.

## Melee Weapons

| Asset ID | Display Name | Role | Base Fantasy | Upgrade Direction | Asset Status |
| --- | --- | --- | --- | --- | --- |
| `melee_baseball_bat` | 棒球棍 | Starter melee | Cheap, simple, light knockback | Nails, reinforced grip | Existing item concept. |
| `tool_fire_axe` | 消防斧 | Heavy melee/tool | High damage, slow, utility | Sharpened edge, weighted head | Item concept. |
| `melee_katana` | 武士刀 | Fast crit melee | High speed, high crit | Sharpening, grip wrap | Needed. |
| `melee_sledgehammer` | 铁锤 | Knockback/armor break | Slow, strong stagger | Heavy head, shock grip | Needed. |
| `melee_chainsaw` | 电锯 | Continuous damage | Fuel/noise tradeoff | Larger tank, sharper chain | Needed. |
| `melee_stun_baton` | 电棍 | Stun melee | Low damage, paralysis | Battery capacity, stun duration | Needed. |
| `tool_crowbar` | 撬棍 | Utility melee | Opens doors, quiet weapon | Reinforced hook | Item concept. |
| `melee_machete` | 砍刀 | Light blade | Fast, quiet, low stamina cost | Serrated edge | Item concept. |

### Melee Notes

- Melee is essential for quiet exploration and ammo conservation.
- Strong melee should cost stamina, durability, exposure risk, or noise.

## Build Archetypes

| Build | Primary Weapon | Secondary Weapon | Fantasy | Notes |
| --- | --- | --- | --- | --- |
| 特种兵 | M4A1 | Glock 17 | High mobility and flexibility | Good all-rounder. |
| 尸潮清理工 | AA-12 | M249 | Crowd clearing | Ammo hungry and loud. |
| 猎杀 Boss | AWM | Desert Eagle | Single-target burst | High skill, low crowd control. |
| 工程师 | RPK | Flamethrower | Base defense specialist | Works well with turrets and traps. |
| 科技狂人 | Railgun | Plasma Rifle | Endgame technology | Requires rare power resources. |
| 潜行搜刮者 | VSS | Suppressed Pistol | Quiet exploration | Low threat, lower burst damage. |
| 酒馆守门人 | M870 | Baseball Bat | Early base defense | Strong first-chapter fantasy. |

## First Asset Priority

If art time is limited, prepare these weapon assets first:

| Priority | Weapons | Why |
| --- | --- | --- |
| 1 | Glock 17, Desert Eagle, AK47, M870, AWM | Covers pistol, hand cannon, rifle, shotgun, sniper. |
| 2 | M4A1, MP5, P90, M249, Fire Axe | Adds modular rifle, SMG, high-capacity PDW, LMG, melee. |
| 3 | AUG, SCAR-H, AA-12, VSS, RPK | Adds mid/late progression and stealth. |
| 4 | Flamethrower, Grenade Launcher, Railgun, RPG | Late-game tech and special weapons. |

## Suggested Asset Bundle Convention

For new weapon assets, prefer snake_case and keep each weapon in its own folder:

```text
assets/weapons/<weapon_id>/preview.png
assets/weapons/<weapon_id>/icon.png
assets/weapons/<weapon_id>/preview_depth.png
assets/weapons/<weapon_id>/hold.png
assets/weapons/<weapon_id>/sounds/<sound_name>.wav
```

Examples:

```text
assets/weapons/weapon_m4a1/preview.png
assets/weapons/weapon_m4a1/icon.png
assets/weapons/weapon_m4a1/preview_depth.png
assets/weapons/weapon_m4a1/hold.png
assets/weapons/weapon_m4a1/sounds/m4a1.wav
```

Older files used flat legacy names such as `hold AK47.png`; new or touched weapon assets should use the folder bundle convention.

## Migration Notes For `weapons.json`

Potential future entry shape:

```json
{
  "id": "weapon_m4a1",
  "display_name": "M4A1",
  "category": "assault_rifle",
  "tier": 3,
  "ammo_type": "ammo_rifle",
  "image_path": "assets/weapons/weapon_m4a1/hold.png",
  "preview_image_path": "assets/weapons/weapon_m4a1/preview.png",
  "icon_image_path": "assets/weapons/weapon_m4a1/icon.png",
  "ui_card_template": "rifle",
  "route_x": 3,
  "route_y": 2,
  "type": 101,
  "magazine_size": 30,
  "reload_duration": 1.8,
  "diameter": 5.56,
  "up": 8.0,
  "damage": 32,
  "full_auto": true,
  "speed_rpm": 750,
  "noise": 75,
  "accuracy": 82,
  "stability": 72,
  "range": 70,
  "allowed_mod_slots": ["muzzle", "barrel", "optic", "underbarrel", "magazine", "stock"],
  "price_shells": 900,
  "unlock_source": "black_market_or_police_station",
  "shoot_sound_path": "assets/weapons/weapon_m4a1/sounds/m4a1.wav"
}
```

Do not migrate all fields at once unless the runtime needs them. It is fine to start by adding `id`, `ammo_type`, `noise`, and `allowed_mod_slots`, then expand later.
