#ifndef VOICE_SYSTEM_H
#define VOICE_SYSTEM_H

#include <string>
#include <vector>
#include <map>
#include <psp2/audioout.h>

// Voice clip structure
struct VoiceClip {
    std::string text;           // The text this clip represents
    std::string filename;       // Path to .ogg file
    int duration_ms;            // Duration in milliseconds
    bool loaded;
    void* audioData;            // Decoded audio data
    size_t audioSize;
};

// Voice playback modes
enum VoiceMode {
    VOICE_SUMMARY,      // Read only summary
    VOICE_STEPS,        // Read step-by-step
    VOICE_FULL,         // Read everything
    VOICE_SOURCES       // Read sources
};

class VoiceSystem {
public:
    VoiceSystem();
    ~VoiceSystem();
    
    bool Initialize(const std::string& voicePackPath);
    void Shutdown();
    
    // Voice pack management
    bool LoadVoicePack();
    void UnloadVoicePack();
    bool HasVoicePack() const { return voicePackLoaded; }
    
    // Playback
    bool Speak(const std::string& text);
    bool SpeakAnswer(const struct Answer& answer, VoiceMode mode);
    void Stop();
    void Pause();
    void Resume();
    
    // Settings
    void SetSpeed(float speed); // 0.5 - 2.0
    void SetVolume(float volume); // 0.0 - 1.0
    float GetSpeed() const { return playbackSpeed; }
    float GetVolume() const { return volume; }
    
    // Status
    bool IsPlaying() const { return playing; }
    bool IsPaused() const { return paused; }
    int GetProgress() const; // 0-100%
    
    // Voice pack info
    int GetClipCount() const;
    std::vector<std::string> GetAvailablePhrases();
    
private:
    std::map<std::string, VoiceClip> voiceClips;
    std::string voicePackPath;
    bool voicePackLoaded;
    
    // Playback state
    bool playing;
    bool paused;
    float playbackSpeed;
    float volume;
    
    // Audio output
    int audioPort;
    std::vector<std::string> playbackQueue;
    size_t currentQueueIndex;
    
    // Fallback TTS (for missing clips)
    bool useFallbackTTS;
    void* ttsEngine; // Opaque pointer to TTS engine (eSpeak or Flite)
    
    // Helpers
    bool LoadClip(const std::string& filename, VoiceClip& clip);
    bool LoadOGG(const std::string& filename, void** data, size_t* size);
    bool PlayClip(const VoiceClip& clip);
    bool SynthesizeTTS(const std::string& text); // Fallback
    
    // Text processing for speech
    std::string PreprocessForSpeech(const std::string& text);
    std::vector<std::string> SplitIntoSpeechUnits(const std::string& text);
    bool FindBestMatch(const std::string& text, VoiceClip& clip);
    
    // Audio thread
    static int AudioThread(unsigned int args, void* argp);
    int audioThreadId;
    bool audioThreadRunning;
};

#endif // VOICE_SYSTEM_H
