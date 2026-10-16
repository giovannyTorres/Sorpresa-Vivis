#include "profile.hpp"
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <limits>
#include <queue>
#include <set>
#include <sstream>
#include <type_traits>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>
#endif

namespace vivis {
std::filesystem::path profilePath() {
#ifdef _WIN32
    if(const char* base=std::getenv("LOCALAPPDATA")) return std::filesystem::path(base)/"SorpresaVivis"/"profile.txt";
    if(const char* base=std::getenv("APPDATA")) return std::filesystem::path(base)/"SorpresaVivis"/"profile.txt";
#else
    if(const char* base=std::getenv("XDG_DATA_HOME")) return std::filesystem::path(base)/"sorpresa-vivis"/"profile.txt";
    if(const char* base=std::getenv("HOME")) return std::filesystem::path(base)/".local"/"share"/"sorpresa-vivis"/"profile.txt";
#endif
    return std::filesystem::temp_directory_path()/"sorpresa-vivis"/"profile.txt";
}
namespace {
constexpr const char* Signature="VIVIS_PROFILE_V3";
constexpr std::uintmax_t MaxProfileBytes=2*1024*1024;

// The same field order serves both directions. Booleans and list sizes are
// checked while parsing, before allocating or modifying the caller's Game.
struct Codec {
    std::istream* input{};
    std::ostream* output{};
    bool good{true};
    template<class T> void value(T& field) {
        if constexpr(std::is_enum_v<T>) {
            int number=static_cast<int>(field); value(number); field=static_cast<T>(number);
        } else if constexpr(std::is_same_v<T,bool>) {
            int number=field?1:0; value(number);
            if(number!=0&&number!=1) good=false;
            field=number==1;
        } else {
            if(input) { if(!(*input>>field)) good=false; }
            else if(output) { *output<<field<<' '; if(!*output) good=false; }
        }
    }
    template<class... Values> void operator()(Values&... fields) { (value(fields),...); }
    void line() { if(output) *output<<'\n'; }
};
void vector(Codec& io,Vector2& value) { io(value.x,value.y); }
void rectangle(Codec& io,Rectangle& value) { io(value.x,value.y,value.width,value.height); }
void enemy(Codec& io,Enemy& value) {
    vector(io,value.pos);
    io(value.hp,value.boss,value.timer,value.flash,value.phase,value.kind,value.maxHp,
       value.windup,value.poisonLeft,value.poisonDps,value.poisonTick,value.cooldown,value.age);
    vector(io,value.velocity); vector(io,value.target); io.line();
}
void pickup(Codec& io,Pickup& value) {
    vector(io,value.pos); io(value.kind,value.amount,value.item); io.line();
}
void projectile(Codec& io,Projectile& value) {
    vector(io,value.pos); vector(io,value.velocity);
    io(value.life,value.damage,value.kind,value.poisonDps,value.poisonSeconds,value.radius,value.piercing,value.lastHit);
    io.line();
}
void ground(Codec& io,GroundEffect& value) {
    vector(io,value.pos); io(value.radius,value.life,value.age,value.friendly,value.kind); io.line();
}
template<class T,class Function> void list(Codec& io,std::vector<T>& values,int maximum,Function fields) {
    int count=static_cast<int>(values.size()); io(count); io.line();
    if(!io.good||count<0||count>maximum) { io.good=false; return; }
    if(io.input) values.resize(static_cast<std::size_t>(count));
    for(auto& value:values) { fields(io,value); if(!io.good) return; }
}
void room(Codec& io,Room& value) {
    io(value.x,value.y,value.layout,value.kind);
    for(auto& neighbor:value.neighbors) io(neighbor);
    io(value.visited,value.cleared,value.rewardClaimed,value.rewardGenerated,value.seed);
    for(auto& offer:value.offers) io(offer);
    io.line();
    list(io,value.enemies,128,enemy);
    list(io,value.pickups,128,pickup);
    list(io,value.obstacles,64,rectangle); io.line();
}
void stats(Codec& io,Stats& value) {
    io(value.damage,value.speed,value.fireInterval,value.shotSpeed,value.range,value.multishot,
       value.wiskyDamage,value.poisonDps,value.poisonDuration,value.wiskyInterval,value.helpCooldown,
       value.helpRadius,value.luck,value.shields,value.toxicCloud,value.digitalBuddy); io.line();
}
void preferences(Codec& io,Preferences& value) {
    io(value.volume,value.muted,value.fullscreen,value.reducedMotion,value.easy); io.line();
}
void run(Codec& io,Game& game) {
    io(game.stage,game.checkpoint,game.hp,game.maxHp,game.extraLives,game.coins,game.defeatedCount,
       game.currentRoom,game.roomShield,game.runSeed,game.rngState); io.line();
    vector(io,game.player); vector(io,game.companion); vector(io,game.facing); vector(io,game.aim);
    vector(io,game.digitalCompanion); vector(io,game.bell); io.line();
    io(game.collected,game.bossDefeated,game.companionLeft,game.showIntro,game.transitionPending,
       game.wiskyCorrupted,game.wiskyRecovered,game.hasChuru,game.malikethFriendly,game.giovannyRescued,
       game.rewardIsShop); io.line();
    io(game.time,game.runTime,game.invincible,game.shootCooldown,game.helpCooldown,game.dashCooldown,game.dashTime,
       game.castTime,game.helpCastTime,game.hurtTime,game.pulse,game.shake,game.introTime,game.transitionTime,
       game.doorCooldown,game.wiskyShotCooldown,game.digitalShotCooldown,game.companionPoisonPulse,game.rewardTime);
    io.line(); stats(io,game.stats);
    for(auto& count:game.inventory) io(count);
    io.line();
    for(auto& choice:game.rewardChoices) io(choice);
    for(auto& cost:game.rewardCosts) io(cost);
    io.line();
    list(io,game.rooms,64,room);
    list(io,game.shots,2048,projectile);
    list(io,game.hazards,2048,projectile);
    list(io,game.grounds,256,ground);
}
bool bounded(float value,float low,float high) { return std::isfinite(value)&&value>=low&&value<=high; }
bool position(Vector2 value,float margin=256) {
    return bounded(value.x,-margin,Width+margin)&&bounded(value.y,-margin,Height+margin);
}
bool item(ItemId value) { return int(value)>=0&&int(value)<ItemCount; }
bool validPreferences(const Preferences& value) { return bounded(value.volume,0,1); }
bool validEnemy(const Enemy& value) {
    return position(value.pos)&&position(value.target,4096)&&bounded(value.velocity.x,-10000,10000)&&
        bounded(value.velocity.y,-10000,10000)&&int(value.kind)>=0&&int(value.kind)<=int(EnemyKind::Maliketh)&&
        bounded(value.maxHp,0.1f,100000)&&bounded(value.hp,-10000,value.maxHp)&&
        bounded(value.timer,-100,3600)&&bounded(value.flash,-1,60)&&value.phase>=0&&value.phase<=1000000&&
        bounded(value.windup,-1,3600)&&bounded(value.poisonLeft,-1,3600)&&bounded(value.poisonDps,0,10000)&&
        bounded(value.poisonTick,-100,3600)&&bounded(value.cooldown,-100,3600)&&bounded(value.age,0,604800);
}
bool validPickup(const Pickup& value) {
    return position(value.pos)&&int(value.kind)>=0&&int(value.kind)<=int(PickupKind::Churu)&&
        value.amount>=0&&value.amount<=10000&&item(value.item);
}
bool validRectangle(Rectangle value) {
    return position({value.x,value.y})&&bounded(value.width,1,Width)&&bounded(value.height,1,Height)&&
        value.x+value.width<=Width+256&&value.y+value.height<=Height+256;
}
bool validProjectile(const Projectile& value) {
    return position(value.pos,4096)&&bounded(value.velocity.x,-10000,10000)&&
        bounded(value.velocity.y,-10000,10000)&&bounded(value.life,-1,120)&&bounded(value.damage,0,10000)&&
        int(value.kind)>=0&&int(value.kind)<=int(ShotKind::Spore)&&bounded(value.poisonDps,0,10000)&&
        bounded(value.poisonSeconds,0,3600)&&bounded(value.radius,0.1f,256)&&value.lastHit>=-1&&value.lastHit<=10000;
}
bool validStats(const Stats& value) {
    return bounded(value.damage,0.01f,10000)&&bounded(value.speed,10,3000)&&bounded(value.fireInterval,0.005f,60)&&
        bounded(value.shotSpeed,10,10000)&&bounded(value.range,10,10000)&&value.multishot>=1&&value.multishot<=64&&
        bounded(value.wiskyDamage,0,10000)&&bounded(value.poisonDps,0,10000)&&bounded(value.poisonDuration,0,3600)&&
        bounded(value.wiskyInterval,0.005f,60)&&bounded(value.helpCooldown,0.005f,3600)&&
        bounded(value.helpRadius,1,3000)&&value.luck>=0&&value.luck<=10000&&value.shields>=0&&value.shields<=1000;
}
bool validRun(const Game& game) {
    if(game.stage<0||game.stage>=FloorCount||game.checkpoint!=game.stage||game.rooms.empty()||game.rooms.size()>64||
       game.currentRoom<0||game.currentRoom>=int(game.rooms.size())||game.maxHp<1||game.maxHp>1000||
       game.hp<1||game.hp>game.maxHp||game.extraLives<0||game.extraLives>1000||game.coins<0||game.coins>1000000||
       game.defeatedCount<0||game.defeatedCount>10000000||game.roomShield<0||game.roomShield>1000||
       !position(game.player,32)||!position(game.companion)||!position(game.digitalCompanion)||!position(game.bell)||
       !bounded(game.facing.x,-2,2)||!bounded(game.facing.y,-2,2)||
       !bounded(game.aim.x,-2,2)||!bounded(game.aim.y,-2,2)||!validStats(game.stats)||
       !bounded(game.time,0,604800)||!bounded(game.runTime,0,604800)||game.rngState==0) return false;
    for(float timer:{game.invincible,game.shootCooldown,game.helpCooldown,game.dashCooldown,game.dashTime,
            game.castTime,game.helpCastTime,game.hurtTime,game.pulse,game.shake,game.introTime,game.transitionTime,
            game.doorCooldown,game.wiskyShotCooldown,game.digitalShotCooldown,game.companionPoisonPulse,game.rewardTime})
        if(!bounded(timer,-1,3600)) return false;
    for(auto count:game.inventory) if(count<0||count>10000) return false;
    for(auto choice:game.rewardChoices) if(!item(choice)) return false;
    for(auto cost:game.rewardCosts) if(cost<0||cost>1000000) return false;
    std::set<std::pair<int,int>> coordinates;
    int starts=0,bosses=0;
    constexpr int dx[]={0,1,0,-1},dy[]={-1,0,1,0};
    for(int index=0;index<int(game.rooms.size());++index) {
        const auto& value=game.rooms[index];
        if(value.x< -32||value.x>32||value.y< -32||value.y>32||value.layout<0||value.layout>10000||
           int(value.kind)<0||int(value.kind)>int(RoomKind::Boss)||!coordinates.emplace(value.x,value.y).second||
           value.enemies.size()>128||value.pickups.size()>128||value.obstacles.size()>64) return false;
        if(value.kind==RoomKind::Start) ++starts;
        if(value.kind==RoomKind::Boss) ++bosses;
        if(value.cleared&&!value.enemies.empty()) return false;
        for(int direction=0;direction<4;++direction) {
            int next=value.neighbors[direction];
            if(next== -1) continue;
            if(next<0||next>=int(game.rooms.size())||next==index) return false;
            const auto& other=game.rooms[next];
            if(other.neighbors[(direction+2)%4]!=index||other.x!=value.x+dx[direction]||other.y!=value.y+dy[direction]) return false;
        }
        for(auto offer:value.offers) if(!item(offer)) return false;
        for(const auto& foe:value.enemies) if(!validEnemy(foe)) return false;
        for(const auto& loot:value.pickups) if(!validPickup(loot)) return false;
        for(auto obstacle:value.obstacles) if(!validRectangle(obstacle)) return false;
    }
    if(starts!=1||bosses!=1||!game.rooms[game.currentRoom].visited) return false;
    std::vector<bool> reached(game.rooms.size()); std::queue<int> frontier;
    frontier.push(0); reached[0]=true; std::size_t count=0;
    while(!frontier.empty()) {
        int index=frontier.front(); frontier.pop(); ++count;
        for(int next:game.rooms[index].neighbors) if(next>=0&&!reached[next]) { reached[next]=true; frontier.push(next); }
    }
    if(count!=game.rooms.size()||game.shots.size()>2048||game.hazards.size()>2048||game.grounds.size()>256) return false;
    for(const auto& shot:game.shots) if(!validProjectile(shot)) return false;
    for(const auto& shot:game.hazards) if(!validProjectile(shot)) return false;
    for(const auto& area:game.grounds)
        if(!position(area.pos)||!bounded(area.radius,1,3000)||!bounded(area.life,-1,3600)||
           !bounded(area.age,0,604800)||int(area.kind)<0||int(area.kind)>int(ShotKind::Spore)) return false;
    return true;
}
bool atEnd(std::istream& input) {
    input>>std::ws;
    return input.peek()==std::char_traits<char>::eof();
}
bool read(Game& game,const std::filesystem::path& path) {
    std::error_code error;
    auto bytes=std::filesystem::file_size(path,error);
    if(error||bytes>MaxProfileBytes) return false;
    std::ifstream input(path); std::string signature;
    if(!std::getline(input,signature)) return false;
    if(!signature.empty()&&signature.back()=='\r') signature.pop_back();
    Game result;
    Codec io{&input,nullptr};
    if(signature=="VIVIS_PROFILE_V2") {
        int checkpoint=-1; float elapsed=0;
        io(checkpoint,elapsed); preferences(io,result.preferences);
        if(!io.good||checkpoint< -1||checkpoint>2||!bounded(elapsed,0,604800)||
           !validPreferences(result.preferences)||!atEnd(input)) return false;
        // V2 checkpoints described a different, linear world. Preserve settings
        // without silently regenerating a different run under the same save.
        result.hasSave=false;
    } else {
        if(signature!=Signature) return false;
        preferences(io,result.preferences); io(result.hasSave); io.line();
        if(!io.good||!validPreferences(result.preferences)) return false;
        if(result.hasSave) {
            run(io,result);
            if(!io.good||!validRun(result)) return false;
            result.enemies=result.room().enemies;
            result.loot=result.room().pickups;
            result.obstacles=result.room().obstacles;
        }
        std::string ending;
        if(!(input>>ending)||ending!="END"||!atEnd(input)) return false;
    }
    result.screen=Screen::Title;
    result.saveRequested=false;
    result.mapOpen=result.inventoryOpen=false;
    game=std::move(result);
    return true;
}
bool replaceFile(const std::filesystem::path& from,const std::filesystem::path& to) {
#ifdef _WIN32
    // Unlike std::filesystem::rename on Windows, this replaces the destination
    // without a remove/rename gap if the application is interrupted.
    return MoveFileExW(from.c_str(),to.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
#else
    std::error_code error;
    std::filesystem::rename(from,to,error);
    return !error;
#endif
}
}
bool loadProfile(Game& game,const std::filesystem::path& path) {
    if(read(game,path)) return true;
    auto backup=path; backup+=".bak";
    return read(game,backup);
}
bool saveProfile(const Game& game,const std::filesystem::path& path) {
    Game snapshot=game;
    if(!validPreferences(snapshot.preferences)) return false;
    if(snapshot.hasSave) {
        if(snapshot.currentRoom<0||snapshot.currentRoom>=int(snapshot.rooms.size())) return false;
        snapshot.storeRoom();
        if(!validRun(snapshot)) return false;
    }
    std::error_code error;
    if(!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path(),error);
    if(error) return false;
    auto temporary=path; temporary+=".tmp";
    auto backup=path; backup+=".bak";
    {
        std::ofstream output(temporary,std::ios::trunc);
        if(!output) return false;
        output<<Signature<<'\n'<<std::setprecision(std::numeric_limits<float>::max_digits10);
        Codec io{nullptr,&output};
        preferences(io,snapshot.preferences); io(snapshot.hasSave); io.line();
        if(snapshot.hasSave) run(io,snapshot);
        output<<"END\n"; output.flush();
        if(!io.good||!output) return false;
    }
    // Only replace the recovery copy with a known-good previous profile.
    // A corrupt primary must never overwrite the last usable backup.
    if(std::filesystem::exists(path,error)) {
        Game previous;
        if(read(previous,path)) {
            auto backupTemporary=backup; backupTemporary+=".tmp";
            std::filesystem::copy_file(path,backupTemporary,std::filesystem::copy_options::overwrite_existing,error);
            if(error||!replaceFile(backupTemporary,backup)) return false;
        }
    }
    return replaceFile(temporary,path);
}
}
