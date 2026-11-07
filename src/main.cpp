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

struct AppState {
    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_Texture * brick_texture;
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

    SDL_RenderTexture(as->renderer, as->brick_texture, NULL, &rectBricks);

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