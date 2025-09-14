// Copyright (c) 2025 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

//------------------------------------------------------------------------------
// Constants and enums
//------------------------------------------------------------------------------
static const char *WINDOW_TITLE = "Basic Render - Primitives";

enum constants
{
  WINDOW_WIDTH = 640,
  WINDOW_HEIGHT = 480,
  WINDOW_WIDTH_HALF = WINDOW_WIDTH >> 1,
  WINDOW_HEIGHT_HALF = WINDOW_HEIGHT >> 1,
  POINT_COUNT = 128,
  COLOR_MAX = 255,
};

typedef struct MyWindow MyWindow;
struct MyWindow
{
  SDL_Window *window;
  SDL_Renderer *renderer;
};

//------------------------------------------------------------------------------
// Globals (argh!)
//------------------------------------------------------------------------------
static MyWindow g_window = { .window = NULL, .renderer = NULL };

//------------------------------------------------------------------------------
// Function declaration
//------------------------------------------------------------------------------
static bool MyWindow_initialize(MyWindow *window, const char *title, int width, int height, SDL_WindowFlags window_flags);
static void MyWindow_destroy(MyWindow *window);

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MyWindow_initialize(MyWindow *window, const char *title, int width, int height, SDL_WindowFlags window_flags)
{
  SDL_Log("\tMyWindow_initialize(%s, %d, %d)", title, width, height);

  if (!window)
  {
    SDL_Log("\t\t*** Erro: Janela/renderizador inválidos (window == NULL).");
    return false;
  }

  return SDL_CreateWindowAndRenderer(title, width, height, window_flags, &window->window, &window->renderer);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyWindow_destroy(MyWindow *window)
{
  SDL_Log(">>> MyWindow_destroy()");

  if (!window)
  {
    SDL_Log("\t*** Erro: Janela/renderizador inválidos (window == NULL).");
    SDL_Log("<<< MyWindow_destroy()");
    return;
  }

  SDL_Log("\tDestruindo MyWindow->renderer...");
  SDL_DestroyRenderer(window->renderer);
  window->renderer = NULL;

  SDL_Log("\tDestruindo MyWindow->window...");
  SDL_DestroyWindow(window->window);
  window->window = NULL;

  SDL_Log("<<< MyWindow_destroy()");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static SDL_AppResult initialize(void)
{
  SDL_Log(">>> initialize()");

  SDL_Log("\tIniciando SDL...");
  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    SDL_Log("\t*** Erro ao iniciar a SDL: %s", SDL_GetError());
    SDL_Log("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  SDL_Log("\tCriando janela e renderizador...");
  if (!MyWindow_initialize(&g_window, WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, 0))
  {
    SDL_Log("\tErro ao criar a janela e/ou renderizador: %s", SDL_GetError());
    SDL_Log("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  SDL_Log("<<< initialize()");
  return SDL_APP_CONTINUE;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static void shutdown(void)
{
  SDL_Log(">>> shutdown()");

  MyWindow_destroy(&g_window);

  SDL_Log("\tEncerrando SDL...");
  SDL_Quit();

  SDL_Log("<<< shutdown()");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static void loop(void)
{
  SDL_Log(">>> loop()");

  srand(time(NULL));

  SDL_Color color = { .r = 0, .g = 0, .b = 0, .a = COLOR_MAX };

  SDL_FRect rect = { .x = 0.0f, .y = 0.0f, .w = 0.0f, .h = 0.0f };

  SDL_FPoint points[POINT_COUNT];
  for (size_t i = 0; i < POINT_COUNT; ++i)
  {
    points[i].x = rand() % WINDOW_WIDTH;
    points[i].y = rand() % WINDOW_HEIGHT;
  }

  SDL_FPoint mouseCursor = { .x = 0.0f, .y = 0.0f };
  SDL_HideCursor();

  SDL_Event event;
  bool isRunning = true;
  while (isRunning)
  {
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
      case SDL_EVENT_QUIT:
        isRunning = false;
        break;

      case SDL_EVENT_MOUSE_MOTION:
        mouseCursor.x = event.motion.x;
        mouseCursor.y = event.motion.y;
        break;
      }
    }

    SDL_SetRenderDrawColor(g_window.renderer, 0, 0, 0, COLOR_MAX);
    SDL_RenderClear(g_window.renderer);

    color.r = rand() % COLOR_MAX;
    color.g = rand() % COLOR_MAX;
    color.b = rand() % COLOR_MAX;
    SDL_SetRenderDrawColor(g_window.renderer, color.r, color.g, color.b, color.a);

    SDL_RenderPoints(g_window.renderer, points, POINT_COUNT);

    rect.x = rand() % WINDOW_WIDTH;
    rect.y = rand() % WINDOW_HEIGHT;
    rect.w = rand() % WINDOW_WIDTH_HALF;
    rect.h = rand() % WINDOW_HEIGHT_HALF;
    SDL_RenderFillRect(g_window.renderer, &rect);

    rect.x = rand() % WINDOW_WIDTH;
    rect.y = rand() % WINDOW_HEIGHT;
    rect.w = rand() % WINDOW_WIDTH_HALF;
    rect.h = rand() % WINDOW_HEIGHT_HALF;
    SDL_RenderRect(g_window.renderer, &rect);

    SDL_SetRenderDrawColor(g_window.renderer, 128, 128, 128, COLOR_MAX);
    SDL_RenderLine(g_window.renderer, WINDOW_WIDTH_HALF, WINDOW_HEIGHT_HALF, mouseCursor.x, mouseCursor.y);

    SDL_SetRenderDrawColor(g_window.renderer, 255, 255, 255, COLOR_MAX);
    SDL_RenderPoint(g_window.renderer, WINDOW_WIDTH_HALF, WINDOW_HEIGHT_HALF);
    SDL_RenderPoint(g_window.renderer, mouseCursor.x, mouseCursor.y);

    SDL_RenderPresent(g_window.renderer);

    SDL_Delay(50);
  }

  SDL_Log("<<< loop()");
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  atexit(shutdown);

  if (initialize() == SDL_APP_FAILURE)
    return SDL_APP_FAILURE;

  loop();

  return 0;
}
