#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <stdlib.h>
#include <time.h>

PSP_MODULE_INFO("SpaceEscape", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

#define MAX_ENEMIES 10

typedef struct
{
    int x;
    int y;
    int speed;
    int active;
} Enemy;

Enemy enemies[MAX_ENEMIES];

int playerX = 220;
int playerY = 220;

int score = 0;
int gameOver = 0;

// Çizim fonksiyonu
void drawRect(int x, int y, int w, int h, unsigned int color)
{
    unsigned int* vram = (unsigned int*)0x44000000;

    for (int yy = y; yy < y + h; yy++)
    {
        for (int xx = x; xx < x + w; xx++)
        {
            if (xx >= 0 && xx < SCREEN_WIDTH && yy >= 0 && yy < SCREEN_HEIGHT)
            {
                vram[xx + yy * 512] = color;
            }
        }
    }
}

void initEnemies()
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].x = rand() % 460;
        enemies[i].y = -(rand() % 300);
        enemies[i].speed = 2 + rand() % 4;
        enemies[i].active = 1;
    }
}

void updateEnemies()
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            enemies[i].y += enemies[i].speed;

            if (enemies[i].y > SCREEN_HEIGHT)
            {
                enemies[i].y = -20;
                enemies[i].x = rand() % 460;
                enemies[i].speed = 2 + rand() % 5;

                score++;
            }
        }
    }
}

int checkCollision(int x1, int y1, int w1, int h1,
                   int x2, int y2, int w2, int h2)
{
    if (x1 < x2 + w2 &&
        x1 + w1 > x2 &&
        y1 < y2 + h2 &&
        y1 + h1 > y2)
    {
        return 1;
    }

    return 0;
}

void checkGameOver()
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (checkCollision(playerX, playerY, 20, 20,
                           enemies[i].x, enemies[i].y, 20, 20))
        {
            gameOver = 1;
        }
    }
}

void drawPlayer()
{
    // Oyuncu gemisi
    drawRect(playerX, playerY, 20, 20, 0x0000FF00);
}

void drawEnemies()
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            drawRect(enemies[i].x, enemies[i].y, 20, 20, 0x00FF0000);
        }
    }
}

void clearScreen()
{
    drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x00000000);
}

int exit_callback(int arg1, int arg2, void *common)
{
    sceKernelExitGame();
    return 0;
}

int CallbackThread(SceSize args, void *argp)
{
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

int SetupCallbacks(void)
{
    int thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);

    if(thid >= 0)
    {
        sceKernelStartThread(thid, 0, 0);
    }

    return thid;
}

int main()
{
    SetupCallbacks();

    pspDebugScreenInit();

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    srand(time(NULL));

    initEnemies();

    while (1)
    {
        SceCtrlData pad;
        sceCtrlReadBufferPositive(&pad, 1);

        if (!gameOver)
        {
            // Hareket
            if (pad.Buttons & PSP_CTRL_LEFT)
                playerX -= 4;

            if (pad.Buttons & PSP_CTRL_RIGHT)
                playerX += 4;

            if (pad.Buttons & PSP_CTRL_UP)
                playerY -= 4;

            if (pad.Buttons & PSP_CTRL_DOWN)
                playerY += 4;

            // Analog destek
            playerX += (pad.Lx - 128) / 32;
            playerY += (pad.Ly - 128) / 32;

            // Ekran sınırı
            if (playerX < 0) playerX = 0;
            if (playerX > 460) playerX = 460;
            if (playerY < 0) playerY = 0;
            if (playerY > 252) playerY = 252;

            updateEnemies();
            checkGameOver();

            clearScreen();

            drawPlayer();
            drawEnemies();

            pspDebugScreenSetXY(0, 0);
            pspDebugScreenPrintf("SPACE ESCAPE\n");
            pspDebugScreenPrintf("Skor: %d\n", score);
            pspDebugScreenPrintf("Kacmak icin hareket et!\n");
        }
        else
        {
            clearScreen();

            pspDebugScreenSetXY(20, 10);
            pspDebugScreenPrintf("=== GAME OVER ===\n\n");
            pspDebugScreenPrintf("Final Skor: %d\n\n", score);
            pspDebugScreenPrintf("START tusuna bas yeniden baslat\n");

            if (pad.Buttons & PSP_CTRL_START)
            {
                score = 0;
                playerX = 220;
                playerY = 220;
                gameOver = 0;
                initEnemies();
            }
        }

        sceDisplayWaitVblankStart();
    }

    return 0;
}