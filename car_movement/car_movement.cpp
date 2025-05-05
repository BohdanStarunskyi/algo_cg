#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>
#include <math.h>
#include <corecrt_math_defines.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define STEP_RATE_IN_MILLISECONDS 25

static SDL_Window* window = NULL;
SDL_GLContext glcontext = NULL;
Uint64 previousTime, currentTime;

// Car properties
float posx = 0.0f;    // X position of the car
float posz = 0.0f;    // Z position of the car
float angle = 0.0f;   // Rotation angle of the car (in radians)
float speed = 0.2f;   // Movement speed
float rotation_speed = 0.1f; // Rotation speed in radians

// Keyboard state
const Uint8* keystate;

/* Replace gluPerspective with this helper */
void setPerspective(float fovY, float aspect, float zNear, float zFar) {
    float ymax = 1;
    float xmax = ymax * aspect;
    glFrustum(-xmax, xmax, -ymax, ymax, zNear, zFar);
}

void drawCar() {
    // Save the current matrix
    glPushMatrix();

    // Apply transformations: first translate to position, then rotate
    glTranslatef(posx, 0.0f, posz);
    glRotatef(angle * 180.0f / M_PI, 0.0f, 1.0f, 0.0f); // Convert radians to degrees

    // Draw the car body (cube)
    glBegin(GL_QUADS);

    // Top face (y = 0.5)
    glColor3f(1.0f, 0.0f, 0.0f); // Red
    glVertex3f(-0.5f, 0.5f, -1.0f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, -1.0f);

    // Bottom face (y = -0.5)
    glColor3f(0.5f, 0.0f, 0.0f); // Dark red
    glVertex3f(-0.5f, -0.5f, -1.0f);
    glVertex3f(0.5f, -0.5f, -1.0f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);

    // Front face (z = 0.5)
    glColor3f(0.0f, 0.0f, 1.0f); // Blue
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);

    // Back face (z = -1.0)
    glColor3f(0.0f, 0.0f, 0.5f); // Dark blue
    glVertex3f(-0.5f, -0.5f, -1.0f);
    glVertex3f(-0.5f, 0.5f, -1.0f);
    glVertex3f(0.5f, 0.5f, -1.0f);
    glVertex3f(0.5f, -0.5f, -1.0f);

    // Left face (x = -0.5)
    glColor3f(0.0f, 1.0f, 0.0f); // Green
    glVertex3f(-0.5f, -0.5f, -1.0f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, -1.0f);

    // Right face (x = 0.5)
    glColor3f(0.0f, 0.5f, 0.0f); // Dark green
    glVertex3f(0.5f, -0.5f, -1.0f);
    glVertex3f(0.5f, 0.5f, -1.0f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);

    glEnd();

    // Restore the matrix
    glPopMatrix();
}

void drawGround() {
    // Draw a grid to represent the ground
    glBegin(GL_LINES);
    glColor3f(0.5f, 0.5f, 0.5f); // Gray

    // Draw lines along X axis
    for (int i = -10; i <= 10; i++) {
        glVertex3f(-10.0f, -0.5f, i);
        glVertex3f(10.0f, -0.5f, i);
    }

    // Draw lines along Z axis
    for (int i = -10; i <= 10; i++) {
        glVertex3f(i, -0.5f, -10.0f);
        glVertex3f(i, -0.5f, 10.0f);
    }

    glEnd();
}

void processInput() {
     const bool* keystate = SDL_GetKeyboardState(NULL);

    // Rotate left with left arrow
    if (keystate[SDL_SCANCODE_LEFT]) {
        angle -= rotation_speed;
        // Keep angle in [0, 2π) range
        if (angle < 0) angle += 2 * M_PI;
    }

    // Rotate right with right arrow
    if (keystate[SDL_SCANCODE_RIGHT]) {
        angle += rotation_speed;
        // Keep angle in [0, 2π) range
        if (angle >= 2 * M_PI) angle -= 2 * M_PI;
    }

    // Move forward with up arrow
    if (keystate[SDL_SCANCODE_UP]) {
        float dx = speed * cosf(angle);
        float dz = speed * sinf(angle);
        posx += dx;
        posz -= dz; // Negative because OpenGL Z is pointing away from viewer
    }
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("Car Movement", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetAppMetadata("Car Movement", "1.0", "OpenGL Car Demo");

    glcontext = SDL_GL_CreateContext(window);
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);  // Sky blue background

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    setPerspective(45.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 1.0f, 500.0f);

    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);

    previousTime = SDL_GetTicks();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    processInput();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Position the camera slightly above and behind the car for better viewing
    // Manual camera positioning instead of gluLookAt
    glTranslatef(0.0f, -2.0f, -10.0f);
    glRotatef(20.0f, 1.0f, 0.0f, 0.0f); // Tilt the camera down a bit

    drawGround();
    drawCar();

    SDL_GL_SwapWindow(window);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    SDL_GL_DestroyContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}