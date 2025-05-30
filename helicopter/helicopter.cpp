#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>
#include <stdio.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

SDL_Window* window = NULL;
SDL_GLContext glcontext = NULL;

// Game variables
float birdY = 240.0f;        // Bird Y position (middle of screen)
float birdVelocity = 0.0f;   // Bird falling/jumping speed
float pipeX = 640.0f;        // Pipe X position (starts off-screen right)
float pipeGap = 120.0f;      // Gap between top and bottom pipes
float pipeGapY = 240.0f;     // Y position of gap center
int gameOver = 0;
int score = 0;

void DrawRect(float x, float y, float width, float height) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void DrawBird() {
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow bird
    DrawRect(100.0f, birdY - 15.0f, 30.0f, 30.0f);
}

void DrawPipes() {
    glColor3f(0.0f, 0.8f, 0.0f); // Green pipes

    // Top pipe
    DrawRect(pipeX, 0.0f, 60.0f, pipeGapY - pipeGap / 2);

    // Bottom pipe
    DrawRect(pipeX, pipeGapY + pipeGap / 2, 60.0f, WINDOW_HEIGHT - (pipeGapY + pipeGap / 2));
}

void DrawBackground() {
    glColor3f(0.5f, 0.8f, 1.0f); // Sky blue
    DrawRect(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT);
}

int CheckCollision() {
    // Bird boundaries
    float birdLeft = 100.0f;
    float birdRight = 130.0f;
    float birdTop = birdY - 15.0f;
    float birdBottom = birdY + 15.0f;

    // Check ground/ceiling collision
    if (birdTop <= 0.0f || birdBottom >= WINDOW_HEIGHT) {
        return 1;
    }

    // Check pipe collision
    if (birdRight > pipeX && birdLeft < pipeX + 60.0f) {
        // Bird is horizontally aligned with pipe
        if (birdTop < pipeGapY - pipeGap / 2 || birdBottom > pipeGapY + pipeGap / 2) {
            return 1; // Hit pipe
        }
    }

    return 0;
}

void ResetGame() {
    birdY = 240.0f;
    birdVelocity = 0.0f;
    pipeX = 640.0f;
    pipeGapY = 150.0f + (float)(rand() % 180); // Random gap position
    gameOver = 0;
    score = 0;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return SDL_APP_FAILURE;

    window = SDL_CreateWindow("Flappy Bird", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) return SDL_APP_FAILURE;

    glcontext = SDL_GL_CreateContext(window);

    // Set up 2D orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1); // Flip Y axis
    glMatrixMode(GL_MODELVIEW);

    glDisable(GL_DEPTH_TEST); // We don't need depth testing for 2D

    ResetGame();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_SPACE) {
            if (gameOver) {
                ResetGame();
            }
            else {
                birdVelocity = -8.0f; // Jump up (negative Y is up)
            }
        }
    }

    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (gameOver) {
            ResetGame();
        }
        else {
            birdVelocity = -8.0f; // Jump up
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    if (!gameOver) {
        // Apply gravity
        birdVelocity += 0.5f; // Gravity acceleration
        birdY += birdVelocity;

        // Move pipe left
        pipeX -= 3.0f;

        // Reset pipe when it goes off screen
        if (pipeX < -60.0f) {
            pipeX = 640.0f;
            pipeGapY = 150.0f + (float)(rand() % 180); // Random gap position
            score++;
        }

        // Check for collisions
        if (CheckCollision()) {
            gameOver = 1;
        }
    }

    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Draw everything
    DrawBackground();
    DrawPipes();
    DrawBird();

    // Draw game over text (simple representation)
    if (gameOver) {
        glColor3f(1.0f, 0.0f, 0.0f); // Red
        DrawRect(200.0f, 200.0f, 240.0f, 80.0f); // Game over "text"
    }

    SDL_GL_SwapWindow(window);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    SDL_GL_DestroyContext(glcontext);
    SDL_DestroyWindow(window);
}