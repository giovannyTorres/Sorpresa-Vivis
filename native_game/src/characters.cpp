#include "visuals.hpp"
#include <array>

namespace vivis {
namespace {
constexpr Rectangle poses[4][4]={
    {{40,53,209,287},{322,55,225,285},{611,59,230,286},{899,64,355,280}},
    {{32,415,225,214},{305,415,250,214},{581,426,254,203},{877,392,374,275}},
    {{39,657,209,308},{335,659,216,311},{606,664,212,304},{870,674,381,291}},
    {{19,991,268,240},{312,1002,265,225},{588,1015,282,221},{873,969,381,266}}
};
constexpr Vector2 pivots[4][4]={
    {{100,286},{123,284},{119,285},{126,279}},
    {{153,213},{164,213},{165,202},{147,237}},
    {{114,308},{110,311},{94,304},{130,291}},
    {{166,239},{143,223},{163,219},{159,264}}
};
constexpr Rectangle emotions[4][2]={
    {{29,27,308,357},{364,24,300,371}},{{70,457,253,268},{375,421,303,304}},
    {{28,775,309,338},{395,730,291,389}},{{19,1149,315,275},{370,1125,325,307}}
};
constexpr Vector2 emotionPivots[4][2]={
    {{141,355},{150,370}},{{130,266},{160,302}},{{143,337},{129,388}},{{177,273},{150,305}}
};
constexpr Rectangle portraits[4]={{691,2,394,374},{705,380,360,345},{690,726,394,385},{697,1111,387,327}};
constexpr Color Pink{248,145,187,255}, Cyan{112,212,249,255}, Purple{183,133,235,255}, Green{164,224,112,255}, Gold{245,213,135,255};
void pixel(Vector2 p,int size,Color c) { DrawRectangle(int(std::round(p.x)),int(std::round(p.y)),size,size,c); }
void sparkle(Vector2 p,Color color,int size=2) {
    DrawRectangle(int(p.x)-size/2,int(p.y)-size*2,size,size*4,color);
    DrawRectangle(int(p.x)-size*2,int(p.y)-size/2,size*4,size,color);
}
void heart(Vector2 p,float scale,Color color) {
    static const char* map[]={"0110110","1221221","1222221","1222221","0122210","0012100","0001000"};
    for(int y=0;y<7;++y) for(int x=0;x<7;++x) if(map[y][x]!='0') {
        Color c=map[y][x]=='1'?Color{124,57,96,color.a}:color;
        DrawRectangle(int(p.x+(x-3.5f)*scale),int(p.y+(y-3.5f)*scale),int(scale+0.5f),int(scale+0.5f),c);
    }
    DrawRectangle(int(p.x-1.5f*scale),int(p.y-2.5f*scale),int(scale),int(scale),{255,237,246,color.a});
}
void pixelRing(Vector2 pos,float radius,Color color,int size=3,int count=42) {
    for(int n=0;n<count;++n) {
        float angle=n*6.2831853f/count;
        pixel(add(pos,{std::cos(angle)*radius-size/2.0f,std::sin(angle)*radius-size/2.0f}),size,color);
    }
}
void spriteMask(const char* const* shape,int rows,Vector2 feet,float scale,Color body,Color accent,Color highlight,bool flash=false) {
    int width=0;for(int y=0;y<rows;++y)for(int x=0;shape[y][x];++x)width=std::max(width,x+1);
    for(int y=0;y<rows;++y)for(int x=0;shape[y][x];++x) {
        char key=shape[y][x];if(key==' ')continue;
        Color color=key=='o'?Color{34,31,47,255}:key=='b'?body:key=='h'?highlight:key=='a'?accent:
            key=='e'?Color{34,31,47,255}:key=='w'?Color{255,247,230,255}:Pink;
        if(flash&&key!='o'&&key!='e')color={255,219,234,255};
        DrawRectangle(int(feet.x+(x-width/2.0f)*scale),int(feet.y+(y-rows)*scale),int(scale),int(scale),color);
    }
}
void littleEnemy(const Enemy& enemy,float clock) {
    static const char* fluff[]={"   oo     oo   ","  ohhooooohho  "," oohhbbbbbhhoo "," obbhbbbbbhbbo ","obbbwwbbbwwbbbo","obbbewbbbwebbbo"," obbppbbbppbbo "," obbbbbbbbbbbo ","  obbbhhbbbbo  "," oobbboobbboo  "," obbbo  obbbo  ","  ooo    ooo   "};
    static const char* hopper[]={"  ooo      ooo "," ohbbo    obbho"," ohbbo    obbho"," obbbooooobbbo ","  obbhhhhbbbo  "," obwwbbbbwwbbo ","obbewbbbbwebbbo"," obbppbbppbbo  ","  obbbbbbbbo   "," oohhboobhhhoo "," obbbboobbbbbo ","  ooo    ooo   "};
    static const char* whisker[]={"      ooo      ","     ohbho     ","   oobbbbboo   ","  obbhhhhbbbo  "," obbwwbbwwbbbo ","obbbeabbbaebbbo","aaaabbbpbbbaaaa","  oabbbbbbao   ","aa obbbbbbo aaa","    obbbbo     ","   oobbbboo    ","    oooooo     "};
    static const char* pouncer[]={" oooo      oooo "," ohhbo    obhho "," obhhboooobhbbo "," obbbbbbbbbbbbo ","  obwwbbbbwwbo  ","  obewbbbbwebo  ","  obbppbbppbbo  "," oobbbboobbbboo ","obbhbbbbbbbbhbbo"," obbbbbbbbbbbbo ","  obbbo  obbbo  ","   ooo    ooo   "};
    static const char* moth[]={" ooo        ooo ","ohhboo    oobhho","ohhhabo  obahhho"," obhhaooooahhbo "," obbaaweew aabbo","  obbabweeb abbo","   obbaaaabbo   ","  obhhaaahhbbo  "," ohhhaooahhhho  "," ohhbo  obhhho  ","  ooo    ooo    "};
    static const char* spore[]={"    oooooo     ","  oobbwwbboo   "," obhbbbbbhbbo  ","obhbwwbbbbbhbo ","obbbbbbbwbbbbo "," obbwwbbbbboo  ","  oooohhoooo   ","    ohhhho     ","    ohwwho     ","    oheeho     ","   oohhhhoo    ","    oooooo     "};
    static const char* mirror[]={"     aaaa      ","   aaohhoaa    ","  aohhhhhhoa   "," aohhwwhhhhoa  "," aohwhhhwhhoa  "," aohhewhehhoa  "," aohhwhhwhhoa  "," aohhhpphhhoa  ","  aohhhhhhoa   ","   aaohhoaa    ","    oobboo     ","   obbbbbbo    "};
    static const char* charger[]={"  aaa      aaa  "," aawaooooooawaa ","  aobbhhhhboa   "," oobbbhhhhbbboo "," obbwwbbbbwwbbo ","obbbewbbbbwebbbo","obbbbbaaaabbbbbo"," obbbbawwabbbbo ","  obbbaeeabbbo  ","  obbbbbbbbbbo  ","  obbbo  obbbo  ","   ooo    ooo   "};
    const char*const* mask=fluff;int rows=12;
    Color body{151,124,185,255},accent{212,188,237,255},highlight{193,170,217,255};
    switch(enemy.kind) {
    case EnemyKind::Hopper:mask=hopper;body={139,190,166,255};accent={247,193,136,255};highlight={189,225,193,255};break;
    case EnemyKind::WhiskerShooter:mask=whisker;body={224,178,104,255};accent=Gold;highlight={255,224,170,255};break;
    case EnemyKind::Pouncer:mask=pouncer;body={209,129,145,255};accent={247,205,151,255};highlight={245,174,181,255};break;
    case EnemyKind::Moth:mask=moth;rows=11;body={133,119,178,255};accent={235,207,244,255};highlight={184,166,222,255};break;
    case EnemyKind::Spore:mask=spore;body={123,164,103,255};accent=Green;highlight={207,225,168,255};break;
    case EnemyKind::Mirror:mask=mirror;body={143,185,205,255};accent={207,167,216,255};highlight={192,225,232,255};break;
    case EnemyKind::Charger:mask=charger;body={119,101,145,255};accent={221,189,155,255};highlight={165,141,183,255};break;
    default:break;
    }
    DrawEllipse(int(enemy.pos.x),int(enemy.pos.y+2),27,8,{5,10,18,70});
    float bob=std::sin(clock*(enemy.kind==EnemyKind::Moth?9:5)+enemy.age)*2;
    if(enemy.kind==EnemyKind::Hopper&&enemy.cooldown>0)bob-=std::sin(enemy.cooldown/0.52f*3.1415926f)*21;
    if(enemy.kind==EnemyKind::Moth)bob-=12;
    Vector2 feet=add(enemy.pos,{0,bob});
    spriteMask(mask,rows,feet,4,body,accent,highlight,enemy.flash>0);
    if(enemy.poisonLeft>0) {
        for(int n=0;n<3;++n) {
            float phase=std::fmod(clock*0.7f+n*0.31f,1.0f);
            pixel(add(feet,{float(n*19-20),-17-phase*45}),3,Fade(Green,1-phase));
        }
        pixelRing(enemy.pos,24,Fade(Green,0.4f),2,18);
    }
    if(enemy.hp<enemy.maxHp) {
        DrawRectangle(int(feet.x-23),int(feet.y-rows*4-10),46,4,{25,24,39,255});
        DrawRectangle(int(feet.x-22),int(feet.y-rows*4-9),int(44*std::clamp(enemy.hp/enemy.maxHp,0.0f,1.0f)),2,enemy.poisonLeft>0?Green:accent);
    }
}
void plushBoss(const Enemy& enemy,float clock) {
    const bool pillow=enemy.kind==EnemyKind::PillowBoss;
    const Color outline{45,36,55,255};
    const Color body=pillow?Color{220,182,206,255}:Color{225,181,118,255};
    const Color light=pillow?Color{249,218,230,255}:Color{250,224,176,255};
    Vector2 p=add(enemy.pos,{0,std::sin(clock*3)*3});
    DrawEllipse(int(p.x),int(enemy.pos.y+3),59,14,{5,10,18,95});
    DrawRectangle(int(p.x-62),int(p.y-101),124,86,outline);
    DrawRectangle(int(p.x-72),int(p.y-88),144,59,outline);
    DrawRectangle(int(p.x-56),int(p.y-110),112,104,outline);
    DrawRectangle(int(p.x-58),int(p.y-96),116,76,body);
    DrawRectangle(int(p.x-67),int(p.y-83),134,49,body);
    DrawRectangle(int(p.x-51),int(p.y-105),102,96,body);
    DrawRectangle(int(p.x-43),int(p.y-99),86,8,light);
    DrawRectangle(int(p.x-61),int(p.y-79),7,39,light);
    if(pillow) {
        // Stitched corners, tiny paws and a crooked button crown.
        for(int n=0;n<5;++n) {
            DrawRectangle(int(p.x-45+n*22),int(p.y-85),7,3,{179,137,168,255});
            DrawRectangle(int(p.x-45+n*22),int(p.y-23),7,3,{179,137,168,255});
        }
        DrawRectangle(int(p.x-24),int(p.y-126),48,18,outline);
        DrawRectangle(int(p.x-20),int(p.y-131),8,22,Gold);
        DrawRectangle(int(p.x-4),int(p.y-136),8,27,Gold);
        DrawRectangle(int(p.x+12),int(p.y-129),8,20,Gold);
        heart(add(p,{0,-115}),1.2f,Pink);
    } else {
        // Huge matching whiskers define this guardian's silhouette.
        for(int side:{-1,1})for(int n=0;n<3;++n) {
            Vector2 from=add(p,{float(side*39),float(-43+n*5)});
            Vector2 end=add(p,{float(side*(95-n*3)),float(-57+n*17)});
            DrawLineEx(from,end,6,outline);DrawLineEx(add(from,{0,-2}),add(end,{0,-2}),3,Gold);
        }
        for(int side:{-1,1}) {
            DrawRectangle(int(p.x+side*44-8),int(p.y-120),17,22,outline);
            DrawRectangle(int(p.x+side*44-4),int(p.y-115),9,18,light);
            DrawRectangle(int(p.x+side*28-10),int(p.y-99),20,7,{174,127,92,255});
        }
    }
    for(int side:{-1,1}) {
        DrawRectangle(int(p.x+side*25-8),int(p.y-66),16,21,outline);
        DrawRectangle(int(p.x+side*25-5),int(p.y-65),5,7,WHITE);
        DrawRectangle(int(p.x+side*43-8),int(p.y-40),17,7,Pink);
        DrawRectangle(int(p.x+side*31-13),int(p.y-11),26,11,outline);
        DrawRectangle(int(p.x+side*31-9),int(p.y-10),18,7,light);
    }
    DrawRectangle(int(p.x-6),int(p.y-44),12,6,outline);
    DrawRectangle(int(p.x-2),int(p.y-38),4,7,outline);
    if(enemy.flash>0)pixelRing(add(p,{0,-55}),62,Fade(Pink,0.8f),4,26);
    if(enemy.poisonLeft>0)pixelRing(enemy.pos,62,Fade(Green,0.55f),3,30);
}
Color shotColor(ShotKind kind) {
    switch(kind) {
    case ShotKind::Venom:return Green;
    case ShotKind::Digital:return Cyan;
    case ShotKind::Fluff:return {239,211,226,255};
    case ShotKind::Whisker:return Gold;
    case ShotKind::Moon:return Purple;
    case ShotKind::Spore:return {179,207,114,255};
    default:return Pink;
    }
}
void drawShot(const Projectile& shot,float clock) {
    const Vector2 p=shot.pos,back=mul(unit(shot.velocity),-1);
    const Color color=shotColor(shot.kind);
    for(int n=1;n<5;++n)pixel(add(p,mul(back,n*7.0f)),std::max(2,6-n),Fade(color,0.4f-n*0.06f));
    switch(shot.kind) {
    case ShotKind::Heart:heart(p,2.0f,color);break;
    case ShotKind::Venom:
        DrawRectangle(int(p.x-7),int(p.y-6),14,12,{64,101,57,255});
        DrawRectangle(int(p.x-5),int(p.y-8),10,16,color);
        DrawRectangle(int(p.x-8),int(p.y-4),16,8,color);
        DrawRectangle(int(p.x-4),int(p.y-5),4,4,{229,255,196,255});
        pixel(add(p,{5,9}),3,Fade(color,0.8f));break;
    case ShotKind::Digital:
        DrawRectangleLinesEx({p.x-7,p.y-7,14,14},2,color);
        DrawRectangle(int(p.x-2),int(p.y-2),5,5,WHITE);
        pixel(add(p,{-11,5}),3,color);pixel(add(p,{8,-10}),3,color);break;
    case ShotKind::Whisker: {
        Vector2 direction=unit(shot.velocity);if(length(direction)<0.1f)direction={1,0};
        DrawLineEx(sub(p,mul(direction,12)),add(p,mul(direction,12)),6,{133,92,68,255});
        DrawLineEx(sub(p,mul(direction,11)),add(p,mul(direction,11)),3,color);
        sparkle(p,{255,242,205,255},2);break;
    }
    case ShotKind::Moon:
        DrawCircleV(p,shot.radius+2,{62,41,88,255});
        DrawCircleV(p,shot.radius,color);
        DrawCircleV(add(p,{4,-2}),shot.radius-1,{62,41,88,255});
        sparkle(add(p,{-7,-7}),{234,211,255,255},1);break;
    case ShotKind::Spore:
        DrawRectangle(int(p.x-8),int(p.y-6),16,12,{80,103,56,255});
        DrawRectangle(int(p.x-6),int(p.y-8),12,16,color);
        pixel(add(p,{-3,-4}),3,{246,245,198,255});pixel(add(p,{3,2}),3,{246,245,198,255});break;
    case ShotKind::Fluff:
        DrawRectangle(int(p.x-9),int(p.y-5),18,10,{163,128,160,255});
        DrawRectangle(int(p.x-6),int(p.y-9),12,18,color);
        DrawRectangle(int(p.x-8),int(p.y-6),16,12,color);
        sparkle(p,{255,246,248,255},2);break;
    }
    if(int(clock*8)%2)sparkle(add(p,{9,-11}),Fade(color,0.7f),1);
}
}

void drawCharacter(const Artwork& art,int row,int pose,Vector2 feet,float height,bool flip,Color tint) {
    row=std::clamp(row,0,3); pose=std::clamp(pose,0,5);
    Rectangle source=pose<4?poses[row][pose]:emotions[row][pose-4];
    Vector2 pivot=pose<4?pivots[row][pose]:emotionPivots[row][pose-4];
    float scale=height/poses[row][0].height;
    DrawEllipse(int(feet.x),int(feet.y+2),row==0||row==2?25:31,7,{4,9,15,90});
    float offset=flip?(source.width-pivot.x):pivot.x;
    Rectangle target{std::round(feet.x-offset*scale),std::round(feet.y-pivot.y*scale),
        std::round(source.width*scale),std::round(source.height*scale)};
    if(flip) source.width=-source.width;
    DrawTexturePro(pose<4?art.characters:art.emotions,source,target,{0,0},0,tint);
}
void drawPortrait(const Artwork& art,int row,Rectangle target,bool victory) {
    row=std::clamp(row,0,3);
    Rectangle source=victory?emotions[row][1]:portraits[row];
    float scale=std::min(target.width/source.width,target.height/source.height);
    Rectangle fit{target.x+(target.width-source.width*scale)/2,target.y+(target.height-source.height*scale)/2,
        source.width*scale,source.height*scale};
    DrawTexturePro(art.emotions,source,fit,{0,0},0,WHITE);
}
void drawEffects(const Game& game,const Artwork& art) {
    float clock=game.preferences.reducedMotion?0:game.time;
    const bool finalRoom=!game.rooms.empty()&&game.stage==3&&game.room().kind==RoomKind::Boss;
    // All dangerous floor areas have a separate warning interval before becoming solid.
    for(const auto& ground:game.grounds) {
        if(ground.life<=0)continue;
        Color color=ground.friendly?Green:shotColor(ground.kind);
        bool warning=!ground.friendly&&ground.age<0.8f;
        DrawCircleV(ground.pos,ground.radius,Fade(color,warning?0.055f:0.17f));
        pixelRing(ground.pos,ground.radius,Fade(color,warning?0.7f:0.88f),warning?3:4,48);
        if(warning) {
            pixelRing(ground.pos,ground.radius*std::clamp(ground.age/0.8f,0.0f,1.0f),Fade(Gold,0.65f),2,32);
            DrawRectangle(int(ground.pos.x-2),int(ground.pos.y-14),4,15,Gold);
            pixel(add(ground.pos,{-2,5}),4,Gold);
        } else for(int n=0;n<6;++n) {
            float a=n*1.0471975f+clock*0.4f;
            Vector2 spot=add(ground.pos,{std::cos(a)*ground.radius*0.55f,std::sin(a)*ground.radius*0.55f});
            sparkle(spot,Fade(color,0.65f),ground.friendly?1:2);
        }
    }
    for(const auto& enemy:game.enemies)if(enemy.windup>0) {
        Color warning=enemy.kind==EnemyKind::WikermanBoss?Green:Gold;
        pixelRing(enemy.pos,enemy.boss?57:33,Fade(warning,0.7f),3,28);
        Vector2 direction=sub(enemy.target,enemy.pos);
        float distanceToTarget=length(direction);
        direction=unit(direction);
        int dots=std::max(1,int(distanceToTarget/14));
        for(int n=1;n<dots;++n)pixel(add(enemy.pos,mul(direction,n*14.0f)),3,Fade(warning,0.4f));
        const bool landing=enemy.kind==EnemyKind::Hopper||enemy.kind==EnemyKind::Spore||
            (enemy.boss&&enemy.phase%3==2);
        if(landing)pixelRing(enemy.target,enemy.kind==EnemyKind::Hopper?24.0f:48.0f,Fade(warning,0.8f),3,28);
        sparkle(add(enemy.pos,{0,enemy.boss?-145.0f:-66.0f}),warning,3);
    }
    for(Vector2 p:game.hearts) {p.y+=std::sin(clock*3)*3;heart(p,3,Pink);}
    for(const auto& pickup:game.loot) {
        Vector2 p=add(pickup.pos,{0,-12+std::sin(clock*3+pickup.pos.x)*3});
        DrawEllipse(int(pickup.pos.x),int(pickup.pos.y+2),16,5,{5,10,18,65});
        if(pickup.kind==PickupKind::Heart)heart(p,3,Pink);
        else if(pickup.kind==PickupKind::Coin) {
            DrawRectangle(int(p.x-8),int(p.y-11),16,22,{138,98,57,255});
            DrawRectangle(int(p.x-11),int(p.y-8),22,16,{191,141,70,255});
            DrawRectangle(int(p.x-7),int(p.y-9),14,18,Gold);
            DrawRectangle(int(p.x-2),int(p.y-6),4,12,{175,118,56,255});
            pixel(add(p,{-5,-7}),3,{255,244,193,255});
        } else if(pickup.kind==PickupKind::Item) {
            Color c=itemInfo(pickup.item).color;
            DrawRectangle(int(p.x-14),int(p.y-12),28,24,{45,40,55,255});
            DrawRectangle(int(p.x-11),int(p.y-10),22,21,c);
            DrawRectangle(int(p.x-3),int(p.y-12),6,25,{255,242,211,255});
            DrawRectangle(int(p.x-13),int(p.y-3),26,5,{255,242,211,255});
            sparkle(add(p,{0,-15}),c,2);
        } else {
            // The churu is a visible, guaranteed boss reward, never a random requirement.
            DrawRectangle(int(p.x-11),int(p.y-28),22,48,{98,58,64,255});
            DrawRectangle(int(p.x-8),int(p.y-25),16,42,{247,173,132,255});
            DrawRectangle(int(p.x-8),int(p.y-13),16,18,{255,242,214,255});
            DrawRectangle(int(p.x-11),int(p.y-28),22,5,{255,218,164,255});
            heart(add(p,{0,-4}),1.2f,Pink);
            sparkle(add(p,{-19,-24}),Gold,2);sparkle(add(p,{18,1}),Gold,2);
        }
    }
    struct Actor {float y;int type,index;};
    std::vector<Actor> actors{{game.player.y,0,0}};
    if(!game.wiskyCorrupted)actors.push_back({game.companion.y,1,0});
    if(game.stats.digitalBuddy)actors.push_back({game.digitalCompanion.y,4,0});
    for(std::size_t i=0;i<game.enemies.size();++i)if(game.enemies[i].hp>0)actors.push_back({game.enemies[i].pos.y,2,int(i)});
    if(finalRoom) {
        actors.push_back({RescuePosition.y,3,0});
        if(game.bossDefeated)actors.push_back({MalikethPosition.y,5,0});
    }
    std::sort(actors.begin(),actors.end(),[](Actor a,Actor b){return a.y<b.y;});
    for(auto actor:actors) {
        int step=1+int(clock*7)%2;
        if(actor.type==0) {
            if(game.invincible>0&&game.hurtTime<=0&&!game.preferences.reducedMotion&&int(clock*12)%2)continue;
            int pose=game.screen==Screen::Won?5:game.hurtTime>0?4:game.castTime>0?3:game.moving?step:0;
            bool flip=(game.castTime>0&&std::abs(game.aim.x)>0.1f)?game.aim.x<0:game.facing.x<0;
            if(game.dashTime>0&&!game.preferences.reducedMotion)for(int n=1;n<4;++n)
                drawCharacter(art,0,step,sub(game.player,mul(game.facing,n*13.0f)),113,flip,{248,180,215,static_cast<unsigned char>(65-n*13)});
            drawCharacter(art,0,pose,game.player,113,flip);
            if(game.roomShield>0)pixelRing(game.player,25,Fade(Purple,0.35f),2,22);
        } else if(actor.type==1) {
            int pose=game.screen==Screen::Won?5:game.helpCastTime>0?3:game.companionMoving?step:0;
            if(game.companionPoisonPulse>0)pixelRing(game.companion,29,Fade(Green,game.companionPoisonPulse*2.5f),3,20);
            drawCharacter(art,1,pose,game.companion,83,game.companionLeft);
        } else if(actor.type==3) {
            drawCharacter(art,2,game.malikethFriendly?5:0,RescuePosition,110,false,WHITE);
            if(!game.malikethFriendly) {
                const float top=RescuePosition.y-122;
                for(int n=-2;n<=2;++n) {
                    DrawRectangle(int(RescuePosition.x+n*23-2),int(top),5,124,{84,86,119,255});
                    DrawRectangle(int(RescuePosition.x+n*23-1),int(top),1,120,{194,181,223,255});
                }
                DrawRectangle(int(RescuePosition.x-59),int(top+1),118,5,{161,155,190,255});
                DrawRectangle(int(RescuePosition.x-59),int(RescuePosition.y+1),118,6,{104,99,137,255});
                DrawRectangle(int(RescuePosition.x-8),int(RescuePosition.y-50),16,22,{62,55,82,255});
                DrawRectangle(int(RescuePosition.x-6),int(RescuePosition.y-48),12,16,Gold);
            } else {
                for(int n=0;n<4;++n)sparkle(add(RescuePosition,{float(n*23-35),-120-std::sin(clock*2+n)*6}),Cyan,2);
            }
        } else if(actor.type==4) {
            DrawRectangle(int(game.digitalCompanion.x-24),int(game.digitalCompanion.y+1),48,4,Fade(Cyan,0.55f));
            drawCharacter(art,2,game.digitalShotCooldown>0.65f?3:0,game.digitalCompanion,70,false,{135,225,255,155});
            pixelRing(game.digitalCompanion,24,Fade(Cyan,0.4f),2,16);
        } else if(actor.type==5) {
            drawCharacter(art,3,game.malikethFriendly?5:4,MalikethPosition,132);
            if(game.malikethFriendly)for(int n=0;n<3;++n) {
                Vector2 p=add(MalikethPosition,{float(n*38-38),-140-std::sin(clock*2+n)*8});
                heart(p,1.5f,Pink);
            }
        } else {
            const Enemy& enemy=game.enemies[actor.index];
            if(!enemy.boss) {littleEnemy(enemy,clock);continue;}
            int pose=enemy.flash>0?4:enemy.windup>0?3:enemy.cooldown>0?step:0;
            if(enemy.kind==EnemyKind::PillowBoss||enemy.kind==EnemyKind::WhiskerBoss)plushBoss(enemy,clock);
            else if(enemy.kind==EnemyKind::WikermanBoss) {
                pixelRing(enemy.pos,56,Fade(Green,0.4f),4,32);
                drawCharacter(art,1,pose,enemy.pos,132,game.player.x<enemy.pos.x,enemy.flash>0?Pink:Color{190,220,182,255});
                for(int n=0;n<4;++n) {
                    float a=clock*0.6f+n*1.5707963f;
                    sparkle(add(enemy.pos,{std::cos(a)*64,std::sin(a)*20-32}),Green,2);
                }
            } else drawCharacter(art,3,pose,enemy.pos,143,game.player.x<enemy.pos.x,enemy.flash>0?Pink:WHITE);
            if(enemy.poisonLeft>0)for(int n=0;n<3;++n)pixel(add(enemy.pos,{float(n*27-27),-96-std::fmod(clock*25+n*15,42.0f)}),4,Green);
        }
    }
    for(const auto& shot:game.shots)if(shot.life>0)drawShot(shot,clock);
    for(const auto& shot:game.hazards)if(shot.life>0)drawShot(shot,clock);
    if(game.pulse>0) {
        float progress=std::clamp(1-game.pulse/0.55f,0.0f,1.0f);
        pixelRing(game.player,game.stats.helpRadius*progress,Fade(Green,1-progress),5,44);
        for(int n=0;n<12;++n) {
            float a=n*6.2831853f/12;
            Vector2 p=add(game.player,{std::cos(a)*game.stats.helpRadius*progress,std::sin(a)*game.stats.helpRadius*progress});
            sparkle(p,Fade(Green,1-progress),2);
        }
    }
    for(const auto& p:game.particles)if(p.maxLife>0)sparkle(p.pos,Fade(p.color,std::clamp(p.life/p.maxLife,0.0f,1.0f)),2);
    if(finalRoom&&game.bossDefeated) {
        const char* label=nullptr;Vector2 target{};
        if(!game.malikethFriendly&&game.hasChuru&&distance(game.player,MalikethPosition)<135) {
            label="[ E ] Dar un churu";target=add(MalikethPosition,{0,-163});
        } else if(game.malikethFriendly&&distance(game.player,RescuePosition)<115) {
            label="[ E ] Rescatar a Giovanny";target=add(RescuePosition,{0,-135});
        }
        if(label) {
            Vector2 size=MeasureTextEx(art.font,label,19,1);
            target.x=std::clamp(target.x,Field.x+size.x/2+12,Field.x+Field.width-size.x/2-12);
            target.y=std::max(165.0f,target.y);
            DrawRectangle(int(target.x-size.x/2-10),int(target.y-5),int(size.x+20),29,{16,24,34,240});
            DrawTextEx(art.font,label,{target.x-size.x/2,target.y},19,1,game.malikethFriendly?Cyan:Pink);
        }
    }
}
}
