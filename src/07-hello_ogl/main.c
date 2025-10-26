// Copyright (c) 2025 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Exemplo: 07-hello_ogl
// O programa exibe um triângulo colorido na janela, usando SDL e OpenGL 3.3.
// O projeto usa duas bibliotecas extras para auxiliar nas operações com a
// API gráfica OpenGL:
// - GLEW: The OpenGL Extension Wrangler Library.
// - OpenGL Mathematics (glm) for C.
//
// Observação:
// - Para simplificar o código de exemplo, o programa não verifica possíveis
// erros que podem acontecer na inicialização do OpenGL e operações seguintes.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <GL/glew.h>    // GLEW: The OpenGL Extension Wrangler Library.
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cglm/cglm.h>  // OpenGL Mathematics (glm) for C.

//------------------------------------------------------------------------------
// Constants, enums and custom types.
//------------------------------------------------------------------------------
static const char *WINDOW_TITLE = "Hello, OpenGL (SDL + GLEW + CGLM)";

enum constants
{
  WINDOW_WIDTH = 640,
  WINDOW_HEIGHT = 480,
};

typedef struct MyOGLWindow MyOGLWindow;
struct MyOGLWindow
{
  SDL_Window *window;
  SDL_GLContext context;
};

//------------------------------------------------------------------------------
// Globals (argh!)
//------------------------------------------------------------------------------
static MyOGLWindow g_window = { .window = NULL, .context = NULL };
static GLuint g_shaderProgram = 0;
static GLuint g_vao = 0;
static GLuint g_vbo = 0;
static GLint g_mvp = -1;

//------------------------------------------------------------------------------
// Vertex shader code.
//------------------------------------------------------------------------------
static const char *VERTEX_SHADER_CODE =
  "#version 330 core\n"
  "layout(location = 0) in vec3 a_Pos;\n"
  "layout(location = 1) in vec3 a_Color;\n"
  "out vec3 v_FragColor;\n"
  "uniform mat4 u_MVPMatrix;\n"
  "void main() {\n"
  "  gl_Position = u_MVPMatrix * vec4(a_Pos, 1.0);\n"
  "  v_FragColor = a_Color;\n"
  "}\0";

//------------------------------------------------------------------------------
// Fragment shader code.
//------------------------------------------------------------------------------
static const char *FRAGMENT_SHADER_CODE =
  "#version 330 core\n"
  "in vec3 v_FragColor;\n"
  "out vec4 f_Color;\n"
  "void main() {\n"
  "  f_Color = vec4(v_FragColor, 1.0);\n"
  "}\0";

//------------------------------------------------------------------------------
// Function declaration
//------------------------------------------------------------------------------
static bool MyOGLWindow_initialize(MyOGLWindow *window, const char *title, int width, int height, SDL_WindowFlags window_flags);
static bool MyOGLWindow_create_context(MyOGLWindow *window);
static void MyOGLWindow_destroy(MyOGLWindow *window);

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MyOGLWindow_initialize(MyOGLWindow *window, const char *title, int width, int height, SDL_WindowFlags window_flags)
{
  SDL_Log("\tMyOGLWindow_initialize(\"%s\", %d, %d)", title, width, height);

  if (!window)
  {
    SDL_Log("\t\t*** Erro: Janela inválida (window == NULL).");
    return false;
  }

  window->window = SDL_CreateWindow(title, width, height, window_flags);
  return window->window != NULL;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MyOGLWindow_create_context(MyOGLWindow *window)
{
  SDL_Log("\tMyOGL_create_context()");

  if (!window)
  {
    SDL_Log("\t\t*** Erro: Janela inválida (window == NULL).");
    return false;
  }

  window->context = SDL_GL_CreateContext(window->window);
  return window->context != NULL;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyOGLWindow_destroy(MyOGLWindow *window)
{
  SDL_Log("\tMyOGLWindow_destroy()");

  if (!window)
  {
    SDL_Log("\t\t*** Erro: Janela inválida (window == NULL).");
    return;
  }

  SDL_Log("\t\tDestruindo MyOGLWindow->context...");
  SDL_GL_DestroyContext(window->context);
  window->context = NULL;

  SDL_Log("\t\tDestruindo MyOGLWindow->window...");
  SDL_DestroyWindow(window->window);
  window->window = NULL;
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

  SDL_Log("\tConfigurando atributos OpenGL (3.3 core)...");
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_Log("\tCriando janela...");
  if (!MyOGLWindow_initialize(&g_window, WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL))
  {
    SDL_Log("\t*** Erro ao criar a janela: %s", SDL_GetError());
    SDL_Log("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  SDL_Log("\tCriando contexto OpenGL...");
  if (!MyOGLWindow_create_context(&g_window))
  {
    SDL_Log("\t*** Erro ao criar contexto OpenGL: %s", SDL_GetError());
    SDL_Log("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  SDL_Log("\tCarregando funções OpenGL (GLEW)...");
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (err != GLEW_OK)
  {
    SDL_Log("\t*** Erro ao iniciar GLEW: %s", glewGetErrorString(err));
    SDL_Log("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  SDL_Log("\tCompilando vertex shader...");
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &VERTEX_SHADER_CODE, NULL);
  glCompileShader(vertexShader);

  SDL_Log("\tCompilando fragment shader...");
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &FRAGMENT_SHADER_CODE, NULL);
  glCompileShader(fragmentShader);

  SDL_Log("\tLinkando programa...");
  g_shaderProgram = glCreateProgram();
  glAttachShader(g_shaderProgram, vertexShader);
  glAttachShader(g_shaderProgram, fragmentShader);
  glLinkProgram(g_shaderProgram);

  SDL_Log("\tLiberando shaders...");
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  SDL_Log("\tObtendo uniform \"u_MVPMatrix\" do shader...");
  g_mvp = glGetUniformLocation(g_shaderProgram, "u_MVPMatrix");
  if (g_mvp == -1)
  {
    SDL_Log("\t*** Erro ao obter a variável uniform 'u_MVPMatrix' do vertex shader.");
    SDL_Log("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  // Dados do triângulo (X, Y, Z, R, G, B de cada vértice).
  GLfloat vertices[] = {
    -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // Esquerda, vermelho.
     0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // Direita, verde.
     0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f // Cima, azul.
  };

  SDL_Log("\tCriando e configurando Vertex Array Object (VAO) e Vertex Buffer Object (VBO)...");
  glGenVertexArrays(1, &g_vao);
  glGenBuffers(1, &g_vbo);

  glBindVertexArray(g_vao);
  {
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Configuração do atributo de posição. Do VERTEX_SHADER_CODE:
    // "layout(location = 0) in vec3 a_Pos;\n"
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);
    
    // Configuração do atributo de cor. Do VERTEX_SHADER_CODE:
    // "layout(location = 1) in vec3 a_Color;\n"
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
  
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  glBindVertexArray(0);

  SDL_Log("\tConfigurando viewport OpenGL...");
  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

  SDL_Log("<<< initialize()");
  return SDL_APP_CONTINUE;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static void shutdown(void)
{
  SDL_Log(">>> shutdown()");

  SDL_Log("\tLiberando recursos OpenGL...");
  glDeleteProgram(g_shaderProgram);
  glDeleteVertexArrays(1, &g_vao);
  glDeleteBuffers(1, &g_vbo);
  g_shaderProgram = 0;
  g_vao = 0;
  g_vbo = 0;
  g_mvp = -1;

  MyOGLWindow_destroy(&g_window);

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

  SDL_Log("\tCriando matriz do modelo...");
  mat4 modelMatrix;
  glm_mat4_identity(modelMatrix);

  SDL_Log("\tCriando matriz de visão (câmera)...");
  vec3 cameraPos = { 0.0f, 0.0f, 3.0f };
  vec3 cameraTarget = { 0.0f, 0.0f, 0.0f };
  vec3 cameraUp = { 0.0f, 1.0f, 0.0f };
  mat4 viewMatrix;
  glm_lookat(cameraPos, cameraTarget, cameraUp, viewMatrix);

  SDL_Log("\tCriando matriz de projeção perspectiva...");
  float fov = glm_rad(45.0f);
  float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
  float nearPlane = 0.1f;
  float farPlane = 100.0f;
  mat4 projectionMatrix;
  glm_perspective(fov, aspect, nearPlane, farPlane, projectionMatrix);

  SDL_Log("\tCalculando matriz MVP (Model-View-Projection)...");
  mat4 mvpMatrix;
  glm_mat4_mul(projectionMatrix, viewMatrix, mvpMatrix);
  glm_mat4_mul(mvpMatrix, modelMatrix, mvpMatrix);

  SDL_Log("\tExibindo triângulo colorido...");
  SDL_Event event;
  bool isRunning = true;
  while (isRunning)
  {
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_EVENT_QUIT)
      {
        isRunning = false;
      }
    }

    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(g_shaderProgram);
    glUniformMatrix4fv(g_mvp, 1, GL_FALSE, (const GLfloat *)mvpMatrix);
    glBindVertexArray(g_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    SDL_GL_SwapWindow(g_window.window);
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
