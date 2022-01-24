#include "raylib.h"
#include "engine.h"

#ifdef WEB
    #include <emscripten.h>
#endif

static void update(void);
static void draw(void);
const int sw = 800;
const int sh = 450;
Game game = { 0 };

int main()
{
    SetTraceLogLevel(LOG_NONE);
    InitWindow(sw, sh, "Fistbump'd");
    InitAudioDevice();
    init_game(&game);
    screen_init(game.current_screen);

#ifdef WEB
    emscripten_set_main_loop(run, 0, 1);
#else
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        run();
    }
#endif

    screen_deinit(game.current_screen);
    deinit_game(&game);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

void run(void)
{
    update();
    draw();
}

static void update(void)
{
    screen_update(game.current_screen);
    SetMasterVolume(game.volume);
}

static void draw(void)
{
    BeginDrawing();
        ClearBackground(BLACK);
        screen_draw(game.current_screen);
    EndDrawing();
}

void screen_init(int screen)
{
    switch (screen) {
        case SCREEN_WAIT: screen_wait_init(&game); break;
        case SCREEN_TITLE: screen_title_init(&game); break;
        case SCREEN_SHOP: screen_shop_init(&game); break;
        case SCREEN_GAME: screen_game_init(&game); break;
        case SCREEN_END: screen_end_init(&game); break;
    }
}

void screen_update(int screen)
{
    switch (screen) {
        case SCREEN_WAIT: screen_wait_update(&game); break;
        case SCREEN_TITLE: screen_title_update(&game); break;
        case SCREEN_SHOP: screen_shop_update(&game); break;
        case SCREEN_GAME: screen_game_update(&game); break;
        case SCREEN_END: screen_end_update(&game); break;
    }
}

void screen_draw(int screen)
{
    switch (screen) {
        case SCREEN_WAIT: screen_wait_draw(&game); break;
        case SCREEN_TITLE: screen_title_draw(&game); break;
        case SCREEN_SHOP: screen_shop_draw(&game); break;
        case SCREEN_GAME: screen_game_draw(&game); break;
        case SCREEN_END: screen_end_draw(&game); break;
    }
}

void screen_deinit(int screen)
{
    switch (screen) {
        case SCREEN_WAIT: screen_wait_deinit(&game); break;
        case SCREEN_TITLE: screen_title_deinit(&game); break;
        case SCREEN_SHOP: screen_shop_deinit(&game); break;
        case SCREEN_GAME: screen_game_deinit(&game); break;
        case SCREEN_END: screen_end_deinit(&game); break;
    }
}

void screen_move(int to)
{
    screen_deinit(game.current_screen);
    screen_init(to);
    game.current_screen = to;
}

