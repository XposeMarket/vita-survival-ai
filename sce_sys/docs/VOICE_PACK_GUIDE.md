# Voice Pack Guide for Vita Survival AI

## Overview
The voice pack provides high-quality pre-recorded audio clips for common phrases, reducing reliance on robotic TTS and providing a more natural-sounding experience.

## Voice Pack Structure

```
ux0:data/survivalkit/voice/pack/
├── manifest.json          # Clip index and metadata
├── numbers/               # Number pronunciation
│   ├── 0.ogg
│   ├── 1.ogg
│   └── ...
├── units/                 # Measurement units
│   ├── meters.ogg
│   ├── feet.ogg
│   └── ...
├── steps/                 # Step indicators
│   ├── step_1.ogg
│   └── ...
├── warnings/              # Warning/safety phrases
│   ├── warning.ogg
│   ├── danger.ogg
│   └── caution.ogg
└── survival/              # Survival-specific terms
    ├── water.ogg
    ├── shelter.ogg
    └── ...
```

## Creating a Voice Pack

### Step 1: Generate Audio Files

Use a high-quality TTS service:

**Option A: Google Cloud TTS**
```python
from google.cloud import texttospeech

client = texttospeech.TextToSpeechClient()

def generate_clip(text, output_file):
    synthesis_input = texttospeech.SynthesisInput(text=text)
    voice = texttospeech.VoiceSelectionParams(
        language_code="en-US",
        name="en-US-Neural2-J"  # Or your preferred voice
    )
    audio_config = texttospeech.AudioConfig(
        audio_encoding=texttospeech.AudioEncoding.OGG_OPUS,
        sample_rate_hertz=48000
    )
    
    response = client.synthesize_speech(
        input=synthesis_input,
        voice=voice,
        audio_config=audio_config
    )
    
    with open(output_file, 'wb') as out:
        out.write(response.audio_content)
```

**Option B: AWS Polly**
```python
import boto3

polly = boto3.client('polly')

def generate_clip(text, output_file):
    response = polly.synthesize_speech(
        Text=text,
        OutputFormat='ogg_vorbis',
        VoiceId='Matthew',  # Or your preferred voice
        SampleRate='48000'
    )
    
    with open(output_file, 'wb') as out:
        out.write(response['AudioStream'].read())
```

**Option C: espeak (Free, offline)**
```bash
espeak -v en-us -s 150 -w temp.wav "step one"
ffmpeg -i temp.wav -c:a libvorbis -q:a 6 -ar 48000 step_1.ogg
```

### Step 2: Required Clips

**Essential Clips (Minimum 100 clips):**

Numbers (0-100):
- "zero" through "one hundred"

Step Indicators:
- "step one", "step two", ... "step ten"

Common Phrases:
- "warning", "danger", "caution", "important", "note"
- "first", "second", "third", "next", "finally"
- "always", "never", "immediately", "carefully"

Survival Terms:
- "water", "shelter", "fire", "food", "first aid"
- "bleeding", "burn", "fracture", "wound"
- "north", "south", "east", "west"
- "signal", "rescue", "emergency"

Units:
- "meters", "feet", "kilometers", "miles"
- "celsius", "fahrenheit"
- "liters", "gallons"

Time:
- "hour", "hours", "minute", "minutes"
- "day", "days", "week"

### Step 3: Audio Specifications

**Format**: Ogg Vorbis
**Sample Rate**: 48000 Hz
**Channels**: Stereo (2)
**Quality**: Medium (q:a 6 or bitrate 128kbps)
**Duration**: Keep clips under 3 seconds when possible

### Step 4: Create Manifest

Create `manifest.json`:

```json
{
  "version": "1.0",
  "voice": "en-US-Neural2-J",
  "created": "2025-12-29T00:00:00Z",
  "clips": [
    {
      "id": "num_0",
      "text": "zero",
      "file": "numbers/0.ogg",
      "duration_ms": 450,
      "category": "number"
    },
    {
      "id": "step_1",
      "text": "step one",
      "file": "steps/step_1.ogg",
      "duration_ms": 680,
      "category": "step"
    },
    {
      "id": "warning",
      "text": "warning",
      "file": "warnings/warning.ogg",
      "duration_ms": 550,
      "category": "safety"
    }
  ]
}
```

### Step 5: Voice Pack Generator Script

```python
#!/usr/bin/env python3
import json
import os
import subprocess
from pathlib import Path

# Phrases to generate
PHRASES = {
    "numbers": [str(i) for i in range(101)],
    "steps": [f"step {i}" for i in ["one", "two", "three", "four", "five", 
                                     "six", "seven", "eight", "nine", "ten"]],
    "warnings": ["warning", "danger", "caution", "important", "note"],
    "survival": ["water", "shelter", "fire", "food", "first aid", "bleeding", 
                 "burn", "fracture", "wound"],
    "units": ["meters", "feet", "kilometers", "miles", "celsius", "fahrenheit"]
}

def generate_voice_pack(output_dir, method="espeak"):
    output_path = Path(output_dir)
    clips = []
    
    for category, phrases in PHRASES.items():
        category_dir = output_path / category
        category_dir.mkdir(parents=True, exist_ok=True)
        
        for i, phrase in enumerate(phrases):
            filename = f"{i}.ogg"
            filepath = category_dir / filename
            
            if method == "espeak":
                # Generate with espeak
                temp_wav = "/tmp/temp.wav"
                subprocess.run([
                    "espeak", "-v", "en-us", "-s", "150", 
                    "-w", temp_wav, phrase
                ])
                subprocess.run([
                    "ffmpeg", "-y", "-i", temp_wav, 
                    "-c:a", "libvorbis", "-q:a", "6", 
                    "-ar", "48000", str(filepath)
                ])
                os.remove(temp_wav)
            
            # Get duration
            result = subprocess.run([
                "ffprobe", "-v", "error", "-show_entries", 
                "format=duration", "-of", 
                "default=noprint_wrappers=1:nokey=1", str(filepath)
            ], capture_output=True, text=True)
            duration = int(float(result.stdout.strip()) * 1000)
            
            clips.append({
                "id": f"{category}_{i}",
                "text": phrase,
                "file": f"{category}/{filename}",
                "duration_ms": duration,
                "category": category
            })
            
            print(f"Generated: {category}/{filename} - {phrase}")
    
    # Create manifest
    manifest = {
        "version": "1.0",
        "voice": method,
        "created": datetime.now().isoformat(),
        "clips": clips
    }
    
    with open(output_path / "manifest.json", 'w') as f:
        json.dump(manifest, f, indent=2)
    
    print(f"\nVoice pack created: {output_dir}")
    print(f"Total clips: {len(clips)}")

if __name__ == "__main__":
    generate_voice_pack("voice_pack", method="espeak")
```

## Voice Pack Size Estimates

- **Minimal Pack** (100 clips): ~2-5 MB
- **Standard Pack** (500 clips): ~10-20 MB
- **Full Pack** (1000+ clips): ~30-50 MB

## Installing Voice Pack

1. Copy the entire `pack/` directory to your Vita:
   ```
   ux0:data/survivalkit/voice/pack/
   ```

2. Ensure manifest.json is in the pack/ directory

3. The app will load the voice pack on startup

4. Missing phrases will fall back to TTS (if implemented)

## Testing Voice Pack

Test individual clips:
```bash
# Play a clip
ffplay numbers/5.ogg

# Check all clips load correctly
python test_voice_pack.py manifest.json
```

## Voice Pack Performance

- Clips are loaded on-demand to save RAM
- Recent clips are cached (LRU cache)
- Playback is instant (no synthesis delay)
- Quality is significantly better than real-time TTS

## Fallback Strategy

If voice pack is missing or incomplete:
1. App checks for voice pack clip
2. If not found, uses fallback TTS (eSpeak/Flite)
3. User can configure voice pack priority in settings

## Credits

Remember to comply with TTS service terms of use and attribute voice sources appropriately.
