#include "engine.h"
#include "raylib.h"

Color color = WHITE;

static int frames_counter = 0;
static void DrawTextCentered(const char *text, int y, int fontSize, Color color);
static Sound ready_sound = { 0 };
static Sound intro_sound = { 0 };

void screen_wait_init(Game *game)
{
    frames_counter = 0;
    color.a = 255;

    ready_sound = LoadSound("assets/ready.wav");
    intro_sound = LoadSound("assets/intro.wav");

    SetSoundVolume(ready_sound, game->sound_volume);

    if (game->wait_situation == WS_GET_READY) {
        PlaySound(ready_sound);
    } else if (game->wait_situation == WS_LOGO) {
        PlaySound(intro_sound);
    }
}

void screen_wait_update(Game *game)
{
    frames_counter++;

    if (frames_counter >= 10) {
        color.a -= 3;
    }
    if (frames_counter >= 50) {
        switch (game->wait_situation) {
            case WS_GET_READY:
                screen_move(SCREEN_GAME);
                break;
            case WS_LOADING_SHOP:
                screen_move(SCREEN_SHOP);
                break;
            case WS_LOADING_TITLE: case WS_LOGO:
                screen_move(SCREEN_TITLE);
                break;
            default: break;
        }
        frames_counter = 0;
    }
}

void screen_wait_draw(Game *game)
{
    switch (game->wait_situation) {
        case WS_GET_READY:
            if (game->mode != MODE_INSANE) {
                DrawTextCentered("GET READY!", GetScreenHeight()/2, 50, WHITE);
            } else {
                DrawTextCentered("down center", GetRandomValue(0, GetScreenHeight()), 60, (Color){ RED.g, RED.b, RED.r, GetRandomValue(0, 255) });
                DrawTextCentered("left left", GetRandomValue(0, GetScreenHeight()), 60, (Color){ RED.g, RED.b, RED.r, GetRandomValue(0, 255) });
                DrawTextCentered("right right", GetRandomValue(0, GetScreenHeight()), 60, (Color){ RED.g, RED.b, RED.r, GetRandomValue(0, 255) });
                DrawTextCentered("up lower", GetRandomValue(0, GetScreenHeight()), 60, (Color){ RED.g, RED.b, RED.r, GetRandomValue(0, 255) });
                DrawText("pay attention", GetRandomValue(0, GetScreenWidth()), GetRandomValue(0, GetScreenHeight()), 40, GRAY);
            }
            break;
        case WS_LOADING_TITLE: case WS_LOADING_SHOP:
            DrawText("Loading...", 10, GetScreenHeight() - 30, 20, WHITE);
            break;
        case WS_LOGO:
            DrawTextCentered("catmanl presents...", GetScreenHeight()/2, 40, color);
            break;
        default: break;
    }
}

void screen_wait_deinit(Game *game)
{
    UnloadSound(ready_sound);
    UnloadSound(intro_sound);
}

static void DrawTextCentered(const char *text, int y, int fontSize, Color color)
{
    int width = MeasureText(text, fontSize);
    DrawText(text, GetScreenWidth()/2 - width/2, y, fontSize, color);
}

