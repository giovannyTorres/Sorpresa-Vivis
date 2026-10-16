#pragma once
#include "game.hpp"
#include <filesystem>
namespace vivis {
std::filesystem::path profilePath();
bool loadProfile(Game& game,const std::filesystem::path& path);
bool saveProfile(const Game& game,const std::filesystem::path& path);
}
