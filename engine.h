#ifndef engine_h
#define engine_h

#include <stdbool.h>

#define AMOUNT_BACKGROUNDS  4
#define AMOUNT_MODES        4

typedef enum {
    SCREEN_WAIT = 0,
    SCREEN_TITLE,
    SCREEN_SHOP,
    SCREEN_GAME,
    SCREEN_END
} Screens;

typedef enum {
    SS_MAIN = 0,
    SS_CONFIG,
    SS_INSTRUCTIONS,
    SS_CUSTOMIZE,
    SS_OPTIONS
} Subscreens;

typedef enum {
    WS_LOGO = 0,
    WS_GET_READY,
    WS_LOADING_TITLE,
    WS_LOADING_SHOP
} WaitSituations;

typedef enum {
    MODE_SURVIVAL = 0,
    MODE_INFINITE,
    MODE_SANDBOX,
    MODE_INSANE,
} Modes;

typedef struct Player {
    int score;
    int tokens;
    int lives;

    float x, y, width, height; // hitbox
    float default_x, default_y, default_width, default_height;
    bool was_attacked;
    int attack_counter;
    bool lost;
    bool beat_insane; // WHAT?!?!??
} Player;

typedef struct Game {
    int current_screen;
    int title_ss;
    int wait_situation;
    int mode;
    int tokens_won;
    int time; // in seconds
    int bg_chosen;
    int frames_counter;
    bool unlocked_modes[AMOUNT_MODES];
    bool unlocked_bg[AMOUNT_BACKGROUNDS];

    float music_volume;
    float sound_volume;
    float volume;
    Player player;
} Game;

void init_game(Game *game);
void deinit_game(Game *game);
void transition_out(Game *game);
void transition_zero(void);
bool transition_done(void);
void run(void);

void emsave(const char *key, int value);
int emload(const char *key);
void emsave_f(const char *key, float value);
float emload_f(const char *key);

void screen_init(int screen);
void screen_update(int screen);
void screen_draw(int screen);
void screen_deinit(int screen);
void screen_move(int to);

void screen_wait_init(Game *game);
void screen_wait_update(Game *game);
void screen_wait_draw(Game *game);
void screen_wait_deinit(Game *game);

void screen_title_init(Game *game);
void screen_title_update(Game *game);
void screen_title_draw(Game *game);
void screen_title_deinit(Game *game);

void screen_shop_init(Game *game);
void screen_shop_update(Game *game);
void screen_shop_draw(Game *game);
void screen_shop_deinit(Game *game);

void screen_game_init(Game *game);
void screen_game_update(Game *game);
void screen_game_draw(Game *game);
void screen_game_deinit(Game *game);

void screen_end_init(Game *game);
void screen_end_update(Game *game);
void screen_end_draw(Game *game);
void screen_end_deinit(Game *game);

#endif // engine_h

