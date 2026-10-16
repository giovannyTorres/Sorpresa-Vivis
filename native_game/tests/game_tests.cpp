#include "game.hpp"
#include "profile.hpp"
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>

using namespace vivis;
namespace {
int passed{},failed{},floorsChecked{},doorCases{};
void expect(bool value,const std::string& message) { if(!value)throw std::runtime_error(message); }
void near(float value,float wanted,float tolerance,const std::string& message) {
    expect(std::fabs(value-wanted)<=tolerance,message+" ("+std::to_string(value)+" vs "+std::to_string(wanted)+")");
}
void scenario(const char* name,const std::function<void()>& test) {
    try {test();++passed;std::cout<<"PASS "<<name<<'\n';}
    catch(const std::exception& e){++failed;std::cerr<<"FAIL "<<name<<": "<<e.what()<<'\n';}
}
void frames(Game& g,int count,Input input={},float dt=1.f/60) {for(int n=0;n<count;++n)g.update(dt,input);}
int roomOf(const Game& g,RoomKind kind) {
    for(int i=0;i<int(g.rooms.size());++i)if(g.rooms[i].kind==kind)return i;
    throw std::runtime_error("missing room kind "+std::to_string(int(kind)));
}
int directionOf(const Game& g) {
    for(int d=0;d<4;++d)if(g.room().neighbors[d]>=0)return d;
    throw std::runtime_error("room has no exits");
}
Game arena() {
    Game g;g.newGame(1974);g.player=RoomCenter;g.enemies.clear();g.obstacles.clear();g.loot.clear();
    g.room().cleared=g.room().rewardClaimed=g.room().rewardGenerated=true;
    g.invincible=g.doorCooldown=0;g.cues.clear();g.saveRequested=false;return g;
}
Enemy foe(Vector2 position,float hp=20,EnemyKind kind=EnemyKind::Fluff) {
    Enemy e;e.pos=position;e.hp=e.maxHp=hp;e.kind=kind;e.timer=100;return e;
}
constexpr Vector2 outward[]={{0,-1},{1,0},{0,1},{-1,0}};
float perpendicular(Vector2 p,int direction) {return direction%2?p.y-405:p.x-640;}
Vector2 approach(int direction,float offset,float depth) {
    Vector2 p;
    if(direction==0)p={640,Field.y+depth};
    else if(direction==1)p={Field.x+Field.width-depth,405};
    else if(direction==2)p={640,Field.y+Field.height-depth};
    else p={Field.x+depth,405};
    if(direction%2)p.y+=offset;else p.x+=offset;return p;
}
Game corridor(int direction,float offset=0,float depth=120) {
    Game g=arena();g.rooms.assign(2,Room{});g.currentRoom=0;
    g.rooms[0].kind=RoomKind::Start;g.rooms[1].kind=RoomKind::Combat;
    g.rooms[0].neighbors[direction]=1;g.rooms[1].neighbors[(direction+2)%4]=0;
    g.rooms[1].x=int(outward[direction].x);g.rooms[1].y=int(outward[direction].y);
    for(auto& room:g.rooms){room.cleared=true;room.rewardGenerated=true;room.rewardClaimed=true;}
    g.enterRoom(0);g.player=approach(direction,offset,depth);g.invincible=1000;
    g.doorCooldown=0;g.wiskyShotCooldown=1000;g.digitalShotCooldown=1000;g.cues.clear();return g;
}
float frameDelta(int style,int frame) {
    constexpr float variable[]={1.f/144,1.f/38,1.f/75,1.f/24,.047f,1.f/90};
    return style==0?1.f/30:style==1?1.f/60:style==2?1.f/120:variable[frame%6];
}
void finish(Game& g) {for(auto& e:g.enemies)g.damageEnemy(e,e.maxHp+1);g.removeDefeated();}
std::string layout(const Game& g) {
    std::ostringstream out;
    for(const auto& r:g.rooms){out<<r.x<<','<<r.y<<','<<int(r.kind)<<','<<r.layout<<','<<r.seed<<':';
        for(int n:r.neighbors)out<<n<<',';
        for(auto item:r.offers)out<<int(item)<<',';
        for(const auto& e:r.enemies)out<<int(e.kind)<<','<<e.hp<<','<<e.pos.x<<','<<e.pos.y<<';';
        out<<'|';}
    return out.str();
}
std::string topology(const Game& g) {
    // Omit raw seeds, offers and enemy stats: changing only metadata does not
    // establish that players actually receive different room maps.
    std::ostringstream out;
    for(const auto& room:g.rooms){out<<room.x<<','<<room.y<<','<<int(room.kind)<<':';
        for(int next:room.neighbors)out<<next<<',';
        out<<'|';}
    return out.str();
}
void auditFloor(const Game& g) {
    expect(g.rooms.size()>=7&&g.rooms.size()<=64,"unusable room count");
    std::array<int,7> kinds{};std::set<std::pair<int,int>> coordinates;
    constexpr int dx[]{0,1,0,-1},dy[]{-1,0,1,0};
    for(int i=0;i<int(g.rooms.size());++i){const auto& r=g.rooms[i];++kinds.at(int(r.kind));
        expect(coordinates.emplace(r.x,r.y).second,"overlapping rooms");
        for(int d=0;d<4;++d){int next=r.neighbors[d];if(next<0)continue;
            expect(next<int(g.rooms.size())&&next!=i,"invalid neighbor");const auto& other=g.rooms[next];
            expect(other.neighbors[(d+2)%4]==i,"nonreciprocal door");
            expect(other.x==r.x+dx[d]&&other.y==r.y+dy[d],"door joins nonadjacent rooms");}
        std::set<EnemyKind> enemyKinds;
        for(const auto& e:r.enemies){enemyKinds.insert(e.kind);expect(e.hp>0&&e.hp<=e.maxHp&&std::isfinite(e.hp),"invalid generated health");
            expect(inside(e.pos,grow(Field,-20)),"enemy outside field");
            for(auto prop:r.obstacles)expect(!inside(e.pos,grow(prop,20)),"enemy spawned inside prop");}
        if(r.kind==RoomKind::Combat||r.kind==RoomKind::Challenge)
            expect(enemyKinds.size()>=std::min<std::size_t>(r.enemies.size(),6),"generated encounter repeats types instead of providing variety");}
    for(auto kind:{RoomKind::Start,RoomKind::Boss,RoomKind::Treasure,RoomKind::Shop,RoomKind::Sanctuary})
        expect(kinds[int(kind)]==1,"missing or duplicated special room");
    const auto& boss=g.rooms[roomOf(g,RoomKind::Boss)];
    expect(boss.enemies.size()==1&&boss.enemies[0].boss,"missing unique guardian");
    expect(boss.enemies[0].kind==EnemyKind(int(EnemyKind::PillowBoss)+g.stage),"wrong floor guardian");
    std::set<int> reached{0};std::queue<int> pending;pending.push(0);
    while(!pending.empty()){int n=pending.front();pending.pop();for(int next:g.rooms[n].neighbors)
        if(next>=0&&reached.insert(next).second)pending.push(next);}
    expect(reached.size()==g.rooms.size(),"disconnected floor");++floorsChecked;
}
struct Temp {
    std::filesystem::path root,path;
    Temp(){static unsigned serial{};root=std::filesystem::temp_directory_path()/
        ("vivis-rogue-qa-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+"-"+std::to_string(serial++));path=root/"nested/profile.txt";}
    ~Temp(){std::error_code error;std::filesystem::remove_all(root,error);}
    void write(const std::string& data){std::filesystem::create_directories(path.parent_path());std::ofstream out(path);out<<data;}
    std::string read()const{std::ifstream in(path);return {std::istreambuf_iterator<char>(in),{}};}
};
std::string snapshot(const Game& g) {
    Temp f;expect(saveProfile(g,f.path),"cannot snapshot valid test fixture");return f.read();
}
std::string changeToken(const std::string& input,int lineNumber,int column,const std::string& replacement) {
    std::istringstream lines(input);std::string line,result;int index{};
    while(std::getline(lines,line)){if(index++==lineNumber){std::istringstream words(line);std::string word;int n{};line.clear();
        while(words>>word){if(n++==column)word=replacement;line+=word+' ';}}result+=line+'\n';}return result;
}
Game populated() {
    Game g;g.newGame(94631);g.loadStage(2);g.enterRoom(roomOf(g,RoomKind::Challenge));
    g.preferences={.37f,true,false,true,true};g.hp=3;g.coins=23;g.extraLives=2;g.defeatedCount=7;
    for(auto item:{ItemId::HeartBrooch,ItemId::VenomFang,ItemId::TwinHearts,ItemId::DigitalChip,ItemId::ToxicCloud})g.grantItem(item,false);
    g.player={620,410};g.companion={570,430};g.aim={1,0};g.facing={0,-1};
    g.enemies[0].hp-=1;g.enemies[0].poisonLeft=1.7f;g.enemies[0].poisonDps=1.2f;
    g.enemies[0].windup=.45f;g.enemies[0].target={620,410};g.enemies[0].phase=2;
    g.loot={{{600,300},PickupKind::Coin,4},{{650,300},PickupKind::Heart,2},{{700,300},PickupKind::Item,1,ItemId::Milk}};
    g.shots={{{700,400},{610,0},.8f,1.4f,ShotKind::Heart},{{580,420},{400,-50},1.1f,1,ShotKind::Venom,1.5f,3,8}};
    g.hazards={{{850,410},{-160,40},2,1,ShotKind::Whisker}};
    g.grounds={{{720,350},65,2,.9f,false,ShotKind::Moon},{{560,400},45,1.5f,.2f,true,ShotKind::Venom}};
    g.wiskyRecovered=true;g.hasChuru=true;g.runTime=94.25f;g.time=12.75f;g.shootCooldown=.17f;
    g.helpCooldown=2.1f;g.dashCooldown=.9f;g.doorCooldown=.12f;g.wiskyShotCooldown=.6f;g.digitalShotCooldown=.3f;g.requestCheckpoint();return g;
}
}

int main() {
    scenario("100 seeds: all four floors are connected reciprocal collision-safe and contain unique special rooms",[]{
        for(unsigned seed=1;seed<=100;++seed){Game g;g.newGame(seed);
            for(int floor=0;floor<FloorCount;++floor){if(floor)g.loadStage(floor);auditFloor(g);}}
    });
    scenario("same seed recreates every floor's graph enemies and offers",[]{
        Game a,b;a.newGame(729);b.newGame(729);for(int floor=0;floor<FloorCount;++floor){
            if(floor){a.loadStage(floor);b.loadStage(floor);}expect(layout(a)==layout(b),"same seed diverged");}
    });
    scenario("different seeds generate materially different adventures",[]{
        std::set<std::string> values;for(unsigned seed=1;seed<=30;++seed){Game g;g.newGame(seed);values.insert(topology(g));}
        expect(values.size()>=25,"seed hardly changes adventure");
    });
    scenario("new run starts safely saved and preserves preferences",[]{
        Game g;g.preferences.reducedMotion=true;g.newGame(80);
        expect(g.screen==Screen::Playing&&g.stage==0&&g.hasSave&&g.saveRequested,"new run not playable/saved");
        expect(g.room().kind==RoomKind::Start&&g.room().visited&&g.enemies.empty(),"unsafe starting room");
        expect(g.hp==MaxHp&&g.coins==0&&g.extraLives==0&&g.preferences.reducedMotion,"bad resources/preferences");
    });
    scenario("title pause reward defeat victory and both overlays freeze simulation",[]{
        for(auto screen:{Screen::Title,Screen::Paused,Screen::Reward,Screen::Lost,Screen::Won}){
            Game g=arena();g.enemies={foe({900,405})};g.screen=screen;frames(g,30,{1,0,true,true,true,0,-1});
            expect(g.runTime==0&&g.player.x==RoomCenter.x&&g.shots.empty(),"nonplaying screen simulated");}
        for(bool inventory:{false,true}){Game g=arena();g.mapOpen=!inventory;g.inventoryOpen=inventory;
            frames(g,30,{1,0,true,true,true,0,-1});expect(g.runTime==0&&g.shots.empty(),"overlay did not freeze");}
    });
    scenario("movement is normalized and consistent at 30 60 120 FPS",[]{
        Game a=arena(),b=arena(),c=arena(),diagonal=arena();frames(a,30,{1,0},1.f/30);frames(b,60,{1,0});frames(c,120,{1,0},1.f/120);
        frames(diagonal,30,{1,-1});near(distance(a.player,b.player),0,.05f,"30 FPS differs");
        near(distance(c.player,b.player),0,.05f,"120 FPS differs");near(distance(diagonal.player,RoomCenter),135,.05f,"diagonal boost");
    });
    scenario("walking dash and large moves cannot tunnel through thin obstacles",[]{
        Game g=arena();g.player={200,405};g.obstacles={{300,250,8,310}};frames(g,90,{1,0});
        expect(g.player.x<284&&g.walkable(g.player),"walking crossed wall");g.player={270,405};frames(g,15,{1,0,false,false,true});
        expect(g.player.x<284&&g.walkable(g.player),"dash crossed wall");expect(g.move({200,405},{900,0}).x<284,"large move tunnelled");
    });
    scenario("travel locks during combat and uses the reciprocal adjacent door",[]{
        Game g;g.newGame(72);int origin=g.currentRoom,d=directionOf(g),next=g.room().neighbors[d];g.enemies={foe({900,400})};
        expect(!g.travel(d)&&g.currentRoom==origin,"travel bypassed lock");g.enemies.clear();
        expect(g.travel(d)&&g.currentRoom==next&&g.room().visited&&g.saveRequested,"clear door did not enter/save adjacent room");
        expect(!g.travel(-1)&&!g.travel(4),"invalid direction accepted");
    });
    scenario("actual frame movement through a doorway enters its neighboring room",[]{
        Game g;g.newGame(51);int d=directionOf(g),next=g.room().neighbors[d];g.doorCooldown=0;
        constexpr Vector2 edges[]={{640,179},{1197,405},{640,633},{83,405}},directions[]={{0,-1},{1,0},{0,1},{-1,0}};
        g.player=edges[d];g.update(1.f/60,{directions[d].x,directions[d].y});expect(g.currentRoom==next,"frame input failed door travel");
    });
    scenario("door traversal works across all directions FPS speeds offsets diagonals and dashes without interaction",[]{
        for(int d=0;d<4;++d)for(int style=0;style<4;++style)for(float speed:{270.f,295.f,420.f})
        for(float offset:{0.f,-30.f,30.f,-45.f,45.f,-60.f,60.f})for(int mode=0;mode<3;++mode){
            Game g=corridor(d,offset,mode==1?52.f:120.f);g.stats.speed=speed;
            Vector2 input=outward[d];
            if(mode==1){float tangent=offset>0?-1.f:1.f;if(d%2)input.y=tangent;else input.x=tangent;}
            int frame=0;float elapsed=0;
            while(g.currentRoom==0&&elapsed<4){float dt=frameDelta(style,frame);
                g.update(dt,{input.x,input.y,false,false,mode==2&&frame==0});elapsed+=dt;++frame;}
            std::string which="dir="+std::to_string(d)+" FPSstyle="+std::to_string(style)+" speed="+std::to_string(speed)+
                " offset="+std::to_string(offset)+" mode="+std::to_string(mode)+" pos=("+std::to_string(g.player.x)+","+std::to_string(g.player.y)+")";
            expect(g.currentRoom==1,"door stuck: "+which);
            expect(std::fabs(perpendicular(g.player,d))<=30.01f,"entry left assisted doorway lane: "+which);
            if(mode!=1&&std::fabs(offset)<=30)near(perpendicular(g.player,d),offset,.05f,"entry snapped perpendicular position: "+which);
            expect(g.walkable(g.player)&&g.walkable(g.companion,12)&&g.walkable(g.digitalCompanion,12),"entry spawned an actor outside the room: "+which);
            for(float hold=0;hold<.24f;){float dt=frameDelta(style,frame++);g.update(dt,{input.x,input.y});hold+=dt;}
            expect(g.currentRoom==1,"held movement bounced back through entry: "+which);
            expect(std::count(g.cues.begin(),g.cues.end(),Cue::Transition)==1,"one doorway generated repeated transitions: "+which);
            ++doorCases;
        }
    });
    scenario("north door no longer stalls at y180 under ordinary 60 FPS walking",[]{
        Game g;g.newGame(51);g.enterRoom(0);g.room().cleared=true;g.enemies.clear();g.obstacles.clear();g.loot.clear();
        g.player=RoomCenter;g.doorCooldown=0;int target=g.room().neighbors[0];expect(target>=0,"repro seed changed north topology");
        for(int frame=0;frame<240&&g.currentRoom==0;++frame)g.update(1.f/60,{0,-1});
        expect(g.currentRoom==target,"original seed51 y180 deadlock returned");
    });
    scenario("door guide ignores parallel movement and actors far outside the opening",[]{
        for(int d=0;d<4;++d){Game parallel=corridor(d,0,30);Vector2 input=d%2?Vector2{0,1}:Vector2{1,0};
            for(int n=0;n<120;++n)parallel.update(1.f/60,{input.x,input.y});
            expect(parallel.currentRoom==0,"parallel movement was sucked through a door");
            Game outside=corridor(d,120,100);frames(outside,240,{outward[d].x,outward[d].y});
            expect(outside.currentRoom==0&&outside.walkable(outside.player),"wall outside opening became a door");
            near(perpendicular(outside.player,d),120,.05f,"far-away actor was pulled toward doorway");}
    });
    scenario("closed combat doors and absent neighbors stay solid at all four edges",[]{
        for(int d=0;d<4;++d){Game closed=corridor(d,0,80);closed.enemies={foe(RoomCenter,100000)};
            frames(closed,240,{outward[d].x,outward[d].y});expect(closed.currentRoom==0&&closed.walkable(closed.player),"combat lock allowed exit");
            Game wall=corridor(d,0,80);wall.room().neighbors[d]=-1;frames(wall,240,{outward[d].x,outward[d].y});
            expect(wall.currentRoom==0&&wall.walkable(wall.player),"absent neighboring room became an exit");}
    });
    scenario("door traversal preserves loot state and permits intentional return after cooldown",[]{
        for(int d=0;d<4;++d){Game g=corridor(d,20,65);g.loot={{{450,405},PickupKind::Coin,3}};
            for(int n=0;n<120&&g.currentRoom==0;++n)g.update(1.f/60,{outward[d].x,outward[d].y});
            expect(g.currentRoom==1,"could not cross for return test");
            frames(g,20);Vector2 reverse=mul(outward[d],-1);
            for(int n=0;n<120&&g.currentRoom==1;++n)g.update(1.f/60,{reverse.x,reverse.y});
            expect(g.currentRoom==0&&g.loot.size()==1&&g.loot[0].amount==3,"intentional return failed or room loot changed");}
    });
    scenario("cleared rooms and collected rewards do not respawn on return",[]{
        Game g;g.newGame(61);int combat=roomOf(g,RoomKind::Combat);g.enterRoom(combat);finish(g);
        expect(g.enemies.empty()&&g.room().cleared&&g.room().rewardGenerated,"room failed to clear");g.loot.clear();g.storeRoom();
        int d=directionOf(g);expect(g.travel(d),"cannot leave clear room");g.enemies.clear();
        expect(g.travel((d+2)%4)&&g.currentRoom==combat,"cannot return by reciprocal door");
        expect(g.enemies.empty()&&g.loot.empty()&&g.room().cleared,"room respawned enemies/loot");
        int kills=g.defeatedCount;g.removeDefeated();g.clearRoom();expect(g.loot.empty()&&g.defeatedCount==kills,"room regenerated loot/kills");
    });
    scenario("partial enemy health poison and room loot survive reentry",[]{
        Game g;g.newGame(75);int combat=roomOf(g,RoomKind::Combat);g.enterRoom(combat);
        g.enemies[0].hp-=1;g.enemies[0].poisonLeft=2;g.enemies[0].poisonDps=1.4f;g.loot.push_back({{500,405},PickupKind::Coin,3});g.storeRoom();
        float hp=g.enemies[0].hp;auto props=g.obstacles.size();g.enterRoom(0);g.enterRoom(combat);
        expect(g.enemies[0].hp==hp&&g.enemies[0].poisonLeft==2&&g.enemies[0].poisonDps==1.4f,"entry reset combat state");
        expect(g.loot.size()==1&&g.obstacles.size()==props,"entry lost loot/layout");
    });
    scenario("treasure grants exactly one seeded choice and cannot be claimed twice",[]{
        Game g;g.newGame(90);g.enterRoom(roomOf(g,RoomKind::Treasure));auto offers=g.room().offers;g.player=RoomCenter;g.help();
        expect(g.screen==Screen::Reward&&g.rewardChoices==offers,"pedestal did not offer seeded items");
        expect(g.chooseReward(1)&&g.screen==Screen::Playing&&g.inventory[int(offers[1])]==1&&g.room().rewardClaimed,"treasure selection failed");
        g.offerReward();expect(g.screen==Screen::Playing&&!g.chooseReward(0),"treasure claimed twice");
    });
    scenario("shop rejects unaffordable and invalid purchases and charges successful selection",[]{
        Game g;g.newGame(86);g.enterRoom(roomOf(g,RoomKind::Shop));g.offerReward(true);auto items=g.inventory;
        expect(!g.chooseReward(0)&&g.inventory==items&&g.coins==0,"free unaffordable shop item");
        expect(!g.chooseReward(-1)&&!g.chooseReward(3),"invalid shop choice accepted");g.coins=30;int cost=g.rewardCosts[2];
        expect(cost>0&&g.chooseReward(2)&&g.coins==30-cost&&g.room().rewardClaimed,"shop charge/selection wrong");
    });
    scenario("sanctuary heals only once without consuming Wisky cooldown",[]{
        Game g;g.newGame(88);int sanctuary=roomOf(g,RoomKind::Sanctuary);g.enterRoom(sanctuary);g.player=RoomCenter;g.hp=1;g.help();
        expect(g.hp==g.maxHp&&g.room().rewardClaimed&&g.helpCooldown==0,"sanctuary failed or consumed help");
        g.hp=2;g.interact();expect(g.hp==2,"sanctuary healed twice");g.storeRoom();g.enterRoom(0);g.enterRoom(sanctuary);g.player=RoomCenter;g.interact();
        expect(g.hp==2,"sanctuary reset after travel");
    });
    scenario("100 duplicate upgrades respect health speed fire multishot poison and shield caps",[]{
        Game g=arena();for(int n=0;n<100;++n)for(auto item:{ItemId::SoftBoots,ItemId::QuickRibbon,ItemId::TwinHearts,
            ItemId::PlushVest,ItemId::HeartBrooch,ItemId::VenomFang,ItemId::WiskyBell,ItemId::Catnip,ItemId::ShieldScarf})g.grantItem(item,false);
        expect(g.maxHp==12&&g.hp<=g.maxHp&&g.stats.speed<=420&&g.stats.multishot==3,"health/speed/multishot cap broken");
        expect(g.stats.fireInterval>=.1099f&&g.stats.damage<=12&&g.stats.poisonDps<=12,"attack cap broken");
        expect(g.stats.wiskyInterval>=.2799f&&g.stats.helpCooldown>=1.7999f&&g.stats.helpRadius<=440&&g.stats.shields<=2,"companion cap broken");
        expect(g.inventory[int(ItemId::SoftBoots)]==100,"capped duplicate items missing from inventory");
    });
    scenario("healing health capacity and extra lives apply without invalid item side effects",[]{
        Game g=arena();g.hp=1;g.grantItem(ItemId::Milk,false);expect(g.hp==4,"milk healing failed");
        g.grantItem(ItemId::PlushVest,false);expect(g.maxHp==8&&g.hp==6,"vest capacity/heal failed");
        g.grantItem(ItemId::ExtraLife,false);g.grantItem(ItemId::ExtraLife,false);expect(g.extraLives==2,"extra lives did not stack");
        auto items=g.inventory;g.grantItem(ItemId(-1));g.grantItem(ItemId::Count);expect(g.inventory==items,"invalid item mutated inventory");
    });
    scenario("scarf absorbs one hit then restores on next room entry",[]{
        Game g=arena();g.grantItem(ItemId::ShieldScarf,false);g.damage();expect(g.hp==g.maxHp&&g.roomShield==0,"shield failed");
        g.invincible=0;g.damage();expect(g.hp==g.maxHp-1,"spent shield still blocked");g.storeRoom();g.enterRoom(0);expect(g.roomShield==1,"shield not restored");
    });
    scenario("arrow aim fires independently from walking and nearest target",[]{
        Game g=arena();g.enemies={foe({900,405})};g.update(1.f/60,{1,0,false,false,false,0,-1});
        auto shot=std::find_if(g.shots.begin(),g.shots.end(),[](const Projectile&p){return p.kind==ShotKind::Heart;});
        expect(g.player.x>RoomCenter.x&&shot!=g.shots.end(),"aim input did not shoot while walking");
        expect(std::fabs(shot->velocity.x)<.01f&&shot->velocity.y<0,"walking/autoaim replaced arrow aim");
    });
    scenario("diagonal multishot has normalized velocity and distinct heart trajectories",[]{
        Game g=arena();g.grantItem(ItemId::TwinHearts,false);g.grantItem(ItemId::TwinHearts,false);g.aim=unit({1,-1});g.shoot();
        expect(g.shots.size()==3,"multishot did not emit three hearts");for(const auto& p:g.shots)near(length(p.velocity),g.stats.shotSpeed,.01f,"diagonal projectile speed boost");
        expect(distance(g.shots[0].velocity,g.shots[1].velocity)>20&&distance(g.shots[1].velocity,g.shots[2].velocity)>20,"all hearts share trajectory");
    });
    scenario("held aim respects firing cooldown",[]{
        Game g=arena();frames(g,60,{0,0,false,false,false,1,0});int count=int(std::count(g.cues.begin(),g.cues.end(),Cue::Shoot));
        expect(count>=3&&count<=5,"held arrow aim bypassed cooldown or failed to repeat");
    });
    scenario("fast hearts and hazards respect props and swept enemy collisions",[]{
        Game g=arena();g.player={200,405};g.enemies={foe({500,405})};g.obstacles={{300,300,8,220}};
        g.shots={{{240,405},{1000,0},1}};g.updateShots(.4f);expect(g.enemies[0].hp==20&&g.shots.empty(),"heart crossed prop");
        g.hazards={{{500,405},{-1000,0},1}};g.updateHazards(.4f);expect(g.hp==g.maxHp&&g.hazards.empty(),"hazard crossed prop");
        g.obstacles.clear();g.shots={{{300,405},{1000,0},1}};g.updateShots(.3f);expect(g.enemies[0].hp==19,"fast heart tunnelled through foe");
    });
    scenario("expired friendly and hostile projectiles cause no invisible hit",[]{
        Game g=arena();g.enemies={foe({750,405})};g.shots={{{650,405},{1000,0},.01f}};g.updateShots(.2f);
        expect(g.enemies[0].hp==20&&g.shots.empty(),"expired friendly shot hit");g.hazards={{{550,405},{1000,0},.01f}};g.updateHazards(.2f);
        expect(g.hp==g.maxHp&&g.hazards.empty(),"expired hostile shot hit");
    });
    scenario("Wisky attacks with poison and never base digital projectiles",[]{
        Game g=arena();g.companion={590,420};g.enemies={foe({850,405})};g.updateWisky(.016f);expect(!g.shots.empty(),"Wisky never attacked");
        for(const auto&p:g.shots)expect(p.kind==ShotKind::Venom&&p.poisonDps>0&&p.poisonSeconds>0,"Wisky emitted digital/nonpoison shot");
        expect(itemInfo(ItemId::VenomFang).wisky&&itemInfo(ItemId::WiskyBell).wisky,"poison upgrades not assigned to Wisky");
    });
    scenario("poison upgrades improve Wisky without changing Vivis heart damage or digital powers",[]{
        Game g=arena();Stats before=g.stats;for(auto id:{ItemId::VenomFang,ItemId::WiskyBell,ItemId::Catnip,ItemId::ToxicCloud})g.grantItem(id,false);
        expect(g.stats.poisonDps>before.poisonDps&&g.stats.wiskyInterval<before.wiskyInterval&&g.stats.helpCooldown<before.helpCooldown,"poison upgrade ineffective");
        expect(g.stats.damage==before.damage&&g.stats.fireInterval==before.fireInterval&&!g.stats.digitalBuddy,"poison upgrade mutated other powers");
    });
    scenario("Giovanny chip adds a separate digital shot while Wisky remains venomous",[]{
        Game g=arena();g.enemies={foe({850,405})};g.grantItem(ItemId::DigitalChip,false);g.updateWisky(.016f);bool venom=false,digital=false;
        for(const auto&p:g.shots){venom|=p.kind==ShotKind::Venom;digital|=p.kind==ShotKind::Digital;}expect(venom&&digital,"digital projection replaced venom or failed");
    });
    scenario("poison integrates finite duration consistently at 30 and 60 FPS",[]{
        Game a=arena(),b=arena();a.player=b.player={150,200};a.enemies=b.enemies={foe({1000,550},100,EnemyKind::WhiskerShooter)};
        a.damageEnemy(a.enemies[0],1,2,1.25f);b.damageEnemy(b.enemies[0],1,2,1.25f);
        for(int n=0;n<120;++n)a.updateEnemies(1.f/60);
        for(int n=0;n<60;++n)b.updateEnemies(1.f/30);
        near(a.enemies[0].hp,96.5f,.002f,"incorrect poison total");near(a.enemies[0].hp,b.enemies[0].hp,.002f,"poison depends on FPS");
        float hp=a.enemies[0].hp;a.updateEnemies(.5f);expect(a.enemies[0].hp==hp,"expired poison still ticks");
    });
    scenario("venom impact applies damage and persistent poison status",[]{
        Game g=arena();g.enemies={foe({800,405})};g.shots={{{730,405},{500,0},1,2,ShotKind::Venom,1.5f,3,8}};g.updateShots(.14f);
        expect(g.enemies[0].hp==18&&g.enemies[0].poisonLeft>0&&g.enemies[0].poisonDps==1.5f,"venom impact/status failed");
    });
    scenario("poison help respects range and cooldown",[]{
        Game g=arena();g.enemies={foe({750,405}),foe({1100,405})};g.help();
        expect(g.enemies[0].hp<20&&g.enemies[0].poisonLeft>0&&g.enemies[1].hp==20,"help range/status incorrect");
        float hp=g.enemies[0].hp;g.help();expect(g.enemies[0].hp==hp&&g.helpCooldown>0,"help bypassed cooldown");
    });
    scenario("Wikerman suppresses venom until defeat restores and strengthens Wisky",[]{
        Game g;g.newGame(56);g.loadStage(2);g.enterRoom(roomOf(g,RoomKind::Boss));expect(g.wiskyCorrupted&&!g.wiskyRecovered,"corruption missing");
        g.player={700,405};g.companion={650,405};g.updateWisky(.016f);g.help();expect(g.shots.empty()&&g.helpCooldown==0,"corrupted Wisky still attacks");
        float before=g.stats.wiskyDamage;finish(g);expect(g.wiskyRecovered&&!g.wiskyCorrupted&&g.stats.wiskyDamage>before,"recovery/buff missing");
        g.enemies={foe({850,405})};g.updateWisky(.016f);expect(!g.shots.empty()&&g.shots[0].kind==ShotKind::Venom,"recovered Wisky cannot attack");
    });
    scenario("hostile grounds telegraph and friendly poison harms only enemies",[]{
        Game g=arena();g.grounds={{g.player,70,3,0,false,ShotKind::Moon}};g.updateHazards(.5f);expect(g.hp==g.maxHp,"ground hurt during warning");
        g.updateHazards(.31f);expect(g.hp==g.maxHp-1,"armed ground never hurt");g.hp=g.maxHp;g.invincible=0;
        g.grounds={{g.player,70,3,1,true,ShotKind::Venom}};g.enemies={foe(g.player)};g.updateHazards(.1f);
        expect(g.hp==g.maxHp&&g.enemies[0].hp<20,"friendly poison harmed Vivis or ignored foe");
    });
    scenario("all guardians warn before firing and use their own attack identity",[]{
        const ShotKind kinds[]={ShotKind::Fluff,ShotKind::Whisker,ShotKind::Venom,ShotKind::Moon};
        for(int floor=0;floor<FloorCount;++floor){Game g;g.newGame(400);g.loadStage(floor);g.enterRoom(roomOf(g,RoomKind::Boss));g.enemies[0].timer=0;g.updateEnemies(.016f);
            expect(g.enemies[0].windup>0&&g.hazards.empty(),"boss fired without warning");for(int n=0;n<70&&g.hazards.empty();++n)g.updateEnemies(1.f/60);
            expect(!g.hazards.empty(),"boss failed to attack");for(const auto&p:g.hazards)expect(p.kind==kinds[floor],"wrong boss attack type");}
    });
    scenario("dash protects against simultaneous contact and active projectiles",[]{
        Game g=arena();g.enemies={foe(g.player,100)};g.hazards={{g.player,{0,0},1}};g.update(1.f/60,{1,0,false,false,true});
        expect(g.hp==g.maxHp&&g.dashCooldown>0,"dash protection/cooldown failed");
    });
    scenario("extra life keeps upgrades then final defeat clears resumable save",[]{
        Game g=arena();g.grantItem(ItemId::HeartBrooch,false);g.grantItem(ItemId::ExtraLife,false);auto items=g.inventory;g.hp=1;
        g.hazards={{{700,405},{-100,0},3}};g.damage();expect(g.screen==Screen::Playing&&g.hp==g.maxHp&&g.extraLives==0&&g.inventory==items,"revive lost upgrades/life");
        expect(g.hazards.empty()&&g.invincible>0&&g.hasSave,"revive did not clear danger/save");g.invincible=0;g.hp=1;g.damage();
        expect(g.screen==Screen::Lost&&!g.hasSave&&g.saveRequested,"final defeat kept Continue");
    });
    scenario("projectile revival safely clears a multi-projectile attack and hostile grounds",[]{
        Game g=arena();g.hp=1;g.extraLives=1;g.hazards={
            {g.player,{0,0},2},{g.player,{0,0},2},{{700,405},{-100,0},3}};
        g.grounds={{g.player,70,3,1,false,ShotKind::Moon}};g.updateHazards(.016f);
        expect(g.screen==Screen::Playing&&g.hp==g.maxHp&&g.extraLives==0,"projectile revival failed");
        expect(g.hazards.empty()&&g.grounds.empty(),"projectile revival left active hazards");
        g.updateHazards(.016f);expect(g.hp==g.maxHp,"cleared projectiles damaged revived player");
    });
    scenario("ground revival safely clears multiple armed areas without invalid iteration",[]{
        Game g=arena();g.hp=1;g.extraLives=1;g.grounds={
            {g.player,70,3,1,false,ShotKind::Moon},{g.player,90,3,1,false,ShotKind::Spore}};
        g.updateHazards(.016f);expect(g.screen==Screen::Playing&&g.hp==g.maxHp&&g.extraLives==0,"ground revival failed");
        expect(g.grounds.empty()&&g.hazards.empty(),"ground revival left hostile areas");
    });
    scenario("all eight enemy types actively accelerate move and complete telegraphed attacks",[]{
        for(int kind=0;kind<int(EnemyKind::PillowBoss);++kind){Game g=arena();g.player={250,550};
            g.enemies={foe({900,300},100,EnemyKind(kind))};Vector2 start=g.enemies[0].pos;
            g.updateEnemies(1.f/60);float firstSpeed=length(g.enemies[0].velocity);
            expect(firstSpeed>0&&firstSpeed<50,"enemy failed to accelerate gradually: "+std::to_string(kind));
            for(int n=1;n<60;++n)g.updateEnemies(1.f/60);
            expect(distance(start,g.enemies[0].pos)>70&&length(g.enemies[0].velocity)>40,"ordinary enemy stood still during roaming: "+std::to_string(kind));
            g.enemies[0].timer=0;
            for(int n=0;n<150;++n)g.updateEnemies(1.f/60);
            expect(kind==0||g.enemies[0].phase>0,"nontrivial enemy never completed a warning/attack cycle");}
    });
    scenario("all enemy types route around a blocking prop without clipping or idle deadlocks",[]{
        for(int kind=0;kind<int(EnemyKind::PillowBoss);++kind){Game g=arena();g.player={955,405};g.invincible=1000;
            g.obstacles={{570,290,120,220}};g.enemies={foe({485,405},100000,EnemyKind(kind))};g.enemies[0].timer=.8f;
            g.wiskyShotCooldown=g.digitalShotCooldown=1000;float traveled=0,idle=0,longestIdle=0,closest=distance(g.player,g.enemies[0].pos);
            for(int n=0;n<900;++n){Vector2 previous=g.enemies[0].pos;g.update(1.f/60,{});const auto& enemy=g.enemies[0];
                float step=distance(previous,enemy.pos);traveled+=step;closest=std::min(closest,distance(g.player,enemy.pos));
                expect(g.walkable(enemy.pos,20),"enemy clipped blocking prop: "+std::to_string(kind));
                if(step<.01f&&enemy.windup<=0&&enemy.cooldown<=0){idle+=1.f/60;longestIdle=std::max(longestIdle,idle);}else idle=0;
            }
            expect(traveled>400&&closest<300,"enemy never routed around prop into engagement range: "+std::to_string(kind));
            expect(longestIdle<.75f,"enemy became stuck outside a warning/charge: "+std::to_string(kind));
        }
    });
    scenario("steering separates overlapping enemies instead of keeping one visual pile",[]{
        Game g=arena();g.player={200,405};g.enemies={foe({800,405}),foe({800,405}),foe({800,405})};
        for(int n=0;n<180;++n)g.updateEnemies(1.f/60);
        for(std::size_t a=0;a<g.enemies.size();++a)for(std::size_t b=a+1;b<g.enemies.size();++b)
            expect(distance(g.enemies[a].pos,g.enemies[b].pos)>20,"overlapping enemies failed to separate");
    });
    scenario("ordinary enemy steering remains consistent across 30 60 and 120 FPS",[]{
        Game a=arena(),b=arena(),c=arena();a.player=b.player=c.player={200,405};
        a.enemies=b.enemies=c.enemies={foe({1000,550})};
        for(int n=0;n<60;++n)a.updateEnemies(1.f/30);
        for(int n=0;n<120;++n)b.updateEnemies(1.f/60);
        for(int n=0;n<240;++n)c.updateEnemies(1.f/120);
        near(distance(a.enemies[0].pos,b.enemies[0].pos),0,8,"30 FPS pursuit diverged");
        near(distance(c.enemies[0].pos,b.enemies[0].pos),0,8,"120 FPS pursuit diverged");
    });
    scenario("poison lethal damage clears the room once and generates collectable rewards",[]{
        Game g;g.newGame(141);g.enterRoom(roomOf(g,RoomKind::Combat));g.enemies={foe({900,300},1)};
        g.damageEnemy(g.enemies[0],0,3,2);int kills=g.defeatedCount;
        for(int n=0;n<60;++n)g.updateEnemies(1.f/60);
        expect(g.enemies.empty()&&g.room().cleared&&g.room().rewardGenerated&&g.defeatedCount==kills+1,"poison kill did not clear/count room");
        auto loot=g.loot.size();g.updateEnemies(.016f);expect(g.loot.size()==loot&&g.defeatedCount==kills+1,"poison kill generated duplicate rewards");
    });
    scenario("a two-minute early-floor fight remains saveable when idle enemy timers expire",[]{
        Temp f;Game g;g.newGame(907);g.enterRoom(roomOf(g,RoomKind::Combat));g.enemies={foe({900,300},100000)};
        g.enemies[0].timer=.8f;g.invincible=1000;frames(g,7201);
        expect(g.screen==Screen::Playing&&!g.enemies.empty(),"long-fight fixture unexpectedly ended");
        expect(saveProfile(g,f.path),"expired idle enemy timer made a live run unsaveable");Game loaded;
        expect(loadProfile(loaded,f.path)&&loaded.enemies.size()==1,"long-fight snapshot failed to reload");
    });
    scenario("retry resets the entire run while retaining preferences",[]{
        Game g=populated();g.screen=Screen::Lost;g.retry();expect(g.stage==0&&g.currentRoom==0&&g.hp==MaxHp&&g.maxHp==MaxHp&&g.coins==0&&g.extraLives==0,"retry inherited resources");
        expect(std::all_of(g.inventory.begin(),g.inventory.end(),[](int n){return n==0;})&&g.runTime==0&&g.defeatedCount==0,"retry inherited progress");
        expect(g.preferences.muted&&g.preferences.easy&&g.preferences.reducedMotion,"retry lost preferences");
    });
    scenario("heart coin item churu pickups apply once and stay collected on reentry",[]{
        Game g=arena();g.hp=2;g.loot={{g.player,PickupKind::Heart,2},{g.player,PickupKind::Coin,5},
            {g.player,PickupKind::Item,1,ItemId::ExtraLife},{g.player,PickupKind::Churu,1}};g.pickups();
        expect(g.hp==4&&g.coins==5&&g.extraLives==1&&g.hasChuru&&g.loot.empty(),"pickup effects incorrect");g.pickups();g.enterRoom(0);
        expect(g.loot.empty()&&g.coins==5&&g.extraLives==1,"pickup respawned/doubled");g.hp=g.maxHp;g.loot={{g.player,PickupKind::Heart,2}};g.pickups();
        expect(g.loot.size()==1,"full health wasted heart");
    });
    scenario("boss portal waits for reward and carries items to the next floor",[]{
        Game g;g.newGame(73);g.enterRoom(roomOf(g,RoomKind::Boss));finish(g);g.player=PortalPosition;g.interact();expect(g.stage==0,"portal skipped reward");
        g.player=RoomCenter;g.interact();expect(g.screen==Screen::Reward&&g.chooseReward(0),"boss reward unavailable");auto inventory=g.inventory;int hp=g.hp;
        g.player=PortalPosition;g.interact();expect(g.stage==1&&g.checkpoint==1&&g.inventory==inventory&&g.hp>=hp,"descent failed/lost items");
    });
    scenario("100 final-room seeds always supply the required churu",[]{
        for(unsigned seed=1;seed<=100;++seed){Game g;g.newGame(seed);g.loadStage(3);g.enterRoom(roomOf(g,RoomKind::Boss));finish(g);
            expect(g.bossDefeated&&std::any_of(g.loot.begin(),g.loot.end(),[](const Pickup&p){return p.kind==PickupKind::Churu;}),"finale has no guaranteed churu");}
    });
    scenario("Maliketh requires churu friendship before Giovanny's rescue",[]{
        Game g;g.newGame(44);g.loadStage(3);g.enterRoom(roomOf(g,RoomKind::Boss));finish(g);g.player=RescuePosition;g.help();
        expect(g.screen==Screen::Playing&&!g.giovannyRescued,"rescue skipped friendship");g.player=MalikethPosition;g.interact();expect(!g.malikethFriendly,"free friendship without churu");
        auto churu=std::find_if(g.loot.begin(),g.loot.end(),[](const Pickup&p){return p.kind==PickupKind::Churu;});expect(churu!=g.loot.end(),"missing churu");
        g.player=churu->pos;g.pickups();expect(g.hasChuru,"churu not collectible");g.player=MalikethPosition;g.help();
        expect(g.malikethFriendly&&!g.hasChuru&&g.screen==Screen::Playing,"feeding failed or rescued too early");g.player=RescuePosition;g.help();
        expect(g.screen==Screen::Won&&g.giovannyRescued&&!g.hasSave,"final rescue failed");float end=g.runTime;frames(g,30,{1,0,true,true});expect(g.runTime==end,"won run still simulated");
    });
    scenario("four-floor journey retains upgrades recovers Wisky and finishes with Maliketh as friend",[]{
        Game g;g.newGame(321);g.grantItem(ItemId::HeartBrooch,false);
        for(int floor=0;floor<FloorCount;++floor){expect(g.stage==floor,"wrong journey floor");g.enterRoom(roomOf(g,RoomKind::Boss));finish(g);
            if(floor<3){g.player=RoomCenter;g.help();expect(g.chooseReward(0),"journey reward failed");g.player=PortalPosition;g.help();}}
        expect(g.wiskyRecovered&&g.inventory[int(ItemId::HeartBrooch)]>=1,"journey lost upgrades/Wisky");g.player=RoomCenter;g.pickups();
        g.player=MalikethPosition;g.help();g.player=RescuePosition;g.help();expect(g.screen==Screen::Won&&g.malikethFriendly&&g.giovannyRescued,"journey cannot finish");
    });
    scenario("V3 disk roundtrip exactly preserves graph combat poison loot items rng and settings",[]{
        Temp f;Game original=populated();std::string before=snapshot(original);expect(saveProfile(original,f.path),"populated save failed");Game loaded;
        expect(loadProfile(loaded,f.path),"populated load failed");expect(snapshot(loaded)==before,"roundtrip changed full gameplay snapshot");
        expect(loaded.currentRoom==original.currentRoom&&loaded.enemies[0].poisonLeft==1.7f&&loaded.shots.size()==2&&loaded.grounds.size()==2,"combat graph/poison/projectiles lost");
        expect(loaded.screen==Screen::Title&&!loaded.mapOpen&&!loaded.inventoryOpen,"load bypassed title/opened overlays");
        expect(loaded.preferences.muted&&loaded.preferences.reducedMotion&&loaded.preferences.easy,"settings lost");near(loaded.preferences.volume,.37f,.00001f,"volume lost");
        expect(original.random()==loaded.random(),"random stream did not resume exactly");expect(!std::filesystem::exists(f.path.string()+".tmp"),"temp file remained");
    });
    scenario("Continue keeps exact room current health poison and projectiles",[]{
        Temp f;Game original=populated();expect(saveProfile(original,f.path),"save failed");Game loaded;expect(loadProfile(loaded,f.path),"load failed");
        std::string graph=layout(loaded);int current=loaded.currentRoom;loaded.resumeCheckpoint();
        expect(loaded.screen==Screen::Playing&&loaded.currentRoom==current&&loaded.hp==3,"Continue regenerated state/healed player");
        expect(layout(loaded)==graph&&loaded.enemies[0].poisonLeft==1.7f&&loaded.shots.size()==2,"Continue reset map/combat");
    });
    scenario("claimed room rewards stay exhausted after save load and reentry",[]{
        Temp f;Game g;g.newGame(36);int treasure=roomOf(g,RoomKind::Treasure);g.enterRoom(treasure);g.offerReward();expect(g.chooseReward(1),"reward setup failed");
        g.enterRoom(0);expect(saveProfile(g,f.path),"save failed");Game loaded;expect(loadProfile(loaded,f.path),"load failed");loaded.resumeCheckpoint();loaded.enterRoom(treasure);loaded.offerReward();
        expect(loaded.room().rewardClaimed&&loaded.screen==Screen::Playing,"disk roundtrip regenerated claimed treasure");
    });
    scenario("friendly final state resumes between feeding Maliketh and rescue",[]{
        Temp f;Game g;g.newGame(56);g.loadStage(3);g.enterRoom(roomOf(g,RoomKind::Boss));finish(g);g.player=RoomCenter;g.pickups();g.player=MalikethPosition;g.help();
        expect(g.malikethFriendly&&saveProfile(g,f.path),"friendly save failed");Game loaded;expect(loadProfile(loaded,f.path),"friendly load failed");
        loaded.resumeCheckpoint();loaded.player=RescuePosition;loaded.help();expect(loaded.screen==Screen::Won&&loaded.giovannyRescued,"friendly save cannot finish");
    });
    scenario("victory and permanent defeat save settings without Continue",[]{
        for(auto screen:{Screen::Won,Screen::Lost}){Temp f;Game g=populated();g.screen=screen;g.hasSave=false;if(screen==Screen::Lost)g.hp=0;
            expect(saveProfile(g,f.path),"end-state save failed");Game loaded;expect(loadProfile(loaded,f.path)&&!loaded.hasSave&&loaded.rooms.empty(),"end-state resurrected run");
            expect(loaded.preferences.easy&&loaded.preferences.reducedMotion,"end-state lost settings");}
    });
    scenario("corrupt primary recovers exact previous valid snapshot",[]{
        Temp f;Game previous=populated();expect(saveProfile(previous,f.path),"first save failed");Game newer=previous;newer.coins+=7;
        expect(saveProfile(newer,f.path),"second save failed");f.write("broken profile\n");Game recovered;
        expect(loadProfile(recovered,f.path)&&snapshot(recovered)==snapshot(previous),"backup failed exact recovery");
    });
    scenario("saving after backup recovery never backs up the corrupt primary",[]{
        Temp f;Game g=populated();expect(saveProfile(g,f.path),"first save failed");g.coins+=1;expect(saveProfile(g,f.path),"second save failed");f.write("corrupt\n");
        Game recovered;expect(loadProfile(recovered,f.path),"recovery failed");std::string old=snapshot(recovered);recovered.coins+=5;
        expect(saveProfile(recovered,f.path),"post-recovery save failed");f.write("corrupt again\n");Game again;
        expect(loadProfile(again,f.path)&&snapshot(again)==old,"corrupt primary destroyed good backup");
    });
    scenario("V2 migration preserves preferences without fabricating a procedural run",[]{
        Temp f;f.write("VIVIS_PROFILE_V2\n2 123.5 0.4 1 0 1 1\n");Game g;expect(loadProfile(g,f.path)&&!g.hasSave&&g.rooms.empty(),"V2 became fake rogue run");
        expect(g.preferences.muted&&g.preferences.reducedMotion&&g.preferences.easy,"V2 flags lost");near(g.preferences.volume,.4f,.00001f,"V2 volume lost");
    });
    scenario("parser rejects invalid graph enum nan counts and truncation without partial mutation",[]{
        Game source=populated();std::string text=snapshot(source);
        // V3 first room record: line 11; columns 4..7 are N/E/S/W neighbor indexes.
        std::vector<std::string> invalid={"unknown schema\n",text.substr(0,text.size()/2),changeToken(text,11,4,"9999"),
            changeToken(text,11,3,"99"),changeToken(text,3,7,"9999"),changeToken(text,4,0,"nan"),changeToken(text,7,0,"inf"),
            changeToken(text,10,0,"10000000"),changeToken(text,1,0,"2.0"),changeToken(text,1,1,"2"),text+"TRAILING\n"};
        for(int d=0;d<4;++d)if(source.rooms[0].neighbors[d]>=0){invalid.push_back(changeToken(text,11,4+d,"-1"));break;}
        for(const auto& bytes:invalid){Temp f;f.write(bytes);Game live=populated();std::string before=snapshot(live);
            expect(!loadProfile(live,f.path),"malformed profile accepted");expect(snapshot(live)==before,"failed load mutated live run");}
    });
    scenario("unsafe snapshots cannot replace the valid disk file",[]{
        Temp f;Game good=populated();expect(saveProfile(good,f.path),"valid save failed");std::string before=f.read();std::vector<Game> unsafe;
        Game bad=good;bad.currentRoom=999;unsafe.push_back(bad);bad=good;bad.hp=bad.maxHp+1;unsafe.push_back(bad);
        bad=good;bad.stats.damage=std::numeric_limits<float>::quiet_NaN();unsafe.push_back(bad);
        bad=good;bad.rooms[0].neighbors[0]=999;unsafe.push_back(bad);bad=good;bad.inventory[0]=-1;unsafe.push_back(bad);
        for(const auto& candidate:unsafe){expect(!saveProfile(candidate,f.path),"unsafe state saved");expect(f.read()==before,"rejected save damaged file");}
    });
    scenario("missing oversized and malformed legacy profiles leave live state unchanged",[]{
        Temp f;Game live=populated();std::string before=snapshot(live);expect(!loadProfile(live,f.path)&&snapshot(live)==before,"missing profile changed run");
        for(const auto& bytes:{std::string(2*1024*1024+1,'x'),std::string("VIVIS_PROFILE_V2\n3 1 .5 0 0 0 0\n"),std::string("VIVIS_PROFILE_V2\n1 0 .5 0 0 0 7\n")}){
            f.write(bytes);expect(!loadProfile(live,f.path)&&snapshot(live)==before,"unsafe legacy/oversized profile accepted");}
    });
    std::cout<<"\n"<<passed<<" passed, "<<failed<<" failed; "<<floorsChecked<<" generated floors and "<<doorCases<<" doorway combinations audited\n";return failed?1:0;
}
