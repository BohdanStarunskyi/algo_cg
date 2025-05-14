#define SDL_MAIN_USE_CALLBACKS 1 
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

const float GRAVITY = 9.81f;     
const float RESTITUTION = 0.85f; 
const float TIME_SCALE = 1.0f;  

float ballX = 0.0f;              
float ballY = WINDOW_HEIGHT / 3.0f; 
float velocityY = 0.0f;         
float velocityX = 0.0f;     

bool isDragging = false;
bool isRunning = false;
float dragStartX, dragStartY;

void drawBall() {
    glColor3f(1.0f, 0.5f, 0.0f);

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(ballX, ballY);

    for (int i = 0; i <= 360; i += 10) {
        float angle_rad = i * M_PI / 180.0f;
        float px = ballX + BALL_RADIUS * cosf(angle_rad);
        float py = ballY + BALL_RADIUS * sinf(angle_rad);
        glVertex2f(px, py);
    }

    glEnd();

    if (isDragging) {
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINES);
        glVertex2f(ballX, ballY);
        glVertex2f(dragStartX, dragStartY);
        glEnd();
    }
}

void drawGround() {
    glColor3f(0.3f, 0.3f, 0.3f); 
    glBegin(GL_QUADS);
    glVertex2f(-WINDOW_WIDTH / 2.0f, GROUND_Y);
    glVertex2f(WINDOW_WIDTH / 2.0f, GROUND_Y);
    glVertex2f(WINDOW_WIDTH / 2.0f, GROUND_Y - 20.0f);
    glVertex2f(-WINDOW_WIDTH / 2.0f, GROUND_Y - 20.0f);
    glEnd();
}

void drawTrajectory() {
    if (!isRunning && fabsf(velocityY) < 0.1f && ballY <= GROUND_Y + BALL_RADIUS + 1.0f) {
        if (fabsf(velocityX) > 0.1f) {
            glColor3f(0.5f, 0.5f, 0.5f);
            glBegin(GL_POINTS);

            float simX = ballX;
            float simY = ballY;
            float simVY = velocityY;

           
            for (int i = 0; i < 100; i++) {
                float dt = 0.1f;

            
                simX += velocityX * dt;
                simY += simVY * dt + 0.5f * GRAVITY * dt * dt;
                simVY += GRAVITY * dt;

             
                if (simY - BALL_RADIUS < GROUND_Y) {
                    simY = GROUND_Y + BALL_RADIUS;
                    simVY = -simVY * RESTITUTION;
                }

               
                glVertex2f(simX, simY);
            }

            glEnd();
        }
    }
}

void drawInstructions() {
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(-WINDOW_WIDTH / 2.0f + 20, -WINDOW_HEIGHT / 2.0f + 20);
}

void updatePhysics(float dt) {
    if (!isRunning) return;

    dt *= TIME_SCALE;

    ballY -= velocityY * dt + 0.5f * GRAVITY * dt * dt;
    velocityY += GRAVITY * dt;

    ballX += velocityX * dt;

    
    if (ballY - BALL_RADIUS < GROUND_Y) {
       
        ballY = GROUND_Y + BALL_RADIUS;
        velocityY = -velocityY * RESTITUTION;

        velocityX *= 0.98f;

      
        if (fabsf(velocityY) < 0.1f && fabsf(velocityX) < 0.1f) {
            velocityY = 0.0f;
            velocityX = 0.0f;
            isRunning = false;
        }
    }

   
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
    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-WINDOW_WIDTH / 2.0f, WINDOW_WIDTH / 2.0f,
        -WINDOW_HEIGHT / 2.0f, WINDOW_HEIGHT / 2.0f,
        -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);

  
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

    if (event->type == SDL_EVENT_KEY_DOWN) {
        switch (event->key.key) {
        case SDLK_R:
            resetBall();
            break;
        case SDLK_SPACE:
            isRunning = !isRunning;
            break;
        case SDLK_ESCAPE:
            return SDL_APP_SUCCESS;
        }
    }
 
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event->button.button == SDL_BUTTON_LEFT) {
     
            float mouseX = event->button.x - WINDOW_WIDTH / 2.0f;
            float mouseY = -event->button.y + WINDOW_HEIGHT / 2.0f;

          
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
           
            float mouseX = event->button.x - WINDOW_WIDTH / 2.0f;
            float mouseY = -event->button.y + WINDOW_HEIGHT / 2.0f;

         
            velocityX = (ballX - mouseX) * 0.3f;
            velocityY = (ballY - mouseY) * 0.3f;

            isDragging = false;
            isRunning = true;
        }
    }
    else if (event->type == SDL_EVENT_MOUSE_MOTION) {
        if (isDragging) {
            dragStartX = event->motion.x - WINDOW_WIDTH / 2.0f;
            dragStartY = -event->motion.y + WINDOW_HEIGHT / 2.0f;
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
   
    currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - previousTime) / 1000.0f;
    previousTime = currentTime;

 
    updatePhysics(deltaTime);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    drawGround();
    drawTrajectory();
    drawBall();
    drawInstructions();

    SDL_GL_SwapWindow(window);

    SDL_Delay(16);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    SDL_GL_DestroyContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}