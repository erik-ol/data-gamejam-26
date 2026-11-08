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

    direction = direction * delta_time; //int float multiply, probably works
    std::cout << "new awesome direction" << direction.first << " " << direction.second << " \n";
    return direction;
}
struct AppState {
    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_Texture * brick_texture;
    Player player;
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    AppState *as = (AppState *)SDL_calloc(1, sizeof(AppState));
    if (!as) {
        return SDL_APP_FAILURE;
    } else {
        *appstate = as;
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return SDL_APP_FAILURE;
    }
    if (!SDL_CreateWindowAndRenderer("GAME NAME", 640, 480, SDL_WINDOW_RESIZABLE, &as->window, &as->renderer)) {
        //SDL_Log("Couldn't create window and renderer: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    prev_time = SDL_GetTicks();
    as->brick_texture = IMG_LoadTexture(as->renderer, "assets/textures/bricks.png");

    SDL_Texture* player_texture = IMG_LoadTexture(as->renderer, "assets/textures/T.png");

    Player player({50, 50}, {32, 32}, player_texture);

    as->player = player;

    return SDL_APP_CONTINUE;
}

float LOW_LIMIT = 0.0167f;
float HIGH_LIMIT = 0.1f;

SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState *as = (AppState *)appstate;
    Uint64 current_time = SDL_GetTicks();
    //https://stackoverflow.com/questions/26576625/how-can-i-correctly-calculate-the-time-delta
    float delta_time = current_time - prev_time; //probably makes zero sense
    // std::cout << delta_time <<"\n";
    // delta_time = std::clamp(delta_time, LOW_LIMIT, HIGH_LIMIT);
    delta_time /= 1000.0; //make it slow....
    std::cout << "time: " << delta_time<<"\n";

    // clear renderer
    SDL_SetRenderDrawColor(as->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(as->renderer);

    // draw colored rectangle
    SDL_FRect rect;
    rect.x = 50;
    rect.y = 50;
    rect.w = 50;
    rect.h = 50;

    SDL_SetRenderDrawColor(as->renderer, 80, 80, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(as->renderer, &rect);

    // draw bricks.png
    SDL_FRect rectBricks;
    rectBricks.x = 100;
    rectBricks.y = 100;
    rectBricks.w = 16;
    rectBricks.h = 16;

    //move player

    float speed = 50;

    std::pair<float,float> newDirection = direction_user_should_move(delta_time);
    std::pair<float,float> newPos = as->player.coords + newDirection * speed;
    std::cout << newPos.first << " " << newDirection.first << " " << as->player.coords.first << "\n";

    as->player.coords = newPos;

    SDL_FRect rectPlayer;
    rectPlayer.x = as->player.coords.first;
    rectPlayer.y = as->player.coords.second;
    rectPlayer.w = as->player.sizes.first;
    rectPlayer.h = as->player.sizes.second;

    SDL_RenderTexture(as->renderer, as->brick_texture, NULL, &rectBricks);
    SDL_RenderTexture(as->renderer, as->player.texture, NULL, &rectPlayer);

    // pass to screen

    SDL_RenderPresent(as->renderer);
    prev_time = current_time;
    return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    switch (event->type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    SDL_free(appstate);
}
