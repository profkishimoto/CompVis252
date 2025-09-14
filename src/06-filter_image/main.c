// Copyright (c) 2025 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Exemplo: 06-filter_image
// O programa carrega o arquivo de imagem indicado na constante IMAGE_FILENAME
// e exibe o conteúdo na janela ("kodim23.png" pertence ao "Kodak Image Set").
//
// Caso a imagem seja maior do que WINDOW_WIDTHxWINDOW_HEIGHT, a janela é
// redimensionada logo após a imagem ser carregada.
//
// As teclas '0' e 'R' restauram a imagem original e a exibe na janela.
// As teclas '1' a '9' aplicam um filtro de média na imagem original e exibem
// a imagem filtrada na janela (cada tecla corresponde a um tamanho diferente
// do filtro - veja o código da função loop()).
//
// Observações:
// O código não está focado em performance e filtros grandes (ex. 29x29) levam
// um certo tempo para processar toda a imagem. Para indicar que o programa
// ainda está filtrando a imagem, o cursor do mouse é alterado para um
// SDL_SYSTEM_CURSOR_WAIT e volta para o padrão após a filtragem ser concluída.
//
// Em um projeto mais realista, o código abaixo provavelmente seria refatorado.
// Alguns exemplos de refatoração do projeto:
// - Uso de headers (.h) e outros arquivos .c (ex. estruturas e operações
//   relacionadas à imagens);
// - Remoção de variáveis globais;
// - Redução de logs (ou melhor, seriam desativados na build release);
// - Arquivo de imagem seria um parâmetro do programa (argv), ao invés de ser
//   uma string constante IMAGE_FILENAME.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

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
// Custom types, structs, constants, etc.
//------------------------------------------------------------------------------
static const char *WINDOW_TITLE = "Filter image";
static const char *IMAGE_FILENAME = "kodim23.png";

enum constants
{
  DEFAULT_WINDOW_WIDTH = 640,
  DEFAULT_WINDOW_HEIGHT = 480,
};

typedef struct MyWindow MyWindow;
struct MyWindow
{
  SDL_Window *window;
  SDL_Renderer *renderer;
};

typedef struct MyImage MyImage;
struct MyImage
{
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_FRect rect;
};

//------------------------------------------------------------------------------
// Globals (argh!)
//------------------------------------------------------------------------------
static MyWindow g_window = { .window = NULL, .renderer = NULL };
static MyImage g_image = {
  .surface = NULL,
  .texture = NULL,
  .rect = { .x = 0.0f, .y = 0.0f, .w = 0.0f, .h = 0.0f }
};

static SDL_Surface *surfaceFilter = NULL;

static SDL_Cursor *defaultMouseCursor = NULL;
static SDL_Cursor *hourglassMouseCursor = NULL;

//------------------------------------------------------------------------------
// Function declaration
//------------------------------------------------------------------------------
static bool MyWindow_initialize(MyWindow *window, const char *title, int width, int height, SDL_WindowFlags window_flags);
static void MyWindow_destroy(MyWindow *window);
static void MyImage_destroy(MyImage *image);
static bool MyImage_update_texture_with_surface(MyImage* image, SDL_Renderer *renderer, SDL_Surface *surface);
static bool MyImage_restore_texture(MyImage* image, SDL_Renderer *renderer);

/**
 * Carrega a imagem indicada no parâmetro `filename` e a converte para o formato
 * RGBA32, eliminando dependência do formato original da imagem. A imagem
 * carregada é armazenada em output_image.
 * Caso ocorra algum erro no processo, a função retorna false.
 */
static bool load_rgba32(const char *filename, SDL_Renderer *renderer, MyImage *output_image);

/**
 * Aplica um filtro de média na imagem original, salva o resultado na variável
 * global surfaceFilter e atualiza o conteúdo da janela.
 */
static bool MyImage_blur(MyImage* image, SDL_Renderer *renderer, Uint32 filter_size);

static void reset_image(void);

static SDL_AppResult initialize(void);
static void shutdown(void);
static void render(void);
static void loop(void);

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
void MyImage_destroy(MyImage *image)
{
  SDL_Log(">>> MyImage_destroy()");

  if (!image)
  {
    SDL_Log("\t*** Erro: Imagem inválida (image == NULL).");
    SDL_Log("<<< MyImage_destroy()");
    return;
  }

  if (image->texture)
  {
    SDL_Log("\tDestruindo MyImage->texture...");
    SDL_DestroyTexture(image->texture);
    image->texture = NULL;
  }

  if (image->surface)
  {
    SDL_Log("\tDestruindo MyImage->surface...");
    SDL_DestroySurface(image->surface);
    image->surface = NULL;
  }

  SDL_Log("\tRedefinindo MyImage->rect...");
  image->rect.x = image->rect.y = image->rect.w = image->rect.h = 0.0f;

  SDL_Log("<<< MyImage_destroy()");
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
bool MyImage_update_texture_with_surface(MyImage* image, SDL_Renderer *renderer, SDL_Surface *surface)
{
  SDL_Log(">>> MyImage_update_texture_with_surface()");

  if (!image)
  {
    SDL_Log("\t*** Erro: Imagem inválida (image == NULL).");
    SDL_Log("<<< MyImage_update_texture_with_surface()");
    return false;
  }

  if (!renderer)
  {
    SDL_Log("\t*** Erro: Renderer inválido (renderer == NULL).");
    SDL_Log("<<< MyImage_update_texture_with_surface()");
    return false;
  }

  if (!surface)
  {
    SDL_Log("\t*** Erro: Superfície inválida (surface == NULL).");
    SDL_Log("<<< MyImage_update_texture_with_surface()");
    return false;
  }

  SDL_DestroyTexture(image->texture);

  image->texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (!image->texture)
  {
    SDL_Log("\t*** Erro ao criar textura: %s", SDL_GetError());
    SDL_Log("<<< MyImage_update_texture_with_surface()");
    return false;
  }

  SDL_Log("\tObtendo dimensões da textura...");
  SDL_GetTextureSize(image->texture, &image->rect.w, &image->rect.h);

  SDL_Log("<<< MyImage_update_texture_with_surface()");
  return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MyImage_restore_texture(MyImage* image, SDL_Renderer *renderer)
{
  SDL_Log(">>> MyImage_restore_texture()");
  
  if (!MyImage_update_texture_with_surface(image, renderer, image->surface))
  {
    SDL_Log("\t*** Erro ao restaurar a textura da imagem.");
    return false;
  }

  SDL_Log("<<< MyImage_restore_texture()");
  return true;  
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool load_rgba32(const char *filename, SDL_Renderer *renderer, MyImage *output_image)
{
  SDL_Log(">>> load_rgba32(\"%s\")", filename);

  if (!filename)
  {
    SDL_Log("\t*** Erro: Nome do arquivo inválido (filename == NULL).");
    SDL_Log("<<< load_rgba32(\"%s\")", filename);
    return false;
  }

  if (!renderer)
  {
    SDL_Log("\t*** Erro: Renderer inválido (renderer == NULL).");
    SDL_Log("<<< load_rgba32(\"%s\")", filename);
    return false;
  }

  if (!output_image)
  {
    SDL_Log("\t*** Erro: Imagem de saída inválida (output_image == NULL).");
    SDL_Log("<<< load_rgba32(\"%s\")", filename);
    return false;
  }

  MyImage_destroy(output_image);

  SDL_Log("\tCarregando imagem \"%s\" em uma superfície...", filename);
  SDL_Surface *surface = IMG_Load(filename);
  if (!surface)
  {
    SDL_Log("\t*** Erro ao carregar a imagem: %s", SDL_GetError());
    SDL_Log("<<< load_rgba32(\"%s\")", filename);
    return false;
  }

  SDL_Log("\tConvertendo superfície para formato RGBA32...");
  output_image->surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
  SDL_DestroySurface(surface);
  if (!output_image->surface)
  {
    SDL_Log("\t*** Erro ao converter superfície para formato RGBA32: %s", SDL_GetError());
    SDL_Log("<<< load_rgba32(\"%s\")", filename);
    return false;
  }

  SDL_Log("\tCriando textura a partir da superfície...");
  if (!MyImage_update_texture_with_surface(output_image, renderer, output_image->surface))
  {
    SDL_Log("\t*** Erro ao criar textura.");
    SDL_Log("<<< load_rgba32(\"%s\")", filename);
    return false;
  }

  SDL_Log("<<< load_rgba32(\"%s\")", filename);
  return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MyImage_blur(MyImage* image, SDL_Renderer *renderer, Uint32 filter_size)
{
  SDL_Log(">>> MyImage_blur(filter_size: %u)", filter_size);

  if (!image || !image->surface)
  {
    SDL_Log("\t*** Erro: Imagem inválida (image == NULL ou image->surface == NULL).");
    SDL_Log("<<< MyImage_blur(filter_size: %u)", filter_size);
    return false;
  }

  if (!renderer)
  {
    SDL_Log("\t*** Erro: Renderer inválido (renderer == NULL).");
    SDL_Log("<<< MyImage_blur(filter_size: %u)", filter_size);
    return false;
  }

  if (!surfaceFilter)
  {
    surfaceFilter = SDL_CreateSurface(g_image.surface->w, g_image.surface->h, g_image.surface->format);
    if (!surfaceFilter)
    {
      SDL_Log("*** Erro: Superfície extra (filter) inválida!");
      SDL_Log("<<< MyImage_blur(filter_size: %u)", filter_size);
      return false;
    }
  }

  SDL_Log("\tExecutando blur com filter_size: %u...", filter_size);
  SDL_SetCursor(hourglassMouseCursor);

  SDL_LockSurface(image->surface);
  SDL_LockSurface(surfaceFilter);

  const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(image->surface->format);
  Uint32 *pixels = (Uint32 *)image->surface->pixels;
  Uint32 *output = (Uint32 *)surfaceFilter->pixels;
  
  const int filterFinalSize = filter_size * filter_size;
  const int filterHalfSize = filter_size >> 1;
  const float average = 1.0f / filterFinalSize;

  SDL_Color filter[filterFinalSize] = { };
  SDL_Color filteredPixel = { .r = 0, .g = 0, .b = 0, .a = 255 };
  Uint32 r = 0;
  Uint32 g = 0;
  Uint32 b = 0;
  Uint32 filterIndex = 0;
  
  for (int row = 0; row < image->surface->h; ++row)
  {
    for (int col = 0; col < image->surface->w; ++col)
    {
      // Obtém as intensidades de cada pixel que "batem" com o filtro.
      filterIndex = 0;
      for (int rowNeighbour = -filterHalfSize; rowNeighbour <= filterHalfSize; ++rowNeighbour)
      {
        for (int colNeighbour = -filterHalfSize; colNeighbour <= filterHalfSize; ++colNeighbour)
        {
          // Casos em que parte do filtro está fora da imagem. Neste exemplo,
          // apenas zeramos a intensidade das posições fora da imagem.
          if ((row + rowNeighbour < 0) || (row + rowNeighbour >= image->surface->h)
            || (col + colNeighbour < 0) || (col + colNeighbour >= image->surface->w))
          {
            filter[filterIndex].r = filter[filterIndex].g = filter[filterIndex].b = 0;
          }
          else
          {
            SDL_GetRGB(pixels[((row + rowNeighbour) * image->surface->w + (col + colNeighbour))], format, NULL,
              &filter[filterIndex].r, &filter[filterIndex].g, &filter[filterIndex].b);
          }
          ++filterIndex;
        }
      }
    
      // Calcula a média dos pixels usados na filtragem e salva na saída.
      r = g = b = 0;
      for (int i = 0; i < filterFinalSize; ++i)
      {
        r += filter[i].r;
        g += filter[i].g;
        b += filter[i].b;
      }
      filteredPixel.r = (Uint8)(r * average);
      filteredPixel.g = (Uint8)(g * average);
      filteredPixel.b = (Uint8)(b * average);

      output[row * image->surface->w + col] = SDL_MapRGB(format, NULL, filteredPixel.r, filteredPixel.g, filteredPixel.b);
    }
  }  

  SDL_UnlockSurface(surfaceFilter);
  SDL_UnlockSurface(image->surface);

  MyImage_update_texture_with_surface(image, renderer, surfaceFilter);
  render();

  SDL_Log("\tBlur com filter_size: %u finalizado...", filter_size);
  SDL_SetCursor(defaultMouseCursor);

  SDL_Log("<<< MyImage_blur(filter_size: %u)", filter_size);
  return true;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
void reset_image(void)
{
  SDL_Log(">>> reset_image()");

  MyImage_restore_texture(&g_image, g_window.renderer);
  render();

  SDL_Log("<<< reset_image()");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SDL_AppResult initialize(void)
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
  if (!MyWindow_initialize(&g_window, WINDOW_TITLE, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, 0))
  {
    SDL_Log("\t*** Erro ao criar a janela e/ou renderizador: %s", SDL_GetError());
    SDL_Log("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  SDL_Log("<<< initialize()");
  return SDL_APP_CONTINUE;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void shutdown(void)
{
  SDL_Log(">>> shutdown()");

  SDL_Log("Destruindo cursores do mouse...");
  SDL_DestroyCursor(hourglassMouseCursor);
  SDL_DestroyCursor(defaultMouseCursor);
  defaultMouseCursor = NULL;
  hourglassMouseCursor = NULL;

  SDL_Log("Destruindo superfície extra (filter)...");
  SDL_DestroySurface(surfaceFilter);
  surfaceFilter = NULL;

  MyImage_destroy(&g_image);
  MyWindow_destroy(&g_window);

  SDL_Log("\tEncerrando SDL...");
  SDL_Quit();

  SDL_Log("<<< shutdown()");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void render(void)
{
  SDL_SetRenderDrawColor(g_window.renderer, 128, 128, 128, 255);
  SDL_RenderClear(g_window.renderer);

  SDL_RenderTexture(g_window.renderer, g_image.texture, &g_image.rect, &g_image.rect);

  SDL_RenderPresent(g_window.renderer);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void loop(void)
{
  SDL_Log(">>> loop()");

  render();

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

      case SDL_EVENT_KEY_DOWN:
        if (!event.key.repeat)
        {
          switch (event.key.key)
          {
            case SDLK_R: // fallthrough.
            case SDLK_0: reset_image(); break;
            case SDLK_1: MyImage_blur(&g_image, g_window.renderer, 3); break;
            case SDLK_2: MyImage_blur(&g_image, g_window.renderer, 5); break;
            case SDLK_3: MyImage_blur(&g_image, g_window.renderer, 7); break;
            case SDLK_4: MyImage_blur(&g_image, g_window.renderer, 11); break;
            case SDLK_5: MyImage_blur(&g_image, g_window.renderer, 15); break;
            case SDLK_6: MyImage_blur(&g_image, g_window.renderer, 29); break;
            case SDLK_7: MyImage_blur(&g_image, g_window.renderer, 41); break;
            case SDLK_8: MyImage_blur(&g_image, g_window.renderer, 73); break;
            case SDLK_9: MyImage_blur(&g_image, g_window.renderer, 101); break;
          }
        }
        break;
      }
    }

    // Breve pausa para diminuir o processamento contínuo do programa...
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

  if (!load_rgba32(IMAGE_FILENAME, g_window.renderer, &g_image))
    return SDL_APP_FAILURE;

  SDL_Log("Criando cursores do mouse...");
  defaultMouseCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
  hourglassMouseCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
  SDL_SetCursor(defaultMouseCursor);

  SDL_Log("Criando superfície extra (filter)...");
  surfaceFilter = SDL_CreateSurface(g_image.surface->w, g_image.surface->h, g_image.surface->format);

  // Altera tamanho da janela se a imagem for maior do que o tamanho padrão
  // e reposiciona no canto superior esquerdo da tela.
  int imageWidth = (int)g_image.rect.w;
  int imageHeight = (int)g_image.rect.h;
  if (imageWidth > DEFAULT_WINDOW_WIDTH || imageHeight > DEFAULT_WINDOW_HEIGHT)
  {
    // Obtém o tamanho da borda da janela. Neste exemplo, só queremos saber
    // o lado superior e o lado esquerdo, para posicionar a janela corretamente
    // (posicionar a janela na coordenada (0, 0) faria com que a borda do
    // programa ficasse fora da região da tela).
    int top = 0;
    int left = 0;
    SDL_GetWindowBordersSize(g_window.window, &top, &left, NULL, NULL);

    SDL_Log("Redefinindo dimensões da janela, de (%d, %d) para (%d, %d), e alterando a posição para (%d, %d).",
      DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, imageWidth, imageHeight, left, top);

    SDL_SetWindowSize(g_window.window, imageWidth, imageHeight);
    SDL_SetWindowPosition(g_window.window, left, top);

    SDL_SyncWindow(g_window.window);
  }

  loop();

  return 0;
}
