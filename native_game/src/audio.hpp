#pragma once
#include "game.hpp"

namespace vivis {

// All music and effects are composed in code. No codec, file or download is
// required by the shipped executable.
class AudioSystem {
public:
    bool ready{};
    AudioSystem() = default;
    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;
    ~AudioSystem() { close(); }
    void init(bool enabled=true);
    void update(Game& game);
    void close();
    // Also works without an audio device; useful for checking a floor's score.
    static bool exportPreview(const char* path,int floor,bool boss=false);
private:
    static constexpr int TrackCount=10, EffectCount=int(Cue::Churu)+1;
    std::array<Sound,EffectCount> effects{};
    std::array<Music,TrackCount> tracks{};
    std::array<std::vector<unsigned char>,TrackCount> musicData;
    std::array<float,TrackCount> gains{};
    std::array<bool,TrackCount> running{};
};
}
