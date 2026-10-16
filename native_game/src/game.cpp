#include "game.hpp"
#include <limits>

namespace vivis {
namespace {
constexpr Color Pink{246,139,185,255}, Blue{97,201,248,255}, Gold{246,214,132,255};
constexpr Vector2 Rescue{1110,295};
float remaining(float value,float dt) { return std::max(0.0f,value-dt); }
float segmentDistance(Vector2 point,Vector2 a,Vector2 b) {
    Vector2 v=sub(b,a); float square=v.x*v.x+v.y*v.y;
    float t=square>0.0001f?std::clamp(((point.x-a.x)*v.x+(point.y-a.y)*v.y)/square,0.0f,1.0f):0;
    return distance(point,add(a,mul(v,t)));
}
bool blocked(Vector2 a,Vector2 b,const std::vector<Rectangle>& obstacles,float margin=3) {
    int samples=std::max(1,int(distance(a,b)/8));
    for(int n=0;n<=samples;++n) {
        Vector2 point=add(a,mul(sub(b,a),float(n)/samples));
        for(auto rect:obstacles) if(inside(point,grow(rect,margin))) return true;
    }
    return false;
}
Vector2 rotate(Vector2 value,float angle) {
    return {value.x*std::cos(angle)-value.y*std::sin(angle),value.x*std::sin(angle)+value.y*std::cos(angle)};
}
template<class T,class Predicate> void discard(std::vector<T>& list,Predicate predicate) {
    list.erase(std::remove_if(list.begin(),list.end(),predicate),list.end());
}
}

const char* zoneName(int index) {
    static const char* names[]={"Pradera de Bigotes","Sendero del Umbral","Torre de Maliketh"};
    return names[std::clamp(index,0,2)];
}
const char* objective(const Game& game) {
    if(game.stage==2) {
        if(game.bossDefeated) return "Acércate a Giovanny y pulsa E para rescatarlo";
        return game.collected?"Derrota a Maliketh · Esquiva su magia lunar":"Recoge el cascabel dorado para romper el escudo";
    }
    if(!game.collected) return "Recoge el cascabel dorado y despeja el camino";
    if(!game.enemies.empty()) return "Cascabel listo · Derrota a las pelusas restantes";
    return "Camino libre · Cruza la puerta de la derecha";
}
void Game::notify(const std::string& text,float duration) { notice=text; noticeTime=duration; }
void Game::requestCheckpoint() { checkpoint=stage; hasSave=true; saveRequested=true; }
void Game::newGame() {
    runTime=0; defeatedCount=0; screen=Screen::Playing;
    loadStage(0); requestCheckpoint();
}
void Game::resumeCheckpoint() {
    screen=Screen::Playing; loadStage(checkpoint);
    notify("Continúas desde el inicio de esta zona · Vida recuperada");
}
void Game::retry() { screen=Screen::Playing; loadStage(stage); }
void Game::loadStage(int index) {
    stage=std::clamp(index,0,2); hp=MaxHp;
    player={145,420}; companion={95,465}; facing={1,0}; moving=false; companionMoving=false;
    companionLeft=false; collected=false; bossDefeated=false; transitionPending=false;
    invincible=1.2f; shootCooldown=helpCooldown=dashCooldown=dashTime=castTime=helpCastTime=hurtTime=pulse=shake=0;
    introTime=2.8f; transitionTime=0.45f; showIntro=true;
    enemies.clear(); shots.clear(); hazards.clear(); particles.clear(); obstacles.clear(); hearts.clear();
    if(stage==0) {
        obstacles={{340,240,130,94},{660,470,136,94}};
        bell={1015,250}; hearts={{570,550}};
        enemies={{{600,280},3},{{940,510},3},{{1040,400},3}};
        notify("Espacio: corazones · E: magia de Wiky · Mayús: esquiva",5.0f);
    } else if(stage==1) {
        obstacles={{330,470,135,90},{655,205,140,100}};
        bell={1030,565}; hearts={{525,260},{895,590}};
        enemies={{{535,400},3},{{830,490},3},{{1030,290},3},{{1100,455},3}};
        notify("El sendero guarda otro cascabel · Tu progreso está guardado",4.0f);
    } else {
        bell={630,555}; hearts={{240,260},{240,590}};
        enemies={{{900,400},38,true,1.8f}};
        notify("Maliketh tiene un escudo · El cascabel lo rompe",4.5f);
    }
}
bool Game::walkable(Vector2 point,float radius) const {
    if(!inside(point,grow(Field,-radius))) return false;
    for(auto rect:obstacles) if(inside(point,grow(rect,radius))) return false;
    return true;
}
Vector2 Game::move(Vector2 origin,Vector2 motion,float radius) const {
    // Short steps prevent tunnelling through ponds and ruins, including a dash.
    int steps=std::max(1,int(std::ceil(length(motion)/7)));
    motion=mul(motion,1.0f/steps);
    for(int i=0;i<steps;++i) {
        Vector2 candidate={origin.x+motion.x,origin.y};
        if(walkable(candidate,radius)) origin.x=candidate.x;
        candidate={origin.x,origin.y+motion.y};
        if(walkable(candidate,radius)) origin.y=candidate.y;
    }
    return origin;
}
void Game::burst(Vector2 pos,Color color,int count) {
    for(int i=0;i<count;++i) {
        float angle=i*6.2831853f/std::max(1,count);
        particles.push_back({pos,{std::cos(angle)*75,std::sin(angle)*75},0.42f,0.42f,color});
    }
}
void Game::shoot() {
    if(screen!=Screen::Playing||shootCooldown>0) return;
    shootCooldown=0.24f; castTime=0.16f;
    Vector2 direction=facing; float nearest=950;
    for(const auto& enemy:enemies) {
        float d=distance(player,enemy.pos);
        if(d<nearest&&!blocked(player,enemy.pos,obstacles)) { nearest=d; direction=unit(sub(enemy.pos,player)); }
    }
    shots.push_back({add(player,mul(direction,24)),mul(direction,650),1.65f});
    cues.push_back(Cue::Shoot);
}
void Game::help() {
    if(screen!=Screen::Playing) return;
    if(stage==2&&bossDefeated&&distance(player,Rescue)<112) {
        screen=Screen::Won; hasSave=false; saveRequested=true; cues.push_back(Cue::Win);
        burst(player,Pink,24); return;
    }
    if(helpCooldown>0) { notify("Wiky recupera energía · Mira la barra azul",1.5f); return; }
    helpCooldown=5; helpCastTime=0.55f; pulse=0.55f; cues.push_back(Cue::Help);
    bool hit=false;
    for(auto& enemy:enemies) if(distance(player,enemy.pos)<230) { damageEnemy(enemy,4); hit=true; }
    removeDefeated(); notify(hit?"¡Wiky libera su magia digital!":"Acércate a una pelusa para alcanzarla con Wiky",2.8f);
}
void Game::dash() {
    if(screen!=Screen::Playing||dashCooldown>0||dashTime>0) return;
    dashCooldown=2; dashTime=0.16f; invincible=std::max(invincible,0.3f);
    burst(player,Blue,8); cues.push_back(Cue::Dash);
}
void Game::damage() {
    if(screen!=Screen::Playing||invincible>0) return;
    hp=std::max(0,hp-1); invincible=preferences.easy?1.8f:1.1f; hurtTime=0.3f;
    shake=preferences.reducedMotion?0:0.18f;
    burst(player,Pink); cues.push_back(Cue::Hit);
    if(hp==0) screen=Screen::Lost;
}
void Game::damageEnemy(Enemy& enemy,int amount) {
    if(enemy.hp<=0) return;
    if(enemy.boss&&!collected) { notify("El cascabel dorado rompe el escudo de Maliketh",2); return; }
    enemy.hp-=amount; enemy.flash=0.18f; burst(enemy.pos,enemy.boss?Color{177,122,231,255}:Pink,8);
}
void Game::removeDefeated() {
    for(const auto& enemy:enemies) if(enemy.hp<=0) {
        ++defeatedCount;
        if(enemy.boss) {
            bossDefeated=true; hazards.clear();
            notify("¡Maliketh se rindió! Acércate a Giovanny y pulsa E",7); cues.push_back(Cue::Win);
        }
    }
    discard(enemies,[](const Enemy& enemy){return enemy.hp<=0;});
}
void Game::updateShots(float dt) {
    for(auto& shot:shots) {
        shot.life-=dt; Vector2 from=shot.pos; shot.pos=add(shot.pos,mul(shot.velocity,dt));
        if(blocked(from,shot.pos,obstacles)) shot.life=0;
        if(shot.life<=0) continue;
        for(auto& enemy:enemies) if(enemy.hp>0&&segmentDistance(enemy.pos,from,shot.pos)<(enemy.boss?43:29)) {
            damageEnemy(enemy,1); shot.life=0; break;
        }
    }
    discard(shots,[](const Projectile& p){return p.life<=0||!inside(p.pos,grow(Field,40));});
    removeDefeated();
}
void Game::updateHazards(float dt) {
    for(auto& shot:hazards) {
        shot.life-=dt; Vector2 from=shot.pos; shot.pos=add(shot.pos,mul(shot.velocity,dt));
        if(shot.life<=0) continue;
        if(segmentDistance(player,from,shot.pos)<23) { damage(); shot.life=0; }
    }
    discard(hazards,[](const Projectile& p){return p.life<=0||!inside(p.pos,grow(Field,40));});
}
void Game::pickups() {
    if(!collected&&distance(player,bell)<40) {
        collected=true; burst(bell,Gold,18); cues.push_back(Cue::Bell);
        notify(stage==2?"¡Escudo roto! Ahora puedes alcanzar a Maliketh":"¡Cascabel encontrado! Despeja la salida",3.5f);
    }
    for(auto iter=hearts.begin();iter!=hearts.end();++iter) if(hp<MaxHp&&distance(player,*iter)<33) {
        hp=std::min(MaxHp,hp+2); burst(*iter,Pink); hearts.erase(iter); cues.push_back(Cue::Heal); break;
    }
}
void Game::update(float dt,const Input& input) {
    if(screen!=Screen::Playing||dt<=0) return;
    dt=std::min(dt,0.05f);
    time+=dt; runTime+=dt;
    invincible=remaining(invincible,dt); shootCooldown=remaining(shootCooldown,dt);
    helpCooldown=remaining(helpCooldown,dt); dashCooldown=remaining(dashCooldown,dt);
    castTime=remaining(castTime,dt); helpCastTime=remaining(helpCastTime,dt); hurtTime=remaining(hurtTime,dt);
    pulse=remaining(pulse,dt); shake=remaining(shake,dt); noticeTime=remaining(noticeTime,dt);
    introTime=remaining(introTime,dt); transitionTime=remaining(transitionTime,dt); showIntro=introTime>0;
    Vector2 direction={input.x,input.y};
    if(length(direction)>1) direction=unit(direction);
    moving=length(direction)>0.05f;
    if(moving) facing=unit(direction);
    if(input.dash) dash();
    bool dashing=dashTime>0;
    Vector2 motion=mul(dashing?facing:direction,(dashing?720.0f:270.0f)*dt);
    player=move(player,motion); dashTime=remaining(dashTime,dt);
    Vector2 goal=add(sub(player,mul(facing,62)),{0,24});
    Vector2 follow=sub(goal,companion); companionMoving=length(follow)>5;
    if(std::fabs(follow.x)>2) companionLeft=follow.x<0;
    companion=add(companion,mul(unit(follow),std::min(length(follow),310*dt)));
    if(input.shoot) shoot();
    if(input.help) help();
    if(screen!=Screen::Playing) return;
    for(auto& enemy:enemies) {
        enemy.flash=remaining(enemy.flash,dt);
        if(enemy.boss) {
            enemy.pos={900+std::sin(time*0.6f)*100,400+std::sin(time*1.05f)*130};
            enemy.timer-=dt;
            if(enemy.timer<=0) {
                enemy.phase=enemy.hp<19?1:0;
                enemy.timer=(enemy.phase?1.2f:1.65f)*(preferences.easy?1.25f:1.0f);
                Vector2 aim=unit(sub(player,enemy.pos));
                int count=enemy.phase?5:3;
                float speed=preferences.easy?175:215;
                for(int n=0;n<count;++n) {
                    Vector2 directionValue=rotate(aim,(n-(count-1)/2.0f)*0.24f);
                    hazards.push_back({enemy.pos,mul(directionValue,speed),6});
                }
            }
        } else {
            Vector2 dir=unit(sub(player,enemy.pos));
            float speed=(stage?99:82)*(preferences.easy?0.7f:1.0f);
            Vector2 next=move(enemy.pos,mul(dir,speed*dt),20);
            if(distance(next,enemy.pos)<speed*dt*0.2f) {
                Vector2 side={-dir.y,dir.x};
                Vector2 left=move(enemy.pos,mul(side,speed*dt),20);
                Vector2 right=move(enemy.pos,mul(side,-speed*dt),20);
                next=distance(left,player)<distance(right,player)?left:right;
            }
            enemy.pos=next;
        }
        if(distance(player,enemy.pos)<(enemy.boss?43:34)) damage();
    }
    if(screen!=Screen::Playing) return;
    updateShots(dt); updateHazards(dt);
    if(screen!=Screen::Playing) return;
    pickups();
    for(auto& particle:particles) { particle.life-=dt; particle.pos=add(particle.pos,mul(particle.velocity,dt)); }
    discard(particles,[](const Particle& p){return p.life<=0;});
    if(gateOpen()&&inside(player,{1150,350,66,120})) {
        loadStage(stage+1); requestCheckpoint(); cues.push_back(Cue::Transition);
    }
}
}
