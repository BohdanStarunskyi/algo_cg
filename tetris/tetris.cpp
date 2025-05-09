#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>
#include <GL/glu.h>
#include <vector>
#include <iostream>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 960
#define STEP_RATE_IN_MILLISECONDS 25

static SDL_Window* window = nullptr;
SDL_GLContext glcontext = nullptr;

class Brick {
public:
    float center_x, center_y;
    float dim;
    float r, g, b;
    float speed;
    bool falling_down;
    int posx;

    Brick() {
        center_x = 0.0f;
        center_y = 3.0f;
        dim = 0.2f;
        r = 1.0f; g = 0.0f; b = 0.0f;
        speed = 0.02f;
        falling_down = true;
        posx = 0;
    }

    void move_down() {
        if (falling_down) center_y -= speed;
    }

    void move_right() {
        if (falling_down && posx < 1) {
            center_x += dim;
            posx++;
        }
    }

    void move_left() {
        if (falling_down && posx > -1) {
            center_x -= dim;
            posx--;
        }
    }

    void draw() {
        glPushMatrix();
        glTranslatef(center_x, center_y, 0.0f);
        glScalef(0.95f * dim, 0.95f * dim, 1.0f);
        glColor3f(r, g, b);
        glBegin(GL_QUADS);
        glVertex3f(0.5f, 0.5f, 0.0f);
        glVertex3f(-0.5f, 0.5f, 0.0f);
        glVertex3f(-0.5f, -0.5f, 0.0f);
        glVertex3f(0.5f, -0.5f, 0.0f);
        glEnd();
        glPopMatrix();
    }
};

std::vector<Brick> elements = { Brick() };

void clear_full_rows() {
    int counts[3] = { 0, 0, 0 };
    for (auto& el : elements) {
        if (!el.falling_down) {
            counts[el.posx + 1]++;
        }
    }

    for (int i = 0; i < 3; i++) {
        if (counts[i] >= 3) {
            int target_posx = i - 1;
            elements.erase(std::remove_if(elements.begin(), elements.end(), [target_posx](Brick& b) {
                return !b.falling_down && b.posx == target_posx;
                }), elements.end());
        }
    }
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("Tetris-Like Game", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    glcontext = SDL_GL_CreateContext(window);
    SDL_SetAppMetadata("Tetris Game", "1.0", "openai.com");

    glClearColor(0.9f, 0.9f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_RIGHT) {
            elements.back().move_right();
        }
        if (event->key.key == SDLK_LEFT) {
            elements.back().move_left();
        }
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f);
    glScalef(3.0f, 1.0f, 1.0f);

    // Draw border
    glPushMatrix();
    glScalef(0.2f, 0.2f, 1.0f);
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex3f(-1.5f, 10.0f, 0.0f);
    glVertex3f(-1.5f, -9.5f, 0.0f);
    glVertex3f(1.5f, 10.0f, 0.0f);
    glVertex3f(1.5f, -9.5f, 0.0f);
    glVertex3f(-1.5f, -9.5f, 0.0f);
    glVertex3f(1.5f, -9.5f, 0.0f);
    glEnd();
    glPopMatrix();

    for (auto& el : elements) {
        el.draw();
        el.move_down();
        if (el.center_y - el.dim < -2.0f)
            el.falling_down = false;
    }

    Brick& falling = elements.back();
    for (int i = 0; i < elements.size() - 1; i++) {
        Brick& b = elements[i];
        if (falling.falling_down &&
            falling.posx == b.posx &&
            b.center_y < falling.center_y &&
            falling.center_y - b.center_y < falling.dim) {
            falling.center_y = b.center_y + falling.dim;
            falling.falling_down = false;
        }
    }

    if (!falling.falling_down) {
        clear_full_rows();
        elements.push_back(Brick());
    }

    SDL_GL_SwapWindow(window);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    SDL_DestroyWindow(window);
    SDL_GL_DestroyContext(glcontext);
    SDL_Quit();
}