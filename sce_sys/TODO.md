# TODO List - Vita Survival AI

## Critical (Must Complete for Phase 1)

### Build System
- [ ] Download SQLite3 amalgamation and place in `libs/sqlite3/`
  - Get from: https://www.sqlite.org/download.html
  - Files needed: sqlite3.h, sqlite3.c
- [ ] Create placeholder PNG files for VPK icons
  - icon0.png (128x128)
  - bg.png (840x500)
  - startup.png (280x158)
- [ ] Test CMake configuration with VitaSDK
- [ ] Fix any VitaSDK-specific compilation issues

### Core Functionality
- [ ] Integrate libzim for Wikipedia reading
  - Repository: https://github.com/openzim/libzim
  - May need to port/build for VitaSDK
  - Alternative: Build your own minimal ZIM reader
- [ ] Implement proper keyboard input using SCE IME/OSK
  - Use `sceImeDialog` for text entry
  - Handle virtual keyboard events
- [ ] Test database operations on actual hardware
- [ ] Verify file I/O works correctly on Vita

## Phase 1 - Offline Wikipedia Reader

- [ ] Complete ZIM reader implementation
- [ ] Basic article browsing
- [ ] Article search functionality
- [ ] Bookmarks/favorites
- [ ] Text rendering with proper formatting
- [ ] Image support (if needed)

## Phase 2 - Vault + Offline Search

- [ ] SQLite FTS5 integration
- [ ] Import sample dataset for testing
- [ ] Search interface
- [ ] View saved references
- [ ] Source attribution display
- [ ] Date formatting

## Phase 3 - Ask Mode

- [ ] Query processing
- [ ] Intent detection refinement
- [ ] Answer generation
- [ ] Source citation
- [ ] Multiple answer types (steps, quotes, etc.)
- [ ] Confidence scoring
- [ ] Save/pin answers

## Phase 4 - PC Collector + Sync

### PC Collector
- [ ] Enhance RSS feed support
- [ ] Add more extraction methods
- [ ] Better error handling
- [ ] Rate limiting
- [ ] Duplicate detection
- [ ] Content validation

### Sync
- [ ] USB transfer guide
- [ ] Network sync (optional)
  - FTP transfer
  - HTTP download
  - Local network discovery
- [ ] Vault pack format
- [ ] Incremental updates
- [ ] Conflict resolution

## Phase 5 - Voice System

### Voice Pack
- [ ] OGG Vorbis decoding
  - Port libvorbis to VitaSDK or use tremor (integer decoder)
- [ ] Audio playback via SceAudioOut
- [ ] Voice pack manifest parser
- [ ] Clip matching algorithm
- [ ] Playback queue management
- [ ] Speed/volume control

### Fallback TTS
- [ ] Port eSpeak NG or Flite to VitaSDK
- [ ] Text preprocessing
- [ ] Synthesis pipeline
- [ ] Audio buffer management
- [ ] Thread-safe audio playback

## Phase 6 - Polish & Features

### UI Improvements
- [ ] Better text wrapping
- [ ] Scroll indicators
- [ ] Loading animations
- [ ] Notification system
- [ ] Settings menu
- [ ] Theme support

### Additional Features
- [ ] Manual/guide viewer
- [ ] Scenario quick-access
- [ ] Toolkit utilities
  - SOS signal patterns
  - Morse code helper
  - Flashlight/strobe
  - Unit converter
- [ ] Offline help system
- [ ] App tutorial

## Testing & Optimization

### Performance
- [ ] Profile memory usage
- [ ] Optimize database queries
- [ ] Cache frequently accessed data
- [ ] Lazy loading for large content
- [ ] Background loading

### Stability
- [ ] Handle low memory conditions
- [ ] Graceful degradation
- [ ] Error recovery
- [ ] Crash reporting
- [ ] Memory leak testing

### Compatibility
- [ ] Test on Vita 1000
- [ ] Test on Vita 2000 (Slim)
- [ ] Test on PSTV
- [ ] Test with different SD card sizes
- [ ] Test with various ZIM files

## Documentation

- [ ] Complete API documentation
- [ ] Code comments
- [ ] User manual
- [ ] Developer guide
- [ ] Contributing guidelines
- [ ] License file

## Content

### Survival Manuals
- [ ] Curate public domain survival guides
- [ ] Convert to text format
- [ ] Index and tag
- [ ] Verify accuracy
- [ ] Add citations

### Scenarios
- [ ] Create scenario templates
- [ ] First aid scenarios
- [ ] Wilderness survival scenarios
- [ ] Emergency preparedness
- [ ] Urban survival

### Voice Pack
- [ ] Generate essential phrases
- [ ] Record common terms
- [ ] Create manifest
- [ ] Test playback
- [ ] Optimize file sizes

## Known Issues to Fix

- [ ] Keyboard input is placeholder (needs SCE IME)
- [ ] ZIM reader is stub (needs libzim)
- [ ] Voice system is incomplete (needs audio codecs)
- [ ] No actual TTS (needs eSpeak/Flite port)
- [ ] Network status detection not implemented
- [ ] Image placeholders needed for VPK
- [ ] No actual online mode (future feature)

## Future Enhancements (Post-Launch)

- [ ] Lightweight summarization model (if feasible)
- [ ] Multi-language support
- [ ] Cloud sync
- [ ] Community content packs
- [ ] AR mode using camera (experimental)
- [ ] GPS integration for location-aware info
- [ ] Sensor integration (compass, etc.)

## Community

- [ ] Set up GitHub repository
- [ ] Create issue templates
- [ ] Write contribution guide
- [ ] Build website/landing page
- [ ] Create demo video
- [ ] Social media presence

---

## Priority Order

1. **Critical** - Complete before first release
2. **Phase 1-3** - Core functionality
3. **Phase 4-5** - Enhanced features
4. **Phase 6** - Polish
5. **Future** - Nice-to-have features

## Time Estimates

- Phase 1: 2-3 weeks
- Phase 2: 1-2 weeks
- Phase 3: 2-3 weeks
- Phase 4: 1-2 weeks
- Phase 5: 3-4 weeks
- Phase 6: 1-2 weeks

**Total**: ~10-16 weeks for complete implementation

## Notes

- Start with Phase 1 (Wikipedia reader) as foundation
- Each phase builds on previous
- Test thoroughly on hardware, not just emulator
- Prioritize stability over features
- Document as you go
