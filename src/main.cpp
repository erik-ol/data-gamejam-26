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

#define GAME_SIZE_X 100
#define GAME_SIZE_Y 100

struct AppState {
    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_Texture * brick_texture;
    SDL_Texture* targetTexture;
};

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

    as->brick_texture = IMG_LoadTexture(as->renderer, "assets/textures/bricks.png");

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState *as = (AppState *)appstate;

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

    SDL_RenderTexture(as->renderer, as->brick_texture, NULL, &rectBricks);
    
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