#include "visuals.hpp"
#include <array>

namespace vivis {
namespace {
constexpr Rectangle poses[4][4]={
    {{42,55,206,283},{323,56,223,282},{612,60,228,284},{900,64,346,276}},
    {{35,417,220,211},{306,418,247,210},{581,429,253,199},{880,426,363,209}},
    {{41,658,206,306},{339,661,209,308},{604,667,210,299},{874,683,369,281}},
    {{32,1010,241,215},{312,1015,249,208},{593,1030,265,198},{884,994,359,235}}
};
constexpr Vector2 pivots[4][4]={
    {{98,283},{122,282},{118,284},{125,276}},
    {{150,211},{163,210},{165,199},{144,209}},
    {{112,306},{106,308},{96,299},{126,281}},
    {{153,215},{143,208},{158,198},{148,235}}
};
constexpr Rectangle emotions[4][2]={
    {{33,33,292,340},{359,27,284,356}},{{76,456,236,250},{372,417,283,289}},
    {{31,769,298,326},{391,717,280,385}},{{31,1175,289,221},{369,1131,302,285}}
};
constexpr Vector2 emotionPivots[4][2]={
    {{137,340},{151,356}},{{124,250},{158,289}},{{139,326},{124,385}},{{174,221},{151,285}}
};
constexpr Rectangle portraits[4]={{705,8,375,360},{713,377,345,329},{704,716,360,380},{710,1101,353,330}};
constexpr Color Pink{248,145,187,255}, Cyan{112,212,249,255}, Purple{183,133,235,255};
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
void fluff(const Enemy& enemy,float time) {
    static const char* shape[]={"   o       o    ","  ohh     oho   "," oohhooooohhoo  ",
        " ophhhpppphhpo  ","oophhhppppphhoo "," ophhpppppppppo ","ooppoyyppoyypoo ",
        " oppoyeppoyepo  "," ooppppppppppoo ","  opppphhppppo  "," ooppppooppppoo ",
        "  ooppppppppoo  "," oooppppppppooo ","    oooooooo    "};
    DrawEllipse(int(enemy.pos.x),int(enemy.pos.y+3),27,8,{5,10,18,75});
    for(int y=0;y<14;++y) for(int x=0;shape[y][x];++x) {
        char key=shape[y][x]; if(key==' ') continue;
        Color c=key=='o'?Color{24,23,38,255}:key=='p'?Color{73,53,85,255}:key=='h'?Color{126,86,137,255}:
            key=='y'?Color{218,188,248,255}:Color{108,217,232,255};
        if(enemy.flash>0) c=Pink;
        DrawRectangle(int(enemy.pos.x+(x-8)*4),int(enemy.pos.y+(y-14)*4+std::sin(time*5)*2),4,4,c);
    }
    if(enemy.hp<3) {
        DrawRectangle(int(enemy.pos.x-21),int(enemy.pos.y-66),42,4,{25,24,39,255});
        DrawRectangle(int(enemy.pos.x-21),int(enemy.pos.y-66),14*std::max(0,enemy.hp),4,Purple);
    }
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
    for(Vector2 p:game.hearts) { p.y+=std::sin(clock*3)*3; heart(p,3,Pink); }
    if(!game.collected) {
        Vector2 p=game.bell; p.y+=std::sin(clock*3.5f)*4;
        DrawCircleV(p,35,{244,209,117,25});
        DrawRectangle(int(p.x-8),int(p.y-17),16,8,{255,242,177,255});
        DrawRectangle(int(p.x-13),int(p.y-10),26,25,{235,190,94,255});
        DrawRectangle(int(p.x-18),int(p.y+10),36,7,{255,226,151,255});
        DrawRectangle(int(p.x-4),int(p.y+18),8,6,{159,113,56,255});
        sparkle(add(p,{-30,-13}),{251,224,159,210},2);
    }
    struct Actor { float y; int type,index; };
    std::vector<Actor> actors;
    actors.push_back({game.player.y,0,0}); actors.push_back({game.companion.y,1,0});
    for(std::size_t i=0;i<game.enemies.size();++i) actors.push_back({game.enemies[i].pos.y,2,int(i)});
    if(game.stage==2) actors.push_back({295,3,0});
    std::sort(actors.begin(),actors.end(),[](Actor a,Actor b){return a.y<b.y;});
    for(auto actor:actors) {
        int step=1+int(clock*7)%2;
        if(actor.type==0) {
            if(game.invincible>0&&game.hurtTime<=0&&!game.preferences.reducedMotion&&int(clock*12)%2) continue;
            int pose=game.screen==Screen::Won?5:game.hurtTime>0?4:game.castTime>0?3:game.moving?step:0;
            if(game.dashTime>0&&!game.preferences.reducedMotion) {
                for(int n=1;n<4;++n) drawCharacter(art,0,step,sub(game.player,mul(game.facing,n*13.0f)),113,
                    game.facing.x<0,Color{150,218,255,static_cast<unsigned char>(65-n*13)});
            }
            drawCharacter(art,0,pose,game.player,113,game.facing.x<0);
        } else if(actor.type==1) {
            int pose=game.screen==Screen::Won?5:game.helpCastTime>0?3:game.companionMoving?step:0;
            drawCharacter(art,1,pose,game.companion,83,game.companionLeft);
        } else if(actor.type==3) {
            drawCharacter(art,2,game.bossDefeated?5:0,{1110,295},110);
            if(!game.bossDefeated) {
                DrawRectangle(1054,184,116,5,{174,150,195,255});
                for(int n=0;n<6;++n) {
                    DrawRectangle(1054+n*23,185,4,116,{106,85,133,255});
                    DrawRectangle(1054+n*23,185,1,116,{214,191,233,255});
                }
                DrawRectangle(1054,300,116,4,{112,90,141,255});
            }
        } else {
            const Enemy& enemy=game.enemies[actor.index];
            if(!enemy.boss) { fluff(enemy,clock); continue; }
            int pose=enemy.flash>0?4:enemy.timer<0.38f?3:step;
            drawCharacter(art,3,pose,enemy.pos,106,game.player.x<enemy.pos.x,enemy.flash>0?Pink:WHITE);
            if(!game.collected) for(int n=0;n<30;++n) {
                float angle=n*6.2831853f/30+clock*0.3f;
                pixel(add(enemy.pos,{std::cos(angle)*68,std::sin(angle)*68-42}),4,Purple);
            }
            // A growing rune announces the boss's next volley.
            if(enemy.timer<0.4f) {
                float radius=36+(0.4f-enemy.timer)*80;
                DrawCircleLines(int(enemy.pos.x),int(enemy.pos.y-40),radius,Fade(Purple,0.65f));
            }
        }
    }
    for(const auto& shot:game.shots) {
        Vector2 p=add(shot.pos,{0,-26}); Vector2 backward=mul(unit(shot.velocity),-1);
        for(int n=1;n<6;++n) pixel(add(p,mul(backward,n*7.0f)),std::max(2,6-n),Fade(Pink,0.6f-n*0.07f));
        heart(p,2.2f,Pink); sparkle(add(p,{9,-12}),{255,237,247,230},1);
    }
    for(const auto& shot:game.hazards) {
        Vector2 p=add(shot.pos,{0,-26}); Vector2 backward=mul(unit(shot.velocity),-1);
        for(int n=1;n<5;++n) pixel(add(p,mul(backward,n*8.0f)),8,Fade(Purple,0.4f-n*0.06f));
        DrawRectangle(int(p.x-7),int(p.y-12),14,24,{105,61,161,255});
        DrawRectangle(int(p.x-12),int(p.y-7),24,14,Purple);
        DrawRectangle(int(p.x-1),int(p.y-7),13,14,{40,28,60,255});
        sparkle(add(p,{-10,-15}),{224,183,255,230},1);
    }
    if(game.pulse>0) {
        float progress=1-game.pulse/0.55f;
        for(int n=0;n<28;++n) {
            float angle=n*6.2831853f/28;
            Vector2 p=add(game.player,{std::cos(angle)*230*progress,std::sin(angle)*230*progress-20});
            int size=5+n%3*2; DrawRectangleLinesEx({p.x-size/2,p.y-size/2,float(size),float(size)},2,Fade(Cyan,1-progress));
        }
    }
    for(const auto& p:game.particles) sparkle(p.pos,Fade(p.color,p.life/p.maxLife),2);
    if(game.stage==2&&game.bossDefeated&&distance(game.player,{1110,295})<112) {
        const char* label="[ E ] Rescatar a Giovanny";
        Vector2 size=MeasureTextEx(art.font,label,20,1);
        DrawRectangle(1090-int(size.x/2)-12,154,int(size.x)+24,33,{16,24,34,240});
        DrawTextEx(art.font,label,{1090-size.x/2,160},20,1,{199,225,166,255});
    }
}
}
