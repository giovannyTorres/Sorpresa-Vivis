#pragma once
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include "raylib.h"

namespace vivis {
constexpr int Width = 1280, Height = 720;
constexpr Rectangle Field{64, 160, 1152, 492};
constexpr int MaxHp = 6;
enum class Screen { Title, Playing, Paused, Settings, Controls, Gallery, Lost, Won };
enum class Cue { Shoot, Hit, Heal, Bell, Help, Dash, Win, Transition };
struct Input { float x{}, y{}; bool shoot{}, help{}, dash{}; };
struct Enemy { Vector2 pos{}; int hp{3}; bool boss{}; float timer{1.4f}, flash{}; int phase{}; };
struct Projectile { Vector2 pos{}, velocity{}; float life{2}; };
struct Particle { Vector2 pos{}, velocity{}; float life{}, maxLife{}; Color color{}; };
struct Preferences { float volume{0.55f}; bool muted{}, fullscreen{}, reducedMotion{}, easy{}; };

inline Vector2 add(Vector2 a, Vector2 b) { return {a.x+b.x,a.y+b.y}; }
inline Vector2 sub(Vector2 a, Vector2 b) { return {a.x-b.x,a.y-b.y}; }
inline Vector2 mul(Vector2 a, float s) { return {a.x*s,a.y*s}; }
inline float length(Vector2 a) { return std::sqrt(a.x*a.x+a.y*a.y); }
inline float distance(Vector2 a, Vector2 b) { return length(sub(a,b)); }
inline Vector2 unit(Vector2 a) { float n=length(a); return n>0.001f?mul(a,1/n):Vector2{}; }
inline bool inside(Vector2 p, Rectangle r) { return p.x>=r.x&&p.y>=r.y&&p.x<r.x+r.width&&p.y<r.y+r.height; }
inline Rectangle grow(Rectangle r,float n) { return {r.x-n,r.y-n,r.width+2*n,r.height+2*n}; }

struct Game {
    Screen screen{Screen::Title};
    Preferences preferences;
    int stage{}, checkpoint{}, hp{MaxHp}, defeatedCount{};
    Vector2 player{145,420}, companion{95,465}, facing{1,0};
    std::vector<Enemy> enemies;
    std::vector<Projectile> shots, hazards;
    std::vector<Particle> particles;
    std::vector<Rectangle> obstacles;
    std::vector<Vector2> hearts;
    std::vector<Cue> cues;
    Vector2 bell{};
    bool collected{}, bossDefeated{}, moving{}, companionMoving{}, hasSave{}, saveRequested{};
    bool companionLeft{}, showIntro{}, transitionPending{};
    float time{}, runTime{}, invincible{}, shootCooldown{}, helpCooldown{}, dashCooldown{}, dashTime{};
    float castTime{}, helpCastTime{}, hurtTime{}, pulse{}, shake{}, noticeTime{}, introTime{}, transitionTime{};
    std::string notice;
    void newGame();
    void loadStage(int index);
    void resumeCheckpoint();
    void retry();
    void update(float dt,const Input& input);
    void shoot();
    void help();
    void dash();
    void damage();
    void damageEnemy(Enemy& enemy,int amount);
    void removeDefeated();
    void updateShots(float dt);
    void updateHazards(float dt);
    void pickups();
    bool walkable(Vector2 point,float radius=17) const;
    Vector2 move(Vector2 origin,Vector2 motion,float radius=17) const;
    bool gateOpen() const { return collected&&enemies.empty()&&stage<2; }
    void notify(const std::string& text,float duration=3.6f);
    void burst(Vector2 pos,Color color,int count=12);
    void requestCheckpoint();
};
const char* zoneName(int stage);
const char* objective(const Game& game);
}
