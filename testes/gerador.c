#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <direct.h>

// Função para gerar um nome aleatório no formato [8caracteres][3numeros]
void gerar_nome_aleatorio(char* buffer) {
    const char letras[] = "abcdefghijklmnopqrstuvwxyz";
    // Gera os 8 caracteres alfabéticos
    for (int i = 0; i < 8; i++) {
        buffer[i] = letras[rand() % 26];
    }
    // Gera os 3 números aleatórios e formata com zeros à esquerda se necessário
    sprintf(&buffer[8], "%03d", rand() % 1000);
}

void criar_arquivo_teste(const char* nome_arquivo, int quantidade) {
    FILE* file = fopen(nome_arquivo, "w");
    if (!file) {
        printf("Erro ao criar o arquivo %s. Certifique-se de que a pasta 'data/' existe.\n", nome_arquivo);
        return;
    }

    char usuario[12];
    for (int i = 0; i < quantidade; i++) {
        gerar_nome_aleatorio(usuario);
        fprintf(file, "%s\n", usuario);
    }

    fclose(file);
    printf("Arquivo '%s' com %d registros gerado com sucesso!\n", nome_arquivo, quantidade);
}

int main() {
    srand(time(NULL)); // Semente para geração aleatória

    // Garante que a pasta data/ existe (no Linux/macOS)
    #if defined(_WIN32)
        _mkdir("data");
    #else
        _mkdir("data", 0777);
    #endif

    // Cria os três cenários exigidos pelo experimento (Parte 3)
    criar_arquivo_teste("data/usuarios_1k.txt", 1000);
    criar_arquivo_teste("data/usuarios_10k.txt", 10000);
    criar_arquivo_teste("data/usuarios_100k.txt", 100000);

    return 0;
}