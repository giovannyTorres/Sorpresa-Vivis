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
// Draw the little creatures on a two-pixel lattice. Their outlines, fur and expressions
// share a palette, while each attack has a recognizable silhouette and anticipation.
constexpr Color EnemyInk{34,29,43,255}, EnemyCream{255,239,214,255};
struct CreaturePaint {
    Vector2 feet{};
    float sx{1},sy{1},unit{2},rotation{};
    Vector2 pivot{};
    bool flash{},mirror{};
    void r(float x,float y,float w,float h,Color color) const {
        float cx=x+w*.5f,cy=y+h*.5f;
        if(rotation!=0) {
            float dx=cx-pivot.x,dy=cy-pivot.y,c=std::cos(rotation),s=std::sin(rotation);
            cx=pivot.x+dx*c-dy*s;cy=pivot.y+dx*s+dy*c;
        }
        if(mirror)cx=-cx;
        float width=std::max(2.0f,std::round(w*unit*sx*.5f)*2),height=std::max(2.0f,std::round(h*unit*sy*.5f)*2);
        int left=int(std::round((feet.x+cx*unit*sx-width*.5f)*.5f))*2;
        int top=int(std::round((feet.y+cy*unit*sy-height*.5f)*.5f))*2;
        if(flash&&(color.r>72||color.g>72||color.b>82)) color={255,224,238,color.a};
        DrawRectangle(left,top,int(width),int(height),color);
    }
    void oval(float x,float y,float rx,float ry,Color color) const {
        for(int row=-int(ry);row<=int(ry);++row) {
            float t=float(row)/std::max(1.0f,ry);
            int half=int(std::round(rx*std::sqrt(std::max(0.0f,1-t*t))));
            if(half>0)r(x-half,y+row,half*2,1,color);
        }
    }
    void blob(float x,float y,float rx,float ry,Color body,Color outline=EnemyInk) const {
        oval(x,y,rx+1,ry+1,outline);oval(x,y,rx,ry,body);
    }
    void line(int x,int y,int ex,int ey,int width,Color color) const {
        int dx=std::abs(ex-x),stepX=x<ex?1:-1,dy=-std::abs(ey-y),stepY=y<ey?1:-1,error=dx+dy;
        for(;;) {
            r(float(x),float(y),float(width),float(width),color);
            if(x==ex&&y==ey)break;
            int twice=error*2;
            if(twice>=dy){error+=dy;x+=stepX;}
            if(twice<=dx){error+=dx;y+=stepY;}
        }
    }
};
void creatureShadow(Vector2 feet,float width,float height,float alpha=80) {
    CreaturePaint p;p.feet=feet;p.unit=1;
    p.oval(0,2,width,height,{5,10,18,static_cast<unsigned char>(alpha)});
}
void catEar(const CreaturePaint& p,int side,float top,float width,float height,Color fur,Color light,Color inner) {
    float x=side<0?-width-6:6;
    for(int row=0;row<int(height);++row) {
        float spread=std::min(width,float(row)*.65f+2);
        float left=side<0?x:x+width-spread;
        p.r(left,top+row,spread,1,EnemyInk);
        if(row>2&&spread>3)p.r(left+1,top+row+1,spread-2,1,fur);
        if(row>4&&row<height-2&&spread>5)p.r(left+2,top+row+1,spread-4,1,inner);
    }
    p.r(side<0?x+1:x+width-2,top+3,1,4,light);
}
void brightEyes(const CreaturePaint& p,float x,float y,float spacing,bool blink,Color iris,int look=0,bool narrow=false) {
    for(int side: {-1,1}) {
        float px=x+side*spacing;
        if(blink) {p.r(px-2,y+2,5,1,EnemyInk);p.r(px+side,y+1,2,1,EnemyInk);continue;}
        p.blob(px,y+2,3,3,EnemyCream);
        p.r(px-1+look,y,3,narrow?3:5,iris);p.r(px+look,y+1,1,3,EnemyInk);
        p.r(px-1+look,y,1,1,WHITE);
        if(narrow)p.r(px-3,y-1,6,2,EnemyInk);
    }
}
void kittyMuzzle(const CreaturePaint& p,float x,float y,Color muzzle,bool open=false) {
    p.oval(x-2,y+1,3,2,muzzle);p.oval(x+2,y+1,3,2,muzzle);
    p.r(x-1,y-1,3,2,{222,146,160,255});p.r(x,y+1,1,2,EnemyInk);
    if(open) {p.r(x-1,y+3,3,3,EnemyInk);p.r(x,y+4,2,1,Pink);}
    else {p.r(x-2,y+3,2,1,EnemyInk);p.r(x+1,y+3,2,1,EnemyInk);}
}
void kittenScarf(const CreaturePaint& p,float y,float flutter,Color scarf,bool moon=false) {
    p.r(-10,y,20,5,EnemyInk);p.r(-9,y+1,18,3,scarf);
    p.r(6,y+3,5,7,EnemyInk);p.r(7,y+4,3,5,scarf);
    p.r(9,y+5,6+flutter,3,scarf);p.r(12+flutter,y+7,3,3,scarf);
    p.r(-3,y+3,5,5,Gold);p.r(-2,y+4,3,3,{180,132,82,255});
    if(moon)p.r(0,y+3,3,3,scarf);else p.r(-1,y+5,1,2,EnemyInk);
}
void tinyPaws(const CreaturePaint& p,float y,float stride,Color fur,Color light,float spread=6) {
    for(int side: {-1,1}) {
        float py=y+(side<0?stride:-stride);
        p.blob(side*spread,py,4,2,fur);p.r(side*spread-2,py-1,4,1,light);
        p.r(side*spread-1,py+1,1,1,{170,129,145,255});
    }
}
void furryBall(const CreaturePaint& p,float x,float y,float radius,Color fur,Color light,Color shade,float time) {
    p.blob(x,y,radius,radius*.85f,fur);
    for(int i=0;i<10;i++) {
        float angle=i*.6283185f;
        float rx=x+std::cos(angle)*(radius-.3f),ry=y+std::sin(angle)*(radius*.83f-.2f);
        int shake=int(std::sin(time*3+i)*1.2f);
        p.r(rx-2,ry-2+shake,4,4,EnemyInk);p.r(rx-1,ry-1+shake,3,3,i%3?fur:shade);
    }
    p.oval(x-3,y-4,radius*.64f,radius*.33f,light);
    p.r(x-7,y-4,3,2,fur);p.r(x+3,y-5,4,2,fur);
    for(int i=0;i<5;i++) {
        int px=int(x)-8+i*4,py=int(y)+4+(i%2)*2;
        p.line(px,py,px+3,py-2,1,shade);
    }
}
void fluffCreature(const CreaturePaint& p,float time,bool blink,bool charging) {
    const Color fur{179,151,171,255},light{224,205,212,255},shade{129,104,139,255};
    furryBall(p,0,-15,13,fur,light,shade,time);
    for(int side: {-1,1}) {
        float flap=std::sin(time*7+side)*1.4f;
        p.blob(side*15,-11+flap,3,3,light);
        p.r(side*12-1,-28,3,4,shade);p.r(side*12-2,-30,3,3,fur);
    }
    p.line(-2,-27,1,-31,1,EnemyInk);p.line(1,-31,4,-31,1,EnemyInk);p.r(4,-30,1,3,shade);
    brightEyes(p,0,-19,5,blink,{205,169,88,255},0,charging);
    p.r(-10,-13,3,1,Pink);p.r(7,-13,3,1,Pink);
    p.r(-3,-9,2,1,EnemyInk);p.r(-1,-8,4,1,EnemyInk);p.r(3,-9,1,1,EnemyInk);
    p.r(1,-7,2,1,EnemyCream);
    p.r(-5,-2,3,2,shade);p.r(3,-2,3,2,shade);
}
void hopperCreature(const CreaturePaint& p,float time,bool blink,bool charging,bool jumping) {
    const Color fur{172,158,134,255},light{219,211,182,255},shade{118,119,108,255};
    furryBall(p,0,-14,13,fur,light,shade,time);
    p.line(-8,-21,-4,-23,1,shade);p.line(4,-20,8,-19,1,shade);
    p.line(-10,-10,-7,-8,1,light);p.line(6,-8,9,-10,1,light);
    p.line(0,-25,3,-29,1,EnemyInk);p.line(3,-29,7,-28,1,EnemyInk);
    p.line(6,-27,3,-26,1,shade);
    brightEyes(p,0,-18,5,blink&&!jumping,{152,174,121,255},0,charging);
    p.r(-10,-13,2,1,{213,168,155,255});p.r(8,-13,2,1,{213,168,155,255});
    p.r(-3,-9,6,2,EnemyInk);p.r(-1,-8,3,3,{229,157,158,255});
    // A small celadon strand makes its hairball origin clear without anything graphic.
    p.r(3,-8,2,6,{151,181,132,255});p.r(3,-2,3,2,{191,209,157,255});
    if(jumping) {p.r(-7,-1,4,2,shade);p.r(4,-1,4,2,shade);}
    else tinyPaws(p,-1,0,fur,light,6);
}
void babyWhisker(const CreaturePaint& p,float time,bool blink,bool charging,float stride) {
    const Color fur{216,168,115,255},light{250,220,177,255},shade{163,118,92,255};
    p.line(8,-8,18,-8,4,EnemyInk);p.line(18,-8,20,-15,4,EnemyInk);
    p.line(8,-8,18,-8,2,fur);p.line(18,-8,20,-15,2,fur);p.r(18,-18,4,4,light);
    p.blob(0,-10,9,10,fur);p.oval(0,-9,6,7,light);
    tinyPaws(p,-1,stride,fur,light,6);
    for(int side: {-1,1})p.blob(side*9,-11+(side<0?stride:-stride),3,4,fur);
    catEar(p,-1,-34,8,12,fur,light,{230,163,164,255});
    catEar(p,1,-34,8,12,fur,light,{230,163,164,255});
    p.blob(0,-23,13,10,fur);p.oval(-2,-26,9,4,light);
    p.r(-14,-21,3,4,fur);p.r(11,-21,3,4,fur);
    p.r(-2,-32,3,4,shade);p.r(-9,-28,4,2,shade);p.r(6,-28,4,2,shade);
    brightEyes(p,0,-25,5,blink,{114,171,199,255},charging?1:0,charging);
    kittyMuzzle(p,0,-19,light,charging);
    for(int side: {-1,1}) {
        p.line(side*8,-19,side*16,-21,1,light);
        p.line(side*9,-17,side*16,-16,1,light);
    }
    kittenScarf(p,-14,std::sin(time*6)*2,{162,119,190,255});
}
void stalkingKitten(const CreaturePaint& p,float time,bool blink,bool charging,float stride,bool pouncing) {
    const Color fur{122,111,132,255},light{193,171,179,255},shade{83,77,106,255};
    int tail=int(std::sin(time*5)*(charging?3:1));
    p.line(7,-7,17,-9,4,EnemyInk);p.line(17,-9,20,-20+tail,4,EnemyInk);
    p.line(7,-7,17,-9,2,fur);p.line(17,-9,20,-20+tail,2,fur);p.r(18,-23+tail,4,4,light);
    p.blob(0,-10,9,10,fur);p.oval(-1,-9,6,7,light);
    tinyPaws(p,-1,pouncing?stride*2:stride,fur,light,7);
    p.blob(-9,-12+stride,3,4,fur);p.blob(9,-12-stride,3,4,fur);
    catEar(p,-1,-43,10,22,fur,light,{186,139,162,255});
    catEar(p,1,-43,10,22,fur,light,{186,139,162,255});
    p.blob(0,-22,11,9,fur);p.oval(-2,-25,7,3,light);
    p.r(-1,-31,3,5,shade);p.r(-9,-27,3,2,shade);p.r(6,-27,3,2,shade);
    brightEyes(p,0,-24,5,blink,{227,186,108,255},1,charging||pouncing);
    kittyMuzzle(p,0,-18,light,false);
    for(int side: {-1,1}) {p.line(side*8,-18,side*13,-19,1,light);p.line(side*8,-16,side*14,-15,1,light);}
    p.r(-7,-12,14,3,{109,83,144,255});p.r(3,-10,4,4,{182,142,198,255});
}
void fluffyMoth(const CreaturePaint& p,float time,bool blink,bool charging) {
    float open=charging?1.0f:.25f+.75f*(.5f+.5f*std::sin(time*12));
    const Color wing{166,143,190,255},cream{225,204,211,255},edge{104,90,145,255};
    for(int side: {-1,1}) {
        float x=side*(9+open*4),radius=5+open*8;
        p.blob(x,-19,radius,13,wing);p.oval(x-side*2,-22,radius*.67f,8,cream);
        p.blob(x-side*1,-6,radius*.8f,7,edge);p.oval(x-side*1,-8,radius*.55f,4,wing);
        p.line(side*6,-21,side*int(12+open*9),-27,1,edge);
        p.line(side*6,-17,side*int(14+open*8),-14,1,edge);
        if(open>.5f) {p.blob(side*(14+open*4),-20,3,4,edge);p.r(side*(14+open*4)-1,-22,2,3,cream);}
    }
    p.blob(0,-15,5,13,{194,177,196,255});p.oval(-1,-17,3,9,EnemyCream);
    p.r(-6,-9,3,2,cream);p.r(4,-9,3,2,cream);
    p.blob(0,-27,7,7,cream);
    p.line(-3,-32,-6,-39,1,EnemyInk);p.line(3,-32,6,-39,1,EnemyInk);
    p.blob(-7,-40,2,2,wing);p.blob(7,-40,2,2,wing);
    brightEyes(p,0,-29,3,blink,{145,112,173,255},0,charging);
    p.r(-1,-22,2,1,{143,104,144,255});
    p.line(-3,-6,-5,-2,1,EnemyInk);p.line(3,-6,5,-2,1,EnemyInk);
}
void crinkleHairball(const CreaturePaint& p,float time,bool blink,bool charging) {
    const Color fur{139,163,103,255},light{206,220,155,255},shade{91,118,83,255};
    furryBall(p,0,-13,14,fur,light,shade,time);
    for(int i=0;i<5;i++) {
        int x=-10+i*5;
        p.line(x,-24,x-2,-28-(i%2)*3,1,EnemyInk);
        p.line(x-1,-24,x-3,-27-(i%2)*2,1,shade);
    }
    p.r(-13,-14,3,6,{161,186,120,255});p.r(10,-11,3,7,{161,186,120,255});
    p.r(11,-5,2,6,light);p.r(10,0,4,2,fur);
    if(blink||charging) {
        for(int side: {-1,1}) {p.line(side*7,-18,side*4,-16,1,EnemyInk);p.line(side*4,-16,side*7,-14,1,EnemyInk);}
    } else {
        p.line(-8,-18,-4,-16,1,EnemyInk);p.line(-4,-16,-8,-14,1,EnemyInk);
        p.line(8,-18,4,-16,1,EnemyInk);p.line(4,-16,8,-14,1,EnemyInk);
        p.r(-9,-14,1,2,EnemyCream);p.r(9,-14,1,2,EnemyCream);
    }
    p.r(-11,-12,3,1,{207,169,132,255});p.r(8,-12,3,1,{207,169,132,255});
    if(charging) {p.blob(0,-8,4,3,EnemyInk);p.r(-1,-7,3,1,{189,201,139,255});}
    else {p.r(-4,-8,3,1,EnemyInk);p.r(-1,-7,3,1,EnemyInk);p.r(2,-8,3,1,EnemyInk);}
    p.r(-3,-5,2,3,{179,201,131,255});p.r(-2,-2,3,2,light);
    p.r(-8,-1,4,2,shade);p.r(5,-1,4,2,shade);
}
void moonApprentice(const CreaturePaint& p,float time,bool blink,bool charging,float stride) {
    const Color fur{193,176,157,255},light{239,221,192,255},mask{92,77,110,255},scarf{139,92,183,255};
    p.line(-6,-7,-16,-9,4,EnemyInk);p.line(-16,-9,-18,-17,4,EnemyInk);
    p.line(-6,-7,-16,-9,2,mask);p.line(-16,-9,-18,-17,2,mask);
    float cape=std::sin(time*5)*2;
    p.r(-12,-17,24,16,EnemyInk);p.r(-13,-11,26,10,EnemyInk);
    p.r(-11,-16,22,14,{88,64,128,255});p.r(-10,-12,8,12,{119,83,155,255});
    p.r(-14,-2,7,3,scarf);p.r(6+cape,-2,8,3,scarf);
    tinyPaws(p,-1,stride,mask,fur,6);
    catEar(p,-1,-34,8,12,fur,light,{190,151,167,255});catEar(p,1,-34,8,12,fur,light,{190,151,167,255});
    p.blob(0,-23,11,10,fur);p.oval(-2,-27,8,4,light);
    p.oval(0,-21,8,7,mask);p.oval(0,-17,4,3,fur);
    brightEyes(p,0,-24,4,blink,{109,189,219,255},0,charging);
    kittyMuzzle(p,0,-18,fur,charging);
    kittenScarf(p,-14,std::sin(time*6)*2,scarf,true);
    float lift=charging?-5:0;
    p.blob(11,-10+lift,3,3,fur);p.line(14,-8+int(lift),14,-27+int(lift),1,{146,115,92,255});
    p.blob(14,-29+lift,4,4,{192,157,225,255});
    p.oval(16,-30+lift,3,3,mask);p.r(12,-31+lift,2,2,EnemyCream);
    if(charging) {
        p.r(20,-36,1,5,Purple);p.r(18,-34,5,1,Purple);
        p.r(-19,-29,1,3,Gold);p.r(-20,-28,3,1,Gold);
    }
}
void rollingHairball(const CreaturePaint& p,float time,bool blink,bool charging,bool rolling,float direction) {
    const Color fur{171,139,124,255},light{222,190,160,255},shade{116,95,112,255};
    p.blob(0,-14,14,13,fur);p.oval(-3,-19,10,5,light);
    CreaturePaint spin=p;spin.pivot={0,-14};
    spin.rotation=rolling?time*17*direction:std::sin(time*2)*.12f;
    for(int i=0;i<8;i++) {
        float angle=i*.785398f;
        float x=std::cos(angle)*13,y=-14+std::sin(angle)*12;
        spin.r(x-2,y-2,4,4,EnemyInk);spin.r(x-1,y-1,3,3,i%3?fur:shade);
    }
    spin.line(-12,-15,-4,-22,1,shade);spin.line(-4,-22,5,-21,1,shade);
    spin.line(-11,-11,-3,-18,1,shade);spin.line(6,-12,12,-17,1,shade);
    spin.line(-9,-5,-3,-10,1,light);spin.line(6,-8,11,-11,1,light);
    brightEyes(spin,0,-16,5,blink,{180,151,184,255},0,charging);
    spin.r(-2,-7,5,2,EnemyInk);spin.r(-1,-6,3,1,Pink);
    if(!rolling) {p.r(-8,-1,5,2,shade);p.r(4,-1,5,2,shade);}
}
void littleEnemy(const Enemy& enemy,float clock,bool reduced=false) {
    float time=reduced?0:enemy.age;
    float speed=length(enemy.velocity),stride=speed>15?std::sin(time*(speed>190?18:10))*1.7f:0;
    bool charging=enemy.windup>0,active=enemy.cooldown>0;
    bool blink=!charging&&std::fmod(time+int(enemy.kind)*.57f,4.7f)>4.54f;
    float bounce=reduced?0:std::sin(time*5+int(enemy.kind))*.8f;
    float lift=0,sx=1,sy=1;
    switch(enemy.kind) {
        case EnemyKind::Fluff:lift=4+bounce*2;break;
        case EnemyKind::Hopper:
            if(active) {
                float jump=std::sin(std::clamp(1-enemy.cooldown/.52f,0.0f,1.0f)*3.14159265f);
                lift=reduced?0:jump*42;sx=1-jump*.11f;sy=1+jump*.07f;
            } else if(charging) {sx=1.19f;sy=.73f;}
            else if(enemy.timer>1.5f) {sx=1.13f;sy=.83f;}
            break;
        case EnemyKind::Pouncer:if(charging||active){sx=active?1.13f:1.08f;sy=active?.80f:.76f;}break;
        case EnemyKind::Moth:lift=reduced?12:15+std::sin(time*5)*5;break;
        case EnemyKind::Spore:if(charging){sx=1.11f;sy=1.08f;}break;
        case EnemyKind::Charger:if(charging){sx=1.10f;sy=.88f;}break;
        default:if(charging){sx=1.04f;sy=.93f;}break;
    }
    if(reduced){sx=1;sy=1;stride=0;}
    creatureShadow(enemy.pos,enemy.kind==EnemyKind::Moth?25:27-lift*.13f,enemy.kind==EnemyKind::Moth?6:7,enemy.kind==EnemyKind::Moth?55:83-lift*.55f);
    CreaturePaint p;p.feet=add(enemy.pos,{0,bounce-lift});p.sx=sx;p.sy=sy;p.flash=enemy.flash>0;
    p.mirror=enemy.kind==EnemyKind::WhiskerShooter||enemy.kind==EnemyKind::Pouncer||enemy.kind==EnemyKind::Mirror;
    p.mirror=p.mirror&&(std::abs(enemy.velocity.x)>8?enemy.velocity.x<0:enemy.target.x<enemy.pos.x);
    if(active&&(enemy.kind==EnemyKind::Pouncer||enemy.kind==EnemyKind::Charger)&&!reduced) {
        Vector2 back=mul(unit(enemy.velocity),-1);
        for(int i=1;i<4;i++) {
            CreaturePaint dust;dust.feet=add(enemy.pos,mul(back,i*13.f));
            dust.oval(0,0,5-i,2,{185,165,161,static_cast<unsigned char>(66-i*13)});
        }
    }
    switch(enemy.kind) {
        case EnemyKind::Hopper:hopperCreature(p,time,blink,charging,active);break;
        case EnemyKind::WhiskerShooter:babyWhisker(p,time,blink,charging,stride);break;
        case EnemyKind::Pouncer:stalkingKitten(p,time,blink,charging,stride,active);break;
        case EnemyKind::Moth:fluffyMoth(p,time,blink,charging);break;
        case EnemyKind::Spore:crinkleHairball(p,time,blink,charging);break;
        case EnemyKind::Mirror:moonApprentice(p,time,blink,charging,stride);break;
        case EnemyKind::Charger:rollingHairball(p,time,blink,charging,active&&!reduced,enemy.velocity.x<0?-1.f:1.f);break;
        default:fluffCreature(p,time,blink,charging);break;
    }
    if(enemy.poisonLeft>0) {
        for(int n=0;n<3;++n) {
            float phase=std::fmod(clock*.7f+n*.31f,1.0f);
            pixel(add(p.feet,{float(n*19-20),-17-phase*45}),3,Fade(Green,1-phase));
        }
        pixelRing(enemy.pos,24,Fade(Green,.4f),2,18);
    }
    if(enemy.hp<enemy.maxHp&&enemy.maxHp>0) {
        float top=enemy.kind==EnemyKind::Moth?98:enemy.kind==EnemyKind::Pouncer?87:82;
        DrawRectangle(int(p.feet.x-25),int(p.feet.y-top),50,5,{25,24,39,255});
        DrawRectangle(int(p.feet.x-24),int(p.feet.y-top+1),int(48*std::clamp(enemy.hp/enemy.maxHp,0.0f,1.0f)),3,enemy.poisonLeft>0?Green:Purple);
    }
}
void plushBoss(const Enemy& enemy,float clock,bool reduced=false) {
    bool pillow=enemy.kind==EnemyKind::PillowBoss,charging=enemy.windup>0,moving=length(enemy.velocity)>15;
    float time=reduced?0:enemy.age,stride=moving&&!reduced?std::sin(time*10)*1.5f:0;
    bool blink=!charging&&std::fmod(time+1.4f,5.1f)>4.94f;
    CreaturePaint p;p.unit=4;p.feet=add(enemy.pos,{0,reduced?0:std::sin(time*2.7f)*2});p.flash=enemy.flash>0;
    if(charging&&!reduced){p.sx=1.06f;p.sy=.94f;}
    if(enemy.cooldown>0&&!reduced){p.sx=1.10f;p.sy=.88f;}
    creatureShadow(enemy.pos,pillow?66:69,13,100);
    if(pillow) {
        const Color fur{213,177,199,255},light{250,218,224,255},shade{170,128,164,255};
        for(int side: {-1,1}) {
            float gesture=charging?-4:std::sin(time*3+side)*1.5f;
            p.blob(side*20,-15+gesture,4,7,fur);p.r(side*20-1,-18+gesture,2,5,light);
            p.r(side*18-2,-29,4,5,shade);p.r(side*18-2,-5,4,5,shade);
            p.r(side*20-1,-31,3,3,light);p.r(side*20-1,-2,3,3,light);
        }
        p.r(-15,-30,30,29,EnemyInk);p.r(-18,-27,36,23,EnemyInk);p.r(-19,-23,38,15,EnemyInk);
        p.r(-14,-29,28,27,fur);p.r(-17,-26,34,21,fur);p.r(-18,-22,36,13,fur);
        p.r(-13,-28,26,3,light);p.r(-16,-25,3,15,light);p.r(-13,-5,27,2,shade);
        for(int stitch=-12;stitch<=12;stitch+=4) {p.r(stitch,-24,2,1,shade);p.r(stitch,-7,2,1,light);}
        for(int side: {-1,1}) {p.r(side*14,-20,1,3,shade);p.r(side*14,-13,1,3,shade);}
        tinyPaws(p,-1,stride,fur,light,10);
        p.r(-7,-35,14,5,EnemyInk);p.r(-6,-35,12,4,Gold);
        for(int col=-1;col<=1;col++) {p.r(col*5-1,-40+(col?2:0),3,7,Gold);p.r(col*5,-40+(col?2:0),1,2,EnemyCream);}
        p.r(-1,-34,3,3,Pink);
        brightEyes(p,0,-19,7,blink,{128,97,138,255},0,charging);
        p.r(-12,-13,4,1,{231,141,173,255});p.r(9,-13,4,1,{231,141,173,255});
        kittyMuzzle(p,0,-11,light,charging);
        heart(add(p.feet,{0,-93*p.sy}),1.35f,Pink);
    } else {
        const Color fur{215,169,115,255},light{250,224,179,255},shade{161,118,91,255};
        p.line(11,-8,23,-10,5,EnemyInk);p.line(23,-10,25,-21,5,EnemyInk);
        p.line(11,-8,23,-10,3,fur);p.line(23,-10,25,-21,3,fur);p.r(23,-24,5,5,light);
        p.blob(0,-12,13,13,fur);p.oval(0,-10,9,9,light);tinyPaws(p,-1,stride,fur,light,10);
        for(int side: {-1,1})p.blob(side*13,-11+(side<0?stride:-stride),4,6,fur);
        catEar(p,-1,-39,9,14,fur,light,{214,151,153,255});catEar(p,1,-39,9,14,fur,light,{214,151,153,255});
        p.blob(0,-24,16,12,fur);p.oval(-3,-29,11,4,light);
        p.r(-2,-34,4,5,shade);p.r(-12,-29,5,2,shade);p.r(8,-29,5,2,shade);
        brightEyes(p,0,-26,7,blink,{145,162,112,255},0,charging);
        for(int side: {-1,1}) {
            p.line(side*3,-29,side*12,-29,1,light);
            for(int row=-1;row<=1;row++) {
                int end=30+(charging?3:0),wobble=int(std::sin(time*3+row)*1.5f);
                p.line(side*10,-20+row*2,side*end,-19+row*7+wobble,2,EnemyInk);
                p.line(side*10,-21+row*2,side*end,-20+row*7+wobble,1,Gold);
                p.r(side*end-1,-22+row*7+wobble,2,2,light);
            }
        }
        kittyMuzzle(p,0,-18,light,charging);
        p.r(-9,-12,18,4,{136,99,159,255});p.r(-3,-9,6,5,Gold);p.r(-1,-8,2,3,{183,135,76,255});
        p.r(-7,-34,14,2,Gold);p.r(-2,-37,5,5,Gold);p.r(0,-37,3,3,fur);
    }
    if(enemy.flash>0)pixelRing(add(p.feet,{0,-55}),66,Fade(Pink,.8f),3,26);
    if(enemy.poisonLeft>0)pixelRing(enemy.pos,62,Fade(Green,.55f),3,30);
    (void)clock;
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
    Vector2 p=shot.pos,back=mul(unit(shot.velocity),-1);
    Color color=shotColor(shot.kind);
    for(int n=1;n<5;++n)pixel(add(p,mul(back,n*7.0f)),std::max(2,6-n),Fade(color,.4f-n*.06f));
    CreaturePaint paint;paint.feet=p;
    switch(shot.kind) {
    case ShotKind::Heart:heart(p,2.0f,color);break;
    case ShotKind::Venom:
        paint.blob(0,0,4,4,color,{62,98,54,255});
        paint.oval(-1,-1,2,2,{205,244,161,255});paint.r(-1,-2,1,1,EnemyCream);
        paint.r(3,3,1,2,color);paint.r(-3,4,2,1,{111,176,86,255});
        break;
    case ShotKind::Digital:
        paint.r(-4,-4,8,1,Cyan);paint.r(-4,3,8,1,Cyan);
        paint.r(-4,-3,1,6,Cyan);paint.r(3,-3,1,6,Cyan);
        paint.r(-1,-1,3,3,WHITE);paint.r(-6,2,2,2,Cyan);paint.r(4,-5,2,2,Cyan);
        break;
    case ShotKind::Whisker:
        paint.rotation=std::atan2(shot.velocity.y,shot.velocity.x);
        paint.line(-7,1,-3,-1,2,{132,94,67,255});paint.line(-3,-1,3,-1,2,{132,94,67,255});
        paint.line(3,-1,7,-3,2,{132,94,67,255});
        paint.line(-7,0,-3,-2,1,Gold);paint.line(-3,-2,3,-2,1,EnemyCream);
        paint.line(3,-2,7,-4,1,Gold);paint.r(-8,-1,2,1,Gold);
        break;
    case ShotKind::Moon: {
        float radius=std::max(3.0f,shot.radius*.5f);
        Color shade{62,41,88,255};
        paint.blob(0,0,radius,radius,color,shade);
        paint.oval(2,-1,radius-.5f,radius-.5f,shade);
        paint.r(-4,-4,1,2,{234,211,255,255});paint.r(-5,-3,3,1,{234,211,255,255});
        break;
    }
    case ShotKind::Spore:
        // A fleck of fluff suspended in a cute, olive-colored sneeze.
        paint.blob(0,0,4,3,{174,194,116,255},{78,101,58,255});
        paint.oval(-1,-1,2,1,{225,228,164,255});paint.r(-3,-4,1,2,{114,115,81,255});
        paint.r(1,-5,1,2,{114,115,81,255});paint.r(2,-4,2,1,{114,115,81,255});
        paint.r(3,2,1,3,{152,183,111,255});paint.r(-2,0,1,1,{107,133,77,255});
        paint.r(1,1,1,1,{107,133,77,255});
        break;
    case ShotKind::Fluff:
        paint.blob(0,0,4,4,{225,194,209,255},{133,105,141,255});
        paint.rotation=clock*4;
        paint.r(-5,-2,2,2,{166,133,163,255});paint.r(2,-5,2,2,{225,194,209,255});
        paint.r(3,3,2,2,{166,133,163,255});paint.r(-3,3,2,2,{225,194,209,255});
        paint.line(-2,-1,0,-2,1,{251,225,226,255});paint.line(0,-2,2,0,1,{251,225,226,255});
        paint.r(-1,1,2,1,{166,133,163,255});
        break;
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
            if(!enemy.boss) {littleEnemy(enemy,clock,game.preferences.reducedMotion);continue;}
            int pose=enemy.flash>0?4:enemy.windup>0?3:enemy.cooldown>0?step:0;
            if(enemy.kind==EnemyKind::PillowBoss||enemy.kind==EnemyKind::WhiskerBoss)plushBoss(enemy,clock,game.preferences.reducedMotion);
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
