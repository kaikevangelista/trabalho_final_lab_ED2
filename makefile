# ============================================================
# Makefile — Sistema de Verificação de Cadastro
# ============================================================
# Uso:
#   make          — compila o projeto
#   make run      — compila e executa
#   make clean    — remove os arquivos gerados
#   make gerar    — gera os arquivos de dados de teste

CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=gnu99
LDFLAGS = -lm

# Arquivos fontes e objeto
SRCS    = src/main.c src/hash.c src/bloom.c
OBJS    = $(SRCS:.c=.o)
TARGET  = sistema_cadastro

# Regra padrão: compila tudo
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Compilado com sucesso: ./$(TARGET)"

# Compila cada .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Executa o programa
run: all
	./$(TARGET)

# Remove binários e objetos
clean:
	rm -f $(OBJS) $(TARGET)
	@echo "Limpeza concluída."

# Remove também os dados gerados
cleanall: clean
	rm -f data/*.txt

.PHONY: all run clean cleanall