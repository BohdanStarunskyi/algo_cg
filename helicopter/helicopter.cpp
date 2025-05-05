#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define STEP_RATE_IN_MILLISECONDS 25

static SDL_Window* window = NULL;
SDL_GLContext glcontext = NULL;
Uint64 previousTime, currentTime;
float mainBladeRotation = 0.0f;
float tailBladeRotation = 0.0f;
float viewAngle = 30.0f;

/* Replace gluPerspective with this helper */
void setPerspective(float fovY, float aspect, float zNear, float zFar) {
    float ymax = 1;
    float xmax = ymax * aspect;
    glFrustum(-xmax, xmax, -ymax, ymax, zNear, zFar);
}

/* DrawCube function - draws a unit cube centered at the origin */
void DrawCube() {
    glBegin(GL_QUADS);
    // Front face (z = 0.5)
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);

    // Back face (z = -0.5)
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);

    // Top face (y = 0.5)
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);

    // Bottom face (y = -0.5)
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);

    // Right face (x = 0.5)
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);

    // Left face (x = -0.5)
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glEnd();
}

void DrawHelicopter() {
    // Main helicopter body
    glPushMatrix();
    glColor3f(0.2f, 0.5f, 0.8f); // Blue for the body
    glScalef(3.0f, 1.0f, 1.2f);  // Scale to make it longer than tall
    DrawCube();
    glPopMatrix();

    // Tail boom
    glPushMatrix();
    glColor3f(0.3f, 0.3f, 0.7f); // Darker blue for tail
    glTranslatef(-2.5f, 0.0f, 0.0f); // Place at the back
    glScalef(3.0f, 0.3f, 0.3f);  // Make it long and thin
    DrawCube();
    glPopMatrix();

    // Main rotor hub
    glPushMatrix();
    glColor3f(0.1f, 0.1f, 0.1f); // Black for the rotor hub
    glTranslatef(0.0f, 0.6f, 0.0f);
    glScalef(0.3f, 0.2f, 0.3f);
    DrawCube();
    glPopMatrix();

    // Main rotor blades (front) with rotation
    glPushMatrix();
    glTranslatef(0.0f, 0.7f, 0.0f); // Position at the top of the body
    glRotatef(mainBladeRotation, 0.0f, 1.0f, 0.0f); // Rotate around Y axis
    glColor3f(0.7f, 0.7f, 0.7f); // Light gray for blades

    // Drawing blade manually with a simple scaled cube
    glScalef(0.1f, 0.05f, 6.0f);
    DrawCube();
    glPopMatrix();

    // Second main rotor blade (perpendicular to the first)
    glPushMatrix();
    glTranslatef(0.0f, 0.7f, 0.0f);
    glRotatef(mainBladeRotation, 0.0f, 1.0f, 0.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f); // Rotate 90 degrees to make perpendicular
    glColor3f(0.7f, 0.7f, 0.7f);

    glScalef(0.1f, 0.05f, 6.0f);
    DrawCube();
    glPopMatrix();

    // Tail rotor hub
    glPushMatrix();
    glColor3f(0.1f, 0.1f, 0.1f); // Black for hub
    glTranslatef(-4.0f, 0.0f, 0.3f);
    glScalef(0.2f, 0.2f, 0.2f);
    DrawCube();
    glPopMatrix();

    // Tail rotor blades with rotation
    glPushMatrix();
    glTranslatef(-4.0f, 0.0f, 0.4f); // Position at the end of the tail
    glRotatef(tailBladeRotation, 0.0f, 0.0f, 1.0f); // Rotate around Z axis
    glColor3f(0.7f, 0.7f, 0.7f); // Light gray for blades

    // Drawing tail blade manually with a simple scaled cube
    glScalef(0.05f, 2.0f, 0.05f);
    DrawCube();
    glPopMatrix();

    // Second tail rotor blade (perpendicular to the first)
    glPushMatrix();
    glTranslatef(-4.0f, 0.0f, 0.4f);
    glRotatef(tailBladeRotation, 0.0f, 0.0f, 1.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f); // Rotate 90 degrees to make perpendicular
    glColor3f(0.7f, 0.7f, 0.7f);

    glScalef(0.05f, 2.0f, 0.05f);
    DrawCube();
    glPopMatrix();
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("Simple Helicopter with Rotating Blades", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetAppMetadata("Simple Helicopter with Rotating Blades", "1.0", "");

    glcontext = SDL_GL_CreateContext(window);
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);  // Sky blue background

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    setPerspective(45.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 1.0f, 500.0f);  // replaces gluPerspective

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
    // Update animations
    currentTime = SDL_GetTicks();
    if (currentTime > previousTime + STEP_RATE_IN_MILLISECONDS) {
        // Rotate the blades
        mainBladeRotation += 15.0f; // Main rotor rotates quickly
        if (mainBladeRotation > 360.0f) mainBladeRotation -= 360.0f;

        tailBladeRotation += 30.0f; // Tail rotor rotates even faster
        if (tailBladeRotation > 360.0f) tailBladeRotation -= 360.0f;

        // Slowly change viewing angle
        viewAngle += 0.2f;
        if (viewAngle > 360.0f) viewAngle -= 360.0f;

        previousTime = currentTime;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Position the camera to view the helicopter
    glTranslatef(0.0f, 0.0f, -15.0f);
    glRotatef(10.0f, 1.0f, 0.0f, 0.0f); // Slight angle from above
    glRotatef(viewAngle, 0.0f, 1.0f, 0.0f); // Rotate view around the helicopter

    // Draw the helicopter
    DrawHelicopter();

    SDL_GL_SwapWindow(window);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    SDL_DestroyWindow(window);
    SDL_GL_DestroyContext(glcontext);
}