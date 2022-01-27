#include "engine.h"
#include "raylib.h"

#define AMOUNT_BUTTONS      5

typedef struct Button {
    Rectangle bounds;
} Button;

static Texture bg_texture = { 0 };
static Texture title_texture = { 0 };
static Texture buttons_texture = { 0 };
static Texture warning_texture = { 0 };

static Sound select_sound = { 0 };
static Sound start_sound = { 0 };

static Music music = { 0 };

static Button buttons[AMOUNT_BUTTONS] = { 0 };

static Rectangle button_play_render = { 0 };
static Rectangle button_shop_render = { 0 };
static Rectangle button_go_render = { 0 };
static Rectangle button_inst_render = { 0 };
static Rectangle button_custom_render = { 0 };

static Rectangle bg_0 = { 0 };
static Rectangle bg_1 = { 0 };
static Rectangle bg_2 = { 0 };
static Rectangle bg_3 = { 0 };

static Texture bg_0_texture = { 0 };
static Texture bg_1_texture = { 0 };
static Texture bg_2_texture = { 0 };
static Texture bg_3_texture = { 0 };

static Texture fist_texture = { 0 };
static Texture prize_texture = { 0 };
static Texture gear_texture = { 0 };

static bool move_to_game = false;
static bool move_to_shop = false;

static const char *mode_descriptions[AMOUNT_MODES] = {
    "Last 1:00 minute on the fist hell!\nYou only get one life!",
    "An endless run. Bonus point: you get three lives!",
    "You don't take damage. Great for practicing!",
    "You're not told when a fist will\ncome at you. Survive for a minute!"
};

static void DrawTextCentered(const char *text, int y, int fontSize, Color color);
static const char *get_mode_as_str(Game *game);
static bool click_button(Rectangle bounds);
static bool down_button(Rectangle bounds);
static void choose_bg(Game *game, int bg);
static void reset_game_data(Game *game);

void screen_title_init(Game *game)
{
#ifdef WEB
    game->unlocked_modes[MODE_SURVIVAL] = emload("mode_survival");
    game->unlocked_modes[MODE_INFINITE] = emload("mode_infinite");
    game->unlocked_modes[MODE_SANDBOX] = emload("mode_sandbox");
    game->unlocked_modes[MODE_INSANE] = emload("mode_insane");

    game->unlocked_bg[0] = emload("bg_0");
    game->unlocked_bg[1] = emload("bg_1");
    game->unlocked_bg[2] = emload("bg_2");
    game->unlocked_bg[3] = emload("bg_3");

    game->player.tokens = emload("tokens");
    game->player.beat_insane = emload("beat_insane");

    game->volume = emload_f("master_volume");
    game->sound_volume = emload_f("sound_volume");
    game->music_volume = emload_f("music_volume");
#endif

    bg_texture = LoadTexture("assets/menu_bg.png");
    title_texture = LoadTexture("assets/menu_title.png");
    buttons_texture = LoadTexture("assets/buttons.png");
    warning_texture = LoadTexture("assets/warning.png");
    prize_texture = LoadTexture("assets/prize.png");

    gear_texture = LoadTexture("assets/gear.png");

    select_sound = LoadSound("assets/select.wav");
    start_sound = LoadSound("assets/start.wav");
    music = LoadMusicStream("assets/song_title.mp3");

    buttons[0].bounds = (Rectangle){ GetScreenWidth() - 300.0f, 150.0f, 240.0f, 64.0f };
    buttons[1].bounds = (Rectangle){ GetScreenWidth() - 300.0f, 250.0f, 240.0f, 64.0f };
    buttons[2].bounds = (Rectangle){ GetScreenWidth()/2 - 240.0f/2, GetScreenHeight() - buttons_texture.height/AMOUNT_BUTTONS - 50, 240.0f, 64.0f };
    buttons[3].bounds = (Rectangle){ GetScreenWidth() - 300.0f, 350.0f, 240.0f, 64.0f };
    buttons[4].bounds = (Rectangle){ GetScreenWidth()/2 - 240.0f/2, GetScreenHeight() - buttons_texture.height/AMOUNT_BUTTONS - 120, 240.0f, 64.0f };

    bg_0_texture = LoadTexture("assets/bg_0.png");
    bg_1_texture = LoadTexture("assets/bg_1.png");
    bg_2_texture = LoadTexture("assets/bg_2.png");
    bg_3_texture = LoadTexture("assets/bg_3.png");
    move_to_game = false;
    move_to_shop = false;

    Image fist = LoadImage("assets/fist.png");
    ImageFlipHorizontal(&fist);
    fist_texture = LoadTextureFromImage(fist);
    UnloadImage(fist);

    PlayMusicStream(music);
}

