#include "visuals.hpp"
#include "profile.hpp"
#include "audio.hpp"
#include "assets_embedded.hpp"
#include <array>
#include <cstdint>
#include <filesystem>
#include <string>

namespace {
using namespace vivis;
Texture2D texture(const unsigned char* bytes,std::size_t count) {
    Image image=LoadImageFromMemory(".png",bytes,int(count));
    Texture2D result=LoadTextureFromImage(image); UnloadImage(image);
    SetTextureFilter(result,TEXTURE_FILTER_POINT); return result;
}
Font font(int size) {
    std::vector<int> glyphs;
    for(int cp=32;cp<=255;++cp) glyphs.push_back(cp);
    for(int cp:{0x2019,0x2022,0x2014,0x2190,0x2192}) glyphs.push_back(cp);
    Font result=LoadFontFromMemory(".ttf",embedded::font,int(embedded::font_size),size,glyphs.data(),int(glyphs.size()));
    SetTextureFilter(result.texture,TEXTURE_FILTER_POINT); return result;
}
bool padPressed(int button) { return IsGamepadAvailable(0)&&IsGamepadButtonPressed(0,button); }
bool padDown(int button) { return IsGamepadAvailable(0)&&IsGamepadButtonDown(0,button); }

struct App {
    Game game;
    Screen returnTo{Screen::Title}, lastScreen{Screen::Title};
    int selected{},gallery{};
    bool quit{},mouseActive{};
    float mouseRecent{};
    std::filesystem::path saveFile;
    void applyFullscreen() {
        if(game.preferences.fullscreen!=IsWindowFullscreen()) ToggleFullscreen();
    }
    void open(Screen screen,Screen parent) {
        returnTo=parent; game.screen=screen; selected=0; mouseActive=false;
    }
    void back() {
        if(game.mapOpen||game.inventoryOpen) { game.mapOpen=game.inventoryOpen=false; return; }
        if(game.screen==Screen::Reward) { game.screen=Screen::Playing; game.rewardTime=0; selected=0; return; }
        if(game.screen==Screen::Paused) game.screen=Screen::Playing;
        else if(game.screen==Screen::Settings||game.screen==Screen::Controls||game.screen==Screen::Gallery) {
            game.screen=returnTo; game.saveRequested=true;
        } else if(game.screen==Screen::Playing) { game.screen=Screen::Paused; game.saveRequested=true; }
        else if(game.screen!=Screen::Title) game.screen=Screen::Title;
        selected=0;
    }
    void adjust(int amount) {
        if(game.screen==Screen::Gallery) gallery=(gallery+amount+4)%4;
        if(game.screen==Screen::Settings&&selected==1) {
            game.preferences.volume=std::clamp(std::round((game.preferences.volume+amount*0.1f)*10)/10,0.0f,1.0f);
            game.saveRequested=true;
        }
    }
    void activate(int index) {
        if(index<0) return;
        if(game.mapOpen||game.inventoryOpen) { game.mapOpen=game.inventoryOpen=false; return; }
        if(game.screen==Screen::Reward) {
            if(index==3) back();
            else if(!game.chooseReward(index)) game.notify("Necesitas más monedas para ese objeto",2);
            return;
        }
        Screen screen=game.screen;
        if(screen==Screen::Gallery&&index>=100&&index<104) { gallery=index-100; return; }
        if(screen==Screen::Settings&&(index==201||index==202)) {
            selected=1; adjust(index==201?-1:1); return;
        }
        game.cues.push_back(Cue::Dash);
        if(screen==Screen::Title) {
            if(game.hasSave&&index==0) { game.resumeCheckpoint(); return; }
            int action=index-(game.hasSave?1:0);
            if(action==0) game.newGame();
            else if(action==1) open(Screen::Gallery,Screen::Title);
            else if(action==2) open(Screen::Settings,Screen::Title);
            else if(action==3) open(Screen::Controls,Screen::Title);
            else if(action==4) quit=true;
        } else if(screen==Screen::Paused) {
            if(index==0) game.screen=Screen::Playing;
            else if(index==1) open(Screen::Settings,Screen::Paused);
            else if(index==2) open(Screen::Controls,Screen::Paused);
            else { game.screen=Screen::Title; game.saveRequested=true; }
        } else if(screen==Screen::Lost) {
            if(index==0) game.retry(); else game.screen=Screen::Title;
        } else if(screen==Screen::Won) {
            if(index==0) game.newGame();
            else if(index==1) open(Screen::Gallery,Screen::Won);
            else game.screen=Screen::Title;
        } else if(screen==Screen::Settings) {
            auto& p=game.preferences;
            if(index==0) p.muted=!p.muted;
            else if(index==1) adjust(1);
            else if(index==2) { p.fullscreen=!p.fullscreen; applyFullscreen(); }
            else if(index==3) p.reducedMotion=!p.reducedMotion;
            else if(index==4) p.easy=!p.easy;
            else back();
            game.saveRequested=true;
        } else if(screen==Screen::Controls) back();
        else if(screen==Screen::Gallery) {
            if(index==0) adjust(-1); else if(index==1) adjust(1); else back();
        }
    }
    void update(float dt,bool testing) {
        if(game.screen!=lastScreen) { selected=0; lastScreen=game.screen; }
        if(!testing&&game.screen==Screen::Playing&&!IsWindowFocused()) game.screen=Screen::Paused;
        if(IsKeyPressed(KEY_F11)) {
            game.preferences.fullscreen=!game.preferences.fullscreen; applyFullscreen(); game.saveRequested=true;
        }
        if(IsKeyPressed(KEY_M)) { game.preferences.muted=!game.preferences.muted; game.saveRequested=true; }
        bool pause=IsKeyPressed(KEY_ESCAPE)||padPressed(GAMEPAD_BUTTON_MIDDLE_RIGHT);
        if(pause) { back(); return; }
        if(game.screen==Screen::Playing) {
            if(IsKeyPressed(KEY_TAB)||padPressed(GAMEPAD_BUTTON_MIDDLE_LEFT)) { game.mapOpen=!game.mapOpen; game.inventoryOpen=false; }
            if(IsKeyPressed(KEY_I)) { game.inventoryOpen=!game.inventoryOpen; game.mapOpen=false; }
            if(game.mapOpen||game.inventoryOpen) return;
            Input input;
            input.x=float(IsKeyDown(KEY_D))-float(IsKeyDown(KEY_A));
            input.y=float(IsKeyDown(KEY_S))-float(IsKeyDown(KEY_W));
            input.aimX=float(IsKeyDown(KEY_RIGHT))-float(IsKeyDown(KEY_LEFT));
            input.aimY=float(IsKeyDown(KEY_DOWN))-float(IsKeyDown(KEY_UP));
            if(IsGamepadAvailable(0)) {
                Vector2 stick{GetGamepadAxisMovement(0,GAMEPAD_AXIS_LEFT_X),GetGamepadAxisMovement(0,GAMEPAD_AXIS_LEFT_Y)};
                if(length(stick)>0.19f) { input.x+=stick.x; input.y+=stick.y; }
                Vector2 right{GetGamepadAxisMovement(0,GAMEPAD_AXIS_RIGHT_X),GetGamepadAxisMovement(0,GAMEPAD_AXIS_RIGHT_Y)};
                if(length(right)>.25f) { input.aimX=right.x; input.aimY=right.y; }
                input.x+=float(padDown(GAMEPAD_BUTTON_LEFT_FACE_RIGHT))-float(padDown(GAMEPAD_BUTTON_LEFT_FACE_LEFT));
                input.y+=float(padDown(GAMEPAD_BUTTON_LEFT_FACE_DOWN))-float(padDown(GAMEPAD_BUTTON_LEFT_FACE_UP));
            }
            input.shoot=IsKeyDown(KEY_SPACE)||padDown(GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
            if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                float scale=std::min(float(GetScreenWidth())/Width,float(GetScreenHeight())/Height);
                Vector2 offset{(GetScreenWidth()-Width*scale)/2,(GetScreenHeight()-Height*scale)/2};
                Vector2 direction=unit(sub(mul(sub(GetMousePosition(),offset),1/scale),add(game.player,{0,-26})));
                input.aimX=direction.x; input.aimY=direction.y; input.shoot=true;
            }
            input.help=IsKeyPressed(KEY_E)||IsKeyPressed(KEY_ENTER)||padPressed(GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
            input.dash=IsKeyPressed(KEY_LEFT_SHIFT)||IsKeyPressed(KEY_RIGHT_SHIFT)||IsKeyPressed(KEY_L)||padPressed(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
            game.update(dt,input);
        } else {
            bool up=IsKeyPressed(KEY_UP)||IsKeyPressed(KEY_W)||padPressed(GAMEPAD_BUTTON_LEFT_FACE_UP);
            bool down=IsKeyPressed(KEY_DOWN)||IsKeyPressed(KEY_S)||padPressed(GAMEPAD_BUTTON_LEFT_FACE_DOWN);
            int count=menuItemCount(game.screen,game.hasSave);
            if(up||down) {
                selected=(selected+(down?1:-1)+count)%std::max(1,count); mouseActive=false;
                game.cues.push_back(Cue::Dash);
            }
            if(game.screen==Screen::Reward) {
                if(IsKeyPressed(KEY_ONE)) { activate(0); return; }
                if(IsKeyPressed(KEY_TWO)) { activate(1); return; }
                if(IsKeyPressed(KEY_THREE)) { activate(2); return; }
                if(IsKeyPressed(KEY_LEFT)||padPressed(GAMEPAD_BUTTON_LEFT_FACE_LEFT)) { selected=(selected+3)%4; mouseActive=false; }
                if(IsKeyPressed(KEY_RIGHT)||padPressed(GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) { selected=(selected+1)%4; mouseActive=false; }
            }
            else if(IsKeyPressed(KEY_LEFT)||padPressed(GAMEPAD_BUTTON_LEFT_FACE_LEFT)) adjust(-1);
            if(game.screen!=Screen::Reward&&(IsKeyPressed(KEY_RIGHT)||padPressed(GAMEPAD_BUTTON_LEFT_FACE_RIGHT))) adjust(1);
            if(IsKeyPressed(KEY_ENTER)||IsKeyPressed(KEY_SPACE)||padPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) activate(selected);
            else if(padPressed(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) back();
            else if(game.screen==Screen::Lost&&IsKeyPressed(KEY_R)) game.retry();
        }
        if(!testing&&game.saveRequested) {
            if(!saveProfile(game,saveFile)) game.notify("No se pudo guardar aquí · La partida sigue disponible en esta sesión",5);
            game.saveRequested=false;
        }
    }
};
void captureState(App& app,int step) {
    auto& g=app.game;
    g.newGame(40211);g.invincible=0;g.showIntro=false;g.introTime=0;g.transitionTime=0;
    auto scene=[&](int floor,RoomKind kind) {
        g.loadStage(floor);
        for(int n=0;n<int(g.rooms.size());++n)if(g.rooms[n].kind==kind){g.enterRoom(n);break;}
        g.invincible=0;g.showIntro=false;g.introTime=g.transitionTime=0;
        g.player={510,455};g.companion={440,480};g.time=1.3f;g.noticeTime=0;
    };
    if(step==0){g.screen=Screen::Title;g.hasSave=false;}
    else if(step==1)scene(0,RoomKind::Combat);
    else if(step==2){scene(0,RoomKind::Combat);g.help();g.shoot();g.updateWisky(.05f);g.updateShots(.08f);g.pulse=.3f;}
    else if(step==3)scene(1,RoomKind::Combat);
    else if(step==4)scene(2,RoomKind::Boss);
    else if(step==5)scene(3,RoomKind::Boss);
    else if(step==6)scene(0,RoomKind::Treasure);
    else if(step==7){scene(1,RoomKind::Shop);g.coins=12;g.offerReward(true);}
    else if(step==8){scene(1,RoomKind::Combat);for(auto&r:g.rooms)r.visited=true;g.mapOpen=true;}
    else if(step==9){scene(2,RoomKind::Start);for(int n=0;n<ItemCount;++n)g.grantItem(ItemId(n),false);g.inventoryOpen=true;}
    else if(step==10||step==11){
        scene(3,RoomKind::Boss);for(auto&e:g.enemies)e.hp=0;g.removeDefeated();g.noticeTime=0;
        if(step==11){g.player=RoomCenter;g.pickups();g.player=MalikethPosition;g.interact();g.player={790,470};g.noticeTime=0;}
    }
    else if(step==12||step==13){g.screen=Screen::Gallery;app.gallery=step==12?2:3;}
    else if(step==14)g.screen=Screen::Settings;
    else if(step==15)g.screen=Screen::Controls;
    else if(step==16){scene(2,RoomKind::Combat);g.screen=Screen::Lost;g.hasSave=false;g.hp=0;}
    else if(step==17){g.screen=Screen::Won;g.runTime=1240;g.giovannyRescued=true;g.malikethFriendly=true;g.wiskyRecovered=true;}
    else if(step==18){scene(0,RoomKind::Treasure);g.offerReward();}
    else if(step==19)scene(0,RoomKind::Boss);
    if(step==4||step==5||step==19){g.enemies[0].windup=.6f;g.enemies[0].target=g.player;g.enemies[0].timer=0;}
}

}

int main(int argc,char** argv) {
    using namespace vivis;
    bool smoke=false; std::string capture;
    for(int n=1;n<argc;++n) {
        std::string arg=argv[n];
        if(arg=="--smoke-test") smoke=true;
        else if(arg=="--capture"&&n+1<argc) capture=argv[++n];
    }
    bool testing=smoke||!capture.empty();
    if(!capture.empty()) std::filesystem::create_directories(capture);
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_VSYNC_HINT|(testing?FLAG_WINDOW_UNDECORATED:0));
    InitWindow(Width,Height,"Sorpresa Vivis · Pequeñas aventuras, grandes sorpresas");
    if(!IsWindowReady()) return 2;
    SetWindowMinSize(960,540); SetExitKey(KEY_NULL); SetTargetFPS(testing?120:60);
    if(!testing) {
        int monitor=GetCurrentMonitor(),monitorWidth=GetMonitorWidth(monitor),monitorHeight=GetMonitorHeight(monitor);
        float fit=std::clamp(std::min((monitorWidth-48.0f)/Width,(monitorHeight-90.0f)/Height),0.75f,1.0f);
        int windowWidth=int(Width*fit),windowHeight=int(Height*fit);
        SetWindowSize(windowWidth,windowHeight);
        Vector2 monitorOrigin=GetMonitorPosition(monitor);
        SetWindowPosition(int(monitorOrigin.x)+(monitorWidth-windowWidth)/2,int(monitorOrigin.y)+(monitorHeight-windowHeight)/2);
    }
    Artwork art;
    art.characters=texture(embedded::characters,embedded::characters_size);
    art.emotions=texture(embedded::emotions,embedded::emotions_size);
    art.title=texture(embedded::title,embedded::title_size);
    art.font=font(48); art.smallFont=font(32);
    if(!art.characters.id||!art.emotions.id||!art.title.id||!art.font.texture.id) { CloseWindow(); return 3; }
    // The runtime icon uses the same bundled character art; no external files are required.
    Image icon=LoadImageFromMemory(".png",embedded::icon,int(embedded::icon_size));
    SetWindowIcon(icon); UnloadImage(icon);
    RenderTexture2D surface=LoadRenderTexture(Width,Height); SetTextureFilter(surface.texture,TEXTURE_FILTER_POINT);
    App app; app.game.loadStage(0); app.game.screen=Screen::Title; app.saveFile=profilePath();
    if(!testing) { loadProfile(app.game,app.saveFile); app.applyFullscreen(); }
    AudioSystem audio; audio.init(!testing);
    int frame=0,previousCapture=-1; float uiTime=0;
    static const char* captureNames[]={"menu","pachonidad","veneno","bigotes","wikerman","maliketh","tesoro","tienda","mapa","inventario","churu","amigos","giovanny","maliketh-retrato","ajustes","controles","derrota","victoria","premio","almohadon"};
    while(!WindowShouldClose()&&!app.quit) {
        float dt=testing?1.0f/60:std::min(GetFrameTime(),0.05f); uiTime+=dt;
        if(!capture.empty()) {
            int step=std::min(19,frame/12);
            if(step!=previousCapture) { captureState(app,step); previousCapture=step; }
        } else app.update(dt,testing);
        audio.update(app.game);
        float scale=std::min(float(GetScreenWidth())/Width,float(GetScreenHeight())/Height);
        Vector2 offset{(GetScreenWidth()-Width*scale)/2,(GetScreenHeight()-Height*scale)/2};
        Vector2 mouse=mul(sub(GetMousePosition(),offset),1/scale);
        Vector2 mouseDelta=GetMouseDelta();
        if(!testing&&length(mouseDelta)>0.5f) { app.mouseActive=true; app.mouseRecent=2; }
        else app.mouseRecent=std::max(0.0f,app.mouseRecent-dt);
        bool mouseVisible=app.game.screen!=Screen::Playing||app.game.mapOpen||app.game.inventoryOpen;
        if(mouseVisible) ShowCursor(); else HideCursor();
        int hover=-1;
        BeginTextureMode(surface); ClearBackground({11,19,28,255});
        if(app.game.screen==Screen::Playing||app.game.screen==Screen::Paused||app.game.screen==Screen::Lost||app.game.screen==Screen::Reward) {
            float shake=app.game.preferences.reducedMotion?0:app.game.shake;
            Camera2D camera{}; camera.zoom=1; camera.offset={std::sin(uiTime*65)*shake*13,std::cos(uiTime*72)*shake*10};
            BeginMode2D(camera); drawWorld(app.game); drawEffects(app.game,art); EndMode2D();
            drawHud(app.game,art);
            if(app.game.screen==Screen::Playing&&app.game.transitionTime>0)
                DrawRectangle(0,145,Width,535,Fade(BLACK,app.game.transitionTime/0.45f));
        }
        if(app.game.screen!=Screen::Playing) hover=drawMenu(app.game,art,app.game.screen,app.selected,mouse,
            app.mouseActive&&inside(mouse,{0,0,float(Width),float(Height)}),uiTime,app.gallery);
        if(app.game.mapOpen) hover=drawMap(app.game,art,mouse,app.mouseActive);
        else if(app.game.inventoryOpen) hover=drawInventory(app.game,art,mouse,app.mouseActive);
        if(app.game.screen==Screen::Playing&&!app.game.mapOpen&&!app.game.inventoryOpen&&app.mouseRecent>0&&inside(mouse,Field)) {
            DrawCircleLines(int(mouse.x),int(mouse.y),8,{251,212,232,220});
            DrawLine(int(mouse.x)-12,int(mouse.y),int(mouse.x)-5,int(mouse.y),{251,212,232,220});
            DrawLine(int(mouse.x)+5,int(mouse.y),int(mouse.x)+12,int(mouse.y),{251,212,232,220});
            DrawLine(int(mouse.x),int(mouse.y)-12,int(mouse.x),int(mouse.y)-5,{251,212,232,220});
            DrawLine(int(mouse.x),int(mouse.y)+5,int(mouse.x),int(mouse.y)+12,{251,212,232,220});
        }
        SetMouseCursor(hover>=0?MOUSE_CURSOR_POINTING_HAND:MOUSE_CURSOR_DEFAULT);
        EndTextureMode();
        BeginDrawing(); ClearBackground({5,9,14,255});
        DrawTexturePro(surface.texture,{0,0,float(Width),-float(Height)},
            {offset.x,offset.y,Width*scale,Height*scale},{0,0},0,WHITE);
        EndDrawing();
        if(!testing&&(app.game.screen!=Screen::Playing||app.game.mapOpen||app.game.inventoryOpen)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&hover>=0) {
            if(hover<100) app.selected=hover;
            app.activate(hover);
        }
        if(!capture.empty()&&frame%12==10) {
            Image screenshot=LoadImageFromScreen();
            ExportImage(screenshot,(std::filesystem::path(capture)/(std::string(captureNames[previousCapture])+".png")).string().c_str());
            UnloadImage(screenshot);
        }
        ++frame;
        if((smoke&&frame>=120)||(!capture.empty()&&frame>=240)) break;
    }
    if(!testing) saveProfile(app.game,app.saveFile);
    audio.close(); UnloadRenderTexture(surface); UnloadFont(art.font); UnloadFont(art.smallFont);
    UnloadTexture(art.characters); UnloadTexture(art.emotions); UnloadTexture(art.title);
    CloseWindow(); return 0;
}
