#include "game.hpp"
#include "profile.hpp"
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace vivis;

namespace {
int passed = 0;
int failed = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

void near(float actual, float expected, float tolerance, const std::string& message) {
    expect(std::fabs(actual - expected) <= tolerance,
           message + " (actual=" + std::to_string(actual) +
           ", expected=" + std::to_string(expected) + ")");
}

void scenario(const char* name, const std::function<void()>& test) {
    try {
        test();
        ++passed;
        std::cout << "PASS " << name << '\n';
    } catch (const std::exception& error) {
        ++failed;
        std::cerr << "FAIL " << name << ": " << error.what() << '\n';
    }
}

Game arena() {
    Game game;
    game.newGame();
    game.obstacles.clear();
    game.enemies.clear();
    game.hearts.clear();
    game.bell = {1100, 600};
    game.invincible = 0;
    game.cues.clear();
    game.saveRequested = false;
    return game;
}

void frames(Game& game, int count, Input input = {}, float dt = 1.0f / 60) {
    for (int i = 0; i < count; ++i) game.update(dt, input);
}

int cues(const Game& game, Cue value) {
    return static_cast<int>(std::count(game.cues.begin(), game.cues.end(), value));
}

void walkTo(Game& game, Vector2 target) {
    for (int i = 0; i < 1200 && distance(game.player, target) > 5; ++i) {
        Vector2 direction = unit(sub(target, game.player));
        game.update(1.0f / 60, {direction.x, direction.y});
        expect(game.screen == Screen::Playing, "journey left the playing state while walking");
    }
    expect(distance(game.player, target) <= 5, "waypoint was not reachable");
}

struct TemporaryProfile {
    std::filesystem::path root;
    std::filesystem::path path;
    TemporaryProfile() {
        static unsigned serial = 0;
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        root = std::filesystem::temp_directory_path() /
               ("sorpresa-vivis-qa-" + std::to_string(stamp) + "-" + std::to_string(serial++));
        path = root / "nested" / "profile.txt";
    }
    ~TemporaryProfile() {
        std::error_code error;
        std::filesystem::remove_all(root, error);
    }
    void write(const std::string& contents) const {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream output(path, std::ios::trunc);
        output << contents;
    }
};
}