void screen_title_update(Game *game)
{
    UpdateMusicStream(music);

    button_play_render = (Rectangle){ 0.0f, 0.0f, buttons_texture.width/2, buttons_texture.height/AMOUNT_BUTTONS };
    button_shop_render = (Rectangle){ 0.0f, 64.0f, buttons_texture.width/2, buttons_texture.height/AMOUNT_BUTTONS };
    button_go_render = (Rectangle){ 0.0f, 128.0f, buttons_texture.width/2, buttons_texture.height/AMOUNT_BUTTONS };
    button_inst_render = (Rectangle){ 0.0f, 192.0f, buttons_texture.width/2, buttons_texture.height/AMOUNT_BUTTONS };
    button_custom_render = (Rectangle){ 0.0f, 256.0f, buttons_texture.width/2, buttons_texture.height/AMOUNT_BUTTONS };

    SetSoundVolume(select_sound, game->sound_volume);
    SetSoundVolume(start_sound, game->sound_volume);
    SetMusicVolume(music, game->music_volume);

    // buttons input
    if (game->title_ss == SS_MAIN) {
        if (CheckCollisionPointRec(GetMousePosition(), buttons[0].bounds)) {
            button_play_render.x = buttons_texture.width/2;

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                game->title_ss = SS_CONFIG;
                PlaySound(select_sound);
            }
        }
        if (CheckCollisionPointRec(GetMousePosition(), buttons[1].bounds)) {
            button_shop_render.x = buttons_texture.width/2;

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !move_to_shop) {
                PlaySound(start_sound);
                transition_zero();
                move_to_shop = true;
            }
        }
        
        if (CheckCollisionPointRec(GetMousePosition(), buttons[3].bounds)) {
            button_inst_render.x = buttons_texture.width/2;

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                game->title_ss = SS_INSTRUCTIONS;
                PlaySound(select_sound);
            }
        }

        if (IsKeyPressed(KEY_DELETE)) {
            reset_game_data(game);
            game->wait_situation = WS_LOGO;
            screen_move(SCREEN_WAIT);
        }

        if (click_button((Rectangle){ GetScreenWidth() - gear_texture.width, 10, gear_texture.width, gear_texture.height })) {
            game->title_ss = SS_OPTIONS;
        }
    } else if (game->title_ss == SS_CONFIG) {
        if (click_button((Rectangle){ 480.0f, 120.0f, 48.0f, 48.0f }) && !move_to_game) {
            game->mode++;
            PlaySound(select_sound);
        }
        if (click_button((Rectangle){ 275.0f, 120.0f, 48.0f, 48.0f }) && !move_to_game) {
            game->mode--;
            PlaySound(select_sound);
        }

        if ((IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) && !move_to_game) {
            game->mode++;
            PlaySound(select_sound);
        }
        if ((IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) && !move_to_game) {
            game->mode--;
            PlaySound(select_sound);
        }

        if (CheckCollisionPointRec(GetMousePosition(), buttons[2].bounds)) {
            button_go_render.x = buttons_texture.width/2;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !move_to_game && game->unlocked_modes[game->mode]) {
                transition_zero();
                PlaySound(start_sound);
                move_to_game = true;
            }
        }

        if (CheckCollisionPointRec(GetMousePosition(), buttons[4].bounds) && !move_to_game) {
            button_custom_render.x = buttons_texture.width/2;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                game->title_ss = SS_CUSTOMIZE;
                PlaySound(select_sound);
            }
        }

        if (IsKeyPressed(KEY_SPACE) && !move_to_game && game->unlocked_modes[game->mode]) {
            PlaySound(start_sound);
            transition_zero();
            move_to_game = true;
        }

        if (update_back_button(5, GetScreenHeight() - 70, 240, 64)) {
            game->title_ss = SS_MAIN;
        }

        if (game->mode < MODE_SURVIVAL) {
            game->mode = MODE_INSANE;
        } else if (game->mode > MODE_INSANE) {
            game->mode = MODE_SURVIVAL;
        }
    } else if (game->title_ss == SS_OPTIONS) {
        // master volume
        if (down_button((Rectangle){ 480.0f, 120.0f, 48.0f, 48.0f })) {
            if (game->volume <= 1.0f) {
                game->volume += 0.01f;
            }

            if (game->volume >= 1.0f) {
                game->volume = 1.0f;
            }
        }
        if (down_button((Rectangle){ 275.0f, 120.0f, 48.0f, 48.0f })) {
            if (game->volume >= 0.00001f) {
                game->volume -= 0.01f;
            }
        }

        // sound volume
        if (down_button((Rectangle){ 480.0f, 220.0f, 48.0f, 48.0f })) {
            if (game->sound_volume <= 1.0f) {
                game->sound_volume += 0.01f;
            }

            if (game->sound_volume >= 1.0f) {
                game->sound_volume = 1.0f;
            }
        }
        if (down_button((Rectangle){ 275.0f, 220.0f, 48.0f, 48.0f })) {
            if (game->sound_volume >= 0.00001f) {
                game->sound_volume -= 0.01f;
            }
        }

        // music volume
        if (down_button((Rectangle){ 480.0f, 320.0f, 48.0f, 48.0f })) {
            if (game->music_volume <= 1.0f) {
                game->music_volume += 0.01f;
            }

            if (game->music_volume >= 1.0f) {
                game->music_volume = 1.0f;
            }
        }
        if (down_button((Rectangle){ 275.0f, 320.0f, 48.0f, 48.0f })) {
            if (game->music_volume >= 0.00001f) {
                game->music_volume -= 0.01f;
            }
        }
        
        if (update_back_button(5, GetScreenHeight() - 70, 240, 64)) {
            game->title_ss = SS_MAIN;
            emsave_f("master_volume", game->volume);
            emsave_f("sound_volume", game->sound_volume);
            emsave_f("music_volume", game->music_volume);
        }
    }

    if (game->title_ss == SS_INSTRUCTIONS || game->title_ss == SS_CUSTOMIZE) {
        if (update_back_button(5, GetScreenHeight() - 70, 240, 64)) {
            game->title_ss -= 2;
        }
    }

    if (IsKeyPressed(KEY_Q) && !move_to_game) {
        switch (game->title_ss) {
            case SS_MAIN: break;
            case SS_CONFIG:
                game->title_ss--;
                break;
            case SS_INSTRUCTIONS: case SS_CUSTOMIZE:
                game->title_ss -= 2;
                break;
            case SS_OPTIONS:
                game->title_ss -= 4;
#ifdef WEB
                emsave_f("master_volume", game->volume);
                emsave_f("sound_volume", game->sound_volume);
                emsave_f("music_volume", game->music_volume);
#endif
                break;
            default: break;
        }

        if (game->title_ss != SS_MAIN) {
            PlaySound(select_sound);
        }
    }

    bg_0 = (Rectangle){ 100.0f, 120.0f, 240.0f, 135.0f };
    bg_1 = (Rectangle){ 450.0f, 120.0f, 240.0f, 135.0f };
    bg_2 = (Rectangle){ 100.0f, 270.0f, 240.0f, 135.0f };
    bg_3 = (Rectangle){ 450.0f, 270.0f, 240.0f, 135.0f };

    if (game->title_ss == SS_CUSTOMIZE) {
        if (click_button(bg_0) && game->unlocked_bg[0]) choose_bg(game, 0);
        if (click_button(bg_1) && game->unlocked_bg[1]) choose_bg(game, 1);
        if (click_button(bg_2) && game->unlocked_bg[2]) choose_bg(game, 2);
        if (click_button(bg_3) && game->unlocked_bg[3]) choose_bg(game, 3);
    }

    // check time
    switch (game->mode) {
        case MODE_SURVIVAL: game->time = 90; break;
        case MODE_INFINITE: case MODE_SANDBOX: game->time = -1; break;
        case MODE_INSANE: game->time = 120; break;
    }

    mode_descriptions[MODE_SURVIVAL] = (!game->unlocked_modes[MODE_SURVIVAL])? "???" : "Last 1:30 minutes on the fist hell!\n       You only get one life!";
    mode_descriptions[MODE_INFINITE] = (!game->unlocked_modes[MODE_INFINITE])? "???" : "An endless run. Bonus point: you get three lives!";
    mode_descriptions[MODE_SANDBOX] = (!game->unlocked_modes[MODE_SANDBOX])? "???" : "You don't take damage. Great for practicing!";
    mode_descriptions[MODE_INSANE] = (!game->unlocked_modes[MODE_INSANE])? "???" : "You're not told when a fist will\ncome at you. Survive for two minutes!";
}

void screen_title_draw(Game *game)
{
    switch (game->title_ss) {
        case SS_MAIN:
            DrawTexture(bg_texture, 0, 0, WHITE);
            DrawTexture(title_texture, GetRandomValue(-2, 2), GetRandomValue(-2, 2), WHITE);
            DrawTexture(fist_texture, GetRandomValue(-70, -65), GetRandomValue(GetScreenHeight()/2 - 50, GetScreenHeight()/2 - 45), WHITE);
            DrawText("Press DEL to delete your data", 10, GetScreenHeight() - 60, 20, BLACK);
            DrawText("Copyright (c) catmanl 2022", 10, GetScreenHeight() - 30, 20, BLACK);
            DrawText(TextFormat("v%.1f", VERSION), GetScreenWidth() - 40, GetScreenHeight() - 30, 20, BLACK);
            DrawTextureRec(buttons_texture, button_play_render, (Vector2){ GetRandomValue(buttons[0].bounds.x - 2, buttons[0].bounds.x + 2), GetRandomValue(buttons[0].bounds.y - 2, buttons[0].bounds.y + 2) }, WHITE);
            DrawTextureRec(buttons_texture, button_shop_render, (Vector2){ GetRandomValue(buttons[1].bounds.x - 2, buttons[1].bounds.x + 2), GetRandomValue(buttons[1].bounds.y - 2, buttons[1].bounds.y + 2) }, WHITE);
            DrawTextureRec(buttons_texture, button_inst_render, (Vector2){ GetRandomValue(buttons[3].bounds.x - 2, buttons[3].bounds.x + 2), GetRandomValue(buttons[3].bounds.y - 2, buttons[3].bounds.y + 2) }, WHITE);
            DrawTexture(gear_texture, GetScreenWidth() - gear_texture.width, 10, WHITE);

            if (game->player.beat_insane) {
                DrawTexture(prize_texture, 340, GetScreenHeight() - prize_texture.height, WHITE);
            }
            if (move_to_shop) {
                StopMusicStream(music);
                game->wait_situation = WS_LOADING_SHOP;
                transition_out(game);

                if (transition_done()) {
                    screen_move(SCREEN_WAIT);
                }
            }

            break;
        case SS_CONFIG:
            DrawTexture(bg_texture, 0, 0, (Color){ 150, 150, 150, 255 });
            DrawTextCentered("CONFIGURE YOUR GAME", 20, 40, WHITE);
            DrawTextCentered("Mode", 90, 30, WHITE);
            DrawText("<", 290, 140, 30, WHITE);
            DrawText(">", 495, 140, 30, WHITE);
            DrawTextCentered(get_mode_as_str(game), 140, 30, WHITE);
            DrawTextCentered(mode_descriptions[game->mode], 180, 30, WHITE);
            DrawTextureRec(buttons_texture, button_go_render, (Vector2){ buttons[2].bounds.x, buttons[2].bounds.y }, WHITE);
            DrawTextureRec(buttons_texture, button_custom_render, (Vector2){ buttons[4].bounds.x, buttons[4].bounds.y }, WHITE);
            draw_back_button(5, GetScreenHeight() - 70);

            if (move_to_game) {
                StopMusicStream(music);
                game->wait_situation = WS_GET_READY;
                transition_out(game);

                if (transition_done()) {
                    screen_move(SCREEN_WAIT);
                }
            }
            break;
        case SS_INSTRUCTIONS:
            DrawTexture(bg_texture, 0, 0, (Color){ 150, 150, 150, 255 });

            draw_back_button(5, GetScreenHeight() - 70);
            DrawText("Dodge the punches using WASD. Hold to dodge, but\ndon't hold for too long!", 10, 10, 30, WHITE);
            DrawText("You will be warned where a fist will come from with\nthis symbol:", 10, 95, 30, WHITE);
            DrawText("For example, say the symbol is to your left.\nHold the D key to avoid the fist coming\nfrom the left.", 10, 185, 30, WHITE);

            DrawTextCentered("Are you ready? Don't get fistbump'd!", 350, 30, WHITE);
            DrawTextureEx(warning_texture, (Vector2){ 198, 135 }, 0.0f, 0.3f, WHITE);
            break;
        case SS_CUSTOMIZE:
            DrawTexture(bg_texture, 0, 0, (Color){ 150, 150, 150, 255 });

            DrawTextureEx(bg_0_texture, (Vector2){ bg_0.x, bg_0.y }, 0.0f, 0.3f, game->unlocked_bg[0]? WHITE : DARKGRAY);
            DrawTextureEx(bg_1_texture, (Vector2){ bg_1.x, bg_1.y }, 0.0f, 0.3f, game->unlocked_bg[1]? WHITE : DARKGRAY);
            DrawTextureEx(bg_2_texture, (Vector2){ bg_2.x, bg_2.y }, 0.0f, 0.3f, game->unlocked_bg[2]? WHITE : DARKGRAY);
            DrawTextureEx(bg_3_texture, (Vector2){ bg_3.x, bg_3.y }, 0.0f, 0.3f, game->unlocked_bg[3]? WHITE : DARKGRAY);

            switch (game->bg_chosen) {
                case 0: DrawRectangleLinesEx(bg_0, 5, WHITE); break;
                case 1: DrawRectangleLinesEx(bg_1, 5, WHITE); break;
                case 2: DrawRectangleLinesEx(bg_2, 5, WHITE); break;
                case 3: DrawRectangleLinesEx(bg_3, 5, WHITE); break;
                default: break;
            }

            DrawTextCentered("CUSTOMIZE", 20, 40, WHITE);
            DrawTextCentered("Choose your background", 70, 30, WHITE);
            draw_back_button(5, GetScreenHeight() - 70);
            break;
        case SS_OPTIONS:
            DrawTexture(bg_texture, 0, 0, (Color){ 150, 150, 150, 255 });
            DrawTextCentered("OPTIONS", 20, 40, WHITE);
            draw_back_button(5, GetScreenHeight() - 70);
            DrawTextCentered("Master Volume", 90, 30, WHITE);
            DrawTextCentered(TextFormat("%.0f", game->volume*100), 140, 30, WHITE);
            DrawText("<", 290, 140, 30, WHITE);
            DrawText(">", 495, 140, 30, WHITE);
            
            DrawTextCentered("Sounds volume", 190, 30, WHITE);
            DrawTextCentered(TextFormat("%.0f", game->sound_volume*100), 240, 30, WHITE);
            DrawText("<", 290, 240, 30, WHITE);
            DrawText(">", 495, 240, 30, WHITE);

            DrawTextCentered("Music volume", 290, 30, WHITE);
            DrawTextCentered(TextFormat("%.0f", game->music_volume*100), 340, 30, WHITE);
            DrawText("<", 290, 340, 30, WHITE);
            DrawText(">", 495, 340, 30, WHITE);
            break;
        default: break;
    }
}

void screen_title_deinit(Game *game)
{
    UnloadTexture(bg_texture);
    UnloadTexture(title_texture);
    UnloadTexture(buttons_texture);
    UnloadTexture(warning_texture);
    UnloadTexture(fist_texture);

    UnloadTexture(gear_texture);

    UnloadTexture(bg_0_texture);
    UnloadTexture(bg_1_texture);
    UnloadTexture(bg_2_texture);
    UnloadTexture(bg_3_texture);
    UnloadSound(select_sound);
    UnloadSound(start_sound);

    UnloadMusicStream(music);
}

static void DrawTextCentered(const char *text, int y, int fontSize, Color color)
{
    int width = MeasureText(text, fontSize);
    DrawText(text, GetScreenWidth()/2 - width/2, y, fontSize, color);
}

static const char *get_mode_as_str(Game *game)
{
    switch (game->mode) {
        case MODE_SURVIVAL: if (game->unlocked_modes[MODE_SURVIVAL]) return "SURVIVAL"; break;
        case MODE_INFINITE: if (game->unlocked_modes[MODE_INFINITE]) return "INFINITE"; break;
        case MODE_SANDBOX: if (game->unlocked_modes[MODE_SANDBOX]) return "SANDBOX"; break;
        case MODE_INSANE: if (game->unlocked_modes[MODE_INSANE]) return "INSANE"; break;
        default: break;
    }

    return "???";
}

static bool click_button(Rectangle bounds)
{
    bool result = false;

    if (CheckCollisionPointRec(GetMousePosition(), bounds)) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            result = true;
        }
    }

    return result;
}

static bool down_button(Rectangle bounds)
{
    bool result = false;

    if (CheckCollisionPointRec(GetMousePosition(), bounds)) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            result = true;
        }
    }

    return result;
}

static void choose_bg(Game *game, int bg)
{
    game->bg_chosen = bg;
}

static void reset_game_data(Game *game)
{
#ifdef WEB
    emsave("mode_survival", true);
    emsave("mode_infinite", false);
    emsave("mode_sandbox", false);
    emsave("mode_insane", false);

    emsave("bg_0", true);
    emsave("bg_1", false);
    emsave("bg_2", false);
    emsave("bg_3", false);

    emsave("tokens", 0);
    emsave("beat_insane", false);
#else
    game->unlocked_modes[MODE_SURVIVAL] = true;
    game->unlocked_modes[MODE_INFINITE] = false;
    game->unlocked_modes[MODE_SANDBOX] = false;
    game->unlocked_modes[MODE_INSANE] = false;

    game->unlocked_bg[0] = true;
    game->unlocked_bg[1] = false;
    game->unlocked_bg[2] = false;
    game->unlocked_bg[3] = false;

    game->player.tokens = 0;
    game->player.beat_insane = false;
#endif
    game->bg_chosen = 0;
}

