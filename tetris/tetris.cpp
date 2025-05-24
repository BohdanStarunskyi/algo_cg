#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>
#include <vector>
#include <algorithm>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

static SDL_Window* window = nullptr;
SDL_GLContext glcontext = nullptr;

void setPerspective(float fovY, float aspect, float zNear, float zFar) {
    float ymax = 1;
    float xmax = ymax * aspect;
    glFrustum(-xmax, xmax, -ymax, ymax, zNear, zFar);
}

class Brick {
public:
    float x = 0.0f;     
    float y = 3.0f; 
    float size = 0.2f;   //width/height
    int col = 0;//column index: -1=left, 0=center, 1=right
    bool falling = true;

    void update() {
        if (falling) y -= 0.02f;
    }

    void moveLeft() {
        if (falling && col > -1) {
            col--;
            x -= size;
        }
    }

    void moveRight() {
        if (falling && col < 1) {
            col++;
            x += size;
        }
    }

    void draw() const {
        //square
        glPushMatrix();
        glTranslatef(x, y, 0.0f);
        glScalef(size * 0.9f, size * 0.9f, 1.0f);
        glColor3f(1.0f, 0.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(-0.5f, -0.5f);
        glVertex2f(0.5f, -0.5f);
        glVertex2f(0.5f, 0.5f);
        glVertex2f(-0.5f, 0.5f);
        glEnd();
        glPopMatrix();
    }
};

//all bricks in game
std::vector<Brick> bricks = { Brick() };

void clearFullRows() {
    std::vector<float> rowYs;

    for (const auto& b : bricks) {
        if (!b.falling) {
            rowYs.push_back(b.y);
        }
    }
    for (float yCheck : rowYs) {
        int colFilled[3] = { 0 };

        for (const auto& b : bricks) {
            if (!b.falling && fabs(b.y - yCheck) < 0.01f) {
                colFilled[b.col + 1]++;
            }
        }

        if (colFilled[0] && colFilled[1] && colFilled[2]) {
            bricks.erase(std::remove_if(bricks.begin(), bricks.end(),
                [=](const Brick& b) {
                    return !b.falling && fabs(b.y - yCheck) < 0.01f;
                }), bricks.end());
        }
    }
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return SDL_APP_FAILURE;

    window = SDL_CreateWindow("Mini Tetris", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) return SDL_APP_FAILURE;

    glcontext = SDL_GL_CreateContext(window);
    glClearColor(0.9f, 0.9f, 1.0f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    setPerspective(45, (double)WINDOW_WIDTH / WINDOW_HEIGHT, 1, 10);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_DEPTH_TEST);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;

    if (event->type == SDL_EVENT_KEY_DOWN) {
        Brick& b = bricks.back();
        if (event->key.key == SDLK_LEFT) b.moveLeft();
        if (event->key.key == SDLK_RIGHT) b.moveRight();
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f);
    glScalef(3.0f, 1.0f, 1.0f); 

    glBegin(GL_LINE_LOOP);
    glColor3f(0, 0, 0);
    glVertex2f(-1.5f, -2.0f);
    glVertex2f(-1.5f, 3.0f);
    glVertex2f(1.5f, 3.0f);
    glVertex2f(1.5f, -2.0f);
    glEnd();

    Brick& active = bricks.back();
    active.update();

    if (active.y - active.size < -2.0f)
        active.falling = false;

    //check for collision with fixed bricks
    for (const auto& b : bricks) {
        if (&b == &active || b.falling) continue;

        if (active.falling && active.col == b.col &&
            b.y < active.y && (active.y - b.y) < active.size) {
            active.y = b.y + active.size;
            active.falling = false;
        }
    }

    //spawn next
    if (!active.falling) {
        clearFullRows();
        bricks.push_back(Brick());
    }

    for (auto& b : bricks)
        b.draw();

    SDL_GL_SwapWindow(window);
    SDL_Delay(16); // ~60 FPS
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    SDL_GL_DestroyContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
