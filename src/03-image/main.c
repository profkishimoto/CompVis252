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
#include <SDL3_image/SDL_image.h>

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

  const char* WINDOW_TITLE = "Hello, SDL_image";
  const char* IMAGE_TEST_BMP = "test.bmp";
  const char* IMAGE_TEST_JPG = "test.jpg";
  const char* IMAGE_TEST_PNG = "test.png";
  enum constants
  {
    WINDOW_WIDTH = 640,
    WINDOW_HEIGHT = 480,
    WINDOW_TITLE_MAX_LENGTH = 64,
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

  SDL_Texture *texBmp = IMG_LoadTexture(renderer, IMAGE_TEST_BMP);
  if (!texBmp)
  {
    SDL_Log("Erro ao carregar a imagem '%s': %s", IMAGE_TEST_BMP, SDL_GetError());
  }
  SDL_FRect bmpRect =
  {
    .x = 0.0f,
    .y = 0.0f,
    .w = texBmp ? texBmp->w : 0.0f,
    .h = texBmp ? texBmp->h : 0.0f,
  };

  SDL_Texture *texJpg = IMG_LoadTexture(renderer, IMAGE_TEST_JPG);
  if (!texJpg)
  {
    SDL_Log("Erro ao carregar a imagem '%s': %s", IMAGE_TEST_JPG, SDL_GetError());
  }
  SDL_FRect jpgRect =
  {
    .x = bmpRect.x + bmpRect.w,
    .y = 0.0f,
    .w = texJpg ? texJpg->w : 0.0f,
    .h = texJpg ? texJpg->h : 0.0f,
  };

  SDL_Texture *texPng = IMG_LoadTexture(renderer, IMAGE_TEST_PNG);
  if (!texPng)
  {
    SDL_Log("Erro ao carregar a imagem '%s': %s", IMAGE_TEST_PNG, SDL_GetError());
  }
  SDL_FRect pngRect = { .x = jpgRect.x + jpgRect.w, .y = 0.0f };
  SDL_GetTextureSize(texPng, &pngRect.w, &pngRect.h);

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
    SDL_RenderTexture(renderer, texBmp, NULL, &bmpRect);
    SDL_RenderTexture(renderer, texJpg, NULL, &jpgRect);
    SDL_RenderTexture(renderer, texPng, NULL, &pngRect);
    SDL_RenderPresent(renderer);
  }

  SDL_DestroyTexture(texBmp);
  texBmp = NULL;
  SDL_DestroyTexture(texJpg);
  texJpg = NULL;
  SDL_DestroyTexture(texPng);
  texPng = NULL;

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  renderer = NULL;
  window = NULL;

  return 0;
}
