#include "engine.h"
#include "raylib.h"
#include <stdio.h>

#define AMOUNT_FISTS         4
#define AMOUNT_PLAYER_STATES 5
#define COOLDOWN             45
#define SHAKE_OFFSET         50

typedef enum {
    STATE_DODGING = 0,
    STATE_IDLE
} PlayerStates;

typedef enum {
    FIST_PLACE_UP = 0,
    FIST_PLACE_LEFT,
    FIST_PLACE_DOWN,
    FIST_PLACE_RIGHT
} FistPlaces;

typedef struct Fist {
    Rectangle hitbox;
    int place;
    bool attacked;
    Texture texture;
} Fist;

static Rectangle player_hitbox = { 0 };
static Rectangle player_default_hitbox = { 0 };
static int key_pressed = KEY_NULL;

static Rectangle punch_up_hitbox = { 0 };
static Rectangle punch_left_hitbox = { 0 };
static Rectangle punch_down_hitbox = { 0 };
static Rectangle punch_right_hitbox = { 0 };

static Rectangle punch_up_default_hitbox = { 0 };
static Rectangle punch_left_default_hitbox = { 0 };
static Rectangle punch_down_default_hitbox = { 0 };
static Rectangle punch_right_default_hitbox = { 0 };

static Color darkness = BLACK;

static Fist fists[AMOUNT_FISTS] = { 0 };
static Rectangle default_hitboxes[AMOUNT_FISTS] = { 0 };

static Texture warning_texture = { 0 };
static Texture fist_up_texture = { 0 };
static Texture fist_left_texture = { 0 };
static Texture fist_down_texture = { 0 };
static Texture fist_right_texture = { 0 };
static Texture player_texture = { 0 };
static Rectangle player_texture_rec = { 0 };

static Sound warning_sound = { 0 };
static Sound warning2_sound = { 0 };
static Sound warning3_sound = { 0 };
static Sound warning4_sound = { 0 };
static Sound punch_sound = { 0 };
static Sound hit_sound = { 0 };

static Sound warning_up_sound = { 0 };
static Sound warning_left_sound = { 0 };
static Sound warning_right_sound = { 0 };

static Music music_normal = { 0 };
static Music music_normal2 = { 0 };
static Music music_normal3 = { 0 };
static Music music_insane = { 0 };

static int frames_counter = 0;
static int frames_counter_time = 0;
static int frames_counter_cooldown = COOLDOWN + 40;
static int fist_chosen;
static bool show_warning = false;
static bool show_hit = false;
static bool move_to_title = false;

static int frames_counter_hit = 0;
static Camera2D camera = { 0 }; // shake effect
static Sound warning_sounds[4] = { 0 };

static int music_idx = 0;
static int player_lock_counter = 0;

static Texture backgrounds[AMOUNT_BACKGROUNDS] = { 0 };
static Music musics[3] = { 0 };

static Rectangle up_event = { 0 };
static Rectangle left_event = { 0 };
static Rectangle down_event = { 0 };
static Rectangle right_event = { 0 };

static bool button_held(Rectangle bounds);

void screen_game_init(Game *game)
{
    // initialize values
    if (game->mode == MODE_INFINITE) {
        game->player.lives = 3;
    } else {
        game->player.lives = 1;
    }

    game->player.was_attacked = false;
    game->player.lost = false;
    game->player.attack_counter = 0;
    frames_counter = 0;
    frames_counter_time = 0;
    frames_counter_cooldown = COOLDOWN + 40;
    fist_chosen = GetRandomValue(FIST_PLACE_UP, FIST_PLACE_RIGHT);
    show_warning = false;
    show_hit = false;
    game->tokens_won = 0;
    game->player.score = 0;
    frames_counter_hit = 0;
    camera.offset = (Vector2){ 0.0f, 0.0f };
    camera.target = (Vector2){ 0.0f, 0.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    move_to_title = false;
    player_lock_counter = 0;

    music_idx = GetRandomValue(0, 2);

    // initialize structures
    game->player.x = GetScreenWidth()/2;
    game->player.y = GetScreenHeight()/2;
    game->player.width = 100;
    game->player.height = 180;
    
    game->player.default_x = GetScreenWidth()/2;
    game->player.default_y = GetScreenHeight()/2;
    game->player.default_width = 100;
    game->player.default_height = 180;

    player_hitbox = (Rectangle){ game->player.x - game->player.width/2, (game->player.y - game->player.height/2) + 40, game->player.width, game->player.height };
    player_default_hitbox = (Rectangle){ game->player.default_x - game->player.default_width/2, (game->player.default_y - game->player.default_height/2) + 40, game->player.default_width, game->player.default_height };

    // punches rectangles
    punch_up_hitbox = (Rectangle){ GetScreenWidth()/2 - 200.0f/2, -380.0f, 200.0f, 400.0f };
    punch_left_hitbox = (Rectangle){ -375.0f, GetScreenHeight()/2 - 200.0f/2, 400.0f, 200.0f };
    punch_down_hitbox = (Rectangle){ GetScreenWidth()/2 - 200.0f/2, GetScreenHeight() - 25.0f, 200.0f, 400.0f };
    punch_right_hitbox = (Rectangle){ GetScreenWidth() - 25.0f, GetScreenHeight()/2 - 200.0f/2, 400.0f, 200.0f };

    punch_up_default_hitbox = punch_up_hitbox;
    punch_left_default_hitbox = punch_left_hitbox;
    punch_down_default_hitbox = punch_down_hitbox;
    punch_right_default_hitbox = punch_right_hitbox;

    default_hitboxes[0] = punch_up_default_hitbox;
    default_hitboxes[1] = punch_left_default_hitbox;
    default_hitboxes[2] = punch_down_default_hitbox;
    default_hitboxes[3] = punch_right_default_hitbox;

    // initialize resources
    warning_texture = LoadTexture("assets/warning.png");
    warning_sound = LoadSound("assets/warning.wav");
    warning2_sound = LoadSound("assets/warning2.wav");
    warning3_sound = LoadSound("assets/warning3.wav");
    warning4_sound = LoadSound("assets/warning4.wav");
    punch_sound = LoadSound("assets/punch.wav");
    hit_sound = LoadSound("assets/hit.wav");

    warning_up_sound = LoadSound("assets/warning_up.wav");
    warning_left_sound = LoadSound("assets/warning_left.wav");
    warning_right_sound = LoadSound("assets/warning_right.wav");
    player_texture = LoadTexture("assets/player.png");
    player_texture_rec = (Rectangle){ 0.0f, 0.0f, player_texture.width/AMOUNT_PLAYER_STATES, player_texture.height };

    Image fist_image = LoadImage("assets/fist.png");
    fist_right_texture = LoadTextureFromImage(fist_image);

    ImageRotateCW(&fist_image);
    fist_down_texture = LoadTextureFromImage(fist_image);

    ImageRotateCW(&fist_image);
    ImageFlipVertical(&fist_image);
    fist_left_texture = LoadTextureFromImage(fist_image);

    ImageRotateCW(&fist_image);
    fist_up_texture = LoadTextureFromImage(fist_image);

    UnloadImage(fist_image);

    fists[0] = (Fist){ punch_up_hitbox, FIST_PLACE_UP, false, fist_up_texture };
    fists[1] = (Fist){ punch_left_hitbox, FIST_PLACE_LEFT, false, fist_left_texture };
    fists[2] = (Fist){ punch_down_hitbox, FIST_PLACE_DOWN, false, fist_down_texture };
    fists[3] = (Fist){ punch_right_hitbox, FIST_PLACE_RIGHT, false, fist_right_texture };

    for (int i = 0; i < AMOUNT_BACKGROUNDS; i++) {
        backgrounds[i] = LoadTexture(TextFormat("assets/bg_%d.png", i));
    }

    music_normal = LoadMusicStream("assets/song_game.mp3");
    music_normal2 = LoadMusicStream("assets/song_game2.mp3");
    music_normal3 = LoadMusicStream("assets/song_game3.mp3");
    music_insane = LoadMusicStream("assets/song_insane.mp3");
    musics[0] = music_normal;
    musics[1] = music_normal2;
    musics[2] = music_normal3;

    if (game->mode != MODE_INSANE) {
        PlayMusicStream(musics[music_idx]);
    } else {
        PlayMusicStream(music_insane);
    }
    SetMusicVolume(music_normal, game->music_volume);
    SetMusicVolume(music_normal2, game->music_volume);
    SetMusicVolume(music_insane, game->music_volume);

    SetSoundVolume(warning_sound, game->sound_volume);
    SetSoundVolume(warning2_sound, game->sound_volume);
    SetSoundVolume(warning3_sound, game->sound_volume);
    SetSoundVolume(warning4_sound, game->sound_volume);
    SetSoundVolume(punch_sound, game->sound_volume);
    SetSoundVolume(hit_sound, game->sound_volume);
    SetSoundVolume(warning_up_sound, game->sound_volume);
    SetSoundVolume(warning_left_sound, game->sound_volume);
    SetSoundVolume(warning_right_sound, game->sound_volume);

    warning_sounds[0] = warning_sound;
    warning_sounds[1] = warning2_sound;
    warning_sounds[2] = warning3_sound;
    warning_sounds[3] = warning4_sound;

    darkness.a = 0;
    game->player.state = STATE_IDLE;
    game->player.locked = false;

    if (game->mode != MODE_SANDBOX) {
        up_event = (Rectangle){ 0.0f, 0.0f, GetScreenWidth(), 150.0f };
    } else {
        up_event = (Rectangle){ 0.0f + 300.0f, 0.0f, GetScreenWidth(), 150.0f };
    }

    left_event = (Rectangle){ 0.0f, 150.0f, 160.0f, 150.0f };
    down_event = (Rectangle){ 0.0f, GetScreenHeight() - 150.0f, GetScreenWidth(), 150.0f };
    right_event = (Rectangle){ GetScreenWidth() - 160.0f, 150.0f, 160.0f, 150.0f };
}

void screen_game_update(Game *game)
{
    if (game->mode != MODE_INSANE) {
        UpdateMusicStream(musics[music_idx]);
    } else {
        UpdateMusicStream(music_insane);
    }

    // player input
    if (!game->player.locked) {
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP) || button_held(up_event) || GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) == -1 || IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP)) {
            player_hitbox.y = 75;
            player_hitbox.height = 65;
            key_pressed = KEY_W;
            player_texture_rec.x = (player_texture.width/AMOUNT_PLAYER_STATES)*4;
            player_lock_counter++;
        } else if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT) || button_held(left_event) || GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) == -1 || IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) {
            player_hitbox.x = 300;
            player_hitbox.width = 50;
            key_pressed = KEY_A;
            player_texture_rec.x = (player_texture.width/AMOUNT_PLAYER_STATES)*2;
            player_lock_counter++;
        } else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN) || button_held(down_event) || GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) == 1 || IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) {
            player_hitbox.y = 305;
            player_hitbox.height = 50;
            player_texture_rec.x = (player_texture.width/AMOUNT_PLAYER_STATES)*1;
            key_pressed = KEY_S;
            player_lock_counter++;
        } else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT) || button_held(right_event) || GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) == 1 || IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) {
            player_hitbox.x = 450;
            player_hitbox.width = 50;
            key_pressed = KEY_D;
            player_texture_rec.x = (player_texture.width/AMOUNT_PLAYER_STATES)*3;
            player_lock_counter++;
        } else {
            key_pressed = KEY_NULL;
            player_lock_counter = 0;
        }
    } else {
        key_pressed = KEY_NULL;
        player_lock_counter += 2;
    }

    if (key_pressed == KEY_NULL) {
        player_hitbox = player_default_hitbox;
        player_texture_rec.x = 0.0f;
    }

    if (player_lock_counter >= 40) {
        game->player.locked = true;
    }

    if (player_lock_counter >= 60) {
        game->player.locked = false;
        player_lock_counter = 0;
    }

    // check if hit
    if (CheckCollisionRecs(player_hitbox, fists[fist_chosen].hitbox)) {
        game->player.attack_counter++;
        game->player.was_attacked = true;
    }

    if (game->player.attack_counter == 1) {
        if (game->mode != MODE_SANDBOX) {
            game->player.lives--;
        } else {
            show_hit = true;
        }
        game->player.attack_counter++;
        PlaySound(hit_sound);
    }

    if (game->player.was_attacked) {
        camera.target.x = GetRandomValue(-SHAKE_OFFSET, SHAKE_OFFSET);
        camera.target.y = GetRandomValue(-SHAKE_OFFSET, SHAKE_OFFSET);
    }

    if (game->player.lives < 1) {
        game->player.lost = true;
        screen_move(SCREEN_END);
    }

    // make the punches faster as the score progresses
    switch (game->player.score) {
        case 200: frames_counter_cooldown = COOLDOWN + 30; break;
        case 400: frames_counter_cooldown = COOLDOWN + 22; break;
        case 600: frames_counter_cooldown = COOLDOWN + 20; break;
        case 800: frames_counter_cooldown = COOLDOWN + 16; break;
        case 1000: frames_counter_cooldown = COOLDOWN + 14; break;
        case 1200: frames_counter_cooldown = COOLDOWN + 12; break;
        case 1400: frames_counter_cooldown = COOLDOWN + 10; break;
        case 1600: frames_counter_cooldown = COOLDOWN + 8; break;
        case 1700: frames_counter_cooldown = COOLDOWN + 5; break;
        case 2000: frames_counter_cooldown = COOLDOWN + 3; break;
        case 2200: frames_counter_cooldown = COOLDOWN + 1; break;
        case 2400: frames_counter_cooldown = COOLDOWN; break;
        case 2600: frames_counter_cooldown = COOLDOWN - 1; break;
        case 2800: frames_counter_cooldown = COOLDOWN - 3; break;
        case 3000: frames_counter_cooldown = COOLDOWN - 5; break;
        case 3200: frames_counter_cooldown = COOLDOWN - 7; break;
        case 3400: frames_counter_cooldown = COOLDOWN - 9; break;
    }

    if ((IsKeyPressed(KEY_Q) || update_back_button(5, 5, 240, 64) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) && game->mode == MODE_SANDBOX) {
        screen_move(SCREEN_TITLE);
    }

    // fists attack
    // -----------
    frames_counter++;

    // select a fist
    if (frames_counter == frames_counter_cooldown - 35) {
        fist_chosen = GetRandomValue(FIST_PLACE_UP, FIST_PLACE_RIGHT);
        show_warning = true;

        if (game->mode == MODE_INSANE) {
            switch (fist_chosen) {
                case FIST_PLACE_UP: PlaySound(warning_up_sound); break;
                case FIST_PLACE_LEFT: PlaySound(warning_left_sound); break;
                case FIST_PLACE_DOWN: PlaySound(warning_sound); break;
                case FIST_PLACE_RIGHT: PlaySound(warning_right_sound); break;
                default: break;
            }
        } else {
            PlaySound(warning_sounds[GetRandomValue(0, 3)]);
        }
    }

    if (game->player.score >= 200) {
        if (game->mode == MODE_INSANE) {
            if (darkness.a < 255) {
                darkness.a++;
            }
        }
    }

    // if the frames_counter reaches the cooldown, attack
    if (frames_counter == frames_counter_cooldown) {
        show_warning = false;
        fists[fist_chosen].attacked = true;
        PlaySound(punch_sound);
    }

    // then, just remove attack
    if (frames_counter == frames_counter_cooldown + 5) {
        fists[fist_chosen].attacked = false;
        frames_counter = 0;
    }

    if (frames_counter == 0) {
        if (!game->player.was_attacked) {
            game->player.score += 100;
            game->tokens_won += GetRandomValue(3, 6);
        }
        game->player.was_attacked = false;
        game->player.attack_counter = 0;
        show_hit = false;
        camera.target = (Vector2){ 0.0f, 0.0f };
    }

    if (fists[fist_chosen].attacked) {
        switch (fist_chosen) {
            case FIST_PLACE_UP:
                fists[FIST_PLACE_UP].hitbox.y = -175.0f;
                break;
            case FIST_PLACE_LEFT:
                fists[FIST_PLACE_LEFT].hitbox.x = 15.0f;
                break;
            case FIST_PLACE_DOWN:
                fists[FIST_PLACE_DOWN].hitbox.y = GetScreenHeight() - 180.0f;
                break;
            case FIST_PLACE_RIGHT:
                fists[FIST_PLACE_RIGHT].hitbox.x = GetScreenWidth() - 400.0f;
                break;
            default: break;
        }
    } else {
        fists[fist_chosen].hitbox = default_hitboxes[fist_chosen];
    }

    // decrease time
    // ------------
    if (game->mode == MODE_INSANE || game->mode == MODE_SURVIVAL) {
        frames_counter_time++;

        if (frames_counter_time >= 60) {
            game->time--;
            frames_counter_time = 0;
        }
    }

    if (game->time <= 0 && !game->player.lost && (game->mode == MODE_INSANE || game->mode == MODE_SURVIVAL)) {
        screen_move(SCREEN_END);
    }
}

void screen_game_draw(Game *game)
{
    BeginMode2D(camera);
    DrawTexture(backgrounds[game->bg_chosen], 0, 0, WHITE);

    Vector2 position = { player_default_hitbox.x - 20.0f, player_default_hitbox.y - 230.0f };
    switch (key_pressed) {
        case KEY_W: position = (Vector2){ player_default_hitbox.x - 20.0f, player_default_hitbox.y - 230.0f }; break;
        case KEY_A: position = (Vector2){ player_default_hitbox.x - 50.0f, player_default_hitbox.y - 230.0f }; break;
        case KEY_S: position = (Vector2){ player_default_hitbox.x - 20.0f, player_default_hitbox.y - 230.0f }; break;
        case KEY_D: position = (Vector2){ player_default_hitbox.x + 50.0f, player_default_hitbox.y - 230.0f }; break;
    }

    DrawTextureRec(player_texture, player_texture_rec, position, WHITE);

    for (int i = 0; i < AMOUNT_FISTS; i++) {
        DrawTexture(fists[i].texture, fists[i].hitbox.x, fists[i].hitbox.y, WHITE);
    }

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), darkness);

    // hud
    if (game->mode != MODE_SANDBOX) {
        DrawText(TextFormat("SCORE: %d", game->player.score), 10, 10, 30, WHITE);
        if (game->mode != MODE_INFINITE) {
            DrawText(TextFormat("%i:%02i", game->time/60, game->time%60), GetScreenWidth() - 70, 10, 30, WHITE);
        }
        if (game->mode == MODE_INFINITE) {
            DrawText(TextFormat("LIVES: %d", game->player.lives), 10, 40, 30, WHITE);
        }
    } else {
        draw_back_button(5, 5);
        DrawText("Ouch!", 10, 40, 30, show_hit? WHITE : BLANK);
    }

    // warning
    if (game->mode != MODE_INSANE) {
        int x = 0;
        int y = 0;
        switch (fist_chosen) {
            case FIST_PLACE_UP:
                x = (fists[FIST_PLACE_UP].hitbox.x + warning_texture.width/2) - 20;
                y = 20;
                break;
            case FIST_PLACE_LEFT:
                x = 30;
                y = (fists[FIST_PLACE_LEFT].hitbox.y);
                break;
            case FIST_PLACE_DOWN:
                x = (fists[FIST_PLACE_DOWN].hitbox.x + warning_texture.width/2) - 20;
                y = GetScreenHeight() - warning_texture.width;
                break;
            case FIST_PLACE_RIGHT:
                x = GetScreenWidth() - warning_texture.width - 40;
                y = (fists[FIST_PLACE_RIGHT].hitbox.y);
                break;
            default: break;
        }
        DrawTexture(warning_texture, x, y, show_warning? WHITE : BLANK);
    }
    EndMode2D();
}

void screen_game_deinit(Game *game)
{
    UnloadTexture(warning_texture);
    UnloadTexture(fist_up_texture);
    UnloadTexture(fist_left_texture);
    UnloadTexture(fist_down_texture);
    UnloadTexture(fist_right_texture);
    UnloadTexture(player_texture);
    UnloadSound(warning_sound);
    UnloadSound(warning2_sound);
    UnloadSound(warning3_sound);
    UnloadSound(warning4_sound);
    UnloadSound(warning_up_sound);
    UnloadSound(warning_left_sound);
    UnloadSound(warning_right_sound);
    UnloadSound(punch_sound);
    UnloadSound(hit_sound);
    UnloadMusicStream(music_normal);
    UnloadMusicStream(music_normal2);
    UnloadMusicStream(music_normal3);
    UnloadMusicStream(music_insane);

    for (int i = 0; i < AMOUNT_BACKGROUNDS; i++) {
        UnloadTexture(backgrounds[i]);
    }
}

static bool button_held(Rectangle bounds)
{
    if (CheckCollisionPointRec(GetMousePosition(), bounds)) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            return true;
        }
    }

    return false;
}

