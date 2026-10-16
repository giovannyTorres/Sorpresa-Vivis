#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include "raylib.h"

namespace vivis {
constexpr int Width=1280, Height=720, FloorCount=4, MaxHp=6;
constexpr Rectangle Field{64,160,1152,492};
constexpr float DoorHalfWidth=50, DoorCaptureDepth=32, DoorApproachDepth=96,
                DoorAssistHalfWidth=72, DoorTravelLane=30;
constexpr Vector2 RoomCenter{640,405}, PortalPosition{640,245}, MalikethPosition{930,410}, RescuePosition{1090,290};
enum class Screen { Title,Playing,Paused,Settings,Controls,Gallery,Lost,Won,Reward };
enum class Cue { Shoot,Hit,Heal,Bell,Help,Dash,Win,Transition,Coin,Item,Poison,Revive,Churu };
enum class RoomKind { Start,Combat,Treasure,Shop,Sanctuary,Challenge,Boss };
enum class EnemyKind { Fluff,Hopper,WhiskerShooter,Pouncer,Moth,Spore,Mirror,Charger,PillowBoss,WhiskerBoss,WikermanBoss,Maliketh };
enum class ShotKind { Heart,Venom,Digital,Fluff,Whisker,Moon,Spore };
enum class PickupKind { Heart,Coin,Item,Churu };
enum class ItemId { HeartBrooch,SoftBoots,QuickRibbon,TwinHearts,PlushVest,Milk,ExtraLife,VenomFang,WiskyBell,Catnip,ToxicCloud,DigitalChip,LuckyWhisker,ShieldScarf,Count };
constexpr int ItemCount=int(ItemId::Count);
struct Input { float x{},y{}; bool shoot{},help{},dash{}; float aimX{},aimY{}; };
struct Enemy {
    Vector2 pos{}; float hp{3}; bool boss{}; float timer{1.4f},flash{}; int phase{};
    EnemyKind kind{EnemyKind::Fluff}; float maxHp{3},windup{},poisonLeft{},poisonDps{},poisonTick{},cooldown{},age{};
    Vector2 velocity{},target{};
};
struct Projectile {
    Vector2 pos{},velocity{}; float life{2},damage{1}; ShotKind kind{ShotKind::Heart};
    float poisonDps{},poisonSeconds{},radius{7}; bool piercing{}; int lastHit{-1};
};
struct Particle { Vector2 pos{},velocity{}; float life{},maxLife{}; Color color{}; };
struct Pickup { Vector2 pos{}; PickupKind kind{PickupKind::Coin}; int amount{1}; ItemId item{ItemId::Milk}; };
struct GroundEffect { Vector2 pos{}; float radius{60},life{3},age{}; bool friendly{true}; ShotKind kind{ShotKind::Venom}; };
struct Preferences { float volume{0.55f}; bool muted{},fullscreen{},reducedMotion{},easy{}; };
struct Stats {
    float damage{1},speed{270},fireInterval{0.29f},shotSpeed{610},range{850}; int multishot{1};
    float wiskyDamage{1},poisonDps{0.8f},poisonDuration{3.5f},wiskyInterval{1.05f},helpCooldown{5},helpRadius{230};
    int luck{},shields{}; bool toxicCloud{},digitalBuddy{};
};
struct Room {
    int x{},y{},layout{}; RoomKind kind{RoomKind::Combat}; std::array<int,4> neighbors{{-1,-1,-1,-1}};
    bool visited{},cleared{},rewardClaimed{},rewardGenerated{}; std::uint32_t seed{};
    std::vector<Enemy> enemies; std::vector<Pickup> pickups; std::vector<Rectangle> obstacles;
    std::array<ItemId,3> offers{{ItemId::HeartBrooch,ItemId::SoftBoots,ItemId::VenomFang}};
};
struct ItemInfo { const char* name; const char* description; Color color; bool wisky; };
const ItemInfo& itemInfo(ItemId id);
const char* roomName(RoomKind kind);
const char* enemyName(EnemyKind kind);
const char* zoneName(int stage);

inline Vector2 add(Vector2 a,Vector2 b) { return {a.x+b.x,a.y+b.y}; }
inline Vector2 sub(Vector2 a,Vector2 b) { return {a.x-b.x,a.y-b.y}; }
inline Vector2 mul(Vector2 a,float s) { return {a.x*s,a.y*s}; }
inline float length(Vector2 a) { return std::sqrt(a.x*a.x+a.y*a.y); }
inline float distance(Vector2 a,Vector2 b) { return length(sub(a,b)); }
inline Vector2 unit(Vector2 a) { float n=length(a); return n>0.001f?mul(a,1/n):Vector2{}; }
inline bool inside(Vector2 p,Rectangle r) { return p.x>=r.x&&p.y>=r.y&&p.x<r.x+r.width&&p.y<r.y+r.height; }
inline Rectangle grow(Rectangle r,float n) { return {r.x-n,r.y-n,r.width+2*n,r.height+2*n}; }

struct Game {
    Screen screen{Screen::Title}; Preferences preferences; Stats stats;
    int stage{},checkpoint{},hp{MaxHp},maxHp{MaxHp},extraLives{},coins{},defeatedCount{},currentRoom{},roomShield{};
    Vector2 player{640,520},companion{575,555},facing{0,-1},aim{0,-1},digitalCompanion{700,550};
    std::vector<Room> rooms;
    std::vector<Enemy> enemies;
    std::vector<Projectile> shots,hazards;
    std::vector<Particle> particles;
    std::vector<GroundEffect> grounds;
    std::vector<Rectangle> obstacles;
    std::vector<Vector2> hearts; // Only used by compatibility capture scenes; actual loot lives in pickups.
    std::vector<Pickup> loot;
    std::vector<Cue> cues;
    std::array<int,ItemCount> inventory{};
    std::array<ItemId,3> rewardChoices{{ItemId::HeartBrooch,ItemId::SoftBoots,ItemId::VenomFang}};
    std::array<int,3> rewardCosts{{0,0,0}};
    std::uint32_t runSeed{},rngState{1};
    Vector2 bell{};
    bool collected{true},bossDefeated{},moving{},companionMoving{},hasSave{},saveRequested{};
    bool companionLeft{},showIntro{},transitionPending{},mapOpen{},inventoryOpen{};
    bool wiskyCorrupted{},wiskyRecovered{},hasChuru{},malikethFriendly{},giovannyRescued{},rewardIsShop{};
    float time{},runTime{},invincible{},shootCooldown{},helpCooldown{},dashCooldown{},dashTime{};
    float castTime{},helpCastTime{},hurtTime{},pulse{},shake{},noticeTime{},introTime{},transitionTime{},doorCooldown{};
    float wiskyShotCooldown{},digitalShotCooldown{},companionPoisonPulse{},rewardTime{};
    std::string notice;
    void newGame(std::uint32_t seed=0);
    void loadStage(int index);
    void generateFloor(int index);
    void enterRoom(int index,int fromDirection=-1);
    void storeRoom();
    bool travel(int direction);
    Room& room() { return rooms.at(currentRoom); }
    const Room& room() const { return rooms.at(currentRoom); }
    bool doorsOpen() const { return enemies.empty(); }
    bool gateOpen() const { return doorsOpen(); }
    int clearedRooms() const;
    std::uint32_t random();
    int randomInt(int upper);
    void resumeCheckpoint();
    void retry();
    void update(float dt,const Input& input);
    void shoot();
    void help();
    void interact();
    void dash();
    void damage();
    void damageEnemy(Enemy& enemy,float amount,float poisonDamage=0,float poisonDuration=0);
    void removeDefeated();
    void updateEnemies(float dt);
    void updateWisky(float dt);
    void updateShots(float dt);
    void updateHazards(float dt);
    void pickups();
    void clearRoom();
    void offerReward(bool shop=false);
    bool chooseReward(int index);
    void grantItem(ItemId item,bool feedback=true);
    bool walkable(Vector2 point,float radius=17) const;
    Vector2 move(Vector2 origin,Vector2 motion,float radius=17) const;
    void notify(const std::string& text,float duration=3.6f);
    void burst(Vector2 pos,Color color,int count=12);
    void requestCheckpoint();
};
const char* objective(const Game& game);
}
