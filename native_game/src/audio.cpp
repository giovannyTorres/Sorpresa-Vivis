#include "audio.hpp"
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <vector>

namespace vivis {
namespace {
constexpr float Pi=3.14159265358979323846f,Tau=2*Pi;
constexpr int SampleRate=22050;
float sine(float phase) { return std::sin(Tau*phase); }
float hz(float midi) { return 440.0f*std::pow(2.0f,(midi-69)/12); }
float noise(std::uint32_t n) {
    n^=n>>16; n*=0x7feb352dU; n^=n>>15; n*=0x846ca68bU; n^=n>>16;
    return float(n&65535)/32768-1;
}
float envelope(float age,float duration,float attack,float release) {
    return std::clamp(age/attack,0.0f,1.0f)*std::clamp((duration-age)/release,0.0f,1.0f);
}
struct Score {
    float bpm; int root; bool minor; int style;
    std::array<int,8> chords;
    std::array<int,32> melody;
};
const std::array<Score,6> Scores{{
    // Warm mallets over a toy-piano bass: Pachonidad.
    {104,60,false,0,{0,9,5,7,0,4,5,7},
     {12,16,19,16,14,12,9,-1,12,14,16,19,21,19,16,-1,
      17,16,14,12,9,12,14,16,19,16,14,11,12,-1,7,-1}},
    // A syncopated plucked theme for searching among whiskers.
    {116,62,false,1,{0,5,9,7,0,9,5,7},
     {12,-1,16,19,14,16,-1,12,17,21,19,-1,17,16,14,-1,
      21,19,16,14,12,-1,14,16,19,14,11,7,12,-1,7,-1}},
    // A mischievous minor ostinato; tense without becoming harsh.
    {110,57,true,2,{0,8,5,7,0,3,8,7},
     {12,15,19,-1,20,19,15,12,15,-1,19,22,20,19,15,-1,
      17,20,24,20,19,15,12,-1,11,14,19,14,12,-1,7,-1}},
    // The lunar refuge is a slow celesta lullaby.
    {82,62,false,3,{0,9,5,7,0,4,5,0},
     {19,-1,16,14,12,-1,14,-1,21,-1,19,16,14,-1,12,-1,
      17,19,21,-1,19,16,14,-1,16,14,12,-1,7,-1,12,-1}},
    // Title theme: recognizable fragments from Pachonidad.
    {96,60,false,4,{0,9,5,7,0,4,5,0},
     {12,16,19,-1,21,19,16,-1,17,16,14,12,14,-1,7,-1,
      12,14,16,19,21,19,16,14,17,16,14,7,12,-1,-1,-1}},
    // Churu and reconciliation resolve the minor adventure into major.
    {88,60,false,5,{0,5,9,7,0,9,5,0},
     {12,16,19,-1,24,23,21,-1,17,19,21,19,16,-1,14,-1,
      12,16,19,21,24,-1,19,-1,17,16,14,7,12,-1,-1,-1}}
}};

std::vector<std::int16_t> compose(int track) {
    const bool boss=track<8&&(track&1);
    const Score& score=Scores[track<8?track/2:track-4];
    float bpm=score.bpm+(boss?22:0), beat=60/bpm, bar=beat*4;
    int frames=int(std::round(bar*8*SampleRate));
    // Stereo with a short circular room response. Its wrapped delay also
    // makes the reverb tail continuous at the loop boundary.
    std::vector<float> left(frames),right(frames);
    const float bassGain=boss?0.115f:0.09f;
    for(int n=0;n<frames;++n) {
        float time=float(n)/SampleRate, beats=time/beat;
        int measure=std::min(7,int(beats/4));
        float localBar=std::fmod(time,bar);
        int chord=score.root+score.chords[measure];
        bool minorChord=score.minor?(score.chords[measure]==0||score.chords[measure]==5):
            (score.chords[measure]==9||score.chords[measure]==4);
        float padEnvelope=envelope(localBar,bar,0.16f,0.26f);
        float pad=0;
        for(int interval:{0,minorChord?3:4,7,14}) {
            float frequency=hz(float(chord+interval));
            pad+=(sine(frequency*time)+0.30f*sine(frequency*1.002f*time))*0.013f*padEnvelope;
        }
        // Root/fifth bass, with small octave lifts in the second phrase.
        int quarter=int(beats)%4;
        float bassAge=std::fmod(time,beat), bassFrequency=hz(float(chord-24+(quarter==2?7:0)));
        float bass=(sine(bassFrequency*bassAge)+0.22f*sine(2*bassFrequency*bassAge))*
            envelope(bassAge,beat,0.012f,0.10f)*std::exp(-bassAge*2.8f)*bassGain;
        // Melody uses quarter notes with occasional eighth-note pickups.
        int melodyIndex=(measure/4*16+int(beats)%16)%32;
        int note=score.melody[melodyIndex];
        float lead=0;
        if(note>=0) {
            float age=bassAge, frequency=hz(float(score.root+note));
            float decay=(score.style==3?1.8f:3.5f)/beat;
            float body=sine(frequency*age)+0.23f*sine(frequency*2*age)+
                (score.style==2?0.13f:0.05f)*sine(frequency*3*age);
            lead=body*std::exp(-age*decay)*envelope(age,beat,0.006f,0.055f)*0.105f;
        }
        // Quiet eighth-note arpeggios give rooms movement without obscuring
        // the attack cues. Boss arrangements add an upper answering voice.
        int eighth=int(beats*2), arpeggioSteps[]={0,7,minorChord?3:4,12,7,14,12,7};
        float arpAge=std::fmod(time,beat/2),arpFreq=hz(float(chord+12+arpeggioSteps[eighth%8]));
        float arp=sine(arpFreq*arpAge)*std::exp(-arpAge*13)*
            envelope(arpAge,beat/2,0.004f,0.025f)*(boss?0.045f:0.021f);
        if(score.style==3) arp*=0.55f;
        // A soft kick, brush/clap and shaker; every floor has its own groove.
        float kickAge=bassAge;
        float kick=(quarter%2==0||boss)?
            sine(52*kickAge+3.8f*(1-std::exp(-kickAge*22)))*std::exp(-kickAge*24)*0.092f:0;
        float brush=(quarter==1||quarter==3)?
            noise(std::uint32_t(n)+31415)*std::exp(-kickAge*34)*0.038f:0;
        float hat=noise(std::uint32_t(n)+91237)*std::exp(-arpAge*100)*(eighth%2?0.013f:0.009f);
        if(score.style==3||score.style==5) { kick*=0.45f; brush*=0.35f; hat*=0.4f; }
        float pan=0.18f*sine(float(measure)/8);
        left[n]=bass+kick+brush+hat+pad+lead*(0.9f-pan)+arp*0.55f;
        right[n]=bass+kick+brush+hat+pad*0.94f+lead*(0.9f+pan)+arp;
    }
    std::vector<std::int16_t> pcm(std::size_t(frames)*2);
    const int early=int(SampleRate*0.137f),late=int(SampleRate*0.293f);
    for(int n=0;n<frames;++n) {
        int a=(n+frames-early)%frames,b=(n+frames-late)%frames;
        float l=left[n]+right[a]*0.16f+left[b]*0.09f;
        float r=right[n]+left[a]*0.16f+right[b]*0.09f;
        // Plenty of headroom for simultaneous attacks and pickup sounds.
        pcm[std::size_t(n)*2]=std::int16_t(std::tanh(l*1.25f)*20000);
        pcm[std::size_t(n)*2+1]=std::int16_t(std::tanh(r*1.25f)*20000);
    }
    return pcm;
}

std::vector<unsigned char> wav(const std::vector<std::int16_t>& pcm,int channels=2) {
    std::vector<unsigned char> data; data.reserve(44+pcm.size()*2);
    auto chars=[&](const char* s,int n) { data.insert(data.end(),s,s+n); };
    auto word=[&](std::uint32_t value,int bytes) { for(int i=0;i<bytes;++i) data.push_back((value>>(8*i))&255); };
    chars("RIFF",4); word(std::uint32_t(pcm.size()*2+36),4); chars("WAVEfmt ",8);
    word(16,4); word(1,2); word(channels,2); word(SampleRate,4); word(SampleRate*channels*2,4);
    word(channels*2,2); word(16,2); chars("data",4); word(std::uint32_t(pcm.size()*2),4);
    for(auto sample:pcm) word(std::uint16_t(sample),2);
    return data;
}

std::vector<std::int16_t> effect(Cue cue) {
    const float duration[]={0.085f,0.17f,0.36f,0.42f,0.36f,0.13f,0.9f,0.5f,0.20f,0.52f,0.23f,0.85f,1.0f};
    float seconds=duration[int(cue)]; int frames=int(seconds*SampleRate);
    std::vector<std::int16_t> pcm(frames);
    float phase=0;
    for(int n=0;n<frames;++n) {
        float t=float(n)/SampleRate,p=t/seconds,frequency=600,signal=0;
        switch(cue) {
        case Cue::Shoot: frequency=660-300*p; break;
        case Cue::Hit: frequency=130-60*p; break;
        case Cue::Heal: frequency=hz(float(72+std::min(2,int(p*3))*4)); break;
        case Cue::Bell: frequency=1046.5f; break;
        case Cue::Help: frequency=330+500*p; break;
        case Cue::Dash: frequency=240+260*p; break;
        case Cue::Win: { constexpr int notes[]={72,76,79,84,79}; frequency=hz(float(notes[std::min(4,int(p*5))])); break; }
        case Cue::Transition: frequency=220+660*p*p; break;
        case Cue::Coin: frequency=p<0.45f?1046.5f:1568; break;
        case Cue::Item: { constexpr int notes[]={72,76,79,84}; frequency=hz(float(notes[std::min(3,int(p*4))])); break; }
        case Cue::Poison: frequency=270-140*p+30*sine(p*4); break;
        case Cue::Revive: frequency=hz(float(60+std::min(7,int(p*8))*3)); break;
        case Cue::Churu: { constexpr int notes[]={67,72,76,79,84}; frequency=hz(float(notes[std::min(4,int(p*5))])); break; }
        }
        phase+=frequency/SampleRate;
        signal=sine(phase)+0.18f*sine(phase*2);
        if(cue==Cue::Hit) signal=signal*0.55f+noise(std::uint32_t(n))*0.45f;
        if(cue==Cue::Poison) signal=signal*0.7f+noise(std::uint32_t(n))*0.15f*sine(p*10);
        if(cue==Cue::Dash) signal=noise(std::uint32_t(n))*0.30f+signal*0.25f;
        if(cue==Cue::Churu) signal+=sine(29*t)*0.22f*(0.6f+0.4f*sine(5*t));
        float attack=envelope(t,seconds,0.006f,seconds*0.35f);
        float gain=(cue==Cue::Shoot?0.095f:0.16f)*attack*(1-0.25f*p);
        pcm[n]=std::int16_t(std::clamp(signal*gain,-0.7f,0.7f)*32767);
    }
    return pcm;
}
}

void AudioSystem::init(bool enabled) {
    if(ready||!enabled) return;
    InitAudioDevice(); ready=IsAudioDeviceReady();
    if(!ready) return;
    for(int i=0;i<EffectCount;++i) {
        auto pcm=effect(Cue(i));
        Wave wave{unsigned(pcm.size()),SampleRate,16,1,pcm.data()};
        effects[i]=LoadSoundFromWave(wave);
        if(IsSoundValid(effects[i])) SetSoundVolume(effects[i],0.9f);
    }
    for(int i=0;i<TrackCount;++i) {
        musicData[i]=wav(compose(i));
        tracks[i]=LoadMusicStreamFromMemory(".wav",musicData[i].data(),int(musicData[i].size()));
        if(IsMusicValid(tracks[i])) { tracks[i].looping=true; SetMusicVolume(tracks[i],0); }
    }
}

void AudioSystem::update(Game& game) {
    if(!ready) { game.cues.clear(); return; }
    SetMasterVolume(game.preferences.muted?0:std::clamp(game.preferences.volume,0.0f,1.0f));
    int desired=std::clamp(game.stage,0,3)*2;
    if(std::any_of(game.enemies.begin(),game.enemies.end(),[](const Enemy& enemy) { return enemy.boss&&enemy.hp>0; })) ++desired;
    if(game.screen==Screen::Lost) desired=std::clamp(game.stage,0,3)*2;
    if(game.screen==Screen::Title||game.screen==Screen::Gallery||game.screen==Screen::Controls) desired=8;
    if(game.malikethFriendly||game.screen==Screen::Won) desired=9;
    float dt=std::clamp(GetFrameTime(),0.0f,0.1f),change=dt/1.1f;
    const bool quiet=game.screen==Screen::Paused||game.screen==Screen::Settings||game.screen==Screen::Reward||
        game.screen==Screen::Lost||game.mapOpen||game.inventoryOpen;
    const float menuVolume=quiet?0.62f:1;
    for(int i=0;i<TrackCount;++i) {
        if(!IsMusicValid(tracks[i])) continue;
        float target=i==desired?1.0f:0.0f;
        gains[i]+=std::clamp(target-gains[i],-change,change);
        if(target>0&&!running[i]) { PlayMusicStream(tracks[i]); running[i]=true; }
        if(running[i]) {
            SetMusicVolume(tracks[i],gains[i]*menuVolume*0.77f);
            UpdateMusicStream(tracks[i]);
            if(gains[i]<=0&&target==0) { StopMusicStream(tracks[i]); running[i]=false; }
        }
    }
    for(Cue cue:game.cues) {
        int id=int(cue);
        if(id>=0&&id<EffectCount&&IsSoundValid(effects[id])) PlaySound(effects[id]);
    }
    game.cues.clear();
}

void AudioSystem::close() {
    if(!ready) return;
    for(int i=0;i<TrackCount;++i) {
        if(IsMusicValid(tracks[i])) UnloadMusicStream(tracks[i]);
        tracks[i]={}; musicData[i].clear(); running[i]=false; gains[i]=0;
    }
    for(auto& sound:effects) { if(IsSoundValid(sound)) UnloadSound(sound); sound={}; }
    CloseAudioDevice(); ready=false;
}

bool AudioSystem::exportPreview(const char* path,int floor,bool boss) {
    if(!path) return false;
    auto data=wav(compose(std::clamp(floor,0,3)*2+(boss?1:0)));
    std::ofstream file(path,std::ios::binary);
    return bool(file.write(reinterpret_cast<const char*>(data.data()),std::streamsize(data.size())));
}
}
