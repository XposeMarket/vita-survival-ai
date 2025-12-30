# Quick Start Guide - Vita Survival AI

## Prerequisites

1. **PS Vita with HENkaku/h-encore/taiHEN** installed
2. **VitaShell** or similar file manager
3. **VitaSDK** (for building from source)
4. **SD2Vita** recommended (app uses significant storage)

## Installation Steps

### Option 1: Install Pre-built VPK (Easiest)

1. Download `SurvivalAI.vpk`
2. Copy to your Vita via USB or FTP
3. Install using VitaShell (X on the VPK file)
4. App will appear in LiveArea

### Option 2: Build from Source

See main README.md for build instructions.

## Initial Setup

### Step 1: Launch App
1. Launch "Survival AI" from LiveArea
2. App will create directory structure at `ux0:data/survivalkit/`
3. First launch may take a moment

### Step 2: Add Wikipedia (Recommended)
1. Download Wikipedia ZIM file from https://wiki.kiwix.org
2. Recommended: `wikipedia_en_simple_all.zim` (~1GB) for first-time users
3. Copy to `ux0:data/survivalkit/zim/wikipedia_en.zim`
4. Restart app to load Wikipedia

### Step 3: Try It Out!
1. From main menu, select "Ask"
2. Press X to enter a question
3. Try: "What is first aid?"
4. View results with sources

## Basic Usage

### Navigation
- **D-Pad**: Move selection
- **X (Cross)**: Select/Confirm
- **O (Circle)**: Back/Cancel
- **△ (Triangle)**: Speak answer (voice mode)
- **Start+Select**: Exit app

### Asking Questions
1. Main Menu → "Ask"
2. Press X
3. Type your question
4. View structured answer with sources

### Example Questions
- "How to treat a burn"
- "What is hypothermia"
- "How to purify water"
- "How to build a fire"

## Adding Content

### Add Wikipedia (Offline Encyclopedia)
```
ux0:data/survivalkit/zim/wikipedia_en.zim
```
Download from: https://wiki.kiwix.org

### Add Vault Data (Web References)
Use PC Collector tool:
```bash
python tools/pc_collector.py --urls "https://example.com/article" --output vault_pack
```
Then copy vault_pack to Vita

### Add Voice Pack (Optional)
See docs/VOICE_PACK_GUIDE.md

## Troubleshooting

### App won't launch
- Ensure you have enough free space (100MB+ recommended)
- Check that VITASDK modules are installed
- Reinstall VPK

### "No Wikipedia file found"
- Verify ZIM file is at `ux0:data/survivalkit/zim/wikipedia_en.zim`
- Check file isn't corrupted (re-download if needed)
- Ensure you have enough space for the ZIM file

### Search returns no results
- Make sure you've added Wikipedia or vault data
- Try simpler search terms
- Check database file isn't corrupted

### Voice not working
- Voice pack is optional
- Check voice pack is at `ux0:data/survivalkit/voice/pack/`
- Verify manifest.json exists in voice pack

### Low performance
- Use smaller Wikipedia ZIM file
- Clear cache: Delete `ux0:data/survivalkit/cache/`
- Restart Vita

## Getting Help

- Check README.md for detailed info
- See docs/ folder for guides
- File issues on GitHub

## Next Steps

1. ✅ Install app
2. ✅ Add Wikipedia ZIM
3. ⬜ Use PC Collector to build vault
4. ⬜ Create voice pack
5. ⬜ Add survival manuals

See docs/DATA_COLLECTION_GUIDE.md for detailed content setup.

## Safety Notice

This app provides reference information only. In real emergencies:
- Call emergency services (911, 112, etc.)
- Seek professional medical help
- Do not rely solely on this app for critical decisions
- Practice survival skills BEFORE emergencies

Stay safe!
