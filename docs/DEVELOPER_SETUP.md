# Developer Setup Guide

## Complete Setup Instructions for Building Vita Survival AI

### Prerequisites

1. **Operating System**: Linux (Ubuntu/Debian recommended) or WSL2 on Windows
2. **Disk Space**: ~5GB for VitaSDK + dependencies
3. **RAM**: 4GB+ recommended
4. **Time**: ~30-60 minutes for full setup

### Step 1: Install VitaSDK

#### On Ubuntu/Debian:
```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y git cmake make pkg-config \
  gcc g++ libssl-dev libzip-dev libtool autoconf

# Download and run VitaSDK installer
git clone https://github.com/vitasdk/vdpm
cd vdpm
./bootstrap-vitasdk.sh

# Add to PATH (add this to ~/.bashrc for persistence)
export VITASDK=/usr/local/vitasdk
export PATH=$PATH:$VITASDK/bin
```

#### Verify Installation:
```bash
echo $VITASDK  # Should show /usr/local/vitasdk
arm-vita-eabi-gcc --version  # Should show GCC version
```

### Step 2: Install Required VitaSDK Libraries

```bash
# Install vita2d (graphics library)
vdpm vita2d

# Install other dependencies
vdpm libpng
vdpm zlib
vdpm freetype
vdpm jpeg
```

### Step 3: Get SQLite3 Amalgamation

```bash
cd vita-survival-ai/libs/sqlite3/

# Download SQLite amalgamation
wget https://www.sqlite.org/2024/sqlite-amalgamation-3450000.zip
unzip sqlite-amalgamation-3450000.zip
mv sqlite-amalgamation-3450000/sqlite3.* ./
rm -rf sqlite-amalgamation-3450000*

# Verify files exist
ls -lh sqlite3.h sqlite3.c
```

### Step 4: Create VPK Icons

```bash
cd vita-survival-ai/

# Install ImageMagick (if not installed)
sudo apt-get install imagemagick

# Run icon creation script
./create_icons.sh

# Or manually create in an image editor and place:
# - sce_sys/icon0.png (128x128)
# - sce_sys/livearea/contents/bg.png (840x500)
# - sce_sys/livearea/contents/startup.png (280x158)
```

See `docs/IMAGE_REQUIREMENTS.md` for details.

### Step 5: Build the Project

```bash
# Run build script
./build.sh

# Or manually:
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

If successful, you should see: `SurvivalAI.vpk`

### Step 6: Transfer to Vita

#### Method 1: USB with VitaShell
1. Connect Vita via USB
2. Open VitaShell, press SELECT for USB mode
3. Copy `build/SurvivalAI.vpk` to Vita
4. Exit USB mode
5. Navigate to VPK and press X to install

#### Method 2: FTP
```bash
# On Vita: Open VitaShell, press SELECT, note IP address

# On PC:
ftp 192.168.1.XXX  # Use your Vita's IP
# login: anonymous (no password)
cd ux0:/data/
put build/SurvivalAI.vpk
bye
```

### Troubleshooting Build Issues

#### CMake can't find VitaSDK
```bash
# Ensure VITASDK is set
export VITASDK=/usr/local/vitasdk
export PATH=$PATH:$VITASDK/bin

# Verify toolchain file exists
ls $VITASDK/share/vita.toolchain.cmake
```

#### Missing libraries
```bash
# Install all recommended libraries
vdpm vita2d freetype libpng zlib jpeg

# List installed packages
vdpm -l
```

#### SQLite errors
```bash
# Ensure you have the amalgamation files
ls libs/sqlite3/sqlite3.h libs/sqlite3/sqlite3.c

# Re-download if needed
cd libs/sqlite3
wget https://www.sqlite.org/2024/sqlite-amalgamation-3450000.zip
# ... (see Step 3)
```

#### Build errors about missing headers
```bash
# Clean build and retry
rm -rf build
mkdir build && cd build
cmake ..
make clean
make -j$(nproc)
```

### Development Workflow

#### Making Changes
1. Edit source files in `src/` or `include/`
2. Rebuild: `cd build && make -j$(nproc)`
3. Transfer new VPK to Vita
4. Test on device

#### Testing on Device
- Use VitaShell to view logs: `ux0:data/tai/vitashell/logs/`
- Check app output in: `ux0:data/survivalkit/`
- Debug with printf statements (output to stdout)

#### Code Style
- Use C++11 features
- Keep functions < 100 lines when possible
- Comment complex logic
- Use meaningful variable names

### Setting Up Development Data

#### 1. Wikipedia ZIM (Optional for testing)
```bash
# Download small test ZIM
wget https://download.kiwix.org/zim/wikipedia_en_simple_all.zim

# Copy to Vita (adjust path)
cp wikipedia_en_simple_all.zim /path/to/vita/ux0/data/survivalkit/zim/wikipedia_en.zim
```

#### 2. Test Vault Data
```bash
# Create sample vault
cd tools
python3 pc_collector.py --output test_vault \
  --urls "https://en.wikipedia.org/wiki/First_aid" \
  --tags test

# Copy to Vita
cp test_vault/vault.sqlite /path/to/vita/ux0/data/survivalkit/db/
cp -r test_vault/items /path/to/vita/ux0/data/survivalkit/vault/
```

### Debugging Tips

#### Enable Verbose Logging
```cpp
// Add to main.cpp
#define DEBUG 1

#ifdef DEBUG
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif
```

#### Check for Memory Leaks
```cpp
// Add memory tracking
void* operator new(size_t size) {
    void* ptr = malloc(size);
    LOG("Alloc: %zu bytes at %p\n", size, ptr);
    return ptr;
}

void operator delete(void* ptr) noexcept {
    LOG("Free: %p\n", ptr);
    free(ptr);
}
```

#### Performance Profiling
```cpp
#include <psp2/kernel/threadmgr.h>

uint64_t start = sceKernelGetProcessTimeWide();
// ... code to profile ...
uint64_t end = sceKernelGetProcessTimeWide();
LOG("Execution time: %llu us\n", (end - start));
```

### Useful Resources

**VitaSDK Documentation**:
- https://docs.vitasdk.org/
- https://github.com/vitasdk

**PS Vita Homebrew**:
- https://vitadb.rinnegatamante.it/
- r/VitaHacks
- GBAtemp PS Vita forums

**Libraries**:
- vita2d: https://github.com/xerpi/vita2d
- SQLite: https://www.sqlite.org/docs.html
- Kiwix/ZIM: https://wiki.kiwix.org/

### Next Steps

1. ✅ Build successfully compiles
2. ✅ VPK installs on Vita
3. ✅ App launches without crashes
4. ⬜ Implement ZIM reader (see TODO.md)
5. ⬜ Implement keyboard input
6. ⬜ Add test data
7. ⬜ Test core features

### Getting Help

- Check TODO.md for known issues
- Review source code comments
- Search VitaSDK documentation
- Ask in homebrew communities

Good luck building! 🎮
