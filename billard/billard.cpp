#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>
#include <math.h>
#include <corecrt_math_defines.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define TABLE_WIDTH 700
#define TABLE_HEIGHT 400
#define BALL_RADIUS 10.0f
#define INITIAL_SPEED 3.0f

static SDL_Window* window = NULL;
SDL_GLContext glcontext = NULL;
Uint64 previousTime, currentTime;

// Ball properties
float x = 0.0f;             // X position of the ball (center)
float y = 0.0f;             // Y position of the ball (center)
float angle = 45.0f;        // Initial angle in degrees
float dx = 0.0f;            // X movement component
float dy = 0.0f;            // Y movement component
float speed = INITIAL_SPEED;// Movement speed
bool ballMoving = false;    // Is the ball currently moving?
bool spacePressed = false;  // Is space currently pressed?

// Table boundaries
float tableLeft = -TABLE_WIDTH / 2.0f;
float tableRight = TABLE_WIDTH / 2.0f;
float tableTop = TABLE_HEIGHT / 2.0f;
float tableBottom = -TABLE_HEIGHT / 2.0f;

// Angle control
bool leftPressed = false;
bool rightPressed = false;
float angleSpeed = 2.0f;    // Degrees per frame for angle adjustment

void drawBall() {
    glColor3f(1.0f, 1.0f, 1.0f); // White ball

    // Draw the ball as a circle
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y); // Center of the circle

    // Draw the circle outline
    for (int i = 0; i <= 360; i += 10) {
        float angle_rad = i * M_PI / 180.0f;
        float px = x + BALL_RADIUS * cosf(angle_rad);
        float py = y + BALL_RADIUS * sinf(angle_rad);
        glVertex2f(px, py);
    }

    glEnd();

    // Draw direction indicator line when ball is not moving
    if (!ballMoving) {
        float rad_angle = angle * M_PI / 180.0f;
        float dirX = x + BALL_RADIUS * 2 * cosf(rad_angle);
        float dirY = y + BALL_RADIUS * 2 * sinf(rad_angle);

        glColor3f(1.0f, 0.0f, 0.0f); // Red line
        glBegin(GL_LINES);
        glVertex2f(x, y);
        glVertex2f(dirX, dirY);
        glEnd();
    }
}

void drawTable() {
    // Draw the green table
    glColor3f(0.0f, 0.5f, 0.0f); // Green
    glBegin(GL_QUADS);
    glVertex2f(tableLeft, tableBottom);
    glVertex2f(tableRight, tableBottom);
    glVertex2f(tableRight, tableTop);
    glVertex2f(tableLeft, tableTop);
    glEnd();

    // Draw the brown border
    glColor3f(0.5f, 0.25f, 0.0f); // Brown
    glLineWidth(10.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(tableLeft, tableBottom);
    glVertex2f(tableRight, tableBottom);
    glVertex2f(tableRight, tableTop);
    glVertex2f(tableLeft, tableTop);
    glEnd();
    glLineWidth(1.0f);
}

void drawAngleIndicator() {
    char angleStr[32];

    // Render text (simple placeholder since we don't have text rendering)
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(-WINDOW_WIDTH / 2.0f + 20, -WINDOW_HEIGHT / 2.0f + 20);
    // In a real implementation, you would use SDL_ttf or similar for text rendering
}

void drawInstructions() {
    // In a real implementation, you would use SDL_ttf or similar for text rendering
    // This is just a placeholder
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(-WINDOW_WIDTH / 2.0f + 20, -WINDOW_HEIGHT / 2.0f + 40);
    // Instructions would be:
    // "LEFT/RIGHT: Change angle, SPACE: Launch ball, R: Reset"
}

void updateBall() {
    if (!ballMoving) {
        // Update the angle based on input
        if (leftPressed) {
            angle -= angleSpeed;
            if (angle < 0) angle += 360.0f;
        }
        if (rightPressed) {
            angle += angleSpeed;
            if (angle >= 360.0f) angle -= 360.0f;
        }

        // Launch the ball if space is pressed
        if (spacePressed) {
            float rad_angle = angle * M_PI / 180.0f;
            dx = speed * cosf(rad_angle);
            dy = speed * sinf(rad_angle);
            ballMoving = true;
            spacePressed = false; // Reset space press
        }
    }
    else {
        // Update ball position
        x += dx;
        y += dy;

        // Collision with horizontal walls
        if (y + BALL_RADIUS > tableTop || y - BALL_RADIUS < tableBottom) {
            dy = -dy;
            // Adjust position to prevent getting stuck in the wall
            if (y + BALL_RADIUS > tableTop) {
                y = tableTop - BALL_RADIUS;
            }
            else if (y - BALL_RADIUS < tableBottom) {
                y = tableBottom + BALL_RADIUS;
            }
        }

        // Collision with vertical walls
        if (x + BALL_RADIUS > tableRight || x - BALL_RADIUS < tableLeft) {
            dx = -dx;
            // Adjust position to prevent getting stuck in the wall
            if (x + BALL_RADIUS > tableRight) {
                x = tableRight - BALL_RADIUS;
            }
            else if (x - BALL_RADIUS < tableLeft) {
                x = tableLeft + BALL_RADIUS;
            }
        }
    }
}

void resetBall() {
    x = 0.0f;
    y = 0.0f;
    ballMoving = false;
    dx = 0.0f;
    dy = 0.0f;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("Billiard Simulation", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    glcontext = SDL_GL_CreateContext(window);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Black background

    // Set up orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-WINDOW_WIDTH / 2.0f, WINDOW_WIDTH / 2.0f,
        -WINDOW_HEIGHT / 2.0f, WINDOW_HEIGHT / 2.0f,
        -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);

    // Initialize ball at the center
    resetBall();

    previousTime = SDL_GetTicks();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    // Handle key press events
    if (event->type == SDL_EVENT_KEY_DOWN) {
        switch (event->key.key) {
        case SDLK_SPACE:
            spacePressed = true;
            break;
        case SDLK_LEFT:
            leftPressed = true;
            break;
        case SDLK_RIGHT:
            rightPressed = true;
            break;
        case SDLK_R:
            resetBall();
            break;
        case SDLK_ESCAPE:
            return SDL_APP_SUCCESS; // Exit on ESC
        }
    }
    // Handle key release events
    else if (event->type == SDL_EVENT_KEY_UP) {
        switch (event->key.key) {
        case SDLK_SPACE:
            spacePressed = false;
            break;
        case SDLK_LEFT:
            leftPressed = false;
            break;
        case SDLK_RIGHT:
            rightPressed = false;
            break;
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    // Get current time for frame rate independence
    currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - previousTime) / 1000.0f; // Convert to seconds
    previousTime = currentTime;

    updateBall();

    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    drawTable();
    drawBall();
    drawAngleIndicator();
    drawInstructions();

    SDL_GL_SwapWindow(window);

    // Cap the frame rate
    SDL_Delay(16); // Target ~60fps

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    SDL_GL_DestroyContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}