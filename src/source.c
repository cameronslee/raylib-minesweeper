#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define SQUARE_SIZE 32
#define ROWS 16
#define COLS 16
#define MAX_BOMBS 20

//Cell states
#define UNOPENED -1
#define OPENED 0
#define FLAG 69
#define OPENED_BOMB 666
#define BOMB 420

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
// 16 x 16
static const int screenWidth = 512;
static const int screenHeight = 512;

int board[ROWS][COLS];

static int framesCounter = 0;
static bool gameOver = false;
static bool pause = false;

static Vector2 offset = { 0 };

static int bombCount = 0;

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization (Note windowTitle is unused on Android)
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "minesweeper");


    InitGame();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update and Draw
        //----------------------------------------------------------------------------------
        UpdateDrawFrame();
        //----------------------------------------------------------------------------------
    }
#endif
    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadGame();         // Unload loaded data (textures, sounds, models...)

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

void dfs(int x, int y) 
{
    if(bombCount == MAX_BOMBS) return;
    if(x < 0 || x > ROWS-1 || y < 0 || y > COLS-1) return;
    if(board[x][y] == BOMB) return;

    board[x][y] = BOMB;
    bombCount++;

    int dx = rand() % (1 + 1 - (-1)) + (-1);
    int dy = rand() % (1 + 1 - (-1)) + (-1);

    dfs(x+dx,y+dy);
}

void InitGame(void)
{
    srand(time(NULL));
    framesCounter = 0;
    gameOver = false;
    pause = false;

    //init board values
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            board[i][j] = UNOPENED;
        }
    }

    while(bombCount != MAX_BOMBS) 
    {
        // randomize bomb placement (Randomized DFS)
        int xRoot = rand() % ROWS + 1;
        int yRoot = rand() % COLS + 1;

        dfs(xRoot,yRoot);
    }
}

//This will return the x index on the board of mouse
int GetMouseClickX() {
    return (int) floor(GetMousePosition().x / SQUARE_SIZE);
}

//This will return the y index on the board of mouse
int GetMouseClickY() {
    return (int) floor(GetMousePosition().y / SQUARE_SIZE);
}

void PrintBoard() 
{
    for (int i = 0; i < screenWidth / SQUARE_SIZE; i++)
    {
        for (int j = 0; j < screenHeight / SQUARE_SIZE; j++)
        {
            printf("%d ", board[j][i]);
        }
        printf("%s", "\n");
    }
}

void UpdateGame(void)
{
    if (!gameOver)
    {
        if (IsKeyPressed('P')) {
            printf("Pause toggle\n");
            pause = !pause;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            int x = GetMouseClickX();
            int y = GetMouseClickY();
            printf("MouseX: %d ", x); 
            printf("MouseY: %d\n", y);
            
            //update board with click event
            board[x][y] = OPENED;
            PrintBoard();
        }
    }
}

void DrawGame(void)
{
        BeginDrawing();

        ClearBackground(RAYWHITE);

        if (!gameOver)
        {
            // Draw grid lines
            for (int i = 0; i < screenWidth/SQUARE_SIZE + 1; i++)
            {
                DrawLineV((Vector2){SQUARE_SIZE*i + offset.x/2, offset.y/2}, (Vector2){SQUARE_SIZE*i + offset.x/2, screenHeight - offset.y/2}, LIGHTGRAY);
            }

            for (int i = 0; i < screenHeight/SQUARE_SIZE + 1; i++)
            {
                DrawLineV((Vector2){offset.x/2, SQUARE_SIZE*i + offset.y/2}, (Vector2){screenWidth - offset.x/2, SQUARE_SIZE*i + offset.y/2}, LIGHTGRAY);
            }
            
            //Draw board
            for (int i = 0; i < screenWidth/SQUARE_SIZE + 1; i++)
            {
                for (int j = 0; j < screenHeight/SQUARE_SIZE + 1; j++) 
                {
                    if (board[i][j] == OPENED) DrawRectangle(i * SQUARE_SIZE, j * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, GREEN);
                    else if (board[i][j] == BOMB) DrawRectangle(i * SQUARE_SIZE, j * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, RED);

                }
            }

            if (pause) DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 - 40, 40, GRAY);
        }
        else DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);

    EndDrawing();

}

void UnloadGame(void)
{
    //Unload all dynamic loaded data

}

void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}