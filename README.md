# PS Vita Survival AI - Offline Knowledge & AI Assistant

A PS Vita homebrew application that provides offline knowledge access, survival information, and an "AI-like" question-answering system using retrieval and structured reasoning.

## Features

### Core Functionality
- **Ask Mode**: Query-based knowledge retrieval with structured answers
- **Offline Wikipedia**: Browse and search Wikipedia offline via ZIM files
- **Vault**: Saved web references and documents for offline access
- **Voice Output**: Text-to-speech with optional voice pack for natural-sounding responses
- **Scenarios**: Quick access to survival guides (bleeding, burns, shelter, etc.)
- **Toolkit**: Useful utilities (SOS signals, Morse code, unit converter)

### Answer Types
- Direct factual answers
- Step-by-step instructions
- Quote collection (with source attribution)
- Comprehensive summaries

### Evidence-Based Responses
- All answers cite sources
- Publication and retrieval dates shown
- Confidence scoring
- Domain and author attribution

## Building from Source

### Prerequisites
1. **VitaSDK** installed and `$VITASDK` environment variable set
2. **SQLite3** source (included in libs/ directory)
3. **CMake** 3.0 or later

### Dependencies to Install
```bash
# Install VitaSDK libraries
vdpm install vita2d
vdpm install freetype
vdpm install libpng
vdpm install zlib
```

### Build Steps
```bash
# Clone the repository
git clone <repo-url>
cd vita-survival-ai

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make

# This creates SurvivalAI.vpk
```

### Installing on PS Vita
1. Copy `SurvivalAI.vpk` to your Vita
2. Install using VitaShell or similar
3. The app will create necessary directories on first run at `ux0:data/survivalkit/`

## Data Setup

### Directory Structure
The app expects data in this structure on your SD card:
```
ux0:data/survivalkit/
├── zim/                    # Wikipedia ZIM files
│   └── wikipedia_en.zim
├── vault/                  # Saved references
│   ├── items/
│   └── media/
├── db/
│   └── vault.sqlite        # Main database
├── cache/
└── voice/
    └── pack/               # Voice clips (.ogg)
```

### Getting Wikipedia Offline (ZIM Files)
1. Download ZIM files from https://wiki.kiwix.org/wiki/Content_in_all_languages
2. Recommended: `wikipedia_en_all_maxi.zim` (full Wikipedia) or `wikipedia_en_simple_all.zim` (smaller)
3. Place in `ux0:data/survivalkit/zim/`
4. Rename to `wikipedia_en.zim` or update code to match your filename

**Note**: Large ZIM files (20GB+) work but may be slower. Consider the "mini" versions for better performance.

### Setting Up the Vault Database

The vault database is populated using the PC Collector tool (see below). For initial testing, you can create an empty database:

```bash
# On your PC, create initial database
sqlite3 vault.sqlite < schema.sql

# Transfer to Vita
cp vault.sqlite /path/to/vita/ux0/data/survivalkit/db/
```

## PC Collector Tool (Recommended Setup)

The PC Collector is a companion tool that runs on your PC to gather, process, and package web references for the Vita.

### Why PC Collector?
- Web scraping is resource-intensive for Vita
- Better extraction quality
- Faster indexing
- Can run scheduled updates

### PC Collector Setup (Python)
```bash
# Install dependencies
pip install requests beautifulsoup4 readability-lxml feedparser

# Run collector
python pc_collector.py --query "survival techniques" --output vault_pack/
```

### PC Collector Features
- RSS feed monitoring
- Web scraping with rate limiting
- Text extraction and cleaning
- Automatic tagging
- Builds SQLite database with FTS5 index
- Exports "Vault Pack" for Vita sync

### Syncing Vault Packs to Vita
1. Run PC Collector to generate new vault pack
2. Connect Vita via USB
3. Copy `vault.sqlite` and `items/` folder to Vita
4. Or use network sync (if implemented)

## Voice System Setup

### Voice Pack (High Quality)
The voice pack contains pre-recorded .ogg clips for common phrases.

**Creating a Voice Pack:**
1. Use TTS engine on PC (e.g., Google TTS, AWS Polly, Azure TTS)
2. Generate clips for common phrases:
   - Numbers (0-100)
   - Units (meters, feet, celsius, fahrenheit)
   - Step phrases ("step one", "step two", etc.)
   - Warnings ("warning", "danger", "caution")
   - Survival terms ("water", "shelter", "fire", "first aid")

3. Convert to Ogg Vorbis (48kHz, stereo):
```bash
ffmpeg -i input.wav -c:a libvorbis -q:a 6 -ar 48000 -ac 2 output.ogg
```

4. Create manifest.json:
```json
{
  "clips": [
    {"text": "step one", "file": "step_1.ogg", "duration_ms": 850},
    {"text": "step two", "file": "step_2.ogg", "duration_ms": 820}
  ]
}
```

5. Copy to `ux0:data/survivalkit/voice/pack/`

### Fallback TTS (Fully Offline)
For missing phrases, the app can use eSpeak NG or Flite (if integrated):
- eSpeak NG: Robotic but very lightweight
- Flite: Better quality, slightly larger

**Note**: TTS integration is marked as TODO in current implementation.

## Usage Guide

### Main Menu
- **Ask**: Enter questions to get structured answers
- **Library**: Browse saved content
- **Wikipedia**: Search and read Wikipedia offline
- **Vault**: Search saved web references
- **Manuals**: Survival and first aid guides
- **Scenarios**: Quick scenario guides
- **Toolkit**: Utility tools
- **Sync**: Update vault packs

### Controls
- **D-Pad Up/Down**: Navigate lists
- **Cross (X)**: Select/Confirm
- **Circle (O)**: Back/Cancel
- **Triangle (△)**: Speak answer (when available)
- **Square (□)**: Additional options
- **Start + Select**: Exit app

### Asking Questions

**Examples:**
- "How to treat a burn"
- "What did Neil Armstrong say about the moon"
- "How to purify water"
- "What is hypothermia"
- "How to build a shelter"

### Answer Display
Each answer shows:
- Answer type (Direct, Steps, Quotes, Summary)
- Main content
- Sources with dates
- Confidence score

Press **Triangle** to have the answer read aloud.

## Development

### Project Structure
```
vita-survival-ai/
├── CMakeLists.txt
├── include/               # Headers
│   ├── survival_ai.h
│   ├── database.h
│   ├── zim_reader.h
│   ├── search_engine.h
│   ├── voice_system.h
│   └── ui.h
├── src/                   # Source files
│   ├── main.cpp
│   ├── database/
│   ├── zim/
│   ├── search/
│   ├── voice/
│   └── ui/
├── libs/                  # Third-party libraries
│   └── sqlite3/
└── sce_sys/              # VPK metadata
```

### Adding New Features

**Adding a Scenario:**
1. Create scenario content file in `data/scenarios/`
2. Add to scenarios list in UI
3. Implement scenario display logic

**Adding Intent Patterns:**
1. Edit `search_engine.cpp` → `AnalyzeQuery()`
2. Add pattern matching in `Matches*Pattern()` functions
3. Implement answer builder for new intent

**Extending Database:**
1. Update schema in `database.cpp` → `CreateTables()`
2. Add new columns/tables
3. Update insert/search functions

### Current TODOs

**High Priority:**
- [ ] Integrate libzim for proper Wikipedia reading
- [ ] Implement actual keyboard input (use SCE IME)
- [ ] Add voice pack loading and playback
- [ ] Network status detection

**Medium Priority:**
- [ ] Implement PC Collector Python script
- [ ] Add manual/guide viewer
- [ ] Scenario content system
- [ ] Toolkit utilities
- [ ] Sync over network

**Nice to Have:**
- [ ] Lightweight summarization model
- [ ] Image support in vault items
- [ ] Export notes/answers
- [ ] Bookmark system

## Contributing

Contributions welcome! Focus areas:
1. ZIM reader integration (libzim)
2. Voice pack creation tools
3. PC Collector improvements
4. Survival content curation
5. UI/UX enhancements

## Credits

- Built with VitaSDK
- Uses SQLite3 for database
- Wikipedia content via Kiwix ZIM files
- Inspired by offline-first knowledge systems

## License

[Add your license here]

## Support

For issues, questions, or feature requests, please open an issue on GitHub.

---

**Note**: This is an early version. Some features are stub implementations and require additional work. See TODOs above.