int main() {
    scenario("title does not simulate movement or combat", [] {
        Game game;
        const Vector2 start = game.player;
        game.update(1.0f / 60, {1, 0, true, true, true});
        expect(game.screen == Screen::Title && game.runTime == 0,
               "title advanced the run");
        near(distance(game.player, start), 0, 0.001f, "title moved player");
        expect(game.shots.empty() && game.cues.empty(), "title accepted combat input");
    });

    scenario("new run is playable and creates a recoverable first checkpoint", [] {
        Game game;
        game.newGame();
        expect(game.screen == Screen::Playing && game.stage == 0 && game.hp == MaxHp,
               "run did not initialize playable first zone");
        expect(game.hasSave && game.saveRequested && game.checkpoint == 0,
               "first zone has no checkpoint");
        expect(game.walkable(game.player) && !game.enemies.empty(), "invalid first zone");
    });

    scenario("one second of directional input moves at the walking speed", [] {
        Game game = arena();
        Vector2 start = game.player;
        frames(game, 60, {1, 0});
        near(game.player.x - start.x, 270, 0.05f, "walking distance");
        near(game.player.y, start.y, 0.01f, "horizontal input drifted vertically");
        expect(game.moving && game.facing.x > 0.99f, "walking animation/facing did not update");
    });

    scenario("diagonal input cannot move faster than straight input", [] {
        Game straight = arena(), diagonal = arena();
        const Vector2 start = straight.player;
        frames(straight, 30, {1, 0});
        frames(diagonal, 30, {1, -1});
        near(distance(diagonal.player, start), distance(straight.player, start),
             0.05f, "diagonal speed boost");
        near(length(diagonal.facing), 1, 0.001f, "diagonal facing is not normalized");
    });

    scenario("movement is consistent at 30 60 and 120 FPS", [] {
        Game low = arena(), medium = arena(), high = arena();
        frames(low, 30, {1, 0}, 1.0f / 30);
        frames(medium, 60, {1, 0});
        frames(high, 120, {1, 0}, 1.0f / 120);
        near(distance(low.player, medium.player), 0, 0.05f, "30 FPS differs from 60 FPS");
        near(distance(high.player, medium.player), 0, 0.05f, "120 FPS differs from 60 FPS");
    });

    scenario("partial analog input preserves fine walking control", [] {
        Game game = arena();
        float start = game.player.x;
        frames(game, 60, {0.5f, 0});
        near(game.player.x - start, 135, 0.05f, "analog input lost its magnitude");
    });

    scenario("invalid delta does not alter the game and long frames are bounded", [] {
        Game game = arena();
        Vector2 start = game.player;
        game.update(0, {1, 0, true});
        game.update(-1, {1, 0, true});
        expect(game.runTime == 0 && game.shots.empty(), "nonpositive delta simulated");
        game.update(10, {1, 0});
        expect(distance(start, game.player) < 14 && game.runTime <= 0.051f,
               "stalled frame teleported player");
    });

    scenario("field edges contain the player's full collision radius", [] {
        Game game = arena();
        frames(game, 240, {-1, -1});
        expect(game.player.x >= Field.x + 17 && game.player.y >= Field.y + 17,
               "player escaped upper or left boundary");
        frames(game, 600, {1, 1});
        expect(game.player.x < Field.x + Field.width - 17 &&
               game.player.y < Field.y + Field.height - 17,
               "player escaped lower or right boundary");
    });

    scenario("walking stops at solid obstacles", [] {
        Game game = arena();
        game.obstacles = {{300, 300, 80, 220}};
        frames(game, 120, {1, 0});
        expect(game.player.x < 284 && game.walkable(game.player),
               "walking crossed wall or left player embedded");
    });

    scenario("sliding along a wall preserves the free movement axis", [] {
        Game game = arena();
        game.player = {280, 350};
        game.obstacles = {{300, 250, 80, 350}};
        frames(game, 60, {1, 1});
        expect(game.player.x < 284 && game.player.y > 500,
               "diagonal wall contact prevented sliding or crossed wall");
    });

    scenario("large displacements cannot tunnel through a narrow obstacle", [] {
        Game game = arena();
        game.obstacles = {{300, 300, 8, 220}};
        Vector2 result = game.move(game.player, {900, 0});
        expect(result.x < 284 && game.walkable(result), "swept movement tunnelled through wall");
    });

    scenario("dash is faster but respects walls and cannot be spammed", [] {
        Game game = arena();
        game.player = {245, 420};
        game.obstacles = {{300, 300, 8, 220}};
        game.update(1.0f / 60, {1, 0, false, false, true});
        expect(game.dashCooldown > 1.9f && game.invincible > 0,
               "dash has no cooldown or protection");
        frames(game, 15, {1, 0, false, false, true});
        expect(game.player.x < 284 && cues(game, Cue::Dash) == 1,
               "dash crossed wall or retriggered during cooldown");
        expect(game.dashTime == 0, "dash never ended");
    });

    scenario("dash without movement follows the last facing direction", [] {
        Game game = arena();
        game.facing = {-1, 0};
        const float start = game.player.x;
        game.update(1.0f / 60, {0, 0, false, false, true});
        expect(game.player.x < start - 10, "stationary dash ignored facing");
    });

    scenario("enemy pursuit respects obstacles over multiple frames", [] {
        Game game = arena();
        game.player = {500, 420};
        game.obstacles = {{300, 170, 60, 460}};
        game.enemies = {{{245, 420}, 3}};
        frames(game, 120);
        expect(game.enemies.front().pos.x < 281 &&
               game.walkable(game.enemies.front().pos, 20), "enemy pursuit crossed wall");
    });

    scenario("autoaim skips a closer enemy hidden behind a wall", [] {
        Game game = arena();
        game.player = {200, 400};
        game.obstacles = {{290, 360, 70, 90}};
        game.enemies = {{{400, 400}, 3}, {{200, 600}, 3}};
        game.shoot();
        expect(game.shots.size() == 1 && std::fabs(game.shots[0].velocity.x) < 0.01f &&
               game.shots[0].velocity.y > 640, "autoaim chose an occluded target");
    });

    scenario("projectiles stop at walls without damaging the enemy beyond", [] {
        Game game = arena();
        game.player = {200, 400};
        game.facing = {1, 0};
        game.obstacles = {{290, 360, 70, 90}};
        game.enemies = {{{400, 400}, 3}};
        game.shoot();
        game.updateShots(0.5f);
        expect(game.enemies[0].hp == 3 && game.shots.empty(), "heart shot passed through wall");
    });

    scenario("swept projectiles hit targets they cross within one frame", [] {
        Game game = arena();
        game.enemies = {{{500, 420}, 3}};
        game.shots = {{{300, 420}, {1000, 0}, 1}};
        game.updateShots(0.3f);
        expect(game.enemies[0].hp == 2 && game.shots.empty(), "fast shot tunnelled through enemy");
    });

    scenario("held fire repeats at a controlled rate", [] {
        Game game = arena();
        frames(game, 60, {0, 0, true});
        expect(cues(game, Cue::Shoot) >= 4 && cues(game, Cue::Shoot) <= 5,
               "held fire stalled or bypassed shot cooldown");
    });

    scenario("combat kill is removed and counted exactly once", [] {
        Game game = arena();
        game.enemies = {{{500, 420}, 3}};
        for (int i = 0; i < 3; ++i) {
            game.shots.push_back({{450, 420}, {1000, 0}, 1});
            game.updateShots(0.05f);
        }
        expect(game.enemies.empty() && game.defeatedCount == 1, "enemy did not die after three hits");
        game.removeDefeated();
        expect(game.defeatedCount == 1, "kill counted more than once");
    });

    scenario("spawn grace protects player before contact becomes damaging", [] {
        Game game;
        game.newGame();
        game.obstacles.clear();
        game.enemies = {{game.player, 100}};
        frames(game, 30);
        expect(game.hp == MaxHp, "spawn grace allowed immediate contact damage");
        frames(game, 50);
        expect(game.hp == MaxHp - 1, "enemy contact never damaged after grace");
    });

    scenario("damage grants invulnerability and later allows a fresh hit", [] {
        Game game = arena();
        game.damage();
        game.damage();
        expect(game.hp == MaxHp - 1 && cues(game, Cue::Hit) == 1,
               "same-frame damage bypassed invulnerability");
        frames(game, 68);
        game.damage();
        expect(game.hp == MaxHp - 2 && cues(game, Cue::Hit) == 2,
               "invulnerability never expired");
    });

    scenario("relaxed mode lengthens protection and reduced motion removes shake", [] {
        Game standard = arena(), relaxed = arena();
        relaxed.preferences.easy = true;
        relaxed.preferences.reducedMotion = true;
        standard.damage();
        relaxed.damage();
        frames(standard, 72);
        frames(relaxed, 72);
        standard.damage();
        relaxed.damage();
        expect(standard.hp == MaxHp - 2 && relaxed.hp == MaxHp - 1,
               "relaxed mode protection did not outlast standard protection");
        expect(relaxed.shake == 0 && standard.shake > 0, "reduced motion still shakes screen");
    });

    scenario("Wiky magic reaches nearby targets and respects its range", [] {
        Game game = arena();
        game.enemies = {{{300, 420}, 3}, {{500, 420}, 3}};
        game.update(1.0f / 60, {0, 0, false, true});
        expect(game.enemies.size() == 1 && game.enemies[0].hp == 3 &&
               game.defeatedCount == 1, "Wiky missed nearby target or hit a distant target");
        expect(game.helpCooldown > 4.9f && game.pulse > 0 && game.helpCastTime > 0,
               "Wiky magic lacks cooldown/visible feedback");
    });

    scenario("Wiky cooldown blocks repeat use and recovers during play", [] {
        Game game = arena();
        game.enemies = {{{300, 420}, 20}};
        game.help();
        game.help();
        expect(game.enemies[0].hp == 16 && cues(game, Cue::Help) == 1,
               "Wiky repeat use bypassed cooldown");
        frames(game, 310);
        game.help();
        expect(cues(game, Cue::Help) == 2 && game.enemies[0].hp == 12,
               "Wiky cooldown did not recover");
    });

    scenario("Maliketh's shield blocks hearts and Wiky before the bell", [] {
        Game game = arena();
        game.loadStage(2);
        game.player = {850, 400};
        game.invincible = 1;
        game.shots = {{{850, 400}, {1000, 0}, 1}};
        game.updateShots(0.05f);
        game.help();
        expect(game.enemies[0].hp == 38 && !game.bossDefeated,
               "boss took damage through the shield");
    });

    scenario("collecting the boss bell unlocks both kinds of damage", [] {
        Game game = arena();
        game.loadStage(2);
        game.player = game.bell;
        game.update(1.0f / 60, {});
        expect(game.collected, "bell did not break boss shield");
        game.player = {850, 400};
        game.enemies[0].pos = {900, 400};
        game.shots = {{{850, 400}, {1000, 0}, 1}};
        game.updateShots(0.05f);
        game.help();
        expect(game.enemies[0].hp == 33, "boss remained immune after bell");
    });

    scenario("crossing lunar hazards damage once even when multiple overlap", [] {
        Game game = arena();
        game.hazards = {{{80, 420}, {1000, 0}, 2}, {{80, 420}, {1000, 0}, 2}};
        game.updateHazards(0.1f);
        expect(game.hp == MaxHp - 1 && game.hazards.empty(),
               "hazard sweep missed player or overlap bypassed invulnerability");
    });

    scenario("expired lunar hazards cannot damage the player", [] {
        Game game = arena();
        game.hazards = {{{80, 420}, {1000, 0}, 0.01f}};
        game.updateHazards(0.1f);
        expect(game.hp == MaxHp && game.hazards.empty(),
               "an already-expired projectile still damaged player");
    });

    scenario("dash protection survives overlapping enemy and lunar hazard", [] {
        Game game = arena();
        game.enemies = {{game.player, 100}};
        game.hazards = {{game.player, {}, 2}};
        game.update(1.0f / 60, {1, 0, false, false, true});
        expect(game.hp == MaxHp, "dash protection failed during same-frame collision");
    });

    scenario("boss launches a stronger fan during its second phase", [] {
        Game game = arena();
        game.loadStage(2);
        game.enemies[0].timer = 0;
        game.update(1.0f / 60, {});
        expect(game.hazards.size() == 3, "first boss phase fan is missing");
        game.hazards.clear();
        game.enemies[0].hp = 18;
        game.enemies[0].timer = 0;
        game.update(1.0f / 60, {});
        expect(game.hazards.size() == 5 && game.enemies[0].phase == 1,
               "low-health boss phase did not change attack");
    });

    scenario("relaxed mode slows lunar hazards and boss attack frequency", [] {
        Game standard = arena(), relaxed = arena();
        standard.loadStage(2);
        relaxed.preferences.easy = true;
        relaxed.loadStage(2);
        standard.enemies[0].timer = relaxed.enemies[0].timer = 0;
        standard.update(1.0f / 60, {});
        relaxed.update(1.0f / 60, {});
        expect(length(relaxed.hazards[1].velocity) < length(standard.hazards[1].velocity),
               "relaxed hazards are not slower");
        expect(relaxed.enemies[0].timer > standard.enemies[0].timer,
               "relaxed boss attacks just as frequently");
    });

    scenario("health pickup heals without exceeding max and full health preserves it", [] {
        Game game = arena();
        game.hearts = {game.player};
        game.update(1.0f / 60, {});
        expect(game.hearts.size() == 1, "full-health player wasted heart pickup");
        game.hp = MaxHp - 1;
        game.update(1.0f / 60, {});
        expect(game.hp == MaxHp && game.hearts.empty() && cues(game, Cue::Heal) == 1,
               "heart pickup failed to heal, cap, or remove once");
    });

    scenario("bell pickup emits its feedback only once", [] {
        Game game = arena();
        game.player = game.bell;
        frames(game, 10);
        expect(game.collected && cues(game, Cue::Bell) == 1,
               "bell pickup repeated every frame or did not collect");
    });

    scenario("exit waits for both the bell and all enemies", [] {
        Game game = arena();
        game.player = {1170, 420};
        game.update(1.0f / 60, {});
        expect(game.stage == 0, "exit opened without bell");
        game.collected = true;
        game.enemies = {{{800, 420}, 3}};
        game.update(1.0f / 60, {});
        expect(game.stage == 0, "exit opened with enemies remaining");
        game.enemies.clear();
        game.update(1.0f / 60, {});
        expect(game.stage == 1 && game.checkpoint == 1 && game.saveRequested,
               "valid exit failed to advance and save checkpoint");
    });

    scenario("zone transition resets hazards cooldowns pickups and health", [] {
        Game game = arena();
        game.player = {1170, 420};
        game.collected = true;
        game.hp = 2;
        game.helpCooldown = 4;
        game.hazards.push_back({{600, 600}, {}, 4});
        game.shots.push_back({{600, 600}, {}, 4});
        game.update(1.0f / 60, {});
        expect(game.stage == 1 && game.hp == MaxHp && !game.collected &&
               game.helpCooldown == 0 && game.hazards.empty() && game.shots.empty(),
               "previous zone state leaked through transition");
        expect(game.showIntro && cues(game, Cue::Transition) == 1,
               "transition gives no introduction or feedback");
    });

    scenario("checkpoint continue restores the saved zone with fresh state", [] {
        Game game = arena();
        game.loadStage(1);
        game.requestCheckpoint();
        game.loadStage(2);
        game.hp = 1;
        game.screen = Screen::Title;
        game.resumeCheckpoint();
        expect(game.screen == Screen::Playing && game.stage == 1 && game.hp == MaxHp &&
               !game.collected && game.enemies.size() == 4,
               "continue did not restore a playable checkpoint");
    });

    scenario("pausing freezes timers movement enemies hazards and actions", [] {
        Game game = arena();
        game.enemies = {{{600, 420}, 3}};
        game.hazards = {{{300, 420}, {-100, 0}, 4}};
        game.helpCooldown = 3;
        game.screen = Screen::Paused;
        Vector2 player = game.player, enemy = game.enemies[0].pos;
        frames(game, 120, {1, 0, true, true, true});
        game.shoot();
        game.help();
        game.dash();
        game.damage();
        expect(game.runTime == 0 && game.helpCooldown == 3 && game.hp == MaxHp &&
               game.shots.empty() && game.hazards[0].life == 4 && game.cues.empty(),
               "pause accepted an action or advanced timers");
        near(distance(player, game.player) + distance(enemy, game.enemies[0].pos),
             0, 0.001f, "pause moved an actor");
    });

    scenario("fatal damage freezes play and retry restarts the current zone", [] {
        Game game = arena();
        game.loadStage(1);
        game.invincible = 0;
        game.hp = 1;
        game.damage();
        expect(game.screen == Screen::Lost && game.hp == 0, "fatal damage did not show defeat");
        Vector2 end = game.player;
        frames(game, 60, {1, 0, true, true, true});
        near(distance(end, game.player), 0, 0.001f, "defeated player could move");
        game.retry();
        expect(game.screen == Screen::Playing && game.stage == 1 && game.hp == MaxHp &&
               game.enemies.size() == 4 && game.shots.empty() && !game.collected,
               "retry failed to rebuild current zone");
    });

    scenario("boss defeat clears dangers and rescue requires proximity and input", [] {
        Game game = arena();
        game.loadStage(2);
        game.collected = true;
        game.hazards = {{{200, 200}, {}, 3}};
        game.damageEnemy(game.enemies[0], 38);
        game.removeDefeated();
        expect(game.bossDefeated && game.enemies.empty() && game.hazards.empty(),
               "boss defeat left combat dangers behind");
        game.help();
        expect(game.screen == Screen::Playing, "rescue succeeded from across the arena");
        game.player = {1100, 295};
        game.update(1.0f / 60, {});
        expect(game.screen == Screen::Playing, "rescue did not wait for interaction");
        game.update(1.0f / 60, {0, 0, false, true});
        expect(game.screen == Screen::Won && !game.hasSave && game.saveRequested,
               "rescue interaction did not finish run and clear save");
        float endTime = game.runTime;
        frames(game, 60, {1, 0, true, true, true});
        expect(game.screen == Screen::Won && game.runTime == endTime,
               "completed run kept simulating");
    });

    scenario("restarting after victory creates an independent new run", [] {
        Game game = arena();
        game.screen = Screen::Won;
        game.runTime = 150;
        game.defeatedCount = 8;
        game.checkpoint = 2;
        game.newGame();
        expect(game.stage == 0 && game.checkpoint == 0 && game.defeatedCount == 0 &&
               game.runTime == 0 && game.hasSave && game.screen == Screen::Playing,
               "new run inherited completed-run progress");
    });

    scenario("complete three-zone loop uses movement fire transitions and rescue", [] {
        Game game;
        game.preferences.easy = true;
        game.newGame();
        // Place foes along an unobstructed training lane; damage, cooldowns,
        // gate activation, checkpoint writes and final interaction stay real.
        for (int stage = 0; stage < 2; ++stage) {
            expect(game.stage == stage, "journey entered wrong stage");
            game.player = {800, 420};
            game.enemies = {{{970, 420}, 3}, {{1060, 420}, 3}};
            frames(game, 180, {0, 0, true});
            expect(game.enemies.empty() && game.screen == Screen::Playing,
                   "ordinary combat could not clear zone");
            walkTo(game, {game.bell.x, 420});
            walkTo(game, game.bell);
            expect(game.collected, "journey failed to collect bell");
            walkTo(game, {1100, 420});
            frames(game, 20, {1, 0});
            expect(game.stage == stage + 1 && game.checkpoint == stage + 1,
                   "walking through cleared exit failed");
        }
        game.player = game.bell;
        game.update(1.0f / 60, {});
        expect(game.collected, "final zone bell did not unlock boss");
        // Move the boss into the lane and suppress attacks for this combat-only
        // section; dedicated cases above verify both lunar attack phases.
        game.enemies[0].timer = 100;
        game.player = {640, 400};
        for (int frame = 0; frame < 1200 && !game.bossDefeated; ++frame) {
            game.update(1.0f / 60, {0, 0, true});
        }
        expect(game.bossDefeated && game.screen == Screen::Playing,
               "boss could not be defeated with normal firing");
        walkTo(game, {1100, 295});
        game.update(1.0f / 60, {0, 0, false, true});
        expect(game.screen == Screen::Won && game.defeatedCount == 5 && !game.hasSave,
               "three-zone journey did not reach a consistent finished state");
    });

    scenario("profile roundtrip restores progress and every setting without starting play", [] {
        TemporaryProfile file;
        Game source;
        source.newGame();
        source.loadStage(2);
        source.requestCheckpoint();
        source.runTime = 123.75f;
        source.preferences = {0.23f, true, true, true, true};
        expect(saveProfile(source, file.path), "could not create profile and nested directories");
        Game restored;
        expect(loadProfile(restored, file.path), "saved profile could not be read");
        expect(restored.hasSave && restored.checkpoint == 2 && restored.screen == Screen::Title,
               "profile failed to restore checkpoint or bypassed title");
        near(restored.runTime, 123.75f, 0.001f, "elapsed time changed during save");
        near(restored.preferences.volume, 0.23f, 0.001f, "volume changed during save");
        expect(restored.preferences.muted && restored.preferences.fullscreen &&
               restored.preferences.reducedMotion && restored.preferences.easy,
               "accessibility/display/audio settings did not survive save");
        restored.resumeCheckpoint();
        expect(restored.stage == 2 && restored.screen == Screen::Playing && restored.hp == MaxHp,
               "disk checkpoint cannot be continued");
        expect(!std::filesystem::exists(file.path.string() + ".tmp"),
               "successful save left a temporary file");
    });

    scenario("completed run preserves preferences while removing continue progress", [] {
        TemporaryProfile file;
        Game source;
        source.hasSave = false;
        source.checkpoint = 2;
        source.preferences.volume = 0.8f;
        source.preferences.reducedMotion = true;
        expect(saveProfile(source, file.path), "could not save completed profile");
        std::ifstream input(file.path);
        std::string signature;
        int storedCheckpoint = 99;
        std::getline(input, signature);
        input >> storedCheckpoint;
        expect(storedCheckpoint == -1, "completed profile still stores resumable checkpoint");
        Game restored;
        restored.hasSave = true;
        expect(loadProfile(restored, file.path) && !restored.hasSave,
               "completed run still exposes continue");
        near(restored.preferences.volume, 0.8f, 0.001f, "completed profile lost volume");
        expect(restored.preferences.reducedMotion, "completed profile lost accessibility setting");
    });

    scenario("overwriting a profile leaves a recoverable previous checkpoint", [] {
        TemporaryProfile file;
        Game source;
        source.newGame();
        source.runTime = 30;
        expect(saveProfile(source, file.path), "first save failed");
        source.loadStage(1);
        source.requestCheckpoint();
        source.runTime = 80;
        expect(saveProfile(source, file.path), "second save failed");
        Game latest;
        expect(loadProfile(latest, file.path) && latest.checkpoint == 1,
               "latest save does not contain updated checkpoint");
        file.write("interrupted or corrupted profile\n");
        Game recovered;
        expect(loadProfile(recovered, file.path) && recovered.hasSave && recovered.checkpoint == 0,
               "corrupt primary did not recover from backup");
        near(recovered.runTime, 30, 0.001f, "backup did not preserve earlier elapsed time");
    });

    scenario("missing and truncated profiles leave the live game unchanged", [] {
        TemporaryProfile file;
        Game game;
        game.checkpoint = 1;
        game.runTime = 42;
        game.preferences.volume = 0.4f;
        expect(!loadProfile(game, file.path), "missing profile was reported as loaded");
        file.write("VIVIS_PROFILE_V2\n2 100\n");
        expect(!loadProfile(game, file.path), "truncated profile was accepted");
        expect(game.checkpoint == 1 && game.runTime == 42,
               "invalid profile partially mutated progress");
        near(game.preferences.volume, 0.4f, 0.001f, "invalid profile partially mutated settings");
    });

    scenario("out-of-range or malformed disk values are rejected without partial updates", [] {
        TemporaryProfile file;
        const std::vector<std::string> rows = {
            "3 0 0.5 0 0 0 0", "-2 0 0.5 0 0 0 0", "1 -1 0.5 0 0 0 0",
            "1 604801 0.5 0 0 0 0", "1 0 -0.1 0 0 0 0", "1 0 1.1 0 0 0 0",
            "1 0 0.5 2 0 0 0", "1 0 0.5 0 -1 0 0", "1 0 0.5 0 0 2 0",
            "1 0 0.5 0 0 0 2", "1 nan 0.5 0 0 0 0", "1 0 inf 0 0 0 0"
        };
        for (const auto& row : rows) {
            file.write("VIVIS_PROFILE_V2\n" + row + "\n");
            Game game;
            game.checkpoint = 2;
            game.runTime = 77;
            expect(!loadProfile(game, file.path), "unsafe profile accepted: " + row);
            expect(game.checkpoint == 2 && game.runTime == 77 && !game.hasSave,
                   "invalid profile changed live game: " + row);
        }
    });

    std::cout << "\n" << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
