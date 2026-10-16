#include "game.hpp"
#include <chrono>
#include <limits>
#include <queue>

namespace vivis {
namespace {
constexpr Color Pink{246,139,185,255}, Green{161,220,110,255}, Gold{246,214,132,255};
constexpr int dx[]={0,1,0,-1},dy[]={-1,0,1,0};
float remaining(float v,float dt) { return std::max(0.0f,v-dt); }
Vector2 rotate(Vector2 v,float a) { return {v.x*std::cos(a)-v.y*std::sin(a),v.x*std::sin(a)+v.y*std::cos(a)}; }
bool blocked(Vector2 a,Vector2 b,const std::vector<Rectangle>& obstacles) {
    int samples=std::max(1,int(distance(a,b)/8));
    for(int n=0;n<=samples;++n) for(auto r:obstacles)
        if(inside(add(a,mul(sub(b,a),float(n)/samples)),grow(r,4))) return true;
    return false;
}
template<class T,class P>void discard(std::vector<T>& v,P p) { v.erase(std::remove_if(v.begin(),v.end(),p),v.end()); }
}
const char* zoneName(int index) {
    static const char* names[]={"Pachonidad","Bigotes Encontrados","El alter ego de Wiky","Refugio de Maliketh"};
    return names[std::clamp(index,0,FloorCount-1)];
}
const char* roomName(RoomKind k) {
    static const char* names[]={"Entrada","Encuentro","Tesoro","Tienda","Descanso","Desafío","Guardián"};
    return names[std::clamp(int(k),0,6)];
}
const char* enemyName(EnemyKind k) {
    static const char* names[]={"Pelusita","Pompón saltarín","Bigote arquero","Gatito veloz","Polilla lunar","Esporita","Eco digital","Ovillo rodante","Rey Almohadón","Los Bigotes Encontrados","Wiky Wikerman · El asesino de wiskys","Maliketh la Pachona"};
    return names[std::clamp(int(k),0,11)];
}
const ItemInfo& itemInfo(ItemId id) {
    static const ItemInfo data[]={
        {"Broche de corazón","Tus corazones hacen más daño.",Pink,false},
        {"Botitas suaves","Muévete más rápido entre los peligros.",{163,206,245,255},false},
        {"Listón veloz","Dispara corazones más seguido.",Pink,false},
        {"Corazones gemelos","Un corazón adicional en cada disparo (máx. 3).",Pink,false},
        {"Chaleco pachón","Dos puntos de vida máxima y dos de curación.",{245,219,171,255},false},
        {"Lechita","Recupera tres puntos de vida.",{238,232,213,255},false},
        {"Una vida más","Revive una vez con toda tu vida.",Gold,false},
        {"Colmillo ponzoñoso","El veneno de Wisky hace más daño.",Green,true},
        {"Cascabel de Wisky","Wisky lanza veneno más seguido.",Green,true},
        {"Hierba gatuna","La ayuda venenosa alcanza más y recarga antes.",Green,true},
        {"Nube ponzoñosa","La ayuda de Wisky deja una nube de veneno.",Green,true},
        {"Chip de Giovanny","Una proyección digital te ayuda a disparar.",{111,215,249,255},false},
        {"Bigote de la suerte","Encuentra más monedas al limpiar habitaciones.",Gold,false},
        {"Bufanda protectora","Absorbe un golpe al entrar en cada habitación.",{190,158,238,255},false}
    };
    return data[std::clamp(int(id),0,ItemCount-1)];
}
const char* objective(const Game& g) {
    if(g.rooms.empty()) return "Una pequeña aventura te espera";
    if(!g.enemies.empty()) return g.room().kind==RoomKind::Boss?"Esquiva las señales y calma al guardián":"Despeja la habitación para abrir las puertas";
    if(g.room().kind==RoomKind::Boss) {
        if(g.stage==3) {
            if(!g.hasChuru&&!g.malikethFriendly) return "Recoge el churu que dejó Maliketh";
            if(!g.malikethFriendly) return "Acércate a Maliketh y dale el churu con E";
            return "Maliketh es tu amiga · Acércate a Giovanny y pulsa E";
        }
        return !g.room().rewardClaimed?"Elige tu premio en el pedestal · E":"Baja al siguiente piso por la escalera · E";
    }
    if(g.room().kind==RoomKind::Treasure&&!g.room().rewardClaimed) return "Un regalo para tu partida · Acércate al pedestal y pulsa E";
    if(g.room().kind==RoomKind::Shop&&!g.room().rewardClaimed) return "Intercambia monedas por una mejora · Pedestal: E";
    if(g.room().kind==RoomKind::Sanctuary&&!g.room().rewardClaimed) return "Descansa aquí: E para recuperar toda tu vida";
    return "Explora las puertas abiertas · TAB: mapa · I: objetos";
}
std::uint32_t Game::random() { rngState^=rngState<<13; rngState^=rngState>>17; rngState^=rngState<<5; if(!rngState)rngState=1; return rngState; }
int Game::randomInt(int upper) { return upper>0?int(random()%std::uint32_t(upper)):0; }
void Game::notify(const std::string& text,float duration) { notice=text; noticeTime=duration; }
void Game::requestCheckpoint() { checkpoint=stage; hasSave=true; saveRequested=true; }
void Game::newGame(std::uint32_t seed) {
    Preferences p=preferences;
    *this=Game{}; preferences=p;
    runSeed=seed?seed:std::uint32_t(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    if(!runSeed)runSeed=1;
    rngState=runSeed;
    screen=Screen::Playing; loadStage(0); requestCheckpoint();
    notify("WASD: moverte · Flechas: disparar · E: Wisky · TAB: mapa",6);
}
void Game::resumeCheckpoint() {
    if(!hasSave||rooms.empty()) { newGame(); return; }
    screen=Screen::Playing; mapOpen=inventoryOpen=false; invincible=std::max(invincible,1.0f);
    notify("Tu mapa, objetos y vida siguen aquí · ¡Vamos juntos!");
}
void Game::retry() { newGame(); }
void Game::loadStage(int index) {
    stage=std::clamp(index,0,FloorCount-1); checkpoint=stage;
    generateFloor(stage); enterRoom(0);
    introTime=2.6f; showIntro=true;
    notify(std::string("Piso ")+std::to_string(stage+1)+" · "+zoneName(stage));
}
void Game::generateFloor(int index) {
    rooms.clear(); stage=std::clamp(index,0,FloorCount-1);
    if(!rngState)rngState=runSeed?runSeed:1;
    rooms.push_back(Room{}); rooms[0].kind=RoomKind::Start;
    const int count=8+stage;
    while(int(rooms.size())<count) {
        int parent=randomInt(int(rooms.size())),dir=randomInt(4);
        int x=rooms[parent].x+dx[dir],y=rooms[parent].y+dy[dir];
        if(std::abs(x)>3||std::abs(y)>3)continue;
        bool exists=false; for(const auto&r:rooms)if(r.x==x&&r.y==y)exists=true;
        if(exists)continue;
        Room r;r.x=x;r.y=y;rooms.push_back(r);
    }
    // Link every touching room, including loops. Every node grew from an existing node.
    for(int i=0;i<count;++i)for(int j=0;j<count;++j)for(int d=0;d<4;++d)
        if(rooms[j].x==rooms[i].x+dx[d]&&rooms[j].y==rooms[i].y+dy[d])rooms[i].neighbors[d]=j;
    std::vector<int> depths(count,-1);std::queue<int> q; q.push(0);depths[0]=0;
    while(!q.empty()) { int n=q.front();q.pop();for(int v:rooms[n].neighbors)if(v>=0&&depths[v]<0){depths[v]=depths[n]+1;q.push(v);} }
    int boss=1;for(int i=2;i<count;++i)if(depths[i]>depths[boss])boss=i;
    rooms[boss].kind=RoomKind::Boss;
    std::vector<int> slots;for(int i=1;i<count;++i)if(i!=boss)slots.push_back(i);
    for(int i=int(slots.size())-1;i>0;--i)std::swap(slots[i],slots[randomInt(i+1)]);
    rooms[slots[0]].kind=RoomKind::Treasure;rooms[slots[1]].kind=RoomKind::Shop;
    rooms[slots[2]].kind=RoomKind::Sanctuary;rooms[slots[3]].kind=RoomKind::Challenge;
    constexpr Vector2 spots[]={{315,290},{640,315},{965,290},{315,530},{640,535},{965,530},{850,410},{430,410}};
    for(auto& r:rooms) {
        r.seed=random();r.layout=randomInt(5);
        for(int i=0;i<3;++i) {
            ItemId choice; do{choice=ItemId(randomInt(ItemCount));}while((i>0&&choice==r.offers[0])||(i>1&&choice==r.offers[1]));
            r.offers[i]=choice;
        }
        bool combat=r.kind==RoomKind::Combat||r.kind==RoomKind::Challenge;
        if(combat) {
            if(r.layout==1)r.obstacles={{400,285,90,80},{790,460,90,80}};
            else if(r.layout==2)r.obstacles={{400,460,90,80},{790,285,90,80}};
            else if(r.layout==3)r.obstacles={{480,305,75,70},{725,450,75,70}};
            else if(r.layout==4)r.obstacles={{290,365,95,75},{895,365,95,75}};
            int foes=3+stage+(r.kind==RoomKind::Challenge?1:0);
            for(int n=0;n<foes;++n) {
                Enemy e; e.pos=spots[(n+int(r.seed%8))%8];
                bool unsafe=true;int attempt=0;
                while(unsafe&&attempt++<40) {
                    unsafe=false;for(auto ob:r.obstacles)if(inside(e.pos,grow(ob,36)))unsafe=true;
                    if(unsafe)e.pos={float(230+randomInt(820)),float(260+randomInt(300))};
                }
                static const EnemyKind pool[4][4]={
                    {EnemyKind::Fluff,EnemyKind::Hopper,EnemyKind::Charger,EnemyKind::Spore},
                    {EnemyKind::WhiskerShooter,EnemyKind::Pouncer,EnemyKind::Hopper,EnemyKind::Fluff},
                    {EnemyKind::Mirror,EnemyKind::Spore,EnemyKind::Charger,EnemyKind::WhiskerShooter},
                    {EnemyKind::Moth,EnemyKind::Pouncer,EnemyKind::Spore,EnemyKind::Mirror}};
                e.kind=pool[stage][randomInt(4)]; e.hp=e.maxHp=3.5f+stage*.8f+(r.kind==RoomKind::Challenge?1:0);
                e.timer=.8f+randomInt(100)*.014f; r.enemies.push_back(e);
            }
        } else if(r.kind==RoomKind::Boss) {
            Enemy e;e.pos={850,395};e.boss=true;e.kind=EnemyKind(int(EnemyKind::PillowBoss)+stage);
            e.hp=e.maxHp=38+stage*15;e.timer=1.8f;r.enemies.push_back(e);
        } else r.cleared=true;
    }
}
void Game::storeRoom() {
    if(currentRoom<0||currentRoom>=int(rooms.size()))return;
    auto&r=room();r.enemies=enemies;r.pickups=loot;r.obstacles=obstacles;
}
void Game::enterRoom(int index,int fromDirection) {
    if(index<0||index>=int(rooms.size()))return;
    currentRoom=index;auto&r=room();r.visited=true;
    enemies=r.enemies;loot=r.pickups;obstacles=r.obstacles;
    shots.clear();hazards.clear();grounds.clear();hearts.clear();particles.clear();
    bossDefeated=r.kind==RoomKind::Boss&&r.cleared;
    wiskyCorrupted=stage==2&&r.kind==RoomKind::Boss&&!r.cleared;
    roomShield=stats.shields;
    player={640,555};
    if(fromDirection==0)player={640,605};else if(fromDirection==1)player={115,405};
    else if(fromDirection==2)player={640,207};else if(fromDirection==3)player={1165,405};
    if(fromDirection>=0&&fromDirection<4)facing={float(dx[fromDirection]),float(dy[fromDirection])};
    companion={std::clamp(player.x-48,90.0f,1190.0f),std::clamp(player.y+24,185.0f,635.0f)};
    digitalCompanion={std::clamp(player.x+50,90.0f,1190.0f),std::clamp(player.y-5,185.0f,635.0f)};
    moving=companionMoving=false;doorCooldown=.45f;invincible=1.2f;transitionTime=.32f;
    shootCooldown=castTime=helpCastTime=hurtTime=dashTime=0;mapOpen=inventoryOpen=false;
    if(wiskyCorrupted)notify("Wiky Wikerman tomó el control · Derrótalo para recuperar a Wisky",6);
    else notify(std::string(roomName(r.kind))+" · "+zoneName(stage),2.5f);
}
bool Game::travel(int direction) {
    if(direction<0||direction>3||rooms.empty()||!doorsOpen())return false;
    int next=room().neighbors[direction];if(next<0)return false;
    storeRoom();enterRoom(next,direction);requestCheckpoint();cues.push_back(Cue::Transition);return true;
}
int Game::clearedRooms() const { int n=0;for(const auto&r:rooms)if(r.visited&&r.cleared)++n;return n; }
bool Game::walkable(Vector2 p,float radius) const {
    if(!inside(p,grow(Field,-radius)))return false;
    for(auto r:obstacles)if(inside(p,grow(r,radius)))return false;
    return true;
}
Vector2 Game::move(Vector2 p,Vector2 motion,float radius) const {
    int steps=std::max(1,int(std::ceil(length(motion)/7)));motion=mul(motion,1.0f/steps);
    for(int n=0;n<steps;++n) {
        Vector2 v{p.x+motion.x,p.y};if(walkable(v,radius))p.x=v.x;
        v={p.x,p.y+motion.y};if(walkable(v,radius))p.y=v.y;
    }return p;
}
void Game::burst(Vector2 p,Color color,int count) {
    for(int n=0;n<count;++n){float a=n*6.2831853f/std::max(1,count);particles.push_back({p,{std::cos(a)*75,std::sin(a)*75},.42f,.42f,color});}
    if(particles.size()>600)particles.erase(particles.begin(),particles.begin()+int(particles.size()-600));
}
void Game::shoot() {
    if(screen!=Screen::Playing||shootCooldown>0||mapOpen||inventoryOpen)return;
    shootCooldown=stats.fireInterval;castTime=.16f;
    Vector2 direction=aim;
    if(length(direction)<.1f) {
        direction=facing;float nearest=1000;
        for(const auto&e:enemies)if(e.hp>0&&distance(player,e.pos)<nearest&&!blocked(player,e.pos,obstacles)) {
            nearest=distance(player,e.pos);direction=unit(sub(e.pos,player));
        }
    }
    for(int n=0;n<stats.multishot;++n) {
        Vector2 d=rotate(direction,(n-(stats.multishot-1)*.5f)*.18f);
        Projectile p;p.pos=add(player,mul(d,24));p.velocity=mul(d,stats.shotSpeed);p.damage=stats.damage;p.life=stats.range/stats.shotSpeed;
        shots.push_back(p);
    }cues.push_back(Cue::Shoot);
}
void Game::interact() {
    if(rooms.empty()||!enemies.empty())return;
    auto&r=room();
    if(r.kind==RoomKind::Boss&&stage==3&&bossDefeated) {
        if(!malikethFriendly&&hasChuru&&distance(player,MalikethPosition)<135) {
            hasChuru=false;malikethFriendly=true;requestCheckpoint();cues.push_back(Cue::Churu);
            burst(MalikethPosition,Pink,28);notify("¡Churu para Maliketh! Ahora es tu amiga · Giovanny te espera",7);return;
        }
        if(malikethFriendly&&distance(player,RescuePosition)<115) {
            giovannyRescued=true;screen=Screen::Won;hasSave=false;saveRequested=true;cues.push_back(Cue::Win);burst(player,Pink,30);return;
        }
    }
    if((r.kind==RoomKind::Treasure||r.kind==RoomKind::Shop||r.kind==RoomKind::Boss)&&!r.rewardClaimed&&distance(player,RoomCenter)<105) {
        offerReward(r.kind==RoomKind::Shop);return;
    }
    if(r.kind==RoomKind::Sanctuary&&!r.rewardClaimed&&distance(player,RoomCenter)<110) {
        hp=maxHp;r.rewardClaimed=true;roomShield=std::max(roomShield,1);cues.push_back(Cue::Heal);requestCheckpoint();notify("Un descanso mullidito · Toda tu vida y un escudo para esta sala");return;
    }
    if(r.kind==RoomKind::Boss&&bossDefeated&&stage<3&&r.rewardClaimed&&distance(player,PortalPosition)<100) {
        loadStage(stage+1);hp=std::min(maxHp,hp+1);requestCheckpoint();cues.push_back(Cue::Transition);return;
    }
}
void Game::help() {
    if(screen!=Screen::Playing||mapOpen||inventoryOpen)return;
    Screen before=screen;int floor=stage;bool friendly=malikethFriendly;bool claimed=rooms.empty()?false:room().rewardClaimed;
    interact();
    if(screen!=before||floor!=stage||friendly!=malikethFriendly||(!rooms.empty()&&claimed!=room().rewardClaimed))return;
    if(wiskyCorrupted){notify("Wisky está bajo el alter ego · Tus corazones pueden recuperarlo",2.4f);return;}
    if(helpCooldown>0){notify("Wisky recarga su veneno · Mira la barra verde",1.5f);return;}
    helpCooldown=stats.helpCooldown;helpCastTime=pulse=.55f;cues.push_back(Cue::Help);
    for(auto&e:enemies)if(distance(player,e.pos)<stats.helpRadius)damageEnemy(e,stats.wiskyDamage*2,stats.poisonDps*1.5f,stats.poisonDuration);
    if(stats.toxicCloud)grounds.push_back({player,stats.helpRadius*.65f,4,0,true,ShotKind::Venom});
    burst(player,Green,22);removeDefeated();notify("¡Wisky libera su veneno!",1.7f);
}
void Game::dash() {
    if(screen!=Screen::Playing||dashCooldown>0||dashTime>0)return;
    dashCooldown=1.6f;dashTime=.16f;invincible=std::max(invincible,.25f);burst(player,Pink,8);cues.push_back(Cue::Dash);
}
void Game::damage() {
    if(screen!=Screen::Playing||invincible>0)return;
    if(roomShield>0){--roomShield;invincible=1;burst(player,Gold,16);notify("La bufanda absorbió el golpe",1.8f);cues.push_back(Cue::Bell);return;}
    hp=std::max(0,hp-1);invincible=preferences.easy?1.8f:1.1f;hurtTime=.3f;shake=preferences.reducedMotion?0:.18f;
    burst(player,Pink);cues.push_back(Cue::Hit);
    if(!hp) {
        if(extraLives>0){--extraLives;hp=maxHp;invincible=3;hazards.clear();grounds.clear();player={640,580};cues.push_back(Cue::Revive);notify("¡Una vida más! Tus objetos siguen contigo",5);requestCheckpoint();}
        else {screen=Screen::Lost;hasSave=false;saveRequested=true;}
    }
}
void Game::damageEnemy(Enemy&e,float amount,float poisonDamage,float poisonDuration) {
    if(e.hp<=0)return;
    e.hp-=std::max(0.0f,amount);
    if(amount>=.4f){e.flash=.13f;burst(e.pos,Pink,4);}
    if(poisonDuration>0){e.poisonLeft=std::max(e.poisonLeft,poisonDuration);e.poisonDps=std::max(e.poisonDps,poisonDamage);}
}
void Game::removeDefeated() {
    bool boss=false;
    for(const auto&e:enemies)if(e.hp<=0){++defeatedCount;if(e.boss)boss=true;else if(randomInt(5)==0)loot.push_back({e.pos,PickupKind::Coin,1});}
    discard(enemies,[](const Enemy&e){return e.hp<=0;});
    if(boss){bossDefeated=true;hazards.clear();grounds.clear();cues.push_back(Cue::Win);if(stage==2){wiskyCorrupted=false;wiskyRecovered=true;stats.wiskyDamage+=.5f;notify("¡Wisky volvió! Su veneno es más fuerte y el alter ego se ha calmado",6);}}
    if(enemies.empty()&&!rooms.empty()&&!room().cleared)clearRoom();
}
void Game::clearRoom() {
    if(rooms.empty()||room().cleared)return;
    auto&r=room();r.cleared=true;
    if(!r.rewardGenerated){
        r.rewardGenerated=true;
        int amount=2+randomInt(3)+stats.luck+(r.kind==RoomKind::Challenge?3:0);
        loot.push_back({{585,405},PickupKind::Coin,amount});
        if(randomInt(3)==0||hp<=2)loot.push_back({{690,405},PickupKind::Heart,2});
        if(r.kind==RoomKind::Challenge)loot.push_back({RoomCenter,PickupKind::Item,1,ItemId(randomInt(ItemCount))});
        if(r.kind==RoomKind::Boss&&stage==3){loot.push_back({RoomCenter,PickupKind::Churu,1});r.rewardClaimed=true;notify("Maliketh está cansada · Recoge el churu y ofréceselo con E",7);}
        else if(r.kind==RoomKind::Boss&&stage!=2)notify("Guardián calmado · Elige un premio y sigue bajando",5);
        else if(r.kind!=RoomKind::Boss)notify("¡Habitación despejada! Puertas abiertas",2);
    }
    storeRoom();requestCheckpoint();
}
void Game::pickups() {
    bool changed=false;
    for(auto it=loot.begin();it!=loot.end();) {
        if(distance(player,it->pos)>35||(it->kind==PickupKind::Heart&&hp>=maxHp)){++it;continue;}
        auto p=*it;it=loot.erase(it);changed=true;
        if(p.kind==PickupKind::Heart){hp=std::min(maxHp,hp+p.amount);cues.push_back(Cue::Heal);burst(p.pos,Pink);}
        else if(p.kind==PickupKind::Coin){coins+=p.amount;cues.push_back(Cue::Coin);burst(p.pos,Gold);}
        else if(p.kind==PickupKind::Item)grantItem(p.item);
        else{hasChuru=true;cues.push_back(Cue::Churu);notify("Churu conseguido · Acércate a Maliketh y pulsa E",6);burst(p.pos,Pink);}
    }
    if(changed){storeRoom();requestCheckpoint();}
}
void Game::offerReward(bool shop) {
    if(rooms.empty()||room().rewardClaimed||!enemies.empty())return;
    rewardIsShop=shop;rewardChoices=room().offers;
    for(int i=0;i<3;++i)rewardCosts[i]=shop?(rewardChoices[i]==ItemId::ExtraLife?10:rewardChoices[i]==ItemId::Milk?3:6):0;
    screen=Screen::Reward;rewardTime=1;cues.push_back(Cue::Bell);
}
bool Game::chooseReward(int index) {
    if(screen!=Screen::Reward||index<0||index>2||rooms.empty()||room().rewardClaimed||coins<rewardCosts[index])return false;
    coins-=rewardCosts[index];grantItem(rewardChoices[index]);room().rewardClaimed=true;
    screen=Screen::Playing;rewardTime=0;storeRoom();requestCheckpoint();return true;
}
void Game::grantItem(ItemId item,bool feedback) {
    int n=int(item);if(n<0||n>=ItemCount)return;++inventory[n];
    switch(item) {
        case ItemId::HeartBrooch:stats.damage=std::min(12.0f,stats.damage+.35f);break;
        case ItemId::SoftBoots:stats.speed=std::min(420.0f,stats.speed+25);break;
        case ItemId::QuickRibbon:stats.fireInterval=std::max(.11f,stats.fireInterval*.85f);break;
        case ItemId::TwinHearts:stats.multishot=std::min(3,stats.multishot+1);break;
        case ItemId::PlushVest:maxHp=std::min(12,maxHp+2);hp=std::min(maxHp,hp+2);break;
        case ItemId::Milk:hp=std::min(maxHp,hp+3);break;
        case ItemId::ExtraLife:++extraLives;break;
        case ItemId::VenomFang:stats.poisonDps=std::min(12.0f,stats.poisonDps+1);break;
        case ItemId::WiskyBell:stats.wiskyInterval=std::max(.28f,stats.wiskyInterval*.8f);break;
        case ItemId::Catnip:stats.helpRadius=std::min(440.0f,stats.helpRadius+50);stats.helpCooldown=std::max(1.8f,stats.helpCooldown*.8f);break;
        case ItemId::ToxicCloud:stats.toxicCloud=true;break;
        case ItemId::DigitalChip:stats.digitalBuddy=true;break;
        case ItemId::LuckyWhisker:++stats.luck;break;
        case ItemId::ShieldScarf:stats.shields=std::min(2,stats.shields+1);roomShield=stats.shields;break;
        default:break;
    }
    if(feedback){notify(std::string(itemInfo(item).name)+" · "+itemInfo(item).description,5);cues.push_back(Cue::Item);burst(player,itemInfo(item).color,18);}
}
void Game::update(float dt,const Input& input) {
    if(screen!=Screen::Playing||dt<=0||mapOpen||inventoryOpen)return;
    dt=std::min(dt,.05f);time+=dt;runTime+=dt;
    for(float* v:{&invincible,&shootCooldown,&helpCooldown,&dashCooldown,&castTime,&helpCastTime,&hurtTime,&pulse,&shake,&noticeTime,&introTime,&transitionTime,&doorCooldown})*v=remaining(*v,dt);
    showIntro=introTime>0;
    Vector2 direction{input.x,input.y};if(length(direction)>1)direction=unit(direction);
    moving=length(direction)>.05f;if(moving)facing=unit(direction);
    aim={input.aimX,input.aimY};if(length(aim)>.15f)aim=unit(aim);else aim={0,0};
    if(input.dash)dash();
    player=move(player,mul(dashTime>0?facing:direction,(dashTime>0?720:stats.speed)*dt));dashTime=remaining(dashTime,dt);
    if(input.shoot||length(aim)>.15f)shoot();
    if(input.help)help();
    if(screen!=Screen::Playing)return;
    updateWisky(dt);updateEnemies(dt);
    if(screen!=Screen::Playing)return;
    updateShots(dt);updateHazards(dt);
    if(screen!=Screen::Playing)return;
    pickups();
    for(auto&p:particles){p.life-=dt;p.pos=add(p.pos,mul(p.velocity,dt));}
    discard(particles,[](const Particle&p){return p.life<=0;});
    if(doorsOpen()&&doorCooldown<=0) {
        if(direction.y<0&&player.y<=Field.y+19&&std::abs(player.x-640)<48)travel(0);
        else if(direction.x>0&&player.x>=Field.x+Field.width-19&&std::abs(player.y-405)<48)travel(1);
        else if(direction.y>0&&player.y>=Field.y+Field.height-19&&std::abs(player.x-640)<48)travel(2);
        else if(direction.x<0&&player.x<=Field.x+19&&std::abs(player.y-405)<48)travel(3);
    }
}
}
