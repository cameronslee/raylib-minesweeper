#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

//----------------------------------------------------------------------------------
// Defines
//----------------------------------------------------------------------------------
#define SQUARE_SIZE 32
#define ROWS 16
#define COLS 16
#define MAX_BOMBS 20

//Cell states
#define UNOPENED 0
#define OPENED 1
#define FLAG 69
#define BOMB 420

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
// 16 x 16
static const int screenWidth = 512;
static const int screenHeight = 512;

int board[ROWS][COLS]; //game board
int bombs[ROWS][COLS]; //bomb locations
int counts[ROWS][COLS]; //num surrounding mines

static int framesCounter = 0;
static bool gameOver = false;
static bool pause = false;
static bool win = false;

static Vector2 offset = { 0 };

static int bombCount = 0;
static int numOpened = 0;

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)
static bool IsWinner(); // Check win state
static void dfs(int x, int y); // Helper to randomly init mines
static int GetMouseClickX(); // Returns X index of game board
static int GetMouseClickY(); // Returns Y index of game board
static void Open(int x, int y); // Helper to mark cell as opened and trigger opening of surrounding
static void Count(int x, int y); // Helper to count surrounding mines

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

bool IsWinner()
{
    if (numOpened == (ROWS * COLS) - bombCount) return true;
    else return false;
}

void dfs(int x, int y) 
{
    if(bombCount == MAX_BOMBS) return;
    if(x < 0 || x > ROWS-1 || y < 0 || y > COLS-1) return;
    if(bombs[x][y] == BOMB) return;

    bombs[x][y] = BOMB;
    bombCount++;

    int dx = rand() % (1 + 1 - (-1)) + (-1);
    int dy = rand() % (1 + 1 - (-1)) + (-1);

    dfs(x+dx,y+dy);
}

void Count(int x, int y)
{
    if(x < 0 || x == ROWS || y < 0 || y == COLS) return; //out of range
    if(bombs[x][y] == BOMB) return;

    if(bombs[x+1][y] == BOMB) counts[x][y]++;
    if(bombs[x-1][y] == BOMB) counts[x][y]++;
    if(bombs[x][y+1] == BOMB) counts[x][y]++;
    if(bombs[x][y-1] == BOMB) counts[x][y]++;

    if(bombs[x+1][y+1] == BOMB) counts[x][y]++;
    if(bombs[x-1][y-1] == BOMB) counts[x][y]++;
    if(bombs[x-1][y+1] == BOMB) counts[x][y]++;
    if(bombs[x+1][y-1] == BOMB) counts[x][y]++;
}

void InitGame(void)
{
    srand(time(NULL));
    framesCounter = 0;
    gameOver = false;
    pause = false;
    win = false;
    bombCount = 0;
    numOpened = 0;

    //init board values
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            board[i][j] = UNOPENED;
            bombs[i][j] = UNOPENED;
            counts[i][j] = UNOPENED;
        }
    }

    //populate bomb map
    while(bombCount != MAX_BOMBS) 
    {
        // randomize bomb placement (Randomized DFS)
        int xRoot = rand() % ROWS + 1;
        int yRoot = rand() % COLS + 1;

        dfs(xRoot,yRoot);
    }

    //populate counts
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            Count(i,j);
        }
    }
}

int GetMouseClickX() {
    return (int) floor(GetMousePosition().x / SQUARE_SIZE);
}

int GetMouseClickY() {
    return (int) floor(GetMousePosition().y / SQUARE_SIZE);
}

void Open(int x, int y)
{
    if(x < 0 || x == ROWS || y < 0 || y == COLS) return; //out of range
    if (bombs[x][y] == BOMB || board[x][y] == OPENED) return;

    board[x][y] = OPENED;
    numOpened++;

    if (counts[x][y] == 0) {
        Open(x-1,y);
        Open(x+1,y);
        Open(x,y-1);
        Open(x,y+1);
    }
}

void UpdateGame(void)
{
    if (!gameOver)
    {
        if (IsWinner()) {
            gameOver = true;
            win = true;
        } 
        if (IsKeyPressed('P')) {
            printf("Pause toggle\n");
            pause = !pause;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            int x = GetMouseClickX();
            int y = GetMouseClickY();
            printf("MouseX: %d ", x); 
            printf("MouseY: %d\n", y);
            
            if (bombs[x][y] == BOMB) 
            {
                gameOver = true;
            }

            // Trigger opening of other cells
            Open(x,y);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            int x = GetMouseClickX();
            int y = GetMouseClickY();
            printf("MouseX: %d ", x); 
            printf("MouseY: %d\n", y);
            
            //update board with click event
            if (board[x][y] == UNOPENED) board[x][y] = FLAG;
            else if (board[x][y] == FLAG) board[x][y] = UNOPENED;
        }
    }
    else
    {
        if (IsKeyPressed(KEY_ENTER)) InitGame();
    }
}

void DrawGame(void)
{
        BeginDrawing();

        ClearBackground(RAYWHITE);

        if (!gameOver)
        {
            // Draw grid lines
            for (int i = 0; i < ROWS; i++)
            {
                DrawLineV((Vector2){SQUARE_SIZE*i + offset.x/2, offset.y/2}, (Vector2){SQUARE_SIZE*i + offset.x/2, screenHeight - offset.y/2}, LIGHTGRAY);
            }

            for (int i = 0; i < COLS; i++)
            {
                DrawLineV((Vector2){offset.x/2, SQUARE_SIZE*i + offset.y/2}, (Vector2){screenWidth - offset.x/2, SQUARE_SIZE*i + offset.y/2}, LIGHTGRAY);
            }
            
            //Draw board
            for (int i = 0; i < screenWidth/SQUARE_SIZE + 1; i++)
            {
                for (int j = 0; j < screenHeight/SQUARE_SIZE + 1; j++) 
                {
                    if (board[i][j] == OPENED)
                    {
                        char c = counts[i][j] + '0';
                        if (counts[i][j] == 0) c = '\0';
                        DrawRectangle(i * SQUARE_SIZE, j * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, DARKGRAY);
                        DrawText(&c, (i * SQUARE_SIZE) + (SQUARE_SIZE / 2), (j * SQUARE_SIZE) + (SQUARE_SIZE / 2), 10, WHITE);
                    }                    
                    else if (board[i][j] == FLAG) DrawRectangle(i * SQUARE_SIZE, j * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, YELLOW);
                }
            }

            if (pause) DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 - 40, 40, GRAY);
        }
        else 
        {
            if (win)
            {
                DrawText("WINNER! PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("WINNER! PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);
            }
            else
            {
                DrawText("GAME OVER! PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("GAME OVER! PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);
            }
        }

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