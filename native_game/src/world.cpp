#include "visuals.hpp"

#include <array>
#include <cstdint>

namespace vivis {
namespace {

Color rgb(unsigned hex, unsigned char alpha=255) {
    return {static_cast<unsigned char>(hex>>16),static_cast<unsigned char>(hex>>8),
            static_cast<unsigned char>(hex),alpha};
}
Color opacity(Color color, unsigned char alpha) { color.a=alpha; return color; }
struct Palette {
    Color back, soil, shade, moss, grass, light, leaf, leafLight, leafShade, bark,
          barkLight, path, pathShade, pathLight, stone, stoneLight, flower, glow;
};
Palette palette(int stage) {
    if(stage==0) return {rgb(0x0e1b23),rgb(0x223b34),rgb(0x182c2b),rgb(0x2b483a),
        rgb(0x3b5942),rgb(0x6a8652),rgb(0x39684b),rgb(0x61875a),rgb(0x244b3c),
        rgb(0x5c4434),rgb(0xa17a4e),rgb(0x655641),rgb(0x4b4436),rgb(0x8b7755),
        rgb(0x59675d),rgb(0x89927a),rgb(0xe491b8),rgb(0xdadd95)};
    if(stage==1) return {rgb(0x111b28),rgb(0x293b40),rgb(0x1e2c35),rgb(0x30474a),
        rgb(0x3f5757),rgb(0x698474),rgb(0x34564f),rgb(0x5b786b),rgb(0x213d3e),
        rgb(0x4f4544),rgb(0x837064),rgb(0x51515b),rgb(0x393d48),rgb(0x74737a),
        rgb(0x60616d),rgb(0x919098),rgb(0xb394cf),rgb(0x9ce0df)};
    if(stage==2) return {rgb(0x101a28),rgb(0x202f3c),rgb(0x172430),rgb(0x2a3949),
        rgb(0x3e5264),rgb(0x6b9aab),rgb(0x29465b),rgb(0x567c88),rgb(0x1c3445),
        rgb(0x3a4052),rgb(0x7292a7),rgb(0x314658),rgb(0x233441),rgb(0x618195),
        rgb(0x435567),rgb(0x7792a4),rgb(0x79c8ee),rgb(0x98ded5)};
    return {rgb(0x131421),rgb(0x302d43),rgb(0x252334),rgb(0x39334b),
        rgb(0x494057),rgb(0x74607e),rgb(0x44324e),rgb(0x775576),rgb(0x30253b),
        rgb(0x524051),rgb(0x9d7c91),rgb(0x514457),rgb(0x342c42),rgb(0x80637e),
        rgb(0x53465f),rgb(0x8b7397),rgb(0xcb91d9),rgb(0xd0b2fa)};
}
const Color Ink=rgb(0x0c1520);

uint32_t hash(int x,int y,int seed=0) {
    uint32_t value=uint32_t(x)*73856093u ^ uint32_t(y)*19349663u ^ uint32_t(seed)*83492791u;
    value^=value>>13; value*=1274126177u; return value^(value>>16);
}
int number(int x,int y,int seed,int bound) { return int(hash(x,y,seed)%uint32_t(bound)); }
void rect(float x,float y,float width,float height,Color color) {
    if(width>0&&height>0) DrawRectangle(int(std::floor(x)),int(std::floor(y)),int(std::ceil(width)),int(std::ceil(height)),color);
}
void rect(Rectangle area,Color color) { rect(area.x,area.y,area.width,area.height,color); }
// Every silhouette is made from short horizontal runs. No filtering or rounded primitives.
void oval(float x,float y,float rx,float ry,Color color,int step=4) {
    for(int offset=-int(ry);offset<=int(ry);offset+=step) {
        float unit=float(offset)/std::max(ry,1.0f);
        int half=int(rx*std::sqrt(std::max(0.0f,1.0f-unit*unit)))/step*step;
        if(half) rect(x-half,y+offset,half*2,step,color);
    }
}
void softLight(Vector2 point,float radius,Color color) {
    oval(point.x,point.y,radius,radius*.70f,opacity(color,5),6);
    oval(point.x,point.y,radius*.72f,radius*.50f,opacity(color,8),4);
    oval(point.x,point.y,radius*.42f,radius*.30f,opacity(color,12),4);
}
void diamond(float x,float y,int radius,Color color,int step=3) {
    for(int offset=-radius;offset<=radius;offset+=step) {
        int half=radius-std::abs(offset);
        rect(x-half,y+offset,half*2+step,step,color);
    }
}
void tuft(float x,float y,const Palette& p,int seed=0) {
    rect(x-4,y+1,16,3,p.shade);
    rect(x,y-7,3,10,p.grass); rect(x-3,y-10,3,5,p.grass);
    rect(x+5,y-13-number(seed,0,3,4),3,16,p.light);
    rect(x+8,y-8,3,10,p.grass); rect(x+10,y-10,3,4,p.grass);
}
void flower(float x,float y,Color petals,const Palette& p,int seed=0) {
    rect(x+1,y-4,2,11,p.leafShade);
    rect(x+3,y+1,5,2,p.grass); rect(x-3,y-2,5,2,p.grass);
    rect(x-3,y-10,10,5,petals); rect(x,y-13,4,11,petals);
    rect(x,y-9,4,3,rgb(0xf2db9f)); rect(x-2,y-11,3,2,opacity(WHITE,80));
    if(seed%3==0) {
        rect(x+9,y-4,5,3,petals); rect(x+10,y-6,3,7,petals);
        rect(x+11,y-4,2,2,rgb(0xf2db9f));
    }
}
void stone(float x,float y,int size,const Palette& p,int seed=0) {
    int w=size*7,h=size*4;
    oval(x,y+1,w+3,4,opacity(Ink,100),2);
    rect(x-w+size,y-h,w*2-size*2,h,Ink); rect(x-w,y-h+size,w*2,h-size,Ink);
    rect(x-w+size,y-h+size,w*2-size*2,h-size,p.stone);
    rect(x-w+size*2,y-h+size,w-size,size,p.stoneLight);
    rect(x+size,y-h+size*2,w-size*2,h-size*2,p.shade);
    if(seed%2==0) rect(x-w+size,y-2,w-size,2,p.grass);
}
void mushrooms(float x,float y,const Palette& p) {
    for(int i=0;i<3;i++) {
        float px=x+i*9,py=y+(i%2)*4;
        rect(px+2,py-4,3,10,rgb(0xb4aa8d));
        rect(px-2,py-9,11,5,p.flower); rect(px+1,py-12,5,3,p.flower);
        rect(px,py-9,3,2,rgb(0xf3dfc0)); rect(px+5,py-7,2,2,rgb(0xf3dfc0));
        rect(px-2,py-4,11,2,p.leafShade);
    }
}

void oak(float x,float feet,float scale,const Palette& p,int seed) {
    float s=scale;
    oval(x+5*s,feet,49*s,9*s,opacity(Ink,130),4);
    rect(x-11*s,feet-57*s,22*s,57*s,Ink);
    rect(x-8*s,feet-55*s,16*s,54*s,p.bark);
    rect(x-6*s,feet-52*s,4*s,48*s,p.barkLight);
    rect(x+2*s,feet-39*s,3*s,34*s,rgb(0x392d2a));
    rect(x-18*s,feet-4*s,15*s,5*s,p.bark); rect(x+6*s,feet-6*s,15*s,7*s,p.bark);
    rect(x-10*s,feet-15*s,5*s,5*s,p.leafShade);
    struct Cluster { int x,y,rx,ry; };
    const std::array<Cluster,7> clusters{{{-27,-75,31,25},{-37,-95,29,27},{-15,-115,35,24},
        {16,-117,32,25},{38,-95,30,29},{25,-71,35,25},{-3,-85,43,32}}};
    for(const auto& c:clusters) oval(x+c.x*s,feet+c.y*s,c.rx*s+3,c.ry*s+3,Ink,4);
    for(size_t i=0;i<clusters.size();i++) {
        const auto& c=clusters[i];
        Color fill=i%3==0?p.leafShade:p.leaf;
        oval(x+c.x*s,feet+c.y*s,c.rx*s,c.ry*s,fill,4);
        oval(x+(c.x-7)*s,feet+(c.y-8)*s,c.rx*.58f*s,c.ry*.37f*s,p.leafLight,4);
        rect(x+(c.x-13)*s,feet+(c.y-7)*s,8*s,3*s,p.leaf);
    }
    for(int i=0;i<30;i++) {
        float px=x+(number(i,seed,2,103)-51)*s;
        float py=feet+(-120+number(i,seed,6,64))*s;
        if(distance({px,py},{x,feet-94*s})<53*s)
            rect(px,py,3*s+number(i,seed,8,3),3*s,i%3==0?p.leafLight:p.leafShade);
    }
}
void pine(float x,float feet,float scale,const Palette& p,int seed) {
    float s=scale;
    oval(x+5*s,feet,40*s,7*s,opacity(Ink,130),3);
    rect(x-7*s,feet-44*s,14*s,44*s,Ink);
    rect(x-4*s,feet-45*s,8*s,44*s,p.bark); rect(x-3*s,feet-42*s,2*s,40*s,p.barkLight);
    for(int layer=4;layer>=0;layer--) {
        int half=14+layer*10;
        float y=feet-(115-layer*18)*s;
        for(int row=0;row<7;row++) {
            float width=(half*(row+2)/8)*s;
            rect(x-width-3,y+row*4*s,width*2+6,5*s,Ink);
        }
        for(int row=0;row<7;row++) {
            float width=(half*(row+2)/8)*s;
            rect(x-width,y+row*4*s,width*2,4*s,row>4?p.leafShade:p.leaf);
            if(row>0&&row<5) rect(x-width+4*s,y+row*4*s,width*.55f,3*s,p.leafLight);
        }
        for(int i=0;i<4;i++) {
            float px=x+(number(i,layer,seed,half)-half/2)*s;
            rect(px,y+(11+i*4)*s,5*s,3*s,p.leafShade);
        }
    }
}

void treeBorders(const Palette& p,int stage) {
    // Viewed from under the trees: a layered canopy, rather than truncated trunks.
    rect(0,145,1280,17,stage?rgb(0x1b3039):rgb(0x19312d));
    for(int i=0;i<13;i++) {
        float x=5+i*108,y=142+(i%3)*3;
        oval(x+8,y+5,70,38,Ink,4);
        oval(x+5,y,68,38,p.leafShade,4);
        oval(x-13,y-3,49,34,p.leaf,4);
        oval(x-26,y+6,26,10,p.leafLight,4);
        oval(x+28,y+12,23,12,p.leaf,4);
        for(int leaf=0;leaf<8;leaf++) {
            float px=x-42+number(leaf,i,stage+5,88),py=y+7+number(leaf,i,3,20);
            rect(px,py,5+number(leaf,i,8,5),3,leaf%3?p.leafShade:p.leafLight);
        }
        if(i%3==1) {
            rect(x+37,y+24,3,15,p.leafShade); rect(x+35,y+36,6,4,p.leaf);
        }
    }
    for(int i=0;i<5;i++) {
        float feet=307+i*109;
        oak(10,feet,.88f,p,i+34); oak(1270,feet+12,.88f,p,i+40);
    }
    for(int i=0;i<12;i++) {
        float x=19+i*117;
        if(i%2) pine(x,760+(i%3)*5,1.10f,p,i+71);
        else oak(x,771,1.03f,p,i+59);
    }
}

void pond(Rectangle area,const Palette& p,float time,int seed) {
    // Rock beds fill the collision bounds; irregular shorelines soften the silhouette.
    const std::array<int,18> left{{9,5,3,1,0,0,1,0,0,1,2,0,1,2,3,6,7,10}};
    const std::array<int,18> right{{11,6,4,3,1,0,0,1,2,1,0,0,1,2,4,5,8,11}};
    for(int i=0;i<18;i++) {
        int insetLeft=left[i]*2,insetRight=right[i]*2;
        float y=area.y+area.height*i/18.0f,h=std::ceil(area.height/18.0f);
        float x=area.x+insetLeft,w=area.width-insetLeft-insetRight;
        rect(x-2,y,w+4,h,Ink); rect(x,y,w,h,rgb(0x53684f));
        if(i>1&&i<16) rect(x+6,y,w-12,h,rgb(0x254b56));
        if(i>2&&i<15) rect(x+10,y,w-20,h,rgb(0x306774));
        if(i>4&&i<13) rect(x+16,y,w-32,h,rgb(0x387c85));
        if(i%3==0) {
            rect(x+1,y,5,3,rgb(0x90a17d)); rect(x+w-6,y+1,4,3,rgb(0x819574));
        }
    }
    for(int i=0;i<13;i++) {
        int x=int(area.x)+25+number(i,seed,17,std::max(1,int(area.width)-51));
        int y=int(area.y)+19+number(i,seed,21,std::max(1,int(area.height)-40));
        rect(x+int(time*2+i)%3*2,y,7+number(i,0,1,8),2,i%3==0?rgb(0x83b3af):rgb(0x4d9299));
    }
    for(int i=0;i<4;i++) {
        float x=area.x+(i%2?area.width-8:9);
        float y=area.y+(i/2?area.height-3:13);
        stone(x,y,2,p,i); tuft(x-4,y+4,p,i);
    }
    float lx=area.x+area.width*.68f,ly=area.y+area.height*.68f;
    oval(lx,ly,11,5,rgb(0x74975c),2); rect(lx+2,ly-1,9,2,rgb(0x3d654d));
    flower(lx-3,ly-2,p.flower,p,seed);
    rect(area.x+area.width*.27f,area.y+area.height*.42f,12,2,rgb(0x91b7ac));
}

void ruin(Rectangle area,const Palette& p,int seed) {
    rect(grow(area,4),opacity(Ink,145)); rect(area,Ink);
    for(int y=int(area.y)+3;y<int(area.y+area.height)-3;y+=18) {
        int row=(y-int(area.y))/18;
        for(int x=int(area.x)+3;x<int(area.x+area.width)-3;x+=28) {
            int width=std::min(26,int(area.x+area.width)-3-x);
            int height=std::min(16,int(area.y+area.height)-3-y);
            if(width<1||height<1) continue;
            Color fill=number(x,y,seed,4)?rgb(0x50535f):rgb(0x646371);
            rect(x,y,width,height,fill); rect(x+1,y+1,std::max(1,width-2),2,p.stoneLight);
            if(width>14&&height>9&&number(x,y,seed,4)==0) {
                rect(x+11,y+4,2,5,rgb(0x30313e)); rect(x+8,y+8,5,2,rgb(0x30313e));
            }
            if(row%2==0) rect(x,y,1,height,rgb(0x3d3d4a));
        }
    }
    rect(area.x+3,area.y+3,area.width-6,5,rgb(0x959096));
    rect(area.x+3,area.y+area.height-13,area.width-6,10,rgb(0x353742));
    for(int i=0;i<int(area.width)/28;i++) {
        float x=area.x+6+i*28;
        int raise=number(i,seed,9,3)*3;
        rect(x,area.y-raise,23,8,p.stone); rect(x+1,area.y-raise,20,2,p.stoneLight);
        if(i%2==0) rect(x+17,area.y-raise+2,5,3,rgb(0x42434e));
    }
    for(int i=0;i<7;i++) {
        float x=area.x+7+i*(area.width-17)/7;
        int length=5+number(i,seed,1,13);
        rect(x,area.y+4,10,length,p.leafShade); rect(x+2,area.y+4,5,3,p.light);
        if(i%3==0) rect(x+4,area.y+length+4,2,7,p.grass);
    }
    for(int i=0;i<3;i++) {
        float x=area.x+15+i*area.width*.29f;
        tuft(x,area.y+area.height+3,p,i);
        if(i%2==0) stone(x-8,area.y+area.height+3,1,p,i);
    }
    // A faded carving and short cracks keep the masonry from looking manufactured.
    float centerX=area.x+area.width*.54f,centerY=area.y+area.height*.51f;
    diamond(centerX,centerY,11,rgb(0x353744),3);
    diamond(centerX,centerY,7,rgb(0x66747c),2);
    diamond(centerX,centerY,4,rgb(0x3f4653),2);
}

void crescent(float x,float y,float radius,Color moon,Color cut,int unit=4) {
    oval(x,y,radius,radius,moon,unit);
    oval(x+radius*.40f,y-radius*.22f,radius*.83f,radius*.86f,cut,unit);
}
void column(float x,float y,const Palette& p) {
    oval(x+12,y+122,35,11,opacity(Ink,120),4);
    rect(x-16,y,34,124,Ink); rect(x-10,y+8,22,106,rgb(0x4c3e5c));
    rect(x-8,y+9,5,104,rgb(0x80698d)); rect(x+1,y+9,3,104,rgb(0x645074));
    rect(x+9,y+9,3,104,rgb(0x30283e));
    for(int offset: {0,8,111,120}) {
        rect(x-20,y+offset,42,6,rgb(0x66516f)); rect(x-18,y+offset,38,2,p.stoneLight);
    }
    rect(x-14,y-4,30,4,rgb(0x9679a3));
}
void archWindow(float x,float y,const Palette& p,bool moon) {
    rect(x-20,y+11,42,49,Ink); rect(x-14,y+4,30,58,Ink); rect(x-6,y,14,62,Ink);
    rect(x-16,y+12,34,44,rgb(0x5f4d71)); rect(x-11,y+7,24,49,rgb(0x675476));
    rect(x-12,y+15,26,40,rgb(0x1a243b)); rect(x-8,y+10,18,45,rgb(0x1a243b));
    if(moon) crescent(x+2,y+27,11,p.glow,rgb(0x1a243b),2);
    else { rect(x-4,y+24,2,2,p.glow); rect(x+7,y+35,2,2,p.glow); }
    rect(x-1,y+10,3,47,rgb(0x54425f)); rect(x-12,y+39,26,3,rgb(0x54425f));
    rect(x-22,y+58,46,6,p.stone); rect(x-20,y+58,42,2,p.stoneLight);
}
void torch(float x,float y,const Game& game,const Palette& p) {
    int phase=game.preferences.reducedMotion?1:int(game.time*8+x)%3;
    softLight({x,y-19},79,p.flower);
    rect(x-6,y+3,12,32,Ink); rect(x-3,y+3,6,29,p.barkLight);
    rect(x-13,y-1,26,7,rgb(0x997f9f)); rect(x-10,y+5,20,5,rgb(0x51415e));
    rect(x-9,y-20,18,18,rgb(0x653da0));
    rect(x-6,y-29-phase*3,12,26+phase*3,rgb(0xa770d8));
    rect(x-2,y-22-phase*2,5,19+phase*2,p.glow);
    rect(x+6,y-35+phase,3,7,rgb(0xa770d8));
    if(phase==2) rect(x-4,y-42,3,3,p.glow);
}

const Room& current(const Game& game) {
    static const Room fallback{};
    return game.currentRoom>=0&&game.currentRoom<int(game.rooms.size())?game.rooms[game.currentRoom]:fallback;
}
int sceneSeed(const Game& game) {
    const Room& room=current(game);
    return int((room.seed^game.runSeed^uint32_t(room.layout)*157u)&0x3fffffffu);
}
Color roomAccent(RoomKind kind,const Palette& p) {
    switch(kind) {
        case RoomKind::Start:return rgb(0xa6d4a7);
        case RoomKind::Treasure:return rgb(0xe5c578);
        case RoomKind::Shop:return rgb(0xe3ba83);
        case RoomKind::Sanctuary:return rgb(0xf2a8ca);
        case RoomKind::Challenge:return rgb(0xd48c99);
        case RoomKind::Boss:return rgb(0xbd91df);
        default:return p.glow;
    }
}
void heart(float x,float y,int unit,Color color) {
    constexpr const char* rows[]={" mm mm ","mmmmmmm","mmmmmmm"," mmmmm ","  mmm  ","   m   "};
    for(int row=0;row<6;row++) for(int col=0;col<7;col++)
        if(rows[row][col]=='m') rect(x+(col-3)*unit,y+(row-3)*unit,unit,unit,color);
}
void paw(float x,float y,float size,Color color) {
    oval(x,y+size*.14f,size*.45f,size*.31f,color,3);
    oval(x-size*.47f,y-size*.25f,size*.17f,size*.22f,color,3);
    oval(x-size*.17f,y-size*.48f,size*.17f,size*.22f,color,3);
    oval(x+size*.18f,y-size*.48f,size*.17f,size*.22f,color,3);
    oval(x+size*.48f,y-size*.25f,size*.17f,size*.22f,color,3);
}
void whiskers(float x,float y,float size,Color color) {
    diamond(x,y,8,color,2);
    for(int side: {-1,1}) for(int row=-1;row<=1;row++) {
        for(int step=0;step<4;step++)
            rect(x+side*(size*.22f+step*size*.15f)-(side<0?size*.16f:0),
                 y+row*(7+step*4),size*.16f,3,color);
    }
}
void catFace(float x,float y,int unit,Color color,Color eyes) {
    constexpr const char* rows[]={
        "mm            mm","mmm          mmm","mmmm        mmmm","mmmmmmmmmmmmmmmm",
        "mmmmmmmmmmmmmmmm","mmm  mmmmm  mmmm","mmm  mmmmm  mmmm","mmmmmmmmmmmmmmmm",
        "mmmmmmm  mmmmmmm","mmmmmmmmmmmmmmmm","  mmmmmmmmmmmm  ","    mmmmmmmm    "};
    for(int row=0;row<12;row++) for(int col=0;col<16;col++)
        if(rows[row][col]=='m') rect(x+(col-8)*unit,y+(row-6)*unit,unit,unit,color);
    rect(x-5*unit,y-unit,2*unit,2*unit,eyes); rect(x+3*unit,y-unit,2*unit,2*unit,eyes);
}

bool clearDecoration(Vector2 point,const Game& game,float margin=20) {
    const Room& room=current(game);
    if(distance(point,RoomCenter)<105) return false;
    if(room.neighbors[0]>=0&&point.y<405&&std::abs(point.x-640)<59) return false;
    if(room.neighbors[2]>=0&&point.y>405&&std::abs(point.x-640)<59) return false;
    if(room.neighbors[1]>=0&&point.x>640&&std::abs(point.y-405)<58) return false;
    if(room.neighbors[3]>=0&&point.x<640&&std::abs(point.y-405)<58) return false;
    if(room.kind==RoomKind::Boss&&distance(point,PortalPosition)<80) return false;
    for(auto obstacle:game.obstacles) if(inside(point,grow(obstacle,margin))) return false;
    for(const auto& pickup:game.loot) if(distance(point,pickup.pos)<29) return false;
    return true;
}
void woodlandFloor(const Game& game,const Palette& p) {
    const int seed=sceneSeed(game);
    rect(Field,p.soil);
    for(int i=0;i<33;i++) {
        Vector2 point{float(91+number(i,3,seed,1091)),float(185+number(i,7,seed,430))};
        if(!clearDecoration(point,game,16)) continue;
        int radius=18+number(i,9,seed,23);
        oval(point.x,point.y,radius,radius*.43f,i%3==0?p.shade:p.moss,4);
        if(i%3==0) oval(point.x-6,point.y-5,radius*.7f,4,p.moss,3);
    }
    for(int i=0;i<91;i++) {
        Vector2 point{float(88+number(i,2,seed,1101)),float(193+number(i,4,seed,427))};
        if(!clearDecoration(point,game)) continue;
        if(i%6==0) { tuft(point.x-7,point.y+3,p,i); tuft(point.x+9,point.y-2,p,i+1); }
        else if(i%3==0) {
            rect(point.x,point.y,6,2,p.grass); rect(point.x+9,point.y+4,4,2,p.moss);
        } else tuft(point.x,point.y,p,i);
    }
    for(int i=0;i<25;i++) {
        Vector2 point{float(105+number(i,6,seed,1065)),float(205+number(i,12,seed,402))};
        if(!clearDecoration(point,game,30)) continue;
        if(i%7==0) stone(point.x,point.y,2,p,i);
        else if(i%5==0) mushrooms(point.x,point.y,p);
        else {
            flower(point.x,point.y,i%2?p.flower:p.glow,p,i);
            if(i%3==0) flower(point.x+13,point.y+7,p.flower,p,i+1);
        }
    }
}
void masonryFloor(const Game& game,const Palette& p) {
    const int seed=sceneSeed(game);
    rect(Field,p.soil);
    for(int row=0,y=160;y<652;y+=30,row++) for(int x=64-(row%2)*28;x<1216;x+=56) {
        int left=std::max(64,x),right=std::min(1216,x+55),bottom=std::min(652,y+29);
        if(right<=left) continue;
        Color fill=number(x,y,seed,9)==0?p.moss:p.soil;
        rect(left,y,right-left,bottom-y,fill);
        rect(left,y,right-left,1,p.shade); rect(left,y,1,bottom-y,p.shade);
        if(number(x,y,seed+1,13)<2&&right-left>35)
            rect(left+8,y+6,11,1,opacity(p.stoneLight,37));
        if(number(x,y,seed+2,27)==0&&right-left>32) {
            rect(left+19,y+9,2,7,p.shade); rect(left+15,y+15,5,2,p.shade);
            rect(left+15,y+17,2,5,p.shade);
        }
    }
    if(game.stage==2) {
        // Sparse traces around the perimeter leave the battle space quiet.
        for(int side=0;side<2;side++) for(int i=0;i<6;i++) {
            int y=198+i*72,x=side?1105:117;
            rect(x,y,53,2,rgb(0x3a626d)); rect(x+51,y,2,16,rgb(0x3a626d));
            rect(x+49,y+14,6,5,rgb(0x58948f));
            if(i%3==0) rect(x+13,y-3,6,6,rgb(0x6c598e));
        }
        for(int i=0;i<7;i++) {
            int x=175+i*150;
            rect(x,197,38,2,rgb(0x365467)); rect(x+36,197,2,12,rgb(0x365467));
            rect(x+34,208,6,3,rgb(0x629199));
            rect(x,613,41,2,rgb(0x365467));
        }
    }
}

void routeSegment(int direction,const Palette& p,int seed,int stage) {
    int start=direction==0?160:direction==3?64:direction==1?640:405;
    int end=direction==0?405:direction==3?640:direction==1?1216:652;
    bool vertical=direction==0||direction==2;
    for(int at=start;at<end;at+=10) {
        int length=std::min(10,end-at),wobble=stage<2?number(at,direction,seed,3)-1:0;
        if(vertical) {
            rect(602+wobble,at,76,length,p.pathShade);
            rect(608+wobble,at,64,length,p.path);
        } else {
            rect(at,367+wobble,length,76,p.pathShade);
            rect(at,373+wobble,length,64,p.path);
        }
        if(number(at,direction,seed,5)==0) {
            if(vertical) rect(616+number(at,1,seed,35),at+2,5,2,p.pathLight);
            else rect(at+2,385+number(at,1,seed,35),5,2,p.pathLight);
        }
    }
}
void routes(const Game& game,const Palette& p) {
    const Room& room=current(game);
    for(int direction=0;direction<4;direction++)
        if(room.neighbors[direction]>=0) routeSegment(direction,p,sceneSeed(game),game.stage);
    oval(640,405,51,29,p.pathShade,3); oval(640,405,46,26,p.path,3);
    if(game.stage>=2) {
        for(int direction=0;direction<4;direction++) {
            if(room.neighbors[direction]<0) continue;
            if(direction==0) { rect(609,165,2,209,p.stone); rect(669,165,2,209,p.stone); }
            if(direction==1) { rect(675,374,536,2,p.stone); rect(675,434,536,2,p.stone); }
            if(direction==2) { rect(609,436,2,211,p.stone); rect(669,436,2,211,p.stone); }
            if(direction==3) { rect(69,374,536,2,p.stone); rect(69,434,536,2,p.stone); }
        }
    }
}
void brook(const Game& game,const Palette& p,float center) {
    float time=game.preferences.reducedMotion?0:game.time;
    for(int y=160;y<653;y+=4) {
        int bend=int(std::sin(float(y)*.013f)*8)/2*2;
        float x=center+bend;
        rect(x-26,y,52,4,p.shade); rect(x-22,y,44,4,rgb(0x465b42));
        rect(x-18,y,36,4,rgb(0x1d404b)); rect(x-12,y,24,4,rgb(0x2c5c66));
        rect(x-5,y,12,4,rgb(0x306a73));
        if(y%28==0) {
            rect(x-16,y,7,2,rgb(0x75917e));
            rect(x-5+int(time*3+y)%3*2,y+1,11,2,rgb(0x5e9699));
        }
    }
    for(int i=0;i<8;i++) {
        int y=197+i*55;
        if(y>345&&y<470) continue;
        float side=i%2?1.f:-1.f;
        tuft(center+side*29,y,p,i);
        if(i%3==0) flower(center-side*31,y+4,p.flower,p,i);
    }
}
void bridge(float center) {
    rect(center-45,366,91,78,Ink);
    for(int x=int(center)-42;x<int(center)+44;x+=9) {
        rect(x,369,8,72,rgb(0x805f42)); rect(x+1,371,2,67,rgb(0xb28a59));
        rect(x+4,381+(x%3)*7,2,31,rgb(0x624a36));
        rect(x+3,374,2,2,Ink); rect(x+3,434,2,2,Ink);
    }
    for(int y: {363,443}) {
        rect(center-49,y,101,7,rgb(0x342a27)); rect(center-47,y,97,3,rgb(0xc29b64));
        for(int x: {int(center)-49,int(center)+42}) {
            rect(x,y-6,10,17,Ink); rect(x+2,y-5,6,15,rgb(0x77563b));
            rect(x+2,y-5,6,3,rgb(0xc6a570));
        }
    }
}
void cushion(Rectangle area,const Palette& p,int seed) {
    rect(area,Ink);
    rect(area.x+2,area.y+3,area.width-4,area.height-5,rgb(0x725c69));
    rect(area.x+5,area.y+4,area.width-10,area.height-13,rgb(0xa48b8f));
    rect(area.x+9,area.y+7,area.width-18,area.height-22,rgb(0xbba29a));
    rect(area.x+12,area.y+8,area.width-24,3,rgb(0xd8bda7));
    rect(area.x+5,area.y+area.height-12,area.width-10,4,rgb(0x8e717d));
    for(int x=int(area.x)+12;x<int(area.x+area.width)-8;x+=14) {
        rect(x,area.y+3,5,2,rgb(0xe0c2b1)); rect(x,area.y+area.height-10,5,2,rgb(0xc8a69d));
    }
    float x=area.x+area.width*.5f,y=area.y+area.height*.48f;
    if(seed%2) paw(x,y,21,rgb(0x886c7b)); else heart(x,y,3,rgb(0x947283));
    for(int side: {-1,1}) rect(x+side*(area.width*.39f)-2,y-4,4,8,rgb(0x8d7380));
    (void)p;
}
void terminal(Rectangle area,const Palette& p,int seed) {
    rect(area,Ink); rect(area.x+3,area.y+4,area.width-6,area.height-8,rgb(0x354756));
    rect(area.x+5,area.y+5,area.width-10,5,rgb(0x71889a));
    rect(area.x+8,area.y+14,area.width-16,area.height-31,rgb(0x162b36));
    rect(area.x+11,area.y+17,area.width-22,area.height-37,rgb(0x203e4a));
    rect(area.x+10,area.y+area.height-13,area.width-20,4,rgb(0x4b6576));
    int slots=std::max(1,int(area.width)/24);
    for(int i=0;i<slots;i++) {
        float x=area.x+13+i*22;
        rect(x,area.y+area.height-12,8,3,i%3==0?rgb(0x91b77b):p.flower);
    }
    float x=area.x+area.width*.50f,y=area.y+area.height*.43f;
    rect(x-19,y-6,14,3,rgb(0x62a2aa)); rect(x+6,y+1,16,3,rgb(0x62a2aa));
    rect(x-6,y-3,3,10,rgb(0x52788f)); rect(x+3,y-8,3,10,rgb(0x52788f));
    if(seed%2) rect(x+3,y+3,7,3,rgb(0x8b7da7));
}

void environmentBorder(const Game& game,const Palette& p) {
    int seed=sceneSeed(game);
    if(game.stage<2) {
        treeBorders(p,game.stage);
        if(game.stage==1) {
            for(int i=0;i<4;i++) {
                int x=119+i*295+number(i,4,seed,27);
                rect(x,152,68,15,rgb(0x323a44));
                for(int block=0;block<3;block++) {
                    int height=9+(block%2)*8;
                    rect(x+block*24,151-height,21,height+8,p.stone);
                    rect(x+block*24+1,151-height,18,3,p.stoneLight);
                }
            }
            float time=game.preferences.reducedMotion?0:game.time;
            for(int i=0;i<3;i++) {
                float x=230+i*372+int(std::sin(time*.09f+i*2)*12)/3*3;
                float y=i%2?578:230;
                oval(x,y,130,12,opacity(p.glow,5),4);
                oval(x+36,y-5,89,8,opacity(p.glow,6),3);
            }
        }
    } else {
        for(int y=146,row=0;y<680;y+=22,row++) for(int side: {0,1218}) for(int col=0;col<2;col++) {
            int x=side+col*31;
            rect(x,y,30,21,p.shade); rect(x+1,y+1,28,2,p.stone);
            if((row+col)%4==0) rect(x+5,y+5,9,1,p.moss);
        }
        for(int x=64;x<1216;x+=32) {
            rect(x,146,31,14,p.stone); rect(x+1,147,29,2,p.stoneLight);
            rect(x,652,31,11,p.shade); rect(x+1,653,29,2,p.stone);
        }
        for(int x: {43,1238}) for(int y: {207,490}) column(x,y,p);
        // Windows avoid the central north and south passages.
        for(int x: {180,430,850,1100}) {
            rect(x-26,146,53,26,Ink); rect(x-22,147,45,21,rgb(0x182339));
            rect(x-21,147,42,2,p.stoneLight); rect(x-2,147,3,22,p.stone);
            rect(x-25,169,51,4,p.stone); rect(x+10,153,2,2,p.glow);
            if(game.stage==3&&x==1100) crescent(x-10,157,8,p.glow,rgb(0x182339),2);
        }
        archWindow(31,245,p,false); archWindow(1247,245,p,game.stage==3);
        for(Vector2 point: std::array<Vector2,4>{{{78,277},{1202,277},{78,565},{1202,565}}}) {
            if(game.stage==3) torch(point.x,point.y,game,p);
            else {
                softLight(point,48,p.flower);
                rect(point.x-7,point.y-22,14,29,Ink);
                rect(point.x-4,point.y-19,8,21,rgb(0x5693ac));
                rect(point.x-2,point.y-16,3,15,rgb(0xc0e4e7));
                rect(point.x-10,point.y+7,20,5,p.stone);
            }
        }
    }
    // The visible room boundary agrees with the simulation's single rectangular field.
    rect(64,160,1152,2,p.shade); rect(64,650,1152,2,p.shade);
    rect(64,160,2,492,p.shade); rect(1214,160,2,492,p.shade);
}

// Local door coordinates: along is measured across the opening, inward toward the arena.
void doorRect(int direction,float along,float inward,float width,float depth,Color color) {
    switch(direction) {
        case 0:rect(640+along,160+inward,width,depth,color);break;
        case 1:rect(1216-inward-depth,405+along,depth,width,color);break;
        case 2:rect(640-along-width,652-inward-depth,width,depth,color);break;
        default:rect(64+inward,405-along-width,depth,width,color);break;
    }
}
void doors(const Game& game,const Palette& p) {
    const Room& room=current(game);
    bool open=game.doorsOpen();
    for(int direction=0;direction<4;direction++) {
        int neighbor=room.neighbors[direction];
        if(neighbor<0) continue;
        Color accent=neighbor<int(game.rooms.size())?roomAccent(game.rooms[neighbor].kind,p):p.glow;
        doorRect(direction,-50,-64,100,94,p.pathShade);
        doorRect(direction,-46,-64,92,89,open?p.path:rgb(0x292635));
        for(int side: {-1,1}) {
            float along=side<0?-59.f:50.f;
            doorRect(direction,along,-19,9,49,Ink);
            doorRect(direction,along+2,-17,5,45,p.stone);
            doorRect(direction,along+2,-15,2,40,p.stoneLight);
            for(int inward: {-6,10,24}) doorRect(direction,along,inward,9,2,p.shade);
        }
        doorRect(direction,-59,-21,118,8,p.stone);
        doorRect(direction,-56,-20,112,2,p.stoneLight);
        doorRect(direction,-7,-21,14,7,accent);
        if(open) {
            doorRect(direction,-44,-10,88,31,opacity(accent,12));
            doorRect(direction,-31,-6,62,24,opacity(accent,13));
            doorRect(direction,-2,7,4,14,accent);
            doorRect(direction,-8,6,16,3,accent);
            doorRect(direction,-5,3,10,3,accent);
            doorRect(direction,-2,0,4,3,accent);
        } else {
            for(int along=-44;along<46;along+=12) {
                doorRect(direction,float(along),-12,8,34,p.stone);
                doorRect(direction,float(along)+1,-11,2,32,p.stoneLight);
            }
            doorRect(direction,-46,2,92,5,Ink);
            doorRect(direction,-8,-5,16,20,rgb(0xa47b80));
            doorRect(direction,-4,-1,8,8,rgb(0xe3a5aa));
            doorRect(direction,-2,8,4,5,Ink);
        }
    }
}

void floorRing(Vector2 center,int radius,const Palette& p,Color accent) {
    oval(center.x,center.y,radius,radius*.60f,p.shade,4);
    oval(center.x,center.y,radius-4,(radius-4)*.60f,opacity(accent,32),4);
    oval(center.x,center.y,radius-8,(radius-8)*.60f,p.soil,4);
    for(int i=0;i<8;i++) {
        float angle=i*3.14159265f/4;
        diamond(center.x+std::cos(angle)*(radius-3),center.y+std::sin(angle)*(radius-3)*.6f,3,opacity(accent,125),2);
    }
}
void pedestal(const Game& game,const Palette& p,Color accent) {
    float x=RoomCenter.x,y=RoomCenter.y;
    oval(x,y+24,40,9,opacity(Ink,150),3);
    rect(x-29,y+8,58,17,Ink); rect(x-26,y+10,52,12,p.stone);
    rect(x-30,y+4,60,8,p.stoneLight); rect(x-27,y+4,54,2,accent);
    rect(x-19,y+15,38,2,p.shade);
    if(!current(game).rewardClaimed) {
        softLight({x,y-5},58,accent);
        rect(x-3,y+15,6,4,accent);
    }
}
void roomFeature(const Game& game,const Palette& p) {
    const Room& room=current(game);
    Color accent=roomAccent(room.kind,p);
    switch(room.kind) {
        case RoomKind::Start:
            floorRing(RoomCenter,87,p,accent);
            paw(640,402,37,opacity(accent,73));
            for(int i=0;i<3;i++) paw(622+(i%2)*29,471+i*30,10,opacity(accent,54));
            break;
        case RoomKind::Treasure:
            floorRing(RoomCenter,96,p,accent);
            for(int side: {-1,1}) diamond(640+side*68,405,9,opacity(accent,120),3);
            pedestal(game,p,accent);
            break;
        case RoomKind::Shop:
            rect(477,345,326,125,Ink); rect(480,348,320,119,rgb(0x755667));
            rect(484,352,312,111,rgb(0x4e3b4b));
            for(int x=495;x<795;x+=24) {
                diamond(x,354,3,accent,2);diamond(x,459,3,accent,2);
            }
            for(int side: {-1,1}) {
                oval(640+side*104,408,13,13,rgb(0xa68c59),3);
                oval(640+side*104,408,9,9,rgb(0xd7bc7c),3);
                rect(638+side*104,402,3,12,rgb(0x987b4c));
            }
            pedestal(game,p,accent);
            break;
        case RoomKind::Sanctuary:
            floorRing(RoomCenter,106,p,accent);
            heart(640,387,6,opacity(accent,99));
            for(int side: {-1,1}) for(int row: {-1,1})
                flower(640+side*69,405+row*32,accent,p,row+side);
            if(!room.rewardClaimed) softLight(RoomCenter,105,accent);
            break;
        case RoomKind::Challenge:
            floorRing(RoomCenter,117,p,room.cleared?p.glow:accent);
            for(int side: {-1,1}) for(int offset=0;offset<3;offset++)
                rect(640+side*(56+offset*15)-3,389+offset*8,6,19,opacity(accent,93));
            diamond(640,405,14,opacity(accent,75),3);
            break;
        case RoomKind::Boss:
            floorRing(RoomCenter,167,p,accent);
            if(game.stage==0) {
                paw(640,403,82,rgb(0x58624c));
                heart(640,414,5,opacity(p.flower,80));
            } else if(game.stage==1) whiskers(640,405,113,rgb(0x63737c));
            else if(game.stage==2) {
                catFace(640,398,8,rgb(0x355266),rgb(0x727d8d));
                rect(575,424,39,5,rgb(0x534c6f)); rect(666,372,33,4,rgb(0x587363));
            } else crescent(640,397,85,rgb(0x594366),p.soil,4);
            if(room.cleared&&!room.rewardClaimed&&game.stage<FloorCount-1)
                pedestal(game,p,rgb(0xe0c68d));
            break;
        case RoomKind::Combat:
            if(room.layout%3==1) paw(640,404,24,opacity(p.stoneLight,25));
            break;
    }
}
void stairs(const Game& game,const Palette& p) {
    const Room& room=current(game);
    if(room.kind!=RoomKind::Boss||!room.cleared||!room.rewardClaimed||game.stage>=FloorCount-1) return;
    float x=PortalPosition.x,y=PortalPosition.y;
    softLight({x,y},75,rgb(0xddc28a));
    rect(x-58,y-34,116,77,Ink); rect(x-54,y-31,108,70,p.stone);
    rect(x-48,y-27,96,61,Ink);
    for(int i=0;i<5;i++) {
        int inset=i*3;
        Color color=rgb(0x70677b); color.r-=i*8;color.g-=i*7;color.b-=i*7;
        rect(x-44+inset,y-23+i*11,88-inset*2,10,color);
        rect(x-43+inset,y-23+i*11,86-inset*2,2,p.stoneLight);
    }
    rect(x-10,y+29,20,3,rgb(0xd6be85)); rect(x-6,y+32,12,3,rgb(0xd6be85));
    rect(x-3,y+35,6,3,rgb(0xd6be85));
    diamond(x,y-45,6,rgb(0xe1c68b),2);
}
void finalRefuge(const Game& game,const Palette& p) {
    if(game.stage!=3||current(game).kind!=RoomKind::Boss) return;
    const float x=RescuePosition.x,y=RescuePosition.y;
    Color metal=game.malikethFriendly?rgb(0xaa9b86):rgb(0x7f6b98);
    rect(x-64,y-2,128,18,Ink); rect(x-60,y-1,120,12,rgb(0x4b3a57));
    rect(x-56,y-5,112,4,metal);
    if(!game.giovannyRescued) {
        // Front bars and characters are drawn later by the character/effect renderer.
        rect(x-62,y-106,6,104,metal); rect(x+56,y-106,6,104,metal);
        rect(x-63,y-109,126,7,metal); rect(x-59,y-108,118,2,p.glow);
        if(game.malikethFriendly) {
            rect(x-77,y-103,5,102,metal);rect(x+72,y-103,5,102,metal);
            heart(x,y-126,3,rgb(0xdfaec4));
        }
    }
    if(!game.bossDefeated) return;
    Vector2 bowl{MalikethPosition.x,MalikethPosition.y+31};
    oval(bowl.x,bowl.y+4,24,8,Ink,2); oval(bowl.x,bowl.y,22,8,rgb(0xbc9dab),2);
    oval(bowl.x,bowl.y-2,18,5,rgb(0x655161),2);
    if(game.malikethFriendly) {
        oval(bowl.x,bowl.y-3,15,4,rgb(0xd8ab8c),2);
        rect(bowl.x-7,bowl.y-5,7,2,rgb(0xf1d1a3));
        softLight(MalikethPosition,91,rgb(0xe4aabf));
        for(int i=0;i<5;i++) {
            float progress=float(i)/5;
            heart(950+progress*111,383-progress*70,2,rgb(0x9c718f));
        }
    } else if(game.hasChuru) {
        rect(bowl.x+25,bowl.y-20,9,23,rgb(0xd69cbb));
        rect(bowl.x+27,bowl.y-17,5,13,rgb(0xf1d7ba));
        rect(bowl.x+24,bowl.y-22,11,4,rgb(0x9d6a95));
        heart(bowl.x+29,bowl.y-30,2,rgb(0xd8a5c6));
    }
}
void fireflies(const Game& game,const Palette& p) {
    float time=game.preferences.reducedMotion?0:game.time;
    int seed=sceneSeed(game);
    for(int i=0;i<17;i++) {
        float x=113+number(i,seed,64,1010),y=199+number(i,seed,82,416);
        if(!clearDecoration({x,y},game,7)) continue;
        int phase=int(time*1.2f+i*.71f)%6;
        if(phase<2) continue;
        int drift=int(std::sin(time*.7f+i)*4)/2*2;
        rect(x-4,y+drift-4,10,10,opacity(p.glow,7));
        rect(x-1,y+drift-1,4,4,opacity(p.glow,20));
        rect(x,y+drift,2,2,opacity(p.glow,phase==5?190:115));
    }
}

} // namespace

void drawWorld(const Game& game) {
    const Palette p=palette(game.stage);
    const Room& room=current(game);
    float time=game.preferences.reducedMotion?0:game.time;
    int seed=sceneSeed(game);
    BeginScissorMode(0,145,Width,535);
    rect(0,145,Width,535,p.back);
    if(game.stage<2) woodlandFloor(game,p); else masonryFloor(game,p);
    float river=0;
    if(game.stage==0&&room.layout%3==0&&(room.kind==RoomKind::Start||room.kind==RoomKind::Combat)) {
        if(room.neighbors[1]>=0) river=858;
        else if(room.neighbors[3]>=0) river=422;
        if(river) brook(game,p,river);
    }
    routes(game,p);
    if(river) bridge(river);
    roomFeature(game,p);
    for(size_t i=0;i<game.obstacles.size();i++) {
        if(game.stage==0) {
            if(room.layout%4==2) pond(game.obstacles[i],p,time,int(i));
            else cushion(game.obstacles[i],p,seed+int(i));
        } else if(game.stage==2) terminal(game.obstacles[i],p,seed+int(i));
        else ruin(game.obstacles[i],p,seed+int(i));
    }
    environmentBorder(game,p);
    stairs(game,p);
    finalRefuge(game,p);
    doors(game,p);
    fireflies(game,p);
    EndScissorMode();
}

} // namespace vivis
