#define SDL_MAIN_USE_CALLBACKS 1  // use the callbacks instead of main()
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>
#include <stdbool.h>
#include <math.h>  // for sinf, cosf

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define BOARD_SIZE 300
#define SQUARE_SIZE (BOARD_SIZE / 3)

static SDL_Window* window = NULL;
SDL_GLContext glcontext = NULL;

typedef enum {
    EMPTY = 0,
    NOUGHT,
    CROSS
} SquareState;

SquareState board[3][3] = {
    {EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY}
};

int activeRow = 1;
int activeCol = 1;
SquareState winner = EMPTY;

// Colors
const float BACKGROUND_COLOR[3] = { 0.2f, 0.2f, 0.2f };
const float GRID_COLOR[3] = { 1.0f, 1.0f, 1.0f };
const float ACTIVE_COLOR[3] = { 0.3f, 0.7f, 0.3f };
const float REGULAR_COLOR[3] = { 0.5f, 0.5f, 0.5f };
const float NOUGHT_COLOR[3] = { 0.0f, 0.0f, 1.0f };
const float CROSS_COLOR[3] = { 1.0f, 0.0f, 0.0f };

// Forward declarations
void drawChar(float x, float y, char c, float size);
void renderSimpleText(float x, float y, const char* text, float size);
SquareState checkWinner();

void drawBoard() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Grid lines
    glColor3fv(GRID_COLOR);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    for (int i = 1; i < 3; i++) {
        float x = -BOARD_SIZE / 2 + i * SQUARE_SIZE;
        glVertex2f(x, -BOARD_SIZE / 2);
        glVertex2f(x, BOARD_SIZE / 2);
    }
    for (int i = 1; i < 3; i++) {
        float y = -BOARD_SIZE / 2 + i * SQUARE_SIZE;
        glVertex2f(-BOARD_SIZE / 2, y);
        glVertex2f(BOARD_SIZE / 2, y);
    }
    glEnd();

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            float x = -BOARD_SIZE / 2 + col * SQUARE_SIZE;
            float y = -BOARD_SIZE / 2 + (2 - row) * SQUARE_SIZE;

            glColor3fv((row == activeRow && col == activeCol) ? ACTIVE_COLOR : REGULAR_COLOR);
            glBegin(GL_QUADS);
            glVertex2f(x, y);
            glVertex2f(x + SQUARE_SIZE, y);
            glVertex2f(x + SQUARE_SIZE, y + SQUARE_SIZE);
            glVertex2f(x, y + SQUARE_SIZE);
            glEnd();

            switch (board[row][col]) {
            case NOUGHT:
                glColor3fv(NOUGHT_COLOR);
                glBegin(GL_LINE_LOOP);
                for (int i = 0; i < 360; i += 10) {
                    float angle = i * 3.14159f / 180.0f;
                    float rx = x + SQUARE_SIZE / 2 + SQUARE_SIZE / 3 * cosf(angle);
                    float ry = y + SQUARE_SIZE / 2 + SQUARE_SIZE / 3 * sinf(angle);
                    glVertex2f(rx, ry);
                }
                glEnd();
                break;

            case CROSS:
                glColor3fv(CROSS_COLOR);
                glBegin(GL_LINES);
                glVertex2f(x + SQUARE_SIZE / 4, y + SQUARE_SIZE * 3 / 4);
                glVertex2f(x + SQUARE_SIZE * 3 / 4, y + SQUARE_SIZE / 4);
                glVertex2f(x + SQUARE_SIZE / 4, y + SQUARE_SIZE / 4);
                glVertex2f(x + SQUARE_SIZE * 3 / 4, y + SQUARE_SIZE * 3 / 4);
                glEnd();
                break;

            default:
                break;
            }
        }
    }

    if (winner != EMPTY) {
        const char* msg = (winner == CROSS) ? "X WINS!" : "O WINS!";
        renderSimpleText(-100.0f, -WINDOW_HEIGHT / 2 + 30.0f, msg, 20.0f);
    }
}

void handleKeyPress(SDL_Keycode key) {
    switch (key) {
    case SDLK_UP:    if (activeRow > 0) activeRow--; break;
    case SDLK_DOWN:  if (activeRow < 2) activeRow++; break;
    case SDLK_LEFT:  if (activeCol > 0) activeCol--; break;
    case SDLK_RIGHT: if (activeCol < 2) activeCol++; break;

    case SDLK_SPACE:
    case SDLK_RETURN: {
        if (board[activeRow][activeCol] == EMPTY) {
            int count = 0;
            for (int r = 0; r < 3; r++)
                for (int c = 0; c < 3; c++)
                    if (board[r][c] != EMPTY) count++;
            board[activeRow][activeCol] = (count % 2 == 0) ? CROSS : NOUGHT;
            winner = checkWinner();
        }
        break;
    }

    case SDLK_R:
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 3; c++)
                board[r][c] = EMPTY;
        winner = EMPTY;
        break;

    default:
        break;
    }
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("Tic-Tac-Toe Navigation", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    glcontext = SDL_GL_CreateContext(window);
    glClearColor(BACKGROUND_COLOR[0], BACKGROUND_COLOR[1], BACKGROUND_COLOR[2], 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-WINDOW_WIDTH / 2, WINDOW_WIDTH / 2, -WINDOW_HEIGHT / 2, WINDOW_HEIGHT / 2, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;

    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_ESCAPE) return SDL_APP_SUCCESS;
        handleKeyPress(event->key.key);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    drawBoard();
    SDL_GL_SwapWindow(window);
    SDL_Delay(16);  // ~60 FPS
    return SDL_APP_CONTINUE;
}

SquareState checkWinner() {
    for (int i = 0; i < 3; i++) {
        if (board[i][0] != EMPTY && board[i][0] == board[i][1] && board[i][1] == board[i][2])
            return board[i][0];
        if (board[0][i] != EMPTY && board[0][i] == board[1][i] && board[1][i] == board[2][i])
            return board[0][i];
    }
    if (board[0][0] != EMPTY && board[0][0] == board[1][1] && board[1][1] == board[2][2])
        return board[0][0];
    if (board[0][2] != EMPTY && board[0][2] == board[1][1] && board[1][1] == board[2][0])
        return board[0][2];

    return EMPTY;
}

void renderSimpleText(float x, float y, const char* text, float size) {
    glColor3f(1.0f, 1.0f, 0.0f);  // Yellow
    float spacing = size + 4;
    for (int i = 0; text[i] != '\0'; i++) {
        drawChar(x + i * spacing, y, text[i], size);
    }
}

void drawChar(float x, float y, char c, float size) {
    glBegin(GL_LINES);
    switch (c) {
    case 'X':
        glVertex2f(x, y);
        glVertex2f(x + size, y + size);
        glVertex2f(x + size, y);
        glVertex2f(x, y + size);
        break;
    case 'O':
        for (int i = 0; i < 360; i += 30) {
            float a1 = i * 3.14159f / 180.0f;
            float a2 = (i + 30) * 3.14159f / 180.0f;
            glVertex2f(x + size / 2 + cosf(a1) * size / 2, y + size / 2 + sinf(a1) * size / 2);
            glVertex2f(x + size / 2 + cosf(a2) * size / 2, y + size / 2 + sinf(a2) * size / 2);
        }
        break;
    case 'W':
        glVertex2f(x, y + size);
        glVertex2f(x + size / 4, y);
        glVertex2f(x + size / 4, y);
        glVertex2f(x + size / 2, y + size / 2);
        glVertex2f(x + size / 2, y + size / 2);
        glVertex2f(x + 3 * size / 4, y);
        glVertex2f(x + 3 * size / 4, y);
        glVertex2f(x + size, y + size);
        break;
    case 'I':
        glVertex2f(x + size / 2, y);
        glVertex2f(x + size / 2, y + size);
        break;
    case 'N':
        glVertex2f(x, y);
        glVertex2f(x, y + size);
        glVertex2f(x, y + size);
        glVertex2f(x + size, y);
        glVertex2f(x + size, y);
        glVertex2f(x + size, y + size);
        break;
    case 'S':
        glVertex2f(x + size, y + size);
        glVertex2f(x, y + size);
        glVertex2f(x, y + size);
        glVertex2f(x, y + size / 2);
        glVertex2f(x, y + size / 2);
        glVertex2f(x + size, y + size / 2);
        glVertex2f(x + size, y + size / 2);
        glVertex2f(x + size, y);
        glVertex2f(x + size, y);
        glVertex2f(x, y);
        break;
    case '!':
        glVertex2f(x + size / 2, y);
        glVertex2f(x + size / 2, y + size * 0.75f);
        glVertex2f(x + size / 2, y + size);
        glVertex2f(x + size / 2, y + size);
        break;
    case ' ':
        break;
    }
    glEnd();
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    SDL_GL_DestroyContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
