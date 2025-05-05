#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define STEP_RATE_IN_MILLISECONDS  25

static SDL_Window* window = NULL;
SDL_GLContext glcontext = NULL;
Uint64 previousTime, currentTime;
float rotationAngle = 0.0f;

/* Replace gluPerspective with this helper */
void setPerspective(float fovY, float aspect, float zNear, float zFar) {
    float ymax = 1;
    float xmax = ymax * aspect;
    glFrustum(-xmax, xmax, -ymax, ymax, zNear, zFar);
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("3D OpenGL House", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetAppMetadata("3D OpenGL House", "1.0", "sites.google.com/view/kamilniedzialomski");

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
    // Update rotation angle for animation
    currentTime = SDL_GetTicks();
    if (currentTime > previousTime + STEP_RATE_IN_MILLISECONDS) {
        rotationAngle += 0.5f;
        if (rotationAngle > 360.0f) {
            rotationAngle -= 360.0f;
        }
        previousTime = currentTime;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Move the camera back to view the house properly
    glTranslatef(0.0f, -1.0f, -15.0f);  // Pull the camera back and slightly down

    // Rotate the house for viewing from different angles
    glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f);

    // Define house dimensions
    float width = 5.0f;   // X-axis
    float height = 3.0f;  // Y-axis
    float depth = 5.0f;   // Z-axis
    float halfWidth = width / 2.0f;
    float halfDepth = depth / 2.0f;

    // Wall color
    float wallColor[3] = { 0.8f, 0.6f, 0.2f };  // Brown

    // Draw the four walls using quads
    glBegin(GL_QUADS);

    // Front wall (Z+)
    glColor3fv(wallColor);
    glVertex3f(-halfWidth, 0.0f, halfDepth);       // Bottom left
    glVertex3f(halfWidth, 0.0f, halfDepth);        // Bottom right
    glVertex3f(halfWidth, height, halfDepth);      // Top right
    glVertex3f(-halfWidth, height, halfDepth);     // Top left

    // Back wall (Z-)
    glColor3fv(wallColor);
    glVertex3f(-halfWidth, 0.0f, -halfDepth);      // Bottom left
    glVertex3f(halfWidth, 0.0f, -halfDepth);       // Bottom right
    glVertex3f(halfWidth, height, -halfDepth);     // Top right
    glVertex3f(-halfWidth, height, -halfDepth);    // Top left

    // Left wall (X-)
    glColor3fv(wallColor);
    glVertex3f(-halfWidth, 0.0f, halfDepth);       // Bottom front
    glVertex3f(-halfWidth, 0.0f, -halfDepth);      // Bottom back
    glVertex3f(-halfWidth, height, -halfDepth);    // Top back
    glVertex3f(-halfWidth, height, halfDepth);     // Top front

    // Right wall (X+)
    glColor3fv(wallColor);
    glVertex3f(halfWidth, 0.0f, halfDepth);        // Bottom front
    glVertex3f(halfWidth, 0.0f, -halfDepth);       // Bottom back
    glVertex3f(halfWidth, height, -halfDepth);     // Top back
    glVertex3f(halfWidth, height, halfDepth);      // Top front

    // Floor
    glColor3f(0.5f, 0.5f, 0.5f);  // Gray for the floor
    glVertex3f(-halfWidth, 0.0f, halfDepth);       // Front left
    glVertex3f(halfWidth, 0.0f, halfDepth);        // Front right
    glVertex3f(halfWidth, 0.0f, -halfDepth);       // Back right
    glVertex3f(-halfWidth, 0.0f, -halfDepth);      // Back left

    glEnd();

    // Draw the pyramid roof using triangles
    float roofHeight = 3.0f;
    float roofPeak = height + roofHeight;
    float roofColor[3] = { 0.9f, 0.2f, 0.1f };  // Red

    glBegin(GL_TRIANGLES);

    // Front face of roof
    glColor3fv(roofColor);
    glVertex3f(-halfWidth, height, halfDepth);     // Bottom left
    glVertex3f(0.0f, roofPeak, 0.0f);              // Top middle (peak)
    glVertex3f(halfWidth, height, halfDepth);      // Bottom right

    // Back face of roof
    glColor3fv(roofColor);
    glVertex3f(-halfWidth, height, -halfDepth);    // Bottom left
    glVertex3f(0.0f, roofPeak, 0.0f);              // Top middle (peak)
    glVertex3f(halfWidth, height, -halfDepth);     // Bottom right

    // Left face of roof
    glColor3fv(roofColor);
    glVertex3f(-halfWidth, height, halfDepth);     // Bottom front
    glVertex3f(0.0f, roofPeak, 0.0f);              // Top middle (peak)
    glVertex3f(-halfWidth, height, -halfDepth);    // Bottom back

    // Right face of roof
    glColor3fv(roofColor);
    glVertex3f(halfWidth, height, halfDepth);      // Bottom front
    glVertex3f(0.0f, roofPeak, 0.0f);              // Top middle (peak)
    glVertex3f(halfWidth, height, -halfDepth);     // Bottom back

    glEnd();

    // Door (on front wall)
    glBegin(GL_QUADS);
    glColor3f(0.4f, 0.2f, 0.1f);  // Dark brown
    float doorWidth = 1.2f;
    float doorHeight = 2.0f;
    glVertex3f(-doorWidth / 2, 0.0f, halfDepth + 0.01f);        // Bottom left
    glVertex3f(doorWidth / 2, 0.0f, halfDepth + 0.01f);         // Bottom right
    glVertex3f(doorWidth / 2, doorHeight, halfDepth + 0.01f);   // Top right
    glVertex3f(-doorWidth / 2, doorHeight, halfDepth + 0.01f);  // Top left
    glEnd();

    // Windows (on front and side walls)
    glBegin(GL_QUADS);
    glColor3f(0.7f, 0.9f, 1.0f);  // Light blue
    float windowSize = 0.8f;
    float windowHeight = 1.8f;

    // Front wall windows (two)
    glVertex3f(-halfWidth + 1.0f, windowHeight, halfDepth + 0.01f);                  // Bottom left
    glVertex3f(-halfWidth + 1.0f + windowSize, windowHeight, halfDepth + 0.01f);     // Bottom right
    glVertex3f(-halfWidth + 1.0f + windowSize, windowHeight + windowSize, halfDepth + 0.01f);  // Top right
    glVertex3f(-halfWidth + 1.0f, windowHeight + windowSize, halfDepth + 0.01f);     // Top left

    glVertex3f(halfWidth - 1.0f - windowSize, windowHeight, halfDepth + 0.01f);      // Bottom left
    glVertex3f(halfWidth - 1.0f, windowHeight, halfDepth + 0.01f);                   // Bottom right
    glVertex3f(halfWidth - 1.0f, windowHeight + windowSize, halfDepth + 0.01f);      // Top right
    glVertex3f(halfWidth - 1.0f - windowSize, windowHeight + windowSize, halfDepth + 0.01f); // Top left

    // Side wall windows
    glVertex3f(-halfWidth - 0.01f, windowHeight, 0.0f);                  // Bottom left
    glVertex3f(-halfWidth - 0.01f, windowHeight, -windowSize);           // Bottom right
    glVertex3f(-halfWidth - 0.01f, windowHeight + windowSize, -windowSize); // Top right
    glVertex3f(-halfWidth - 0.01f, windowHeight + windowSize, 0.0f);     // Top left

    glVertex3f(halfWidth + 0.01f, windowHeight, 0.0f);                   // Bottom left
    glVertex3f(halfWidth + 0.01f, windowHeight, -windowSize);            // Bottom right
    glVertex3f(halfWidth + 0.01f, windowHeight + windowSize, -windowSize);  // Top right
    glVertex3f(halfWidth + 0.01f, windowHeight + windowSize, 0.0f);      // Top left
    glEnd();

    // Window frames
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);  // Black

    // Front window frames
    // First window
    glVertex3f(-halfWidth + 1.0f + windowSize / 2, windowHeight, halfDepth + 0.02f);
    glVertex3f(-halfWidth + 1.0f + windowSize / 2, windowHeight + windowSize, halfDepth + 0.02f);
    glVertex3f(-halfWidth + 1.0f, windowHeight + windowSize / 2, halfDepth + 0.02f);
    glVertex3f(-halfWidth + 1.0f + windowSize, windowHeight + windowSize / 2, halfDepth + 0.02f);

    // Second window
    glVertex3f(halfWidth - 1.0f - windowSize / 2, windowHeight, halfDepth + 0.02f);
    glVertex3f(halfWidth - 1.0f - windowSize / 2, windowHeight + windowSize, halfDepth + 0.02f);
    glVertex3f(halfWidth - 1.0f - windowSize, windowHeight + windowSize / 2, halfDepth + 0.02f);
    glVertex3f(halfWidth - 1.0f, windowHeight + windowSize / 2, halfDepth + 0.02f);

    // Side window frames
    // Left wall window
    glVertex3f(-halfWidth - 0.02f, windowHeight + windowSize / 2, 0.0f);
    glVertex3f(-halfWidth - 0.02f, windowHeight + windowSize / 2, -windowSize);
    glVertex3f(-halfWidth - 0.02f, windowHeight, -windowSize / 2);
    glVertex3f(-halfWidth - 0.02f, windowHeight + windowSize, -windowSize / 2);

    // Right wall window
    glVertex3f(halfWidth + 0.02f, windowHeight + windowSize / 2, 0.0f);
    glVertex3f(halfWidth + 0.02f, windowHeight + windowSize / 2, -windowSize);
    glVertex3f(halfWidth + 0.02f, windowHeight, -windowSize / 2);
    glVertex3f(halfWidth + 0.02f, windowHeight + windowSize, -windowSize / 2);
    glEnd();

    SDL_GL_SwapWindow(window);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    SDL_DestroyWindow(window);
    SDL_GL_DestroyContext(glcontext);
}