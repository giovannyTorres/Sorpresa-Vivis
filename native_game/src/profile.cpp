#include "profile.hpp"
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace vivis {
std::filesystem::path profilePath() {
#ifdef _WIN32
    if(const char* base=std::getenv("LOCALAPPDATA")) return std::filesystem::path(base)/"SorpresaVivis"/"profile.txt";
    if(const char* base=std::getenv("APPDATA")) return std::filesystem::path(base)/"SorpresaVivis"/"profile.txt";
#else
    if(const char* base=std::getenv("XDG_DATA_HOME")) return std::filesystem::path(base)/"sorpresa-vivis"/"profile.txt";
    if(const char* base=std::getenv("HOME")) return std::filesystem::path(base)/".local"/"share"/"sorpresa-vivis"/"profile.txt";
#endif
    return std::filesystem::temp_directory_path()/"sorpresa-vivis"/"profile.txt";
}
namespace {
bool read(Game& game,const std::filesystem::path& path) {
    std::ifstream input(path); std::string signature;
    if(!std::getline(input,signature)||signature!="VIVIS_PROFILE_V2") return false;
    int checkpoint=-1; float elapsed=0,volume=0.55f; int mute=0,fullscreen=0,reduced=0,easy=0;
    if(!(input>>checkpoint>>elapsed>>volume>>mute>>fullscreen>>reduced>>easy)) return false;
    if(checkpoint< -1||checkpoint>2||!std::isfinite(elapsed)||elapsed<0||elapsed>604800||
       !std::isfinite(volume)||volume<0||volume>1) return false;
    if((mute!=0&&mute!=1)||(fullscreen!=0&&fullscreen!=1)||(reduced!=0&&reduced!=1)||(easy!=0&&easy!=1)) return false;
    game.hasSave=checkpoint>=0; game.checkpoint=std::max(0,checkpoint); game.runTime=elapsed;
    game.preferences={volume,bool(mute),bool(fullscreen),bool(reduced),bool(easy)};
    return true;
}
}
bool loadProfile(Game& game,const std::filesystem::path& path) {
    if(read(game,path)) return true;
    auto backup=path; backup+=".bak";
    return read(game,backup);
}
bool saveProfile(const Game& game,const std::filesystem::path& path) {
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(),error);
    if(error) return false;
    auto temporary=path; temporary+=".tmp";
    auto backup=path; backup+=".bak";
    {
        std::ofstream output(temporary,std::ios::trunc);
        if(!output) return false;
        const auto& prefs=game.preferences;
        output<<"VIVIS_PROFILE_V2\n"<<(game.hasSave?game.checkpoint:-1)<<' '
              <<std::setprecision(9)<<game.runTime<<' '<<prefs.volume<<' '<<int(prefs.muted)<<' '
              <<int(prefs.fullscreen)<<' '<<int(prefs.reducedMotion)<<' '<<int(prefs.easy)<<'\n';
        output.flush();
        if(!output) return false;
    }
    if(std::filesystem::exists(path,error)) {
        std::filesystem::copy_file(path,backup,std::filesystem::copy_options::overwrite_existing,error);
        if(error) return false;
        std::filesystem::remove(path,error);
        if(error) return false;
    }
    std::filesystem::rename(temporary,path,error);
    return !error;
}
}
