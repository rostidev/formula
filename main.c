#define _GNU_SOURCE

#include <SDL3/SDL.h>
#include <math.h>

#include "main.h"

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} AppState;

AppState *state;

const float FPS = 60;
const float DT = 1/FPS;

point_t screen(point_t p) {
    int width, height;
    SDL_GetWindowSize(state->window, &width, &height);

    // -1..1 => 0..2 => 0..1 => 0..w
    return (point_t){
        x: (p.x + 1) / 2 * width,
        y: (1 - (p.y + 1) / 2) * height,
    };
}

void point(point_t p) {
    const float s = 10;
    SDL_SetRenderDrawColor(state->renderer, 0x50, 0xFF, 0x50, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(state->renderer, &(SDL_FRect){x: p.x - s/2, y: p.y - s/2, s, s});
}

point_t project(point3d_t p) {
    return (point_t){
        x: p.x / p.z,
        y: p.y / p.z
    };
}

point3d_t translate_z(point3d_t p, float dz) {
    return (point3d_t){
        x: p.x,
        y: p.y,
        z: p.z + dz};
}

point3d_t rotate_xz(point3d_t p, float angle) {
    double c = cos(angle);
    double s = sin(angle);
    return (point3d_t){
        x: p.x * c - p.z * s,
        y: p.y,
        z: p.x * s + p.z * c,
    };
}

point3d_t rotate_yz(point3d_t p, float angle) {
    double c = cos(angle);
    double s = sin(angle);
    return (point3d_t){
        x: p.x,
        y: p.y * c - p.z * s,
        z: p.y * s + p.z * c,
    };
}

void line(point_t p1, point_t p2) {
    SDL_RenderLine(state->renderer, p1.x, p1.y, p2.x, p2.y);
}

void frame(point3d_t vs[], fs_t fs[], size_t fs_len, float angle, float dz, float shift_x, float shift_y, point3d_t (* rotate)(point3d_t, float)) {
    for (int i = 0; i < fs_len; i++) {
        fs_t lines = fs[i];
        for (int j = 0; j < lines.size; j++) {
            point3d_t a = vs[lines.lines[j]];
            point3d_t b = vs[lines.lines[(j + 1) % lines.size]];

            a = (point3d_t){x: a.x + shift_x, y: a.y + shift_y, z: a.z};
            b = (point3d_t){x: b.x + shift_x, y: b.y + shift_y, z: b.z};

            point_t pa = screen(project(translate_z(rotate(a, angle), dz)));
            point_t pb = screen(project(translate_z(rotate(b, angle), dz)));

            line(pa, pb);
        }
    }
}

int main(int argc, char *argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    state = SDL_calloc(1, sizeof(AppState));
    if (!state) {
        SDL_Quit();
        return 1;
    }

    if (!SDL_CreateWindowAndRenderer("Formula demonstration",
                                     900, 900,
                                     SDL_WINDOW_RESIZABLE,
                                     &state->window,
                                     &state->renderer)) {
        SDL_Log("Failed to create window and renderer: %s", SDL_GetError());
        SDL_free(state);
        SDL_Quit();
        return 1;
    }

    float dz1, dz2, angle1, angle2;
    int i = 0;

    for (bool running = true; running;) {
        for (SDL_Event event; SDL_PollEvent(&event);) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        SDL_SetRenderDrawColor(state->renderer, 0x10, 0x10, 0x10, SDL_ALPHA_OPAQUE); // background
        SDL_RenderClear(state->renderer);
        SDL_SetRenderDrawColor(state->renderer, 0x50, 0xFF, 0x50, SDL_ALPHA_OPAQUE); // foreground

        if (i % 800 > 0) {
            dz1 += 1 * DT;
            dz2 += 1 * DT;
            angle1 += M_PI * DT;
            angle2 += M_PI * DT;
        } else {
            dz1 = 1 * DT;
            dz2 = 0.25 * DT;
            angle1 = 0;
            angle2 = 0;
            i = 0;
        }

        frame(vs_penguin, fs_penguin, sizeof fs_penguin / sizeof fs_penguin[0], angle1, dz1, 0, 0, rotate_xz);

        frame(vs_cube, fs_cube, sizeof fs_cube / sizeof fs_cube[0], angle2, dz2, -1.5, 0, rotate_yz);
        frame(vs_cube, fs_cube, sizeof fs_cube / sizeof fs_cube[0], angle2, dz2, +1.5, 0, rotate_yz);
        frame(vs_cube, fs_cube, sizeof fs_cube / sizeof fs_cube[0], angle2, dz2, 0, -1.5, rotate_yz);
        frame(vs_cube, fs_cube, sizeof fs_cube / sizeof fs_cube[0], angle2, dz2, 0, +1.5, rotate_yz);

        frame(vs_cube, fs_cube, sizeof fs_cube / sizeof fs_cube[0], angle2, dz2, -1.5, -1.5, rotate_yz);
        frame(vs_cube, fs_cube, sizeof fs_cube / sizeof fs_cube[0], angle2, dz2, -1.5, +1.5, rotate_yz);
        frame(vs_cube, fs_cube, sizeof fs_cube / sizeof fs_cube[0], angle2, dz2, +1.5, -1.5, rotate_yz);
        frame(vs_cube, fs_cube, sizeof fs_cube / sizeof fs_cube[0], angle2, dz2, +1.5, +1.5, rotate_yz);

        SDL_RenderPresent(state->renderer);
        SDL_Delay(1000/FPS);

        i++;
    }

    if (state->renderer) {
        SDL_DestroyRenderer(state->renderer);
    }
    if (state->window) {
        SDL_DestroyWindow(state->window);
    }
    SDL_free(state);
    SDL_Quit();

    return 0;
}
