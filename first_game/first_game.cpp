#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>
#include <math.h>
#include <corecrt_math_defines.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define BOARD_SIZE 300
#define SQUARE_SIZE (BOARD_SIZE / 3)

static SDL_Window* window = NULL;
SDL_GLContext glcontext = NULL;

int activeRow = 1;
int activeCol = 1;

// 0 = empty, 1 = X, 2 = O
int board[3][3] = { 0 };

void drawSymbol(int type, float x, float y) {
    if (type == 1) {
        //x
        glColor3f(1, 0, 0);
        glLineWidth(4);
        glBegin(GL_LINES);
        glVertex2f(x + 10, y + 10);
        glVertex2f(x + SQUARE_SIZE - 10, y + SQUARE_SIZE - 10);
        glVertex2f(x + SQUARE_SIZE - 10, y + 10);
        glVertex2f(x + 10, y + SQUARE_SIZE - 10);
        glEnd();
    }
    else if (type == 2) {
        //O
        glColor3f(0, 0, 1);
        glLineWidth(4);
        float cx = x + SQUARE_SIZE / 2;
        float cy = y + SQUARE_SIZE / 2;
        float radius = SQUARE_SIZE / 2 - 10;
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 100; ++i) {
            float angle = i * 2.0f * M_PI / 100;
            glVertex2f(cx + cosf(angle) * radius, cy + sinf(angle) * radius);
        }
        glEnd();
    }
}

void drawBoard() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    //grid
    glColor3f(1, 1, 1);
    glLineWidth(2);
    glBegin(GL_LINES);
    for (int i = 1; i < 3; i++) {
        float offset = -BOARD_SIZE / 2 + i * SQUARE_SIZE;
        glVertex2f(offset, -BOARD_SIZE / 2);
        glVertex2f(offset, BOARD_SIZE / 2);
        glVertex2f(-BOARD_SIZE / 2, offset);
        glVertex2f(BOARD_SIZE / 2, offset);
    }
    glEnd();

    //squares and active highlight
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            float x = -BOARD_SIZE / 2 + col * SQUARE_SIZE;
            float y = -BOARD_SIZE / 2 + (2 - row) * SQUARE_SIZE;

            //active square
            if (row == activeRow && col == activeCol) {
                glColor3f(0.3f, 0.8f, 0.3f);
                glBegin(GL_QUADS);
                glVertex2f(x, y);
                glVertex2f(x + SQUARE_SIZE, y);
                glVertex2f(x + SQUARE_SIZE, y + SQUARE_SIZE);
                glVertex2f(x, y + SQUARE_SIZE);
                glEnd();
            }

            //draw symbol
            if (board[row][col] != 0) {
                drawSymbol(board[row][col], x, y);
            }
        }
    }
}

void handleKey(SDL_Keycode key) {
    if (key == SDLK_UP && activeRow > 0) activeRow--;
    if (key == SDLK_DOWN && activeRow < 2) activeRow++;
    if (key == SDLK_LEFT && activeCol > 0) activeCol--;
    if (key == SDLK_RIGHT && activeCol < 2) activeCol++;
    if (key == SDLK_X && board[activeRow][activeCol] == 0)
        board[activeRow][activeCol] = 1;
    if (key == SDLK_O && board[activeRow][activeCol] == 0)
        board[activeRow][activeCol] = 2;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return SDL_APP_FAILURE;

    window = SDL_CreateWindow("Tic-Tac-Toe", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) return SDL_APP_FAILURE;

    glcontext = SDL_GL_CreateContext(window);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-WINDOW_WIDTH / 2, WINDOW_WIDTH / 2, -WINDOW_HEIGHT / 2, WINDOW_HEIGHT / 2, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
    if (event->type == SDL_EVENT_KEY_DOWN) {
        handleKey(event->key.key);
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    drawBoard();
    SDL_GL_SwapWindow(window);
	SDL_Delay(16); //~60 FPS
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    SDL_GL_DestroyContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
