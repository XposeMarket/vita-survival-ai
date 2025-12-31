#include "voice_system.h"
#include <psp2/kernel/threadmgr.h>
#include <algorithm>

// NOTE: This is a stub implementation
// Full voice requires integration with:
// - Audio decoding (libvorbis for .ogg)
// - TTS engine (eSpeak NG or Flite)

VoiceSystem::VoiceSystem() : voicePackLoaded(false), playing(false), paused(false),
                              playbackSpeed(1.0f), volume(1.0f), audioPort(-1),
                              currentQueueIndex(0), useFallbackTTS(false),
                              ttsEngine(nullptr), audioThreadId(-1), audioThreadRunning(false) {
}

VoiceSystem::~VoiceSystem() {
    Shutdown();
}

bool VoiceSystem::Initialize(const std::string& voicePackPath) {
    this->voicePackPath = voicePackPath;
    
    // Initialize audio port
    audioPort = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, 1024, 48000, SCE_AUDIO_OUT_MODE_STEREO);
    if (audioPort < 0) {
        return false;
    }
    
    // Try to load voice pack
    LoadVoicePack();
    
    return true;
}

void VoiceSystem::Shutdown() {
    Stop();
    
    if (audioThreadRunning) {
        audioThreadRunning = false;
        if (audioThreadId >= 0) {
            sceKernelWaitThreadEnd(audioThreadId, nullptr, nullptr);
        }
    }
    
    UnloadVoicePack();
    
    if (audioPort >= 0) {
        sceAudioOutReleasePort(audioPort);
        audioPort = -1;
    }
}

bool VoiceSystem::LoadVoicePack() {
    // TODO: Load voice pack manifest and clips
    // Format: JSON manifest with clip metadata + .ogg files
    
    voicePackLoaded = false;
    return false;
}

void VoiceSystem::UnloadVoicePack() {
    for (auto& pair : voiceClips) {
        if (pair.second.audioData) {
            free(pair.second.audioData);
            pair.second.audioData = nullptr;
        }
    }
    voiceClips.clear();
    voicePackLoaded = false;
}

bool VoiceSystem::Speak(const std::string& text) {
    if (text.empty()) return false;
    
    Stop(); // Stop current playback
    
    // Preprocess text
    std::string processedText = PreprocessForSpeech(text);
    
    // Split into speech units
    auto units = SplitIntoSpeechUnits(processedText);
    
    // Build playback queue
    playbackQueue.clear();
    for (const auto& unit : units) {
        playbackQueue.push_back(unit);
    }
    
    currentQueueIndex = 0;
    playing = true;
    paused = false;
    
    // TODO: Start audio playback thread
    
    return true;
}

bool VoiceSystem::SpeakAnswer(const struct Answer& answer, VoiceMode mode) {
    std::string textToSpeak;
    
    switch (mode) {
        case VOICE_SUMMARY:
            textToSpeak = answer.summary;
            break;
            
        case VOICE_STEPS:
            textToSpeak = answer.summary + ". ";
            for (size_t i = 0; i < answer.steps.size(); i++) {
                textToSpeak += "Step " + std::to_string(i+1) + ". " + answer.steps[i] + ". ";
            }
            break;
            
        case VOICE_FULL:
            textToSpeak = answer.summary + ". ";
            if (!answer.steps.empty()) {
                for (size_t i = 0; i < answer.steps.size(); i++) {
                    textToSpeak += "Step " + std::to_string(i+1) + ". " + answer.steps[i] + ". ";
                }
            }
            if (!answer.quotes.empty()) {
                textToSpeak += "Quotes. ";
                for (const auto& quote : answer.quotes) {
                    textToSpeak += quote + ". ";
                }
            }
            break;
            
        case VOICE_SOURCES:
            textToSpeak = "Sources. ";
            for (size_t i = 0; i < answer.sources.size(); i++) {
                textToSpeak += "Source " + std::to_string(i+1) + ". " + 
                              answer.sources[i].title + " from " + 
                              answer.sources[i].domain + ". ";
            }
            break;
    }
    
    return Speak(textToSpeak);
}

void VoiceSystem::Stop() {
    playing = false;
    paused = false;
    playbackQueue.clear();
    currentQueueIndex = 0;
}

void VoiceSystem::Pause() {
    if (playing) {
        paused = true;
    }
}

void VoiceSystem::Resume() {
    if (playing && paused) {
        paused = false;
    }
}

void VoiceSystem::SetSpeed(float speed) {
    playbackSpeed = std::max(0.5f, std::min(2.0f, speed));
}

void VoiceSystem::SetVolume(float vol) {
    volume = std::max(0.0f, std::min(1.0f, vol));
}

int VoiceSystem::GetProgress() const {
    if (playbackQueue.empty()) return 0;
    return (currentQueueIndex * 100) / playbackQueue.size();
}

int VoiceSystem::GetClipCount() const {
    return voiceClips.size();
}

std::vector<std::string> VoiceSystem::GetAvailablePhrases() {
    std::vector<std::string> phrases;
    for (const auto& pair : voiceClips) {
        phrases.push_back(pair.first);
    }
    return phrases;
}

bool VoiceSystem::LoadClip(const std::string& filename, VoiceClip& clip) {
    // TODO: Load and decode .ogg file
    // Use libvorbis or similar
    clip.loaded = false;
    return false;
}

bool VoiceSystem::LoadOGG(const std::string& filename, void** data, size_t* size) {
    // TODO: Decode OGG Vorbis file to PCM
    return false;
}

bool VoiceSystem::PlayClip(const VoiceClip& clip) {
    if (!clip.loaded || !clip.audioData) return false;
    
    // TODO: Queue audio data to audioPort
    
    return true;
}

bool VoiceSystem::SynthesizeTTS(const std::string& text) {
    // TODO: Use eSpeak NG or Flite to synthesize speech
    // This is the fallback when voice pack doesn't have the phrase
    return false;
}

std::string VoiceSystem::PreprocessForSpeech(const std::string& text) {
    std::string result = text;
    
    // Replace common abbreviations
    // TODO: Add more replacements
    
    return result;
}

std::vector<std::string> VoiceSystem::SplitIntoSpeechUnits(const std::string& text) {
    std::vector<std::string> units;
    
    // Split on sentence boundaries
    std::string current;
    for (char c : text) {
        current += c;
        if (c == '.' || c == '!' || c == '?') {
            if (!current.empty()) {
                units.push_back(current);
                current.clear();
            }
        }
    }
    
    if (!current.empty()) {
        units.push_back(current);
    }
    
    return units;
}

bool VoiceSystem::FindBestMatch(const std::string& text, VoiceClip& clip) {
    // TODO: Find best matching clip from voice pack
    // Use fuzzy matching or phonetic matching
    
    return false;
}

int VoiceSystem::AudioThread(unsigned int args, void* argp) {
    VoiceSystem* voice = static_cast<VoiceSystem*>(argp);
    
    while (voice->audioThreadRunning) {
        if (voice->playing && !voice->paused && 
            voice->currentQueueIndex < voice->playbackQueue.size()) {
            
            // TODO: Process and play next audio chunk
            
            // For now, just sleep
            sceKernelDelayThread(16666); // ~60 FPS
        } else {
            sceKernelDelayThread(100000); // 100ms
        }
    }
    
    return 0;
}
