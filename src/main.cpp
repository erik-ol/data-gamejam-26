#include <SDL3/SDL_init.h>
#include <SDL3/SDL_rect.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <SDL3_image/SDL_image.h>

#include <string>

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
struct Player { //or like a collision_object
    //I need position to draw
    // top left position
    std::pair<int, int> coords;
    //x and y scale respectively
    std::pair<int, int> sizes;

    // left/right is as you'd expect (number line),
    // up/down is inverted,
    // and these should be floats
    std::pair<int, int> direction;

    //maybe also a texture?
    SDL_Texture* texture;

    Player(std::pair<int, int> co, std::pair<int, int> sz, SDL_Texture* txt) {
        this->coords = co;
        this->sizes = sz;
        this->direction = {1, 0};
        this->texture = txt;
    }
    // det här skulle kunna vara bättre som ett objeckt så att man kan
    // konstruera det så fort man deklarerar ett objekt.
    // nåja
};

//https://stackoverflow.com/questions/21956119/add-stdpair-with-operator
// i'm only really intending to use this for ints and floats
// but why not
template <typename T,typename U>
std::pair<T,U> operator+(const std::pair<T,U> & l,const std::pair<T,U> & r) {
    return {l.first+r.first,l.second+r.second};
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

    as->brick_texture = IMG_LoadTexture(as->renderer, "assets/textures/bricks.png");

    SDL_Texture* player_texture = IMG_LoadTexture(as->renderer, "assets/textures/T.png");

    Player player({50, 50}, {32, 32}, player_texture);

    as->player = player;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState *as = (AppState *)appstate;

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

    //change direction
    if (as->player.coords.first > 100) {
        as->player.direction = std::pair(-1, 0);
    } else if ( as->player.coords.first < 0) {
        as->player.direction = std::pair(1, 0);
    }

    //move player
    int newPosX = as->player.coords.first + as->player.direction.first;
    int newPosy = as->player.coords.second + as->player.direction.second;
    as->player.coords = {newPosX, newPosy};

    SDL_FRect rectPlayer;
    rectPlayer.x = as->player.coords.first;
    rectPlayer.y = as->player.coords.second;
    rectPlayer.w = as->player.sizes.first;
    rectPlayer.h = as->player.sizes.second;

    SDL_RenderTexture(as->renderer, as->brick_texture, NULL, &rectBricks);
    SDL_RenderTexture(as->renderer, as->player.texture, NULL, &rectPlayer);

    // pass to screen

    SDL_RenderPresent(as->renderer);

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
