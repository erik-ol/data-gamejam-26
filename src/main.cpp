#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_timer.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <SDL3_image/SDL_image.h>

#include <string>
#include <algorithm>
#include <iostream>
// Web save game
// #ifdef __EMSCRIPTEN__
//     EM_ASM({
//     localStorage.setItem('torskGameVar', JSON.stringify($0));
//     }, var);
// #endif

// web load game
// #ifdef __EMSCRIPTEN__
//     var = EM_ASM_INT({
//     return JSON.parse(localStorage.getItem('torskGameVar'));
//     });
// #endif

#define GAME_SIZE_X 100
#define GAME_SIZE_Y 100

Uint64 prev_time;
struct Player { //or like a collision_object
    //I need position to draw
    // top left position
    std::pair<float, float> coords;
    //x and y scale respectively
    std::pair<int, int> sizes;

    // left/right is as you'd expect (number line),
    // up/down is inverted,
    // and these should be floats
    std::pair<float, float> direction;

    //maybe also a texture?
    SDL_Texture* texture;

    Player(std::pair<int, int> co, std::pair<int, int> sz, SDL_Texture* txt) {
        this->coords = co;
        this->sizes = sz;
        this->direction = {1.0, 0.0};
        this->texture = txt;
    }
};

//https://stackoverflow.com/questions/21956119/add-stdpair-with-operator
// i'm only really intending to use this for ints and floats
// but why not
//overloading += might be harder if I don't know LHS
//would have to do with classes
template <typename T,typename U>
std::pair<T,U> operator+(const std::pair<T,U> & l,const std::pair<T,U> & r) {
    return {l.first+r.first,l.second+r.second};
}
template <typename T,typename U, typename V>
std::pair<T,U> operator*(const std::pair<T,U> & l,const V & r) {
    return {l.first*r,l.second*r};
}

//taken from SDL3 guide #https://wiki.libsdl.org/SDL3/BestKeyboardPractices
/* returns 1 if moving forward with this keypress, -1 if moving backward, 0 if not moving. */
std::pair<float,float> direction_user_should_move(float delta_time)
{
    const bool *key_states = SDL_GetKeyboardState(NULL);
    std::pair<float,float> direction = {0, 0};

    if (key_states[SDL_SCANCODE_W]) {
        direction = direction + std::pair(0.0f,-1.0f);
    }

    if (key_states[SDL_SCANCODE_A]) {
        direction = direction + std::pair(-1.0f,0.0f);
    }

    if (key_states[SDL_SCANCODE_S]) {
        direction = direction + std::pair(0.0f, 1.0f);
    }

    if (key_states[SDL_SCANCODE_D]) {
        direction = direction + std::pair(1.0f,0.0f);
    }

    direction = direction * delta_time;
    return direction;
}


//https://wiki.libsdl.org/SDL3/BestKeyboardPractices
enum Action { ACTION_NONE, ACTION_THROW, ACTION_HOLD};

struct AppState {
    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_Texture * brick_texture;
    SDL_Texture* targetTexture;

    //rock information? behöver size
    Action player_holding;

    Player player;
    Player enemy;
    Player projectile;
};

Action action_user_should_take(const AppState* as, const SDL_Event *e) {
    if (!(e->key.scancode == SDL_SCANCODE_N)) {
        return as->player_holding;
    }
    if (e->type == SDL_EVENT_KEY_DOWN && e->key.scancode == SDL_SCANCODE_N) {
        return ACTION_HOLD;
    } else if (e->type == SDL_EVENT_KEY_UP && e->key.scancode == SDL_SCANCODE_N) {
        return ACTION_THROW;
    }
    return ACTION_NONE;
}
//1. lägg till sten
// 2. när man trycker på N så ska stenen dyka upp vid karaktärerna och följa karaktärern
// 3. när man släpper N så ska stenen flyga iväg i riktningen spelaren tittar på
SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    AppState *as = (AppState *)SDL_calloc(1, sizeof(AppState));
    if (!as) {
        return SDL_APP_FAILURE;
    } else {
        *appstate = as;
    }

    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "0");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("GAME NAME", GAME_SIZE_X, GAME_SIZE_Y, SDL_WINDOW_RESIZABLE, &as->window, &as->renderer)) {
        //SDL_Log("Couldn't create window and renderer: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    //SDL_SetWindowFullscreen(as->window, true);

    as->targetTexture = SDL_CreateTexture(as->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, GAME_SIZE_X, GAME_SIZE_Y);

    prev_time = SDL_GetTicks();
    as->player_holding = ACTION_NONE;
    as->brick_texture = IMG_LoadTexture(as->renderer, "assets/textures/bricks.png");

    SDL_Texture* player_texture = IMG_LoadTexture(as->renderer, "assets/textures/T.png");
    SDL_Texture* bug_texture = IMG_LoadTexture(as->renderer, "assets/textures/bug.png");
    SDL_Texture* rock_texture = IMG_LoadTexture(as->renderer, "assets/textures/rock.png");

    Player player({50, 50}, {32, 32}, player_texture);
    Player enemy({300, 300}, {87, 36}, bug_texture);

    //position doesn't matter because it's going to be spawned at the player
    Player projectile({0, 0}, {200, 146}, rock_texture);

    as->player = player;
    as->enemy = enemy;
    as->projectile = projectile;
    return SDL_APP_CONTINUE;
}

//kanske borde definiera konstanter,
float LOW_LIMIT = 0.0167f;
float HIGH_LIMIT = 0.1f;

SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState *as = (AppState *)appstate;
    Uint64 current_time = SDL_GetTicks();
    //https://stackoverflow.com/questions/26576625/how-can-i-correctly-calculate-the-time-delta
    float delta_time = current_time - prev_time; //probably makes zero sense
    delta_time /= 1000.0; //make it slow....

    const bool *key_states = SDL_GetKeyboardState(NULL);

    // render to pixel perfect canvas
    SDL_SetRenderTarget(as->renderer, as->targetTexture);
    SDL_SetTextureScaleMode(as->targetTexture, SDL_SCALEMODE_NEAREST);

    // clear renderer
    SDL_SetRenderDrawColor(as->renderer, 30, 30, 30, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(as->renderer);

    // draw colored rectangle
    SDL_FRect rect;
    rect.x = 20;
    rect.y = 20;
    rect.w = 20;
    rect.h = 20;

    SDL_SetRenderDrawColor(as->renderer, 80, 80, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(as->renderer, &rect);

    // draw bricks.png
    SDL_FRect rectBricks;
    rectBricks.x = 10;
    rectBricks.y = 10;
    rectBricks.w = 16;
    rectBricks.h = 16;

    //kanske vertikal och horisontell speed för att simulera att ramla
    float speed = 50;

    std::pair<float,float> newDirection = direction_user_should_move(delta_time);
    //kanske en newpos så att jag slipper göra två saker
    std::pair<float,float> newPos = as->player.coords + newDirection * speed;

    as->player.coords = newPos;

    SDL_FRect rectPlayer;
    rectPlayer.x = as->player.coords.first;
    rectPlayer.y = as->player.coords.second;
    rectPlayer.w = as->player.sizes.first;
    rectPlayer.h = as->player.sizes.second;

    SDL_FRect rectEnemy;
    rectEnemy.x = as->enemy.coords.first;
    rectEnemy.y = as->enemy.coords.second;
    rectEnemy.w = as->enemy.sizes.first;
    rectEnemy.h = as->enemy.sizes.second;

    if (as->player_holding == ACTION_HOLD) {
        SDL_FRect rectRock;
        rectRock.x = as->player.coords.first-50.0f;
        rectRock.y = as->player.coords.second-50.0f;
        rectRock.w = as->projectile.sizes.first;
        rectRock.h = as->projectile.sizes.second;
        SDL_RenderTexture(as->renderer, as->projectile.texture, NULL, &rectRock);
    } else if (as->player_holding == ACTION_THROW) {
        //throw rock
    }

    SDL_RenderTexture(as->renderer, as->brick_texture, NULL, &rectBricks);
    SDL_RenderTexture(as->renderer, as->player.texture, NULL, &rectPlayer);
    SDL_RenderTexture(as->renderer, as->enemy.texture, NULL, &rectEnemy);
    
    // render from perfect canvas to full screen

    int windowWidth;
    int windowHeight;
    SDL_GetWindowSizeInPixels(as->window, &windowWidth, &windowHeight);

    int gameScale = 1;
    int ratioX = windowWidth / GAME_SIZE_X;
    int ratioY = windowHeight / GAME_SIZE_Y;
    if (ratioX < ratioY) gameScale = ratioX;
    else gameScale = ratioY;

    SDL_FRect rectGame;
    rectGame.w = GAME_SIZE_X * gameScale;
    rectGame.h = GAME_SIZE_Y * gameScale;
    rectGame.x = (windowWidth/2)-(rectGame.w/2);
    rectGame.y = (windowHeight/2)-(rectGame.h/2);

    SDL_SetRenderTarget(as->renderer, NULL);
    SDL_SetRenderDrawColor(as->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(as->renderer);
    SDL_RenderTexture(as->renderer, as->targetTexture, NULL, &rectGame);
    
    // pass to screen
    SDL_RenderPresent(as->renderer);
    prev_time = current_time;
    return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    AppState *as = (AppState *) appstate;

    switch (event->type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    case SDL_EVENT_KEY_DOWN:
        std::cout << "hey boss\n";
        as->player_holding = action_user_should_take(as, event);
        break;
    case SDL_EVENT_KEY_UP:
        as->player_holding = action_user_should_take(as, event);
        break;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    SDL_free(appstate);
}
