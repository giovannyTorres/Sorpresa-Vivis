#pragma once
#include "game.hpp"
namespace vivis {
struct Artwork { Texture2D characters{}, emotions{}, title{}; Font font{}, smallFont{}; };
void drawWorld(const Game& game);
void drawCharacter(const Artwork& art,int row,int pose,Vector2 feet,float height=108,bool flip=false,Color tint=WHITE);
void drawPortrait(const Artwork& art,int row,Rectangle target,bool victory=false);
void drawEffects(const Game& game,const Artwork& art);
void drawHud(const Game& game,const Artwork& art);
// Hover index, or 100+portrait, 201/202 for volume arrows. App handles activation.
int drawMenu(const Game& game,const Artwork& art,Screen screen,int selected,Vector2 mouse,
             bool mouseActive,float uiTime,int galleryCharacter=0);
int menuItemCount(Screen screen,bool hasSave);
const char* menuItem(Screen screen,int index,bool hasSave);
// Rewards: 0..2 selects an offer; 3 returns to the room. Overlays: 0 closes.
int drawReward(const Game& game,const Artwork& art,int selected,Vector2 mouse,bool mouseActive);
int drawMap(const Game& game,const Artwork& art,Vector2 mouse,bool mouseActive);
int drawInventory(const Game& game,const Artwork& art,Vector2 mouse,bool mouseActive);
}
