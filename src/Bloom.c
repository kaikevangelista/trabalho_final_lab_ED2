/* ============================================================
 * bloom.c — Implementação do Filtro de Bloom
 * Autores: Equipe do Projeto Final
 * Data: 2026
 * ============================================================ */

#include "bloom.h"

/* ------------------------------------------------------------
 * setBit() e getBit()
 * Manipulam bits individuais dentro do vetor de bytes.
 *
 * Como o vetor é de unsigned char (8 bits por posição),
 * para acessar o bit na posição P:
 *   byte  = P / 8    → qual byte do vetor
 *   deslo = P % 8    → qual bit dentro do byte
 *
 * setBit: usa OR para marcar o bit como 1
 * getBit: usa AND com máscara para ler o bit
 * ------------------------------------------------------------ */
void setBit(FiltroBoom *fb, int pos) {
    int byte  = pos / 8;
    int deslo = pos % 8;
    fb->vetor[byte] |= (1 << deslo); /* ativa o bit específico */
}

int getBit(FiltroBoom *fb, int pos) {
    int byte  = pos / 8;
    int deslo = pos % 8;
    return (fb->vetor[byte] >> deslo) & 1; /* retorna 0 ou 1 */
}

/* ------------------------------------------------------------
 * criarFiltroBloom()
 * Aloca e inicializa o filtro de bloom.
 * calloc garante que todos os bits comecem em 0.
 * ------------------------------------------------------------ */
FiltroBoom *criarFiltroBloom(int tamanho_bits, int num_hash) {
    FiltroBoom *fb = (FiltroBoom *)malloc(sizeof(FiltroBoom));
    if (!fb) {
        fprintf(stderr, "Erro: sem memória para o Filtro de Bloom.\n");
        exit(EXIT_FAILURE);
    }

    fb->tamanho_bits = tamanho_bits;
    fb->num_hash     = num_hash;
    fb->quantidade   = 0;
    fb->consultas    = 0;
    fb->evitadas     = 0;
    fb->falsos_pos   = 0;

    /* Calcula quantos bytes são necessários para guardar tamanho_bits bits.
     * Ex: 1.000.000 bits → (1000000 + 7) / 8 = 125.000 bytes (~122 KB) */
    int num_bytes = (tamanho_bits + 7) / 8;
    fb->vetor = (unsigned char *)calloc(num_bytes, sizeof(unsigned char));
    if (!fb->vetor) {
        fprintf(stderr, "Erro: sem memória para o vetor de bits.\n");
        free(fb);
        exit(EXIT_FAILURE);
    }

    return fb;
}

/* ------------------------------------------------------------
 * destruirFiltroBloom()
 * Libera a memória alocada pelo filtro.
 * ------------------------------------------------------------ */
void destruirFiltroBloom(FiltroBoom *fb) {
    if (!fb) return;
    free(fb->vetor);
    free(fb);
}

/* ============================================================
 * FUNÇÕES HASH DO FILTRO DE BLOOM
 * ------------------------------------------------------------
 * Usamos 7 funções hash independentes para maximizar a
 * distribuição dos bits e minimizar falsos positivos.
 *
 * Por que funções diferentes?
 * Se usarmos a mesma função, todos os bits marcados estarão
 * sempre no mesmo índice — sem diversidade, o filtro falha.
 *
 * As funções abaixo usam algoritmos distintos:
 *   1. djb2        — multiplicação por 33
 *   2. sdbm        — multiplicação por 65599
 *   3. fnv-1a      — XOR + multiplicação por primo FNV
 *   4. Bernstein   — variante com XOR
 *   5. rotate      — rotação de bits
 *   6. lose lose   — soma simples (Kernighan & Ritchie)
 *   7. ap hash     — alternância de operações bit a bit
 * ============================================================ */

/* Hash 1: djb2 */
unsigned int hashBloom1(const char *chave, int tamanho) {
    unsigned long h = 5381;
    int c;
    while ((c = (unsigned char)*chave++))
        h = ((h << 5) + h) + c;
    return (unsigned int)(h % tamanho);
}

/* Hash 2: sdbm */
unsigned int hashBloom2(const char *chave, int tamanho) {
    unsigned long h = 0;
    int c;
    while ((c = (unsigned char)*chave++))
        h = c + (h << 6) + (h << 16) - h;
    return (unsigned int)(h % tamanho);
}

/* Hash 3: FNV-1a (Fowler-Noll-Vo) — excelente avalanche de bits */
unsigned int hashBloom3(const char *chave, int tamanho) {
    unsigned long h = 2166136261UL; /* offset basis FNV-32 */
    int c;
    while ((c = (unsigned char)*chave++)) {
        h ^= c;
        h *= 16777619UL; /* primo FNV */
    }
    return (unsigned int)(h % tamanho);
}

/* Hash 4: Bernstein com XOR em vez de adição */
unsigned int hashBloom4(const char *chave, int tamanho) {
    unsigned long h = 5381;
    int c;
    while ((c = (unsigned char)*chave++))
        h = ((h << 5) + h) ^ c;
    return (unsigned int)(h % tamanho);
}

