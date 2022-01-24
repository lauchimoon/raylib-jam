#include "engine.h"
#include "raylib.h"

#define AMOUNT_TABS  2
#define AMOUNT_ITEMS 5 // two modes and three backgrounds

typedef enum {
    TAB_MODES = 0,
    TAB_BACKGROUNDS
} Tabs;

typedef enum {
    TYPE_MODE = 0,
    TYPE_BACKGROUND
} ItemType;

typedef struct ShopItem {
    const char *name;
    const char *description;
    int price;
    int type;
    Texture preview;
} ShopItem;

static Texture bg_texture = { 0 };
static Texture tick_texture = { 0 };
static int texture_x = 0;
static int texture_y = 0;

static int current_tab = TAB_MODES;
static int current_item = 0;

static const char *tabs[AMOUNT_TABS] = {
    "MODES",
    "BACKGROUNDS"
};

static ShopItem items[AMOUNT_ITEMS] = { 0 };

static Rectangle window = { 0 };
static Rectangle modes_bounds = { 0 };
static Rectangle bgs_bounds = { 0 };

static Rectangle sandbox = { 0 };
static Rectangle insane = { 0 };
static Rectangle out = { 0 };
static Rectangle mc = { 0 };
static Rectangle beach = { 0 };

static Sound select_sound = { 0 };
static Sound buy_sound = { 0 };
static Sound start_sound = { 0 };

static Music music = { 0 };

static Color color_selected = { 201, 215, 199, 255 };
static Color color_unselected = { 144, 172, 140, 255 };
static bool move_to_title = false;

static bool click_button(Rectangle bounds);
static ShopItem new_item(const char *name, const char *description, int price, int type, const char *texture_path);
static void buy(Game *game, int item);
static void DrawTextCentered(const char *text, int y, int fontSize, Color color);

void screen_shop_init(Game *game)
{
    Image image = GenImageChecked(800, 450, 25, 25, GREEN, LIME);
    bg_texture = LoadTextureFromImage(image);
    UnloadImage(image);
    
    texture_x = 0;
    texture_y = 0;

    window = (Rectangle){ 20.0f, 20.0f, GetScreenWidth() - 40.0f, GetScreenHeight() - 40.0f };

    modes_bounds = (Rectangle){ 40.0f, 10.0f, 128.0f, 64.0f };
    bgs_bounds = (Rectangle){ 60.0f + modes_bounds.width, 10.0f, 168.0f, 64.0f };

    select_sound = LoadSound("assets/select.wav");
    buy_sound = LoadSound("assets/buy.wav");
    start_sound = LoadSound("assets/start.wav");
    tick_texture = LoadTexture("assets/tick.png");
    music = LoadMusicStream("assets/song_shop.mp3");

    items[0] = new_item("Sandbox Mode", "A mode where you don't take damage. Great for practicing!", 2000, TYPE_MODE, "assets/shop_0.png");
    items[1] = new_item("Insane Mode", "The hardest mode of them all. Try it, if you can.", 4500, TYPE_MODE, "assets/shop_1.png");
    items[2] = new_item("Outside background", "Take some fresh air!", 1000, TYPE_BACKGROUND, "assets/bg_1.png");
    items[3] = new_item("Cave background", "A place so familiar it feels nice.", 1000, TYPE_BACKGROUND, "assets/bg_2.png");
    items[4] = new_item("Beach background", "Let's have some fun!", 1000, TYPE_BACKGROUND, "assets/bg_3.png");

    sandbox = (Rectangle){ 40.0f, GetScreenHeight()/2 - 125.0f, 240.0f, 135.0f };
    insane = (Rectangle){ 180.0f + modes_bounds.width, GetScreenHeight()/2 - 125.0f, 240.0f, 135.0f };
    out = (Rectangle){ 80.0f, GetScreenHeight()/2 - 125.0f, 200.0f, 112.0f };
    mc = (Rectangle){ 170.0f + modes_bounds.width, GetScreenHeight()/2 - 125.0f, 200.0f, 112.0f };
    beach = (Rectangle){ 390.0f + modes_bounds.width, GetScreenHeight()/2 - 125.0f, 200.0f, 112.0f };
    move_to_title = false;

    PlayMusicStream(music);
    SetMusicVolume(music, game->music_volume);

    SetSoundVolume(select_sound, game->sound_volume);
    SetSoundVolume(buy_sound, game->sound_volume);
    SetSoundVolume(start_sound, game->sound_volume);
}

