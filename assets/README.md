# Asset Layout

This folder is the start of the modern asset pipeline for the SDL3 version.

## Intended Structure

- `assets/backgrounds`
  Sky, skyline, building, and other environment textures used directly by the SDL3 runtime.
- `assets/characters`
  Hero, zombie, NPC sprite sheets plus metadata.
- `assets/effects`
  Explosion, smoke, grenade, and other effect atlases plus metadata.
- `assets/ui`
  Control skins, icon sheets, and UI metadata.
- `assets/weapons`
  Weapon sprites, shop images, and weapon metadata.
- `assets/collision`
  Collision and trigger source images during the current migration phase.

## First Migration Target

`boom1_*.bmp` in `image/boom/` can be packed into a single horizontal strip with:

```powershell
python tools/pack_spritesheet.py `
  --input-dir image/boom `
  --pattern "boom1_*.bmp" `
  --output-image assets/effects/boom1_strip.png `
  --output-meta assets/effects/boom1_strip.sheet
```

At runtime, the SDL3 prototype now reads directly from `assets/`.

## Existing Character Sheets

The hero and zombie walk strips already exist as packed sprite sheets in the legacy `image/` folder. Their modern metadata now lives in:

- `assets/characters/man1.sheet`
- `assets/characters/zom1.sheet`

This lets the SDL3 branch use the same metadata-driven animation path with the images and metadata living under one stable asset root.
