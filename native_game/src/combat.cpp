#include "game.hpp"
#include <limits>

namespace vivis {
namespace {
constexpr float Pi=3.14159265358979323846f;
Vector2 rotate(Vector2 p,float angle) {
    return {p.x*std::cos(angle)-p.y*std::sin(angle),p.x*std::sin(angle)+p.y*std::cos(angle)};
}
float enemyRadius(const Enemy& enemy) { return enemy.boss?29.0f:20.0f; }
Vector2 limitTarget(Vector2 p,float margin=45) {
    return {std::clamp(p.x,Field.x+margin,Field.x+Field.width-margin),
        std::clamp(p.y,Field.y+margin,Field.y+Field.height-margin)};
}
bool blocked(const Game& game,Vector2 p,float radius) {
    if(!inside(p,grow(Field,-radius))) return true;
    for(const auto& rect:game.obstacles) if(inside(p,grow(rect,radius))) return true;
    return false;
}
void bullet(Game& game,Vector2 origin,Vector2 direction,float speed,ShotKind kind,float radius=7) {
    direction=unit(direction);
    if(length(direction)<0.1f) direction={0,1};
    Projectile p;
    p.pos=origin; p.velocity=mul(direction,speed); p.life=7; p.kind=kind; p.radius=radius;
    game.hazards.push_back(p);
}
void fan(Game& game,Vector2 origin,Vector2 direction,int count,float spread,float speed,ShotKind kind,float radius=7) {
    direction=unit(direction);
    for(int n=0;n<count;++n) bullet(game,origin,rotate(direction,(n-(count-1)*0.5f)*spread),speed,kind,radius);
}
void ring(Game& game,Vector2 origin,int count,float speed,ShotKind kind,float offset=0,int skip=-1,float radius=7) {
    for(int n=0;n<count;++n) if(n!=skip) {
        float a=offset+n*2*Pi/count;
        bullet(game,origin,{std::cos(a),std::sin(a)},speed,kind,radius);
    }
}
bool segmentHits(Vector2 a,Vector2 b,Rectangle rect) {
    const Vector2 motion=sub(b,a);
    float enter=0,leave=1;
    for(int axis=0;axis<2;++axis) {
        const float origin=axis?a.y:a.x,delta=axis?motion.y:motion.x;
        const float low=axis?rect.y:rect.x,high=low+(axis?rect.height:rect.width);
        if(std::abs(delta)<0.0001f) {if(origin<low||origin>high)return false;}
        else {
            float first=(low-origin)/delta,last=(high-origin)/delta;
            if(first>last)std::swap(first,last);
            enter=std::max(enter,first);leave=std::min(leave,last);
            if(enter>leave)return false;
        }
    }
    return true;
}
bool clearPath(const Game& game,Vector2 a,Vector2 b,float radius) {
    for(const auto& rect:game.obstacles)if(segmentHits(a,b,grow(rect,std::max(0.0f,radius-0.1f))))return false;
    return true;
}
Vector2 safeGoal(const Game& game,Vector2 goal,float radius) {
    goal=limitTarget(goal,radius+12);
    // Orbiting targets can fall inside a pillow or pillar. Push the target to
    // its closest free side before constructing a route around that obstacle.
    for(std::size_t pass=0;pass<=game.obstacles.size();++pass)for(auto rect:game.obstacles) {
        Rectangle padded=grow(rect,radius+10);
        if(!inside(goal,padded))continue;
        const float offsets[]={goal.x-padded.x,padded.x+padded.width-goal.x,goal.y-padded.y,padded.y+padded.height-goal.y};
        int side=int(std::min_element(offsets,offsets+4)-offsets);
        if(side==0)goal.x=padded.x-1;else if(side==1)goal.x=padded.x+padded.width+1;
        else if(side==2)goal.y=padded.y-1;else goal.y=padded.y+padded.height+1;
        goal=limitTarget(goal,radius+12);
    }
    return goal;
}
Vector2 routeDirection(const Game& game,const Enemy& enemy,Vector2 goal) {
    const float radius=enemyRadius(enemy);
    goal=safeGoal(game,goal,radius);
    if(clearPath(game,enemy.pos,goal,radius))return unit(sub(goal,enemy.pos));
    // A tiny visibility graph gives the creature a real route around props,
    // including two-corner paths that simple wall sliding cannot resolve.
    std::vector<Vector2> nodes{enemy.pos,goal};
    for(auto rect:game.obstacles) {
        Rectangle padded=grow(rect,radius+10);
        const Vector2 corners[]={{padded.x,padded.y},{padded.x+padded.width,padded.y},
            {padded.x,padded.y+padded.height},{padded.x+padded.width,padded.y+padded.height}};
        for(auto point:corners)if(game.walkable(point,radius))nodes.push_back(point);
    }
    std::vector<float> costs(nodes.size(),std::numeric_limits<float>::infinity());
    std::vector<int> next(nodes.size(),-1);std::vector<bool> visited(nodes.size(),false);
    costs[1]=0;
    for(std::size_t pass=0;pass<nodes.size();++pass) {
        int current=-1;
        for(std::size_t n=0;n<nodes.size();++n)if(!visited[n]&&(current<0||costs[n]<costs[current]))current=int(n);
        if(current<0||!std::isfinite(costs[current])||current==0)break;
        visited[current]=true;
        for(std::size_t n=0;n<nodes.size();++n)if(!visited[n]&&clearPath(game,nodes[current],nodes[n],radius)) {
            float candidate=costs[current]+distance(nodes[current],nodes[n]);
            if(candidate<costs[n]){costs[n]=candidate;next[n]=current;}
        }
    }
    return unit(sub(next[0]>=0?nodes[next[0]]:goal,enemy.pos));
}
void stepEnemy(Game& game,Enemy& enemy,Vector2 motion) {
    Vector2 next=game.move(enemy.pos,motion,enemyRadius(enemy));
    if(enemy.cooldown>0&&distance(next,enemy.pos)<length(motion)*0.2f) {
        // A committed charge stops at a prop; it never turns after its warning.
        enemy.cooldown=0;enemy.velocity={};
    }
    enemy.pos=next;
}
void steerEnemy(Game& game,Enemy& enemy,Vector2 goal,float speed,float dt) {
    Vector2 desired=mul(routeDirection(game,enemy,goal),speed);
    const float separation=enemy.boss?100.0f:62.0f;
    for(const auto& other:game.enemies) {
        if(&other==&enemy||other.hp<=0)continue;
        Vector2 away=sub(enemy.pos,other.pos);float gap=length(away);
        if(gap>=separation)continue;
        if(gap<0.5f) {
            const float angle=float(&enemy-game.enemies.data())*2.3999632f;
            away={std::cos(angle),std::sin(angle)};
        } else away=mul(away,1/gap);
        desired=add(desired,mul(away,speed*1.1f*(1-gap/separation)));
    }
    const float playerGap=distance(enemy.pos,game.player);
    if(game.invincible>0&&playerGap<85)desired=add(desired,mul(unit(sub(enemy.pos,game.player)),speed*2*(1-playerGap/85)));
    if(length(desired)>speed*1.15f)desired=mul(unit(desired),speed*1.15f);
    enemy.velocity=add(enemy.velocity,mul(sub(desired,enemy.velocity),std::min(1.0f,dt*8)));
    Vector2 before=enemy.pos;
    stepEnemy(game,enemy,mul(enemy.velocity,dt));
    if(dt>0)enemy.velocity=mul(sub(enemy.pos,before),1/dt);
}
Vector2 flankGoal(const Game& game,const Enemy& enemy,float preferred,float strafe) {
    Vector2 away=unit(sub(enemy.pos,game.player));
    if(length(away)<0.1f)away={1,0};
    Vector2 side={-away.y,away.x};
    const float handed=(enemy.phase%2)?-1.0f:1.0f;
    return add(game.player,add(mul(away,preferred),mul(side,strafe*handed)));
}
void telegraph(Enemy& enemy,const Game& game,float duration) {
    enemy.windup=duration;
    enemy.target=limitTarget(game.player);
    enemy.velocity={};
}
void startDash(Enemy& enemy,float speed,float duration) {
    enemy.velocity=mul(unit(sub(enemy.target,enemy.pos)),speed);
    enemy.cooldown=duration;
}
void bossAttack(Game& game,Enemy& enemy) {
    const Vector2 direction=sub(enemy.target,enemy.pos);
    const bool fierce=enemy.hp<enemy.maxHp*0.5f;
    const int pattern=enemy.phase%3;
    switch(enemy.kind) {
    case EnemyKind::PillowBoss:
        if(pattern==0) fan(game,enemy.pos,direction,5,0.22f,fierce?162:138,ShotKind::Fluff,10);
        else if(pattern==1) startDash(enemy,fierce?315:270,0.64f);
        else ring(game,enemy.pos,10,fierce?148:120,ShotKind::Fluff,0.18f,enemy.phase%10,9);
        break;
    case EnemyKind::WhiskerBoss:
        if(pattern==0) fan(game,enemy.pos,direction,fierce?7:5,0.17f,178,ShotKind::Whisker,7);
        else if(pattern==1) {
            // Four crossed whiskers always leave broad diagonal escape lanes.
            for(int n=0;n<4;++n) fan(game,enemy.pos,{std::cos(n*Pi/2),std::sin(n*Pi/2)},3,0.11f,155,ShotKind::Whisker);
        } else {
            game.grounds.push_back({enemy.target,74,3.7f,0,false,ShotKind::Whisker});
            fan(game,enemy.pos,direction,3,0.42f,125,ShotKind::Whisker,10);
        }
        break;
    case EnemyKind::WikermanBoss:
        if(pattern==0) {
            // The corrupted cat remains venomous; digital effects belong to Giovanny.
            fan(game,enemy.pos,direction,fierce?5:3,0.24f,180,ShotKind::Venom,9);
            game.grounds.push_back({enemy.target,65,4.2f,0,false,ShotKind::Venom});
        } else if(pattern==1) startDash(enemy,fierce?390:340,0.55f);
        else {
            ring(game,enemy.pos,12,145,ShotKind::Venom,enemy.age*0.21f,enemy.phase%12,8);
            if(fierce) game.grounds.push_back({enemy.pos,90,4.2f,0,false,ShotKind::Venom});
        }
        break;
    case EnemyKind::Maliketh:
        if(pattern==0) fan(game,enemy.pos,direction,fierce?7:5,0.19f,165,ShotKind::Moon,10);
        else if(pattern==1) {
            ring(game,enemy.pos,12,130,ShotKind::Moon,enemy.phase*0.31f,enemy.phase%12,8);
            if(fierce) fan(game,enemy.pos,direction,3,0.33f,175,ShotKind::Moon,8);
        } else {
            game.grounds.push_back({enemy.target,78,3.7f,0,false,ShotKind::Moon});
            Vector2 left=limitTarget(add(enemy.target,{-145,0}));
            Vector2 right=limitTarget(add(enemy.target,{145,0}));
            game.grounds.push_back({left,50,3.4f,0,false,ShotKind::Moon});
            game.grounds.push_back({right,50,3.4f,0,false,ShotKind::Moon});
        }
        break;
    default: break;
    }
    ++enemy.phase;
    enemy.timer=(game.preferences.easy?2.3f:1.9f)*(fierce?0.85f:1);
}
}

void Game::updateEnemies(float dt) {
    if(dt<=0) return;
    for(auto& enemy:enemies) {
        if(enemy.hp<=0) continue;
        enemy.age+=dt;
        enemy.flash=std::max(0.0f,enemy.flash-dt);
        if(enemy.poisonLeft>0) {
            const float poisonedTime=std::min(dt,enemy.poisonLeft);
            enemy.poisonLeft=std::max(0.0f,enemy.poisonLeft-dt);
            damageEnemy(enemy,enemy.poisonDps*poisonedTime);
            if(enemy.hp<=0) continue;
        }
        const float pace=preferences.easy?0.82f:1.0f;
        if(enemy.cooldown>0) {
            const float active=std::min(dt,enemy.cooldown);
            stepEnemy(*this,enemy,mul(enemy.velocity,active));
            enemy.cooldown=std::max(0.0f,enemy.cooldown-dt);
            if(enemy.cooldown<=0) enemy.velocity={};
        } else if(enemy.boss) {
            if(enemy.windup>0) {
                enemy.windup=std::max(0.0f,enemy.windup-dt);
                if(enemy.windup<=0) bossAttack(*this,enemy);
            } else {
                enemy.timer=std::max(0.0f,enemy.timer-dt);
                // Guardians keep padding around the room between their attacks.
                steerEnemy(*this,enemy,flankGoal(*this,enemy,190,95),72*pace,dt);
                if(enemy.timer<=0) telegraph(enemy,*this,preferences.easy?1.0f:0.78f);
            }
        } else {
            bool fired=false;
            if(enemy.windup>0) {
                enemy.windup=std::max(0.0f,enemy.windup-dt);
                fired=enemy.windup<=0;
            } else enemy.timer=std::max(0.0f,enemy.timer-dt);
            const bool roaming=enemy.windup<=0&&!fired;
            const Vector2 direction=sub(enemy.target,enemy.pos);
            switch(enemy.kind) {
            case EnemyKind::Fluff:
                if(roaming) {
                    Vector2 side=rotate(unit(sub(player,enemy.pos)),Pi/2);
                    Vector2 goal=add(player,mul(side,std::sin(enemy.age*2.5f+enemy.phase)*38));
                    float approach=distance(player,enemy.pos)<75?0.70f:1.0f;
                    steerEnemy(*this,enemy,goal,(98+stage*5)*pace*approach,dt);
                }
                if(stage>=2&&enemy.timer<=0) {
                    if(fired) {fan(*this,enemy.pos,direction,3,0.34f,143,ShotKind::Fluff,8);enemy.timer=3.1f;++enemy.phase;}
                    else if(enemy.windup<=0)telegraph(enemy,*this,0.75f);
                }
                break;
            case EnemyKind::Hopper:
                if(fired) {
                    float jumpDistance=distance(enemy.target,enemy.pos);
                    startDash(enemy,std::min(450.0f,jumpDistance/0.52f),0.52f);
                    enemy.timer=preferences.easy?1.7f:1.25f;++enemy.phase;
                    if(stage>=2)ring(*this,enemy.pos,4,125,ShotKind::Fluff,Pi/4);
                } else if(enemy.timer<=0&&enemy.windup<=0) {
                    telegraph(enemy,*this,preferences.easy?0.80f:0.62f);
                    Vector2 landing=sub(enemy.target,enemy.pos);
                    enemy.target=limitTarget(add(enemy.pos,mul(unit(landing),std::min(234.0f,length(landing)))));
                } else if(roaming)steerEnemy(*this,enemy,flankGoal(*this,enemy,48,42),104*pace,dt);
                break;
            case EnemyKind::WhiskerShooter:
                if(roaming)steerEnemy(*this,enemy,flankGoal(*this,enemy,235,105),110*pace,dt);
                if(fired) {
                    fan(*this,enemy.pos,direction,stage>=1?3:1,0.20f,172+stage*8,ShotKind::Whisker);
                    enemy.timer=preferences.easy?2.05f:1.55f;++enemy.phase;
                } else if(enemy.timer<=0&&enemy.windup<=0)telegraph(enemy,*this,0.68f);
                break;
            case EnemyKind::Pouncer:
                if(fired) {
                    startDash(enemy,(stage>=2?435:405)*pace,0.48f);
                    enemy.timer=preferences.easy?1.95f:1.50f;++enemy.phase;
                } else if(enemy.timer<=0&&enemy.windup<=0)telegraph(enemy,*this,preferences.easy?0.85f:0.68f);
                else if(roaming)steerEnemy(*this,enemy,flankGoal(*this,enemy,90,72),118*pace,dt);
                break;
            case EnemyKind::Moth:
                if(roaming)steerEnemy(*this,enemy,flankGoal(*this,enemy,190,165),137*pace,dt);
                if(fired) {
                    fan(*this,enemy.pos,direction,3,0.38f,145+stage*8,ShotKind::Moon,8);
                    enemy.timer=preferences.easy?2.40f:1.90f;++enemy.phase;
                } else if(enemy.timer<=0&&enemy.windup<=0)telegraph(enemy,*this,0.72f);
                break;
            case EnemyKind::Spore:
                if(fired) {
                    // A silly hairball spits visible globules on every floor.
                    fan(*this,enemy.pos,direction,stage>=2?3:1,0.29f,163+stage*5,ShotKind::Spore,9);
                    if(enemy.phase%3==2)grounds.push_back({enemy.target,stage>=2?60.0f:48.0f,3.0f,0,false,ShotKind::Spore});
                    enemy.timer=preferences.easy?2.35f:1.85f;++enemy.phase;
                } else if(enemy.timer<=0&&enemy.windup<=0)telegraph(enemy,*this,0.72f);
                else if(roaming)steerEnemy(*this,enemy,flankGoal(*this,enemy,205,95),98*pace,dt);
                break;
            case EnemyKind::Mirror:
                if(fired) {
                    if(enemy.phase%2==0)ring(*this,enemy.pos,4,155,ShotKind::Moon,Pi/4);
                    else fan(*this,enemy.pos,direction,3,0.20f,175,ShotKind::Moon);
                    enemy.timer=preferences.easy?2.20f:1.75f;++enemy.phase;
                } else if(enemy.timer<=0&&enemy.windup<=0)telegraph(enemy,*this,0.76f);
                else if(roaming)steerEnemy(*this,enemy,flankGoal(*this,enemy,155,140),125*pace,dt);
                break;
            case EnemyKind::Charger:
                if(fired) {
                    startDash(enemy,490*pace,0.65f);
                    enemy.timer=preferences.easy?2.10f:1.65f;++enemy.phase;
                } else if(enemy.timer<=0&&enemy.windup<=0) {
                    telegraph(enemy,*this,preferences.easy?0.98f:0.80f);
                    Vector2 delta=sub(enemy.target,enemy.pos);
                    if(std::abs(delta.x)>std::abs(delta.y))enemy.target.y=enemy.pos.y;
                    else enemy.target.x=enemy.pos.x;
                } else if(roaming)steerEnemy(*this,enemy,flankGoal(*this,enemy,100,72),108*pace,dt);
                break;
            default:break;
            }
        }
        if(enemy.hp>0&&dashTime<=0&&distance(player,enemy.pos)<(enemy.boss?43.0f:31.0f))damage();
    }
    removeDefeated();
}

void Game::updateWisky(float dt) {
    Vector2 behind=sub(player,mul(facing,63));
    behind.y+=22;
    Vector2 delta=sub(behind,companion);
    companionMoving=length(delta)>8;
    if(std::abs(delta.x)>2) companionLeft=delta.x<0;
    if(length(delta)>210) companion=move(player,mul(unit(delta),-40),12);
    else companion=move(companion,mul(delta,std::min(1.0f,dt*5.2f)),12);
    Vector2 digitalTarget=add(player,{65,-25});
    digitalCompanion=move(digitalCompanion,mul(sub(digitalTarget,digitalCompanion),std::min(1.0f,dt*5.0f)),12);
    wiskyShotCooldown=std::max(0.0f,wiskyShotCooldown-dt);
    digitalShotCooldown=std::max(0.0f,digitalShotCooldown-dt);
    companionPoisonPulse=std::max(0.0f,companionPoisonPulse-dt);
    if(enemies.empty()) return;
    const Enemy* nearest=nullptr;
    float nearestDistance=580;
    for(const auto& enemy:enemies) if(enemy.hp>0&&distance(enemy.pos,companion)<nearestDistance) {
        nearest=&enemy; nearestDistance=distance(enemy.pos,companion);
    }
    if(!wiskyCorrupted&&nearest&&wiskyShotCooldown<=0) {
        Projectile venom;
        venom.pos=add(companion,{0,-24});
        venom.velocity=mul(unit(sub(nearest->pos,venom.pos)),485);
        venom.life=1.5f; venom.damage=stats.wiskyDamage; venom.kind=ShotKind::Venom;
        venom.poisonDps=stats.poisonDps; venom.poisonSeconds=stats.poisonDuration; venom.radius=8;
        shots.push_back(venom);
        wiskyShotCooldown=std::max(0.22f,stats.wiskyInterval);
        companionPoisonPulse=0.22f;
        if(stats.toxicCloud) grounds.push_back({companion,47,2.5f,0,true,ShotKind::Venom});
    }
    if(stats.digitalBuddy&&digitalShotCooldown<=0) {
        const Enemy* target=nullptr;
        float gap=650;
        for(const auto& enemy:enemies) if(enemy.hp>0&&distance(enemy.pos,digitalCompanion)<gap) {
            gap=distance(enemy.pos,digitalCompanion); target=&enemy;
        }
        if(target) {
            Projectile pixel;
            pixel.pos=add(digitalCompanion,{0,-28});
            pixel.velocity=mul(unit(sub(target->pos,pixel.pos)),610);
            pixel.life=1.4f; pixel.damage=0.8f+stats.damage*0.3f;
            pixel.kind=ShotKind::Digital; pixel.radius=6; pixel.piercing=true;
            shots.push_back(pixel);
            digitalShotCooldown=0.82f;
        }
    }
}

void Game::updateShots(float dt) {
    for(auto& shot:shots) {
        // Expired shots never produce a final, invisible hit.
        shot.life-=dt;
        if(shot.life<=0) continue;
        float travelDistance=length(shot.velocity)*dt;
        int steps=std::max(1,int(std::ceil(travelDistance/8)));
        Vector2 motion=mul(shot.velocity,dt/steps);
        for(int step=0;step<steps&&shot.life>0;++step) {
            shot.pos=add(shot.pos,motion);
            if(blocked(*this,shot.pos,shot.radius)) { shot.life=0; break; }
            for(std::size_t index=0;index<enemies.size();++index) {
                auto& enemy=enemies[index];
                // A small per-projectile bit mask prevents overlapping enemies
                // from repeatedly resetting the previous target of a piercing shot.
                const int hitBit=index<30?(1<<index):0;
                const int hitMask=std::max(0,shot.lastHit);
                if(enemy.hp<=0||(shot.piercing&&hitBit&&(hitMask&hitBit))) continue;
                if(distance(shot.pos,enemy.pos)>enemyRadius(enemy)+shot.radius) continue;
                damageEnemy(enemy,shot.damage,shot.poisonDps,shot.poisonSeconds);
                shot.lastHit=shot.piercing?(hitMask|hitBit):int(index);
                if(!shot.piercing) { shot.life=0; break; }
            }
        }
    }
    shots.erase(std::remove_if(shots.begin(),shots.end(),[](const Projectile& p){return p.life<=0;}),shots.end());
    // Enemy indexes can change when defeated foes are removed. Clearing surviving
    // piercing shots prevents one target from being hit twice under a shifted index.
    bool fallen=std::any_of(enemies.begin(),enemies.end(),[](const Enemy& e){return e.hp<=0;});
    if(fallen) {
        shots.erase(std::remove_if(shots.begin(),shots.end(),[](const Projectile& p){return p.piercing&&p.lastHit>=0;}),shots.end());
        removeDefeated();
    }
}

void Game::updateHazards(float dt) {
    for(auto& shot:hazards) {
        shot.life-=dt;
        if(shot.life<=0) continue;
        int steps=std::max(1,int(std::ceil(length(shot.velocity)*dt/8)));
        Vector2 motion=mul(shot.velocity,dt/steps);
        for(int step=0;step<steps&&shot.life>0;++step) {
            shot.pos=add(shot.pos,motion);
            if(blocked(*this,shot.pos,shot.radius)) { shot.life=0; break; }
            if(distance(shot.pos,player)<15+shot.radius) {
                // A revival clears both hazard containers. Finish this projectile
                // before calling damage, then leave without touching invalidated
                // references or range-for iterators if a life was consumed.
                shot.life=0;
                const int previousLives=extraLives;
                if(dashTime<=0) damage();
                if(extraLives!=previousLives||screen==Screen::Lost) return;
            }
        }
    }
    hazards.erase(std::remove_if(hazards.begin(),hazards.end(),[](const Projectile& p){return p.life<=0;}),hazards.end());
    for(auto& ground:grounds) {
        ground.life-=dt;
        ground.age+=dt;
        if(ground.life<=0) continue;
        if(ground.friendly) {
            for(auto& enemy:enemies) if(enemy.hp>0&&distance(enemy.pos,ground.pos)<ground.radius+enemyRadius(enemy)*0.35f) {
                damageEnemy(enemy,(0.8f+stats.poisonDps*0.25f)*dt,stats.poisonDps,0.8f);
            }
        } else if(ground.age>=0.8f&&dashTime<=0&&distance(player,ground.pos)<ground.radius-8) {
            const int previousLives=extraLives;
            damage();
            if(extraLives!=previousLives||screen==Screen::Lost) return;
        }
    }
    grounds.erase(std::remove_if(grounds.begin(),grounds.end(),[](const GroundEffect& ground){return ground.life<=0;}),grounds.end());
    removeDefeated();
}
}