/* Hash 5: rotação de bits — espalha padrões repetitivos */
unsigned int hashBloom5(const char *chave, int tamanho) {
    unsigned long h = 0;
    int c;
    while ((c = (unsigned char)*chave++)) {
        /* rotaciona h 4 bits à esquerda, depois XOR com c */
        h = (h << 4) ^ (h >> 28) ^ c;
    }
    return (unsigned int)(h % tamanho);
}

/* Hash 6: lose lose (simples, mas com semente diferente) */
unsigned int hashBloom6(const char *chave, int tamanho) {
    unsigned long h = 31; /* semente para diferenciar das demais */
    int c;
    while ((c = (unsigned char)*chave++))
        h += c * 31;
    return (unsigned int)(h % tamanho);
}

/* Hash 7: AP Hash (Arash Partow) — usa alternância par/ímpar */
unsigned int hashBloom7(const char *chave, int tamanho) {
    unsigned int h = 0xAAAAAAAA; /* constante inicial */
    int i = 0;
    int c;
    while ((c = (unsigned char)*chave++)) {
        if ((i & 1) == 0) /* posição par */
            h ^= ((h << 7) ^ c * (h >> 3));
        else               /* posição ímpar */
            h ^= (~((h << 11) + (c ^ (h >> 5))));
        i++;
    }
    return (unsigned int)(h % tamanho);
}

/* ------------------------------------------------------------
 * inserirBloom()
 * Aplica as K funções hash à chave e marca os bits resultantes.
 * Cada função hash ativa uma posição diferente no vetor de bits.
 * ------------------------------------------------------------ */
void inserirBloom(FiltroBoom *fb, const char *chave) {
    /* Array de ponteiros para as 7 funções hash */
    unsigned int (*funcoesHash[7])(const char *, int) = {
        hashBloom1, hashBloom2, hashBloom3, hashBloom4,
        hashBloom5, hashBloom6, hashBloom7
    };

    /* Marca fb->num_hash bits no vetor */
    for (int i = 0; i < fb->num_hash && i < 7; i++) {
        unsigned int pos = funcoesHash[i](chave, fb->tamanho_bits);
        setBit(fb, pos);
    }

    fb->quantidade++;
}

/* ------------------------------------------------------------
 * consultarBloom()
 * Verifica se TODOS os K bits da chave estão marcados.
 *
 * Retorna:
 *   0 — "definitivamente NÃO existe" (pelo menos um bit é 0)
 *   1 — "possivelmente existe" (todos os bits são 1)
 *
 * Nota: quando retorna 0, temos certeza absoluta de ausência.
 *       quando retorna 1, pode ser falso positivo.
 * ------------------------------------------------------------ */
int consultarBloom(FiltroBoom *fb, const char *chave) {
    unsigned int (*funcoesHash[7])(const char *, int) = {
        hashBloom1, hashBloom2, hashBloom3, hashBloom4,
        hashBloom5, hashBloom6, hashBloom7
    };

    fb->consultas++;

    for (int i = 0; i < fb->num_hash && i < 7; i++) {
        unsigned int pos = funcoesHash[i](chave, fb->tamanho_bits);
        if (getBit(fb, pos) == 0) {
            /* Bit 0 encontrado: elemento definitivamente ausente */
            fb->evitadas++; /* economizamos uma consulta à hash */
            return 0;
        }
    }

    /* Todos os bits estão marcados: possivelmente existe */
    return 1;
}

/* ------------------------------------------------------------
 * estatisticasBloom()
 * Exibe informações e métricas do filtro de bloom.
 * ------------------------------------------------------------ */
void estatisticasBloom(FiltroBoom *fb) {
    /* Conta quantos bits estão marcados */
    int bits_marcados = 0;
    int num_bytes = (fb->tamanho_bits + 7) / 8;
    for (int i = 0; i < num_bytes; i++) {
        unsigned char b = fb->vetor[i];
        /* Conta bits 1 no byte usando Brian Kernighan's trick */
        while (b) { bits_marcados += (b & 1); b >>= 1; }
    }

    double taxa_bits = (double)bits_marcados / fb->tamanho_bits * 100.0;
    double taxa_fp   = (fb->consultas > 0)
                       ? (double)fb->falsos_pos / fb->consultas * 100.0
                       : 0.0;

    /* Taxa teórica de falsos positivos: (1 - e^(-k*n/m))^k */
    double kn_m = (double)fb->num_hash * fb->quantidade / fb->tamanho_bits;
    double taxa_fp_teorica = pow(1.0 - exp(-kn_m), fb->num_hash) * 100.0;

    printf("\n--- Estatisticas do Filtro de Bloom ---\n");
    printf("  Tamanho do vetor   : %d bits\n", fb->tamanho_bits);
    printf("  Memoria usada      : %d bytes (~%.1f KB)\n",
           num_bytes, num_bytes / 1024.0);
    printf("  Funcoes hash (k)   : %d\n", fb->num_hash);
    printf("  Elementos inseridos: %d\n", fb->quantidade);
    printf("  Bits marcados      : %d (%.2f%%)\n", bits_marcados, taxa_bits);
    printf("  Taxa FP teorica    : %.4f%%\n", taxa_fp_teorica);
    printf("  Consultas feitas   : %lld\n", fb->consultas);
    printf("  Consultas evitadas : %lld\n", fb->evitadas);
    printf("  Falsos positivos   : %lld (%.4f%%)\n", fb->falsos_pos, taxa_fp);
}