# VPK Icon Requirements

For the VPK to build successfully, you need to create these image files:

## Required Images

### 1. icon0.png
- **Location**: `sce_sys/icon0.png`
- **Size**: 128x128 pixels
- **Format**: PNG with transparency
- **Usage**: App icon in LiveArea

**How to create**:
```bash
# Using ImageMagick
convert -size 128x128 xc:blue -pointsize 32 -draw "text 20,70 'Survival AI'" icon0.png

# Or use any image editor and create a 128x128 icon
```

### 2. bg.png
- **Location**: `sce_sys/livearea/contents/bg.png`
- **Size**: 840x500 pixels
- **Format**: PNG
- **Usage**: LiveArea background

**How to create**:
```bash
# Simple gradient background
convert -size 840x500 gradient:blue-darkblue bg.png

# Or create custom background in Photoshop/GIMP
```

### 3. startup.png
- **Location**: `sce_sys/livearea/contents/startup.png`
- **Size**: 280x158 pixels
- **Format**: PNG
- **Usage**: App startup screen

**How to create**:
```bash
# Simple splash screen
convert -size 280x158 xc:black -pointsize 24 -fill white \
  -draw "text 60,80 'Survival AI'" startup.png
```

## Quick Setup Script

Save as `create_icons.sh`:

```bash
#!/bin/bash
# Create placeholder icons for VPK

echo "Creating placeholder icons..."

# Create directories
mkdir -p sce_sys/livearea/contents

# icon0.png (128x128 - blue with text)
convert -size 128x128 xc:"#0078D7" -pointsize 20 -fill white \
  -gravity center -draw "text 0,0 'Survival\nAI'" sce_sys/icon0.png

# bg.png (840x500 - gradient background)
convert -size 840x500 gradient:"#0078D7"-"#004080" \
  sce_sys/livearea/contents/bg.png

# startup.png (280x158 - simple splash)
convert -size 280x158 xc:"#000000" -pointsize 28 -fill white \
  -gravity center -draw "text 0,0 'Survival AI'" \
  sce_sys/livearea/contents/startup.png

echo "Icons created successfully!"
ls -lh sce_sys/icon0.png
ls -lh sce_sys/livearea/contents/bg.png
ls -lh sce_sys/livearea/contents/startup.png
```

Run: `chmod +x create_icons.sh && ./create_icons.sh`

## Manual Creation (Without ImageMagick)

If you don't have ImageMagick, create the images manually:

1. Open any image editor (GIMP, Photoshop, Paint.NET, etc.)
2. Create new images with the sizes above
3. Add text/graphics as desired
4. Export as PNG
5. Place in the correct locations

## Design Tips

**icon0.png**:
- Should be recognizable at small size
- Use clear, simple graphics
- Consider using a compass, first aid cross, or survival symbol
- High contrast for visibility

**bg.png**:
- Can be more detailed
- Consider survival-themed imagery (compass, map, wilderness)
- Keep it professional but distinctive

**startup.png**:
- Simple splash screen
- App name clearly visible
- Quick to render

## Alternative: Use Provided Icons

Download pre-made icons from:
- VitaSDK sample projects
- Community icon packs
- Create your own in Figma/Canva (free tools)

Just ensure they match the required dimensions.
