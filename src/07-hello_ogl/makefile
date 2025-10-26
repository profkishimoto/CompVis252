# Atualizar TARGET com nome do executavel a ser gerado na compilacao.
TARGET = main

# Atualizar SDL_DIR com o local onde SDL3 esta instalado.
# Uso da barra '\\' especifico para Windows.
SDL_DIR = d:\\dev\\compvis\\libs\\SDL3
SDL_INC_DIR = $(SDL_DIR)\\include
SDL_LIB_DIR = $(SDL_DIR)\\lib
SDL_DLL_DIR = $(SDL_DIR)\\bin
SDL_DLL_FILE = SDL3.dll

# Atualizar GLEW_DIR com o local onde GLEW esta instalado.
GLEW_DIR = d:\\dev\\compvis\\libs\\glew
GLEW_INC_DIR = $(GLEW_DIR)\\include
GLEW_LIB_DIR = $(GLEW_DIR)\\lib\\Release\\x64
GLEW_DLL_DIR = $(GLEW_DIR)\\bin\\Release\\x64
GLEW_DLL_FILE = glew32.dll

# Atualizar CGLM_DIR com o local onde GLEW esta instalado.
CGLM_DIR = d:\\dev\\compvis\\libs\\cglm
CGLM_INC_DIR = $(CGLM_DIR)\\include

CC = gcc
CFLAGS = -std=c23 -Wall -Wextra -Wpedantic -Wno-unused-result -g
LDFLAGS = -L$(SDL_LIB_DIR) -L$(GLEW_LIB_DIR)
LDLIBS = -lSDL3 -lglew32 -lopengl32
INC_DIRS = $(addprefix -I, $(SDL_INC_DIR) $(GLEW_INC_DIR) $(CGLM_INC_DIR))

# Incluir subdiretorio(s) em SUBDIR, caso exista (ex. organizacao de projeto).
SUBDIR = 
INC = $(wildcard *.h $(foreach fd, $(SUBDIR), $(fd)/*.h))
SRC = $(wildcard *.c $(foreach fd, $(SUBDIR), $(fd)/*.c))
OBJ = $(SRC:.c=.o)

.PHONY: all clean

all: $(TARGET)

# Comandos especificos para Windows (del, copy).
clean:
	del /S *.o
	del /S $(SDL_DLL_FILE)
	del /S $(GLEW_DLL_FILE)
	del /S $(TARGET).exe

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(INC_DIRS) -o $@ $^ $(LDFLAGS) $(LDLIBS)
	copy $(SDL_DLL_DIR)\\$(SDL_DLL_FILE) .\\$(SDL_DLL_FILE)
	copy $(GLEW_DLL_DIR)\\$(GLEW_DLL_FILE) .\\$(GLEW_DLL_FILE)

%.o: %.c $(INC)
	$(CC) $(CFLAGS) $(INC_DIRS) -c $< -o $@
