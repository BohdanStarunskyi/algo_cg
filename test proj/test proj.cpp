#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <corecrt_math_defines.h>
#include <stdio.h>

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
bool gameOver = false;
bool printedGameOver = false;

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
    glColor3f(0.918f, 0.675f, 0.545f);
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
    glColor3f(0.427f, 0.349f, 0.478f);
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
    glColor3f(0.710f, 0.396f, 0.463f);

    glBegin(GL_QUADS);
    glVertex2f(-WINDOW_WIDTH / 2.0f, GROUND_Y);
    glVertex2f(WINDOW_WIDTH / 2.0f, GROUND_Y);
    glVertex2f(WINDOW_WIDTH / 2.0f, -WINDOW_HEIGHT + GROUND_Y);
    glVertex2f(-WINDOW_WIDTH / 2.0f, -WINDOW_HEIGHT + GROUND_Y);
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

void checkCollision() {
    float birdX = -WINDOW_WIDTH / 4.0f;

    for (int i = 0; i < NUM_PIPES; ++i) {
        float pipeX = pipes[i].x;
        float pipeLeft = pipeX;
        float pipeRight = pipeX + PIPE_WIDTH;
        float gapY = pipes[i].gapY;

        bool withinX = birdX + BIRD_RADIUS > pipeLeft && birdX - BIRD_RADIUS < pipeRight;
        bool outsideGap = birdY + BIRD_RADIUS > gapY + PIPE_GAP / 2.0f ||
            birdY - BIRD_RADIUS < gapY - PIPE_GAP / 2.0f;

        if (withinX && outsideGap) {
            gameOver = true;
            return;
        }
    }

    if (birdY - BIRD_RADIUS < GROUND_Y) {
        gameOver = true;
    }
}

void flap() {
    velocityY = FLAP_STRENGTH;
}

void resetGame() {
    birdY = 0.0f;
    velocityY = 0.0f;
    initPipes();
    gameOver = false;
    printedGameOver = false;
    previousTime = SDL_GetTicks();  // Prevent big dt
}

void drawGameOver() {
    // Draw a semi-transparent rectangle behind the X
    glColor3f(0.110f, 0.114f, 0.129f);
    glBegin(GL_QUADS);
    glVertex2f(-200.0f, 200.0f);
    glVertex2f(200.0f, 200.0f);
    glVertex2f(200.0f, -200.0f);
    glVertex2f(-200.0f, -200.0f);
    glEnd();

    // Draw a red X using two thick diagonal lines
    glLineWidth(10.0f);
    glColor3f(0.733f, 0.608f, 0.690f);
    glBegin(GL_LINES);
    glVertex2f(-150.0f, -150.0f);
    glVertex2f(150.0f, 150.0f);

    glVertex2f(-150.0f, 150.0f);
    glVertex2f(150.0f, -150.0f);
    glEnd();

    if (!printedGameOver) {
        printf("GAME OVER - Press SPACE to restart\n");
        printedGameOver = true;
    }
}



SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Flappy Bird Clone", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    glcontext = SDL_GL_CreateContext(window);

    glClearColor(0.208f, 0.314f, 0.439f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-WINDOW_WIDTH / 2.0f, WINDOW_WIDTH / 2.0f,
        -WINDOW_HEIGHT / 2.0f, WINDOW_HEIGHT / 2.0f,
        -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);

    resetGame();
    previousTime = SDL_GetTicks();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;

    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_SPACE) {
            if (gameOver) {
                resetGame();
            }
            else {
                flap();
            }
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    currentTime = SDL_GetTicks();
    float dt = (currentTime - previousTime) / 1000.0f;
    previousTime = currentTime;

    if (!gameOver) {
        updatePhysics(dt);
        checkCollision();
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    drawGround();
    drawPipes();
    drawBird();

    if (gameOver) {
        drawGameOver();  // <-- Draw the game over screen
    }

    SDL_GL_SwapWindow(window);
    SDL_Delay(16);

    return SDL_APP_CONTINUE;
}


void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    SDL_GL_DestroyContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
