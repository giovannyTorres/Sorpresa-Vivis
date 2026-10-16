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

bool clearDecoration(Vector2 point,const Game& game,float margin=20) {
    if(point.y>366&&point.y<479) return false;
    if(point.x>1110&&point.y>320&&point.y<490) return false;
    if(distance(point,game.bell)<38) return false;
    for(auto obstacle:game.obstacles) if(inside(point,grow(obstacle,margin))) return false;
    for(auto heart:game.hearts) if(distance(point,heart)<25) return false;
    return true;
}
void ground(const Game& game,const Palette& p) {
    rect(Field,p.soil);
    // Broad clusters create worn and mossy areas; the center remains easy to read.
    for(int i=0;i<42;i++) {
        Vector2 point{float(88+number(i,3,game.stage,1096)),float(182+number(i,7,game.stage,439))};
        if(!clearDecoration(point,game,15)) continue;
        int radius=19+number(i,9,4,25);
        oval(point.x,point.y,radius,radius*.43f,i%3==0?p.shade:p.moss,4);
        if(i%3==0) oval(point.x-6,point.y-5,radius*.7f,4,p.moss,3);
    }
    for(int i=0;i<115;i++) {
        Vector2 point{float(80+number(i,2,game.stage+2,1120)),float(181+number(i,4,game.stage+2,449))};
        if(!clearDecoration(point,game)) continue;
        if(i%6==0) {
            tuft(point.x-7,point.y+3,p,i); tuft(point.x+9,point.y-2,p,i+1);
        } else if(i%3==0) {
            rect(point.x,point.y,6,2,p.grass); rect(point.x+9,point.y+4,4,2,p.moss);
        } else tuft(point.x,point.y,p,i);
    }
    for(int i=0;i<30;i++) {
        Vector2 point{float(100+number(i,6,game.stage,1030)),float(204+number(i,12,game.stage,408))};
        if(!clearDecoration(point,game,30)) continue;
        if(i%7==0) stone(point.x,point.y,2,p,i);
        else if(i%5==0) mushrooms(point.x,point.y,p);
        else {
            flower(point.x,point.y,i%2?p.flower:p.glow,p,i);
            if(i%3==0) flower(point.x+13,point.y+7,p.flower,p,i+1);
        }
    }
}

void branchPath(const Game& game,const Palette& p) {
    if(game.stage>=2) return;
    float destination=game.bell.y;
    int start=int(std::min(destination,422.0f)),end=int(std::max(destination,422.0f));
    for(int y=start;y<end;y+=8) {
        int wobble=number(y,8,game.stage,3)*2-2;
        rect(game.bell.x-24+wobble,y,48,8,p.pathShade);
        rect(game.bell.x-19+wobble,y,38,8,p.path);
        if(y%24==0) rect(game.bell.x-8,y+2,6,2,p.pathLight);
    }
    oval(game.bell.x,destination,35,17,p.pathShade,3);
    oval(game.bell.x,destination,31,14,p.path,3);
}
void mainPath(const Game& game,const Palette& p) {
    branchPath(game,p);
    for(int x=64;x<1216;x+=12) {
        int wobble=number(x,1,18,4)-2;
        rect(x,380+wobble,12,82,p.shade);
        rect(x,384+wobble,12,74,p.pathShade);
        rect(x,389+wobble,12,65,p.path);
        if(x%3==0) rect(x,385+wobble,7,3,p.grass);
        if(x%5==0) rect(x+4,455+wobble,6,3,p.grass);
    }
    for(int i=0;i<88;i++) {
        int x=76+number(i,5,9,1125),y=395+number(i,6,9,48);
        rect(x,y,4+number(i,7,2,6),2,i%4==0?p.pathLight:p.pathShade);
    }
    if(game.stage==1) {
        for(int i=0;i<21;i++) {
            int x=91+i*52,y=405+number(i,4,7,16);
            rect(x-2,y-2,35,18,Ink); rect(x,y,32,15,p.stone);
            rect(x+2,y+1,27,2,p.stoneLight); rect(x+7,y+12,24,2,p.pathShade);
        }
    }
}

void stream(float time,const Palette& p) {
    for(int y=160;y<653;y+=4) {
        int bend=int(std::sin(float(y)*.013f)*8)/2*2;
        float x=620+bend;
        rect(x-29,y,58,4,p.shade); rect(x-25,y,50,4,rgb(0x465b42));
        rect(x-20,y,40,4,rgb(0x1d404b)); rect(x-15,y,30,4,rgb(0x2c5c66));
        rect(x-7,y,16,4,rgb(0x306a73));
        if(y%28==0) {
            rect(x-17,y,9,2,rgb(0x75917e));
            int drift=int(time*3+y)%3*2;
            rect(x-6+drift,y+1,13,2,rgb(0x5e9699));
        }
    }
    for(int i=0;i<9;i++) {
        int y=190+i*50;
        if(y>360&&y<485) continue;
        float side=i%2?1.f:-1.f;
        tuft(620+side*31,y,p,i);
        stone(620+side*30,y+12,1,p,i);
        if(i%3==0) flower(620-side*33,y+4,p.flower,p,i);
    }
}
void bridge() {
    // Planks run across the shallow stream. This bridge adds no artificial collision.
    rect(578,384,91,78,Ink);
    for(int x=581;x<667;x+=9) {
        rect(x,387,8,72,rgb(0x805f42)); rect(x+1,389,2,67,rgb(0xb28a59));
        rect(x+4,399+(x%3)*7,2,31,rgb(0x624a36));
        rect(x+3,392,2,2,Ink); rect(x+3,452,2,2,Ink);
    }
    for(int y: {381,461}) {
        rect(574,y,101,7,rgb(0x342a27)); rect(576,y,97,3,rgb(0xc29b64));
        for(int x: {574,665}) {
            rect(x,y-6,10,17,Ink); rect(x+2,y-5,6,15,rgb(0x77563b));
            rect(x+2,y-5,6,3,rgb(0xc6a570));
        }
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

void mist(const Game& game,const Palette& p) {
    float time=game.preferences.reducedMotion?0:game.time;
    const std::array<Vector2,4> patches{{{260,241},{803,568},{183,612},{950,198}}};
    for(size_t i=0;i<patches.size();i++) {
        float drift=int(std::sin(time*.09f+float(i)*2)*15)/3*3;
        Vector2 point{patches[i].x+drift,patches[i].y};
        oval(point.x,point.y,160,14,opacity(p.glow,5),4);
        oval(point.x+42,point.y-5,107,9,opacity(p.glow,6),3);
        oval(point.x-55,point.y+5,108,6,opacity(p.glow,5),3);
        rect(point.x-61,point.y-2,67,2,opacity(p.glow,5));
    }
}

void forestDetails(const Game& game,const Palette& p) {
    treeBorders(p,game.stage);
    // Mossy perimeter stones and roots make the forest edge solid and readable.
    for(int x=70;x<1210;x+=38) {
        if(x>590&&x<655&&game.stage==0) continue;
        rect(x,651,30,5,p.shade); rect(x+2,653,24,2,p.moss);
        if(number(x,0,4,4)==0) tuft(x+10,648,p,x);
    }
    if(game.stage==1) {
        for(int x: {123,420,828,1050}) {
            rect(x,153,69,15,rgb(0x323a44));
            for(int i=0;i<3;i++) {
                int height=9+(i%2)*8;
                rect(x+i*24,151-height,21,height+8,p.stone);
                rect(x+i*24+1,151-height,18,3,p.stoneLight);
                rect(x+i*24+4,151-height+3,5,4,p.leafShade);
            }
        }
        mist(game,p);
    }
}

void gate(const Game& game,const Palette& p) {
    constexpr Rectangle opening{1150,350,66,120};
    bool open=game.gateOpen();
    softLight({1181,410},72,open?p.glow:p.flower);
    rect(1142,339,82,140,Ink);
    rect(1147,344,72,132,rgb(0x686a54));
    rect(opening,open?rgb(0x142b30):rgb(0x34303b));
    for(int x: {1144,1212}) {
        rect(x,341,9,137,rgb(0x7d795b)); rect(x+1,344,3,130,rgb(0xb6ab7b));
        for(int y=356;y<472;y+=20) rect(x,y,9,2,rgb(0x454c40));
        tuft(x-2,479,p,x);
    }
    rect(1144,338,77,11,rgb(0x868262)); rect(1148,339,69,3,rgb(0xc9ba8a));
    rect(1156,335,54,4,rgb(0x535744));
    if(open) {
        for(int i=0;i<4;i++) rect(1157+i*4,355+i*6,51-i*8,108-i*12,opacity(p.glow,15+i*9));
        rect(1169,408,17,4,rgb(0xdfedb3)); rect(1183,402,4,16,rgb(0xdfedb3));
        rect(1187,406,4,8,rgb(0xdfedb3));
        for(int i=0;i<4;i++) {
            int y=366+i*25+(int(game.preferences.reducedMotion?0:game.time*7)%15);
            rect(1166+(i%2)*25,y,3,3,p.glow);
        }
    } else {
        for(int x=1155;x<1214;x+=10) {
            rect(x,352,8,115,rgb(0x66503d)); rect(x+1,353,2,111,rgb(0x927650));
        }
        for(int y: {370,449}) {
            rect(1152,y,62,6,rgb(0x29303a));
            for(int x: {1157,1205}) rect(x,y+2,3,2,rgb(0x9da9a2));
        }
        rect(1176,403,16,18,rgb(0xd9bd71)); rect(1179,397,10,8,rgb(0x927d4c));
        rect(1182,407,4,8,rgb(0x604a3a));
    }
}

void fireflies(const Game& game,const Palette& p) {
    float time=game.preferences.reducedMotion?0:game.time;
    for(int i=0;i<22;i++) {
        float x=113+number(i,game.stage,64,1010),y=199+number(i,game.stage,82,416);
        if(y>385&&y<465) continue;
        int phase=int(time*1.2f+i*.71f)%6;
        if(phase<2) continue;
        int drift=int(std::sin(time*.7f+i)*4)/2*2;
        rect(x-4,y+drift-4,10,10,opacity(p.glow,7));
        rect(x-1,y+drift-1,4,4,opacity(p.glow,20));
        rect(x,y+drift,2,2,opacity(p.glow,phase==5?190:115));
    }
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
void castleFloor(const Palette& p) {
    rect(Field,p.soil);
    for(int row=0,y=160;y<652;y+=30,row++) {
        for(int x=64-(row%2)*28;x<1216;x+=56) {
            int left=std::max(64,x),right=std::min(1216,x+55);
            int bottom=std::min(652,y+29);
            if(right<=left) continue;
            Color fill=number(x,y,7,8)==0?rgb(0x383247):p.soil;
            rect(left,y,right-left,bottom-y,fill);
            rect(left,y,right-left,1,p.shade); rect(left,y,1,bottom-y,p.shade);
            if(number(x,y,8,11)<2&&right-left>35) {
                rect(left+8,y+6,11,1,rgb(0x484052));
                rect(left+19,y+7,5,1,rgb(0x484052));
            }
            if(number(x,y,2,23)==0&&right-left>32) {
                rect(left+19,y+9,2,7,p.shade); rect(left+15,y+15,5,2,p.shade);
                rect(left+15,y+17,2,5,p.shade);
            }
        }
    }
    // A carved lunar seal sits beneath Maliketh rather than competing with the characters.
    oval(900,410,155,155,rgb(0x28263a),4); oval(900,410,149,149,rgb(0x544260),4);
    oval(900,410,146,146,p.soil,4); oval(900,410,136,136,rgb(0x46374f),4);
    oval(900,410,133,133,p.soil,4);
    for(int i=0;i<16;i++) {
        float angle=i*3.14159265f/8;
        float x=900+std::cos(angle)*142,y=410+std::sin(angle)*142;
        diamond(x,y,4,rgb(0x80658a),2);
        if(i%2==0) diamond(900+std::cos(angle)*120,410+std::sin(angle)*120,5,rgb(0x544261),2);
    }
    crescent(905,335,58,rgb(0x4d3d59),p.soil,4);
    diamond(878,416,9,rgb(0x62506e),3);
    // Raised masonry around the rescue alcove is a floor motif, not an extra obstacle.
    rect(1044,190,128,115,rgb(0x252335)); rect(1049,195,118,105,rgb(0x44364d));
    rect(1053,199,110,97,rgb(0x342a40));
    for(int x=1059;x<1160;x+=17) rect(x,292,8,2,rgb(0x7a6084));
}
void carpet(const Palette& p) {
    rect(95,385,1060,78,Ink); rect(96,388,1059,72,rgb(0x68485e));
    rect(99,390,1053,68,rgb(0x957389)); rect(101,393,1049,62,rgb(0x4d334d));
    rect(104,397,1043,54,rgb(0x58384f));
    rect(104,399,1043,2,rgb(0x79526b)); rect(104,448,1043,2,rgb(0x79526b));
    for(int x=110;x<1143;x+=28) {
        diamond(x,394,3,rgb(0xbf97a5),2); diamond(x,454,3,rgb(0xbf97a5),2);
        if(x%3==0) { rect(x,411,13,1,rgb(0x604056)); rect(x+5,436,9,1,rgb(0x604056)); }
    }
    for(int x: {92,1153}) for(int y=392;y<456;y+=8) rect(x,y,5,3,rgb(0x9b7c8e));
    for(int x: {280,530,780,1030}) {
        diamond(x,425,17,rgb(0x72506b),3); diamond(x,425,12,rgb(0x4d334d),3);
        diamond(x,425,4,rgb(0xa47b99),2);
    }
    // Small candle clusters on the carpet edge echo the reference's purple magic.
    for(int x: {191,536,1035}) for(int i=0;i<3;i++) {
        int y=i%2?478:370;
        oval(x+i*10,y+4,6,3,opacity(Ink,130),2);
        rect(x+i*10-2,y-10-i%2*5,5,14+i%2*5,rgb(0x9d82a5));
        rect(x+i*10-1,y-16-i%2*5,3,6,p.glow);
    }
}
void tower(const Game& game,const Palette& p) {
    castleFloor(p); carpet(p);
    for(int y=146,row=0;y<680;y+=22,row++) {
        for(int side: {0,1218}) for(int col=0;col<2;col++) {
            int x=side+col*31;
            rect(x,y,30,21,rgb(0x2a263b)); rect(x+1,y+1,28,2,rgb(0x4a3c58));
            if((row+col)%4==0) rect(x+5,y+5,9,1,rgb(0x3b3149));
        }
    }
    for(int x=64;x<1216;x+=32) {
        rect(x,146,31,14,rgb(0x44354f)); rect(x+1,147,29,2,rgb(0x766183));
        rect(x,652,31,11,rgb(0x252235)); rect(x+1,653,29,2,rgb(0x55415e));
    }
    for(int x: {43,1238}) for(int y: {207,490}) column(x,y,p);
    archWindow(31,363,p,false); archWindow(1247,363,p,true);
    for(int x: {178,430,684,941,1135}) {
        // Shallow upper windows retain an unobstructed walkable floor below them.
        rect(x-26,146,53,26,Ink); rect(x-22,147,45,21,rgb(0x182339));
        rect(x-21,147,42,2,rgb(0x6e597b)); rect(x-2,147,3,22,rgb(0x554161));
        rect(x-25,169,51,4,rgb(0x695173)); rect(x+10,153,2,2,p.glow);
        if(x==941) crescent(x-10,157,8,p.glow,rgb(0x182339),2);
    }
    for(Vector2 point: std::array<Vector2,4>{{{74,284},{1206,284},{74,567},{1206,567}}})
        torch(point.x,point.y,game,p);
    for(Vector2 point: std::array<Vector2,5>{{{238,249},{435,561},{600,250},{1041,608},{1118,336}}}) {
        diamond(point.x,point.y,4,rgb(0x64516e),2);
        rect(point.x-9,point.y,3,2,rgb(0x45354f)); rect(point.x+8,point.y,3,2,rgb(0x45354f));
    }
    // Light falls from the window in discrete, translucent bands.
    for(int i=0;i<5;i++) rect(1114-i*12,179+i*15,58+i*10,3,opacity(p.glow,5));
    // Obstacles are still represented if a future level introduces them.
    for(size_t i=0;i<game.obstacles.size();i++) ruin(game.obstacles[i],p,int(i));
}

} // namespace

void drawWorld(const Game& game) {
    const Palette p=palette(game.stage);
    float time=game.preferences.reducedMotion?0:game.time;
    // Keep the HUD and menu footer independent from decorative scenery.
    BeginScissorMode(0,145,Width,535);
    rect(0,145,Width,535,p.back);
    if(game.stage>=2) tower(game,p);
    else {
        ground(game,p);
        if(game.stage==0) stream(time,p);
        mainPath(game,p);
        if(game.stage==0) bridge();
        for(size_t i=0;i<game.obstacles.size();i++) {
            if(game.stage==0) pond(game.obstacles[i],p,time,int(i));
            else ruin(game.obstacles[i],p,int(i));
        }
        forestDetails(game,p);
        gate(game,p);
    }
    fireflies(game,p);
    EndScissorMode();
}

} // namespace vivis