void screen_shop_update(Game *game)
{
    UpdateMusicStream(music);

    if (click_button(modes_bounds)) {
        PlaySound(select_sound);
        current_tab = TAB_MODES;
    }
    if (click_button(bgs_bounds)) {
        PlaySound(select_sound);
        current_tab = TAB_BACKGROUNDS;
    }

    if (IsKeyPressed(KEY_Q) && !move_to_title) {
        emsave("tokens", game->player.tokens);
        StopMusicStream(music);
        PlaySound(start_sound);
        transition_zero();
        move_to_title = true;
    }

#ifdef WEB
    game->unlocked_modes[MODE_SANDBOX] = emload("mode_sandbox");
    game->unlocked_modes[MODE_INSANE] = emload("mode_insane");
    game->unlocked_bg[1] = emload("bg_1");
    game->unlocked_bg[2] = emload("bg_2");
    game->unlocked_bg[3] = emload("bg_3");
#endif

    // items
    switch (current_tab) {
        case TAB_MODES:
            if (CheckCollisionPointRec(GetMousePosition(), sandbox)) {
                current_item = 0;
            } else if (CheckCollisionPointRec(GetMousePosition(), insane)) {
                current_item = 1;
            }

            if (click_button(sandbox) && game->player.tokens >= items[0].price && !game->unlocked_modes[MODE_SANDBOX]) {
                buy(game, 0);
                game->player.tokens -= items[0].price;
            }
            if (click_button(insane) && game->player.tokens >= items[1].price && !game->unlocked_modes[MODE_INSANE]) {
                buy(game, 1);
                game->player.tokens -= items[1].price;
            }
            break;
        case TAB_BACKGROUNDS:
            if (CheckCollisionPointRec(GetMousePosition(), out)) {
                current_item = 2;
            } else if (CheckCollisionPointRec(GetMousePosition(), mc)) {
                current_item = 3;
            } else if (CheckCollisionPointRec(GetMousePosition(), beach)) {
                current_item = 4;
            }

            if (click_button(out) && game->player.tokens >= items[2].price && !game->unlocked_bg[1]) {
                buy(game, 2);
                game->player.tokens -= items[2].price;
            }
            if (click_button(mc) && game->player.tokens >= items[3].price && !game->unlocked_bg[2]) {
                buy(game, 3);
                game->player.tokens -= items[3].price;
            }
            if (click_button(beach) && game->player.tokens >= items[4].price && !game->unlocked_bg[3]) {
                buy(game, 4);
                game->player.tokens -= items[4].price;
            }
            break;
        default: break;
    }
}

void screen_shop_draw(Game *game)
{
    Rectangle src = { (float)texture_x, (float)texture_y, bg_texture.width, bg_texture.height };
    Rectangle dst = { 0.0f, 0.0f, bg_texture.width, bg_texture.height };
    Vector2 origin = { 0.0f, 0.0f };
    float rotation = 0.0f;
    DrawTexturePro(bg_texture, src, dst, origin, rotation, (Color){ 50, 50, 50, 255 });

    DrawRectangleRec(window, (Color){ 201, 215, 199, 255 });
    DrawRectangleLinesEx(window, 5, DARKGREEN);

    DrawRectangleRec(modes_bounds, (current_tab == TAB_MODES)? color_selected : color_unselected);
    DrawRectangleLinesEx(modes_bounds, 5, DARKGREEN);

    DrawRectangleRec(bgs_bounds, (current_tab == TAB_BACKGROUNDS)? color_selected : color_unselected);
    DrawRectangleLinesEx(bgs_bounds, 5, DARKGREEN);

    DrawText(tabs[0], modes_bounds.width/2, modes_bounds.height/2, 20, (current_tab == TAB_MODES)? LIME : DARKGREEN);
    DrawText(tabs[1], (bgs_bounds.x/2 + bgs_bounds.width/2) + 18, bgs_bounds.height/2, 20, (current_tab == TAB_BACKGROUNDS)? LIME : DARKGREEN);

    DrawText(TextFormat("TOKENS: %d", game->player.tokens), window.x + 20, window.height - 20, 20, BLACK);
    DrawText("Press Q to return", window.x + 20, window.height - 50, 20, BLACK);

    switch (current_tab) {
        case TAB_MODES:
            DrawTextureEx(items[0].preview, (Vector2){ sandbox.x, sandbox.y }, 0.0f, 0.3f, game->unlocked_modes[MODE_SANDBOX]? GRAY : WHITE);
            DrawTextureEx(tick_texture, (Vector2){ sandbox.x, sandbox.y }, 0.0f, 0.3f, game->unlocked_modes[MODE_SANDBOX]? WHITE : BLANK);
            DrawTextureEx(items[1].preview, (Vector2){ insane.x, insane.y }, 0.0f, 0.3f, game->unlocked_modes[MODE_INSANE]? GRAY : WHITE);
            DrawTextureEx(tick_texture, (Vector2){ insane.x, insane.y }, 0.0f, 0.3f, game->unlocked_modes[MODE_INSANE]? WHITE : BLANK);

            DrawText(items[0].name, sandbox.x + 45, (sandbox.y + sandbox.height) + 10, 20, BLACK);
            DrawText(items[1].name, insane.x + 50, (insane.y + insane.height) + 10, 20, BLACK);

            DrawRectangleLinesEx(sandbox, 5, (current_item == 0)? DARKGREEN : BLANK);
            DrawRectangleLinesEx(insane, 5, (current_item == 1)? DARKGREEN : BLANK);

            break;
        case TAB_BACKGROUNDS:
            DrawTextureEx(items[2].preview, (Vector2){ out.x, out.y }, 0.0f, 0.25f, game->unlocked_bg[1]? GRAY : WHITE);
            DrawTextureEx(tick_texture, (Vector2){ out.x, out.y }, 0.0f, 0.25f, game->unlocked_bg[1]? WHITE : BLANK);
            DrawTextureEx(items[3].preview, (Vector2){ mc.x, mc.y }, 0.0f, 0.25f, game->unlocked_bg[2]? GRAY : WHITE);
            DrawTextureEx(tick_texture, (Vector2){ mc.x, mc.y }, 0.0f, 0.25f, game->unlocked_bg[2]? WHITE : BLANK);
            DrawTextureEx(items[4].preview, (Vector2){ beach.x, beach.y }, 0.0f, 0.25f, game->unlocked_bg[3]? GRAY : WHITE);
            DrawTextureEx(tick_texture, (Vector2){ beach.x, beach.y }, 0.0f, 0.25f, game->unlocked_bg[3]? WHITE : BLANK);

            DrawText(items[2].name, out.x, (out.y + out.height) + 10, 20, BLACK);
            DrawText(items[3].name, mc.x, (mc.y + mc.height) + 10, 20, BLACK);
            DrawText(items[4].name, beach.x, (beach.y + beach.height) + 10, 20, BLACK);

            DrawRectangleLinesEx(out, 5, (current_item == 2)? DARKGREEN : BLANK);
            DrawRectangleLinesEx(mc, 5, (current_item == 3)? DARKGREEN : BLANK);
            DrawRectangleLinesEx(beach, 5, (current_item == 4)? DARKGREEN : BLANK);
            break;
        default: break;
    }

    DrawTextCentered(items[current_item].description, (window.y + window.height) - 150, 20, BLACK);
    DrawText(TextFormat("PRICE: %d", items[current_item].price), (window.x + window.width) - 140, 40, 20, BLACK);

    if (move_to_title) {
        game->wait_situation = WS_LOADING_TITLE;
        transition_out(game);

        if (transition_done()) {
            screen_move(SCREEN_WAIT);
        }
    }
}

void screen_shop_deinit(Game *game)
{
    UnloadTexture(bg_texture);
    UnloadSound(select_sound);
    UnloadSound(buy_sound);
    UnloadSound(start_sound);
    UnloadMusicStream(music);

    for (int i = 0; i < AMOUNT_ITEMS; i++) {
        UnloadTexture(items[i].preview);
    }
    UnloadTexture(tick_texture);
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

static ShopItem new_item(const char *name, const char *description, int price, int type, const char *texture_path)
{
    Texture texture = LoadTexture(texture_path);
    return (ShopItem){
        .name = name,
        .description = description,
        .price = price,
        .type = type,
        .preview = texture
    };
}

static void buy(Game *game, int item)
{
#ifdef WEB
    switch (item) {
        case 0: emsave("mode_sandbox", true); break;
        case 1: emsave("mode_insane", true); break;
        case 2: emsave("bg_1", true); break;
        case 3: emsave("bg_2", true); break;
        case 4: emsave("bg_3", true); break;
    }
#else
    switch (item) {
        case 0: game->unlocked_modes[MODE_SANDBOX] = true; break;
        case 1: game->unlocked_modes[MODE_INSANE] = true; break;
        case 2: game->unlocked_bg[1] = true; break;
        case 3: game->unlocked_bg[2] = true; break;
        case 4: game->unlocked_bg[3] = true; break;
    }
#endif
    PlaySound(buy_sound);
}

static void DrawTextCentered(const char *text, int y, int fontSize, Color color)
{
    int width = MeasureText(text, fontSize);
    DrawText(text, GetScreenWidth()/2 - width/2, y, fontSize, color);
}

