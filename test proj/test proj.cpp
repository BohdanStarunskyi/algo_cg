#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>
#include <math.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

#include <corecrt_math_defines.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BIRD_RADIUS 15.0f
#define PIPE_WIDTH 80.0f
#define PIPE_GAP 200.0f
#define PIPE_SPEED 200.0f
#define NUM_PIPES 3

float birdY = 0.0f;
float velocityY = 0.0f;
const float GRAVITY = -900.0f;
const float FLAP_STRENGTH = 300.0f;
const float GROUND_Y = -WINDOW_HEIGHT / 2.0f + 50.0f;

SDL_Window* window = NULL;
SDL_GLContext glcontext = NULL;
Uint64 previousTime, currentTime;

struct Pipe {
    float x;
    float gapY;
};

struct Pipe pipes[NUM_PIPES];

void initPipes() {
    for (int i = 0; i < NUM_PIPES; ++i) {
        pipes[i].x = WINDOW_WIDTH / 2.0f + i * 300.0f;
        pipes[i].gapY = (rand() % 300) - 150;
    }
}


void drawBird() {
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(-WINDOW_WIDTH / 4.0f, birdY);
    for (int i = 0; i <= 360; i += 10) {
        float angle = i * M_PI / 180.0f;
        float x = BIRD_RADIUS * cosf(angle);
        float y = BIRD_RADIUS * sinf(angle);
        glVertex2f(-WINDOW_WIDTH / 4.0f + x, birdY + y);
    }
    glEnd();
}


void drawPipes() {
    glColor3f(0.2f, 1.0f, 0.2f);
    for (int i = 0; i < NUM_PIPES; ++i) {
        float x = pipes[i].x;
        float gapY = pipes[i].gapY;

        // Top pipe
        glBegin(GL_QUADS);
        glVertex2f(x, gapY + PIPE_GAP / 2.0f);
        glVertex2f(x + PIPE_WIDTH, gapY + PIPE_GAP / 2.0f);
        glVertex2f(x + PIPE_WIDTH, WINDOW_HEIGHT / 2.0f);
        glVertex2f(x, WINDOW_HEIGHT / 2.0f);
        glEnd();

        // Bottom pipe
        glBegin(GL_QUADS);
        glVertex2f(x, -WINDOW_HEIGHT / 2.0f);
        glVertex2f(x + PIPE_WIDTH, -WINDOW_HEIGHT / 2.0f);
        glVertex2f(x + PIPE_WIDTH, gapY - PIPE_GAP / 2.0f);
        glVertex2f(x, gapY - PIPE_GAP / 2.0f);
        glEnd();
    }
}

void drawGround() {
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(-WINDOW_WIDTH / 2.0f, GROUND_Y);
    glVertex2f(WINDOW_WIDTH / 2.0f, GROUND_Y);
    glVertex2f(WINDOW_WIDTH / 2.0f, GROUND_Y - 20.0f);
    glVertex2f(-WINDOW_WIDTH / 2.0f, GROUND_Y - 20.0f);
    glEnd();
}

void updatePhysics(float dt) {
    birdY += velocityY * dt;
    velocityY += GRAVITY * dt;

    for (int i = 0; i < NUM_PIPES; ++i) {
        pipes[i].x -= PIPE_SPEED * dt;

        if (pipes[i].x + PIPE_WIDTH < -WINDOW_WIDTH / 2.0f) {
            pipes[i].x += NUM_PIPES * 300.0f;
            pipes[i].gapY = (rand() % 300) - 150;
        }
    }

    if (birdY - BIRD_RADIUS < GROUND_Y) {
        birdY = GROUND_Y + BIRD_RADIUS;
        velocityY = 0.0f;
    }
}

void flap() {
    velocityY = FLAP_STRENGTH;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Flappy Bird Clone", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    glcontext = SDL_GL_CreateContext(window);

    glClearColor(0.0f, 0.0f, 0.3f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-WINDOW_WIDTH / 2.0f, WINDOW_WIDTH / 2.0f,
        -WINDOW_HEIGHT / 2.0f, WINDOW_HEIGHT / 2.0f,
        -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);

    birdY = 0.0f;
    velocityY = 0.0f;
    initPipes();
    previousTime = SDL_GetTicks();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
    if (event->type == SDL_EVENT_KEY_DOWN) flap();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    currentTime = SDL_GetTicks();
    float dt = (currentTime - previousTime) / 1000.0f;
    previousTime = currentTime;

    updatePhysics(dt);

    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    drawGround();
    drawPipes();
    drawBird();

    SDL_GL_SwapWindow(window);
    SDL_Delay(16);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    SDL_GL_DestroyContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
