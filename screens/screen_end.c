#include "engine.h"
#include "raylib.h"

#define COOLDOWN        35
#define AMOUNT_PRO_TIPS 6

static void DrawTextCentered(const char *text, int y, int fontSize, Color color);
static const char *get_mode_as_str(Game *game);
static int frames_counter = 0;

static const char *pro_tips[AMOUNT_PRO_TIPS] = {
    "Pro Tip: The fists get faster. Keep that in mind!",
    "Pro Tip: Wait until the punches are fully gone",
    "Pro Tip: git gud",
    "Pro Tip: Be fast!",
    "Pro Tip: Dodge!",
    "Insane mode may be hard but not impossible. Do not give up."
};

static bool show_score_won = false;
static bool show_tokens_won = false;
static bool show_mode = false;
static bool show_pro = false;
static int pro_tip = 0;

static Sound result_sound = { 0 };
static Sound cool_sound = { 0 };
static Sound end_sound = { 0 };
static Sound win_sound = { 0 };

void screen_end_init(Game *game)
{
    frames_counter = 0;
    show_score_won = false;
    show_tokens_won = false;
    show_mode = false;
    show_pro = false;
    pro_tip = GetRandomValue(0, AMOUNT_PRO_TIPS - 1);

    result_sound = LoadSound("assets/result.wav");
    cool_sound = LoadSound("assets/cool.wav");
    end_sound = LoadSound("assets/end.wav");
    win_sound = LoadSound("assets/win.wav");

    if (game->player.lost) {
        PlaySound(end_sound);
    } else {
        PlaySound(win_sound);
    }
}

void screen_end_update(Game *game)
{
    SetSoundVolume(result_sound, game->sound_volume);
    SetSoundVolume(cool_sound, game->sound_volume);
    SetSoundVolume(end_sound, game->sound_volume);
    SetSoundVolume(win_sound, game->sound_volume);

    if (!IsSoundPlaying(win_sound)) {
        frames_counter++;
    }

    if (frames_counter == COOLDOWN) {
        PlaySound(result_sound);
        show_score_won = true;
    }
    if (frames_counter == COOLDOWN*2) {
        PlaySound(result_sound);
        show_tokens_won = true;
    }
    if (frames_counter == COOLDOWN*3) {
        PlaySound(result_sound);
        show_mode = true;
    }
    if (frames_counter == COOLDOWN*5) {
        if (game->mode != MODE_INSANE) {
            PlaySound(cool_sound);
        } else {
            PlaySound(result_sound);
        }
        show_pro = true;
    }
    if (frames_counter == COOLDOWN*8) {
        game->player.tokens += game->tokens_won;
        emsave("tokens", game->player.tokens);
        game->title_ss = SS_CONFIG;
        if (!game->unlocked_modes[MODE_INFINITE] && game->mode == MODE_SURVIVAL && !game->player.lost) {
#ifdef WEB
            emsave("mode_infinite", true);
#else
            game->unlocked_modes[MODE_INFINITE] = true;
#endif
        }
        if (game->mode == MODE_INSANE && !game->player.lost) {
#ifdef WEB
            emsave("beat_insane", true);
#else
            game->player.beat_insane = true;
#endif
        }
        screen_move(SCREEN_TITLE);
    }
}

void screen_end_draw(Game *game)
{
    DrawTextCentered(game->player.lost? "YOU GOT PUNCHED" : "CONGRATULATIONS! YOU SURVIVED!", 10, 30, RED);
    if (show_score_won) {
        DrawTextCentered(TextFormat("SCORE WON: %d", game->player.score), 50, 30, WHITE);
    }
    if (show_tokens_won) {
        DrawTextCentered(TextFormat("TOKENS WON: %d", game->tokens_won), 90, 30, WHITE);
    }
    if (show_mode) {
        if (game->mode == MODE_INFINITE) {
            DrawTextCentered(get_mode_as_str(game), (game->mode == MODE_INSANE)? 180 : 130, (game->mode == MODE_INSANE)? 40 : 30, WHITE);
        } else {
            DrawTextCentered(TextFormat("SURVIVED UNTIL: %i:%02i", game->time/60, game->time%60), 130, 30, WHITE);

            if (!game->player.lost) {
                if (!game->unlocked_modes[MODE_INFINITE]) {
                    DrawTextCentered("YOU UNLOCKED THE INFINITE MODE!", 170, 30, WHITE);
                }
            }
        }
    }
    if (show_pro) {
        if (game->mode != MODE_INSANE) {
            DrawTextCentered(pro_tips[pro_tip], GetScreenHeight() - 70, 30, WHITE);
        } else {
            DrawTextCentered(pro_tips[5], GetScreenHeight() - 70, 20, RED);
        }
    }
}

void screen_end_deinit(Game *game)
{
    UnloadSound(result_sound);
    UnloadSound(cool_sound);
    UnloadSound(end_sound);
    UnloadSound(win_sound);
}

static void DrawTextCentered(const char *text, int y, int fontSize, Color color)
{
    int width = MeasureText(text, fontSize);
    DrawText(text, GetScreenWidth()/2 - width/2, y, fontSize, color);
}

static const char *get_mode_as_str(Game *game)
{
    switch (game->mode) {
        case MODE_SURVIVAL: return "MODE: SURVIVAL"; break;
        case MODE_INFINITE: return "MODE: INFINITE"; break;
        case MODE_INSANE:
            if (!game->player.beat_insane) {
                return "MODE: INSANE! ARE YOU CRAZY?!";
            } else {
                return "YOU ARE THE BEST!!!";
            }
            break;
        default: break;
    }

    return "";
}

