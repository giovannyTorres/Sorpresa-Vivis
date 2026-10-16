#include "visuals.hpp"
#include "profile.hpp"
#include "assets_embedded.hpp"
#include <array>
#include <cstdint>
#include <filesystem>
#include <string>

namespace {
using namespace vivis;
struct Audio {
    std::array<Sound,9> effects{};
    Sound music{};
    bool ready{};
    Sound synth(float frequency,float duration,bool ambient=false) {
        const int rate=22050,frames=int(duration*rate);
        std::vector<std::int16_t> samples(frames);
        constexpr float notes[]={220.0f,261.63f,329.63f,293.66f,196.0f,246.94f,293.66f,261.63f};
        for(int n=0;n<frames;++n) {
            float t=float(n)/rate;
            float f=ambient?notes[int(t*2)%8]:frequency;
            float envelope=ambient?0.025f*(0.45f+0.55f*std::sin(3.14159265f*std::fmod(t*2,1.0f))):
                0.16f*std::min(1.0f,t/0.008f)*(1-t/duration);
            float wave=std::sin(6.2831853f*f*t)+0.2f*std::sin(6.2831853f*f*2*t);
            samples[n]=std::int16_t(wave*envelope*32700);
        }
        Wave wave{static_cast<unsigned>(frames),rate,16,1,samples.data()};
        return LoadSoundFromWave(wave);
    }
    void init(bool enabled) {
        if(!enabled) return;
        InitAudioDevice(); ready=IsAudioDeviceReady();
        if(!ready) return;
        float frequencies[]={740,120,880,1040,420,520,1175,680,960};
        for(std::size_t n=0;n<effects.size();++n) effects[n]=synth(frequencies[n],n==6?0.35f:0.09f);
        music=synth(220,8,true);
    }
    void update(Game& game) {
        if(!ready) { game.cues.clear(); return; }
        SetMasterVolume(game.preferences.muted?0:game.preferences.volume);
        if(game.preferences.muted) StopSound(music);
        else if(!IsSoundPlaying(music)) PlaySound(music);
        for(Cue cue:game.cues) PlaySound(effects[static_cast<int>(cue)]);
        game.cues.clear();
    }
    void close() {
        if(!ready) return;
        for(auto sound:effects) UnloadSound(sound);
        UnloadSound(music); CloseAudioDevice();
    }
};
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
        if(game.screen==Screen::Paused) game.screen=Screen::Playing;
        else if(game.screen==Screen::Settings||game.screen==Screen::Controls||game.screen==Screen::Gallery) {
            game.screen=returnTo; game.saveRequested=true;
        } else if(game.screen==Screen::Playing) game.screen=Screen::Paused;
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
            Input input;
            input.x=float(IsKeyDown(KEY_D)||IsKeyDown(KEY_RIGHT))-float(IsKeyDown(KEY_A)||IsKeyDown(KEY_LEFT));
            input.y=float(IsKeyDown(KEY_S)||IsKeyDown(KEY_DOWN))-float(IsKeyDown(KEY_W)||IsKeyDown(KEY_UP));
            if(IsGamepadAvailable(0)) {
                Vector2 stick{GetGamepadAxisMovement(0,GAMEPAD_AXIS_LEFT_X),GetGamepadAxisMovement(0,GAMEPAD_AXIS_LEFT_Y)};
                if(length(stick)>0.19f) { input.x+=stick.x; input.y+=stick.y; }
                input.x+=float(padDown(GAMEPAD_BUTTON_LEFT_FACE_RIGHT))-float(padDown(GAMEPAD_BUTTON_LEFT_FACE_LEFT));
                input.y+=float(padDown(GAMEPAD_BUTTON_LEFT_FACE_DOWN))-float(padDown(GAMEPAD_BUTTON_LEFT_FACE_UP));
            }
            input.shoot=IsKeyDown(KEY_SPACE)||padDown(GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
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
            if(IsKeyPressed(KEY_LEFT)||padPressed(GAMEPAD_BUTTON_LEFT_FACE_LEFT)) adjust(-1);
            if(IsKeyPressed(KEY_RIGHT)||padPressed(GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) adjust(1);
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
    if(step==0) { g.screen=Screen::Title; g.hasSave=false; }
    else if(step>=1&&step<=4) {
        g.screen=Screen::Playing; g.loadStage(step<=2?0:step-2); g.invincible=0; g.showIntro=false; g.transitionTime=0;
        g.player={510,420}; g.companion={440,460};
        if(step==2) { g.time=1.3f; g.shoot(); g.help(); g.updateShots(.06f); g.pulse=.3f; }
    } else if(step==5) { g.screen=Screen::Gallery; app.gallery=0; }
    else if(step==6) g.screen=Screen::Settings;
    else if(step==7) g.screen=Screen::Controls;
    else if(step==8) g.screen=Screen::Lost;
    else if(step==9) { g.screen=Screen::Won; g.runTime=154; }
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
    Audio audio; audio.init(!testing);
    int frame=0,previousCapture=-1; float uiTime=0;
    static const char* captureNames[]={"menu","pradera","magia","sendero","torre","personajes","ajustes","controles","derrota","victoria"};
    while(!WindowShouldClose()&&!app.quit) {
        float dt=testing?1.0f/60:std::min(GetFrameTime(),0.05f); uiTime+=dt;
        if(!capture.empty()) {
            int step=std::min(9,frame/12);
            if(step!=previousCapture) { captureState(app,step); previousCapture=step; }
        } else app.update(dt,testing);
        audio.update(app.game);
        float scale=std::min(float(GetScreenWidth())/Width,float(GetScreenHeight())/Height);
        Vector2 offset{(GetScreenWidth()-Width*scale)/2,(GetScreenHeight()-Height*scale)/2};
        Vector2 mouse=mul(sub(GetMousePosition(),offset),1/scale);
        Vector2 mouseDelta=GetMouseDelta();
        if(!testing&&length(mouseDelta)>0.5f) { app.mouseActive=true; app.mouseRecent=2; }
        else app.mouseRecent=std::max(0.0f,app.mouseRecent-dt);
        bool mouseVisible=app.game.screen!=Screen::Playing;
        if(mouseVisible) ShowCursor(); else HideCursor();
        int hover=-1;
        BeginTextureMode(surface); ClearBackground({11,19,28,255});
        if(app.game.screen==Screen::Playing||app.game.screen==Screen::Paused||app.game.screen==Screen::Lost) {
            float shake=app.game.preferences.reducedMotion?0:app.game.shake;
            Camera2D camera{}; camera.zoom=1; camera.offset={std::sin(uiTime*65)*shake*13,std::cos(uiTime*72)*shake*10};
            BeginMode2D(camera); drawWorld(app.game); drawEffects(app.game,art); EndMode2D();
            drawHud(app.game,art);
            if(app.game.screen==Screen::Playing&&app.game.transitionTime>0)
                DrawRectangle(0,145,Width,535,Fade(BLACK,app.game.transitionTime/0.45f));
        }
        if(app.game.screen!=Screen::Playing) hover=drawMenu(app.game,art,app.game.screen,app.selected,mouse,
            app.mouseActive&&inside(mouse,{0,0,float(Width),float(Height)}),uiTime,app.gallery);
        SetMouseCursor(hover>=0?MOUSE_CURSOR_POINTING_HAND:MOUSE_CURSOR_DEFAULT);
        EndTextureMode();
        BeginDrawing(); ClearBackground({5,9,14,255});
        DrawTexturePro(surface.texture,{0,0,float(Width),-float(Height)},
            {offset.x,offset.y,Width*scale,Height*scale},{0,0},0,WHITE);
        EndDrawing();
        if(!testing&&app.game.screen!=Screen::Playing&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&hover>=0) {
            if(hover<100) app.selected=hover;
            app.activate(hover);
        }
        if(!capture.empty()&&frame%12==10) {
            Image screenshot=LoadImageFromScreen();
            ExportImage(screenshot,(std::filesystem::path(capture)/(std::string(captureNames[previousCapture])+".png")).string().c_str());
            UnloadImage(screenshot);
        }
        ++frame;
        if((smoke&&frame>=120)||(!capture.empty()&&frame>=120)) break;
    }
    if(!testing) saveProfile(app.game,app.saveFile);
    audio.close(); UnloadRenderTexture(surface); UnloadFont(art.font); UnloadFont(art.smallFont);
    UnloadTexture(art.characters); UnloadTexture(art.emotions); UnloadTexture(art.title);
    CloseWindow(); return 0;
}
