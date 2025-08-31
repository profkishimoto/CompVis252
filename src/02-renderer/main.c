// Copyright (c) 2025 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
void shutdown(void)
{
  SDL_Log("shutdown()");
  SDL_Quit();
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  atexit(shutdown);

  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    SDL_Log("Erro ao iniciar a SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  const char* WINDOW_TITLE = "Hello, SDL_Renderer";
  enum constants
  {
    WINDOW_WIDTH = 640,
    WINDOW_HEIGHT = 480,
    WINDOW_HEIGHT_HALF = WINDOW_HEIGHT >> 1,
    WINDOW_TITLE_MAX_LENGTH = 64,
    LINE_OFFSET = 10,
  };

  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  if (!SDL_CreateWindowAndRenderer(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, 0,
    &window, &renderer))
  {
    SDL_Log("Erro ao criar a janela e/ou renderizador: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  char windowTitle[WINDOW_TITLE_MAX_LENGTH] = { 0 };

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
          snprintf(windowTitle, WINDOW_TITLE_MAX_LENGTH,
            "%s (%.0f, %.0f)", WINDOW_TITLE, event.motion.x, event.motion.y);
          SDL_SetWindowTitle(window, windowTitle);
          break;
      }
    }

    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderLine(renderer,
      LINE_OFFSET, WINDOW_HEIGHT_HALF - LINE_OFFSET,
      WINDOW_WIDTH - LINE_OFFSET, WINDOW_HEIGHT_HALF - LINE_OFFSET);

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderLine(renderer,
      LINE_OFFSET, WINDOW_HEIGHT_HALF,
      WINDOW_WIDTH - LINE_OFFSET, WINDOW_HEIGHT_HALF);

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderLine(renderer,
      LINE_OFFSET, WINDOW_HEIGHT_HALF + LINE_OFFSET,
      WINDOW_WIDTH - LINE_OFFSET, WINDOW_HEIGHT_HALF + LINE_OFFSET);

    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  renderer = NULL;
  window = NULL;

  return 0;
}
