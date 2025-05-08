#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>
#include <math.h>
#include <corecrt_math_defines.h>
#include <stdbool.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BALL_RADIUS 20.0f
#define GROUND_Y (-WINDOW_HEIGHT / 2.0f + 50)

static SDL_Window* window = NULL;
SDL_GLContext glcontext = NULL;
Uint64 previousTime, currentTime;

// Physics constants
const float GRAVITY = 9.81f;      // Acceleration due to gravity (m/s²)
const float RESTITUTION = 0.85f;  // Coefficient of restitution (increased bounciness)
const float TIME_SCALE = 1.0f;    // Scale factor for time (increased for faster simulation)

// Ball properties
float ballX = 0.0f;               // X position of the ball center
float ballY = WINDOW_HEIGHT / 3.0f; // Y position of the ball center (starts high)
float velocityY = 0.0f;           // Vertical velocity (initially at rest)
float velocityX = 0.0f;           // Horizontal velocity (for enhancements)

// User interaction
bool isDragging = false;
bool isRunning = false;
float dragStartX, dragStartY;

void drawBall() {
    // Draw the ball as a circle
    glColor3f(1.0f, 0.5f, 0.0f); // Orange ball

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(ballX, ballY); // Center of the circle

    // Draw the circle outline
    for (int i = 0; i <= 360; i += 10) {
        float angle_rad = i * M_PI / 180.0f;
        float px = ballX + BALL_RADIUS * cosf(angle_rad);
        float py = ballY + BALL_RADIUS * sinf(angle_rad);
        glVertex2f(px, py);
    }

    glEnd();

    // If we're dragging, draw a line showing the potential velocity
    if (isDragging) {
        glColor3f(1.0f, 1.0f, 1.0f); // White line
        glBegin(GL_LINES);
        glVertex2f(ballX, ballY);
        glVertex2f(dragStartX, dragStartY);
        glEnd();
    }
}

void drawGround() {
    // Draw the ground
    glColor3f(0.3f, 0.3f, 0.3f); // Dark gray
    glBegin(GL_QUADS);
    glVertex2f(-WINDOW_WIDTH / 2.0f, GROUND_Y);
    glVertex2f(WINDOW_WIDTH / 2.0f, GROUND_Y);
    glVertex2f(WINDOW_WIDTH / 2.0f, GROUND_Y - 20.0f);
    glVertex2f(-WINDOW_WIDTH / 2.0f, GROUND_Y - 20.0f);
    glEnd();
}

void drawTrajectory() {
    // If the ball is stationary, draw the predicted trajectory
    if (!isRunning && fabsf(velocityY) < 0.1f && ballY <= GROUND_Y + BALL_RADIUS + 1.0f) {
        // Only draw if there's some horizontal velocity
        if (fabsf(velocityX) > 0.1f) {
            glColor3f(0.5f, 0.5f, 0.5f); // Gray dotted line
            glBegin(GL_POINTS);

            float simX = ballX;
            float simY = ballY;
            float simVY = velocityY;

            // Simulate the trajectory for a number of steps
            for (int i = 0; i < 100; i++) {
                float dt = 0.1f;

                // Update position using physics equations
                simX += velocityX * dt;
                simY += simVY * dt + 0.5f * GRAVITY * dt * dt;
                simVY += GRAVITY * dt;

                // Check for ground collision
                if (simY - BALL_RADIUS < GROUND_Y) {
                    simY = GROUND_Y + BALL_RADIUS;
                    simVY = -simVY * RESTITUTION;
                }

                // Draw the point
                glVertex2f(simX, simY);
            }

            glEnd();
        }
    }
}

void drawInstructions() {
    // In a real implementation, you would use SDL_ttf for text rendering
    // This is just a placeholder for where text would go
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(-WINDOW_WIDTH / 2.0f + 20, -WINDOW_HEIGHT / 2.0f + 20);
    // Instructions would be: "Click and drag to set velocity. Press R to reset."
}

void updatePhysics(float dt) {
    if (!isRunning) return;

    // Scale the time to slow down the simulation
    dt *= TIME_SCALE;

    // Update position and velocity using the equations of motion
    ballY -= velocityY * dt + 0.5f * GRAVITY * dt * dt; // Note: SDL Y is inverted
    velocityY += GRAVITY * dt;

    // Apply horizontal velocity (if any)
    ballX += velocityX * dt;

    // Check for collision with ground
    if (ballY - BALL_RADIUS < GROUND_Y) {
        // Bounce - reflect velocity with energy loss
        ballY = GROUND_Y + BALL_RADIUS; // Place exactly at ground level
        velocityY = -velocityY * RESTITUTION;

        // Apply friction to horizontal velocity
        velocityX *= 0.98f;

        // Stop the ball if it's almost stopped
        if (fabsf(velocityY) < 0.1f && fabsf(velocityX) < 0.1f) {
            velocityY = 0.0f;
            velocityX = 0.0f;
            isRunning = false;
        }
    }

    // Check for collision with walls
    if (ballX - BALL_RADIUS < -WINDOW_WIDTH / 2.0f) {
        ballX = -WINDOW_WIDTH / 2.0f + BALL_RADIUS;
        velocityX = -velocityX * RESTITUTION;
    }
    else if (ballX + BALL_RADIUS > WINDOW_WIDTH / 2.0f) {
        ballX = WINDOW_WIDTH / 2.0f - BALL_RADIUS;
        velocityX = -velocityX * RESTITUTION;
    }
}

void resetBall() {
    ballX = 0.0f;
    ballY = WINDOW_HEIGHT / 3.0f;
    velocityX = 0.0f;
    velocityY = 0.0f;
    isRunning = false;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("Ball in Gravity Field", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    glcontext = SDL_GL_CreateContext(window);
    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);  // Dark blue background

    // Set up orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-WINDOW_WIDTH / 2.0f, WINDOW_WIDTH / 2.0f,
        -WINDOW_HEIGHT / 2.0f, WINDOW_HEIGHT / 2.0f,
        -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);

    // Enable point smoothing for trajectory
    glEnable(GL_POINT_SMOOTH);
    glPointSize(2.0f);

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
        case SDLK_R:
            resetBall();
            break;
        case SDLK_SPACE:
            isRunning = !isRunning;
            break;
        case SDLK_ESCAPE:
            return SDL_APP_SUCCESS; // Exit on ESC
        }
    }
    // Handle mouse events
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event->button.button == SDL_BUTTON_LEFT) {
            // Convert mouse coordinates to our coordinate system
            float mouseX = event->button.x - WINDOW_WIDTH / 2.0f;
            float mouseY = -event->button.y + WINDOW_HEIGHT / 2.0f;

            // Check if clicked on the ball
            float dx = mouseX - ballX;
            float dy = mouseY - ballY;
            float distSq = dx * dx + dy * dy;

            if (distSq <= BALL_RADIUS * BALL_RADIUS) {
                isDragging = true;
                dragStartX = mouseX;
                dragStartY = mouseY;
                isRunning = false;
            }
        }
    }
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (event->button.button == SDL_BUTTON_LEFT && isDragging) {
            // Convert mouse coordinates to our coordinate system
            float mouseX = event->button.x - WINDOW_WIDTH / 2.0f;
            float mouseY = -event->button.y + WINDOW_HEIGHT / 2.0f;

            // Set the initial velocity based on the drag vector
            // Increased velocity factor from 0.1 to 0.3 for faster launch
            velocityX = (ballX - mouseX) * 0.3f;
            velocityY = (ballY - mouseY) * 0.3f;

            isDragging = false;
            isRunning = true;
        }
    }
    else if (event->type == SDL_EVENT_MOUSE_MOTION) {
        if (isDragging) {
            // Update drag position
            dragStartX = event->motion.x - WINDOW_WIDTH / 2.0f;
            dragStartY = -event->motion.y + WINDOW_HEIGHT / 2.0f;
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

    // Update physics
    updatePhysics(deltaTime);

    // Render the scene
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    drawGround();
    drawTrajectory();
    drawBall();
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