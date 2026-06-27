#ifndef BLOOM_H
#define BLOOM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ============================================================
 * FILTRO DE BLOOM
 * ------------------------------------------------------------
 * Estrutura probabilística que responde "definitivamente NÃO"
 * ou "possivelmente SIM" para uma consulta de pertencimento.
 *
 * Funcionamento:
 *   - Possui um vetor de bits (todos iniciados em 0).
 *   - Inserção: aplica K funções hash e marca K bits como 1.
 *   - Consulta: se qualquer um dos K bits for 0 → não existe
 *               se todos os K bits forem 1 → possivelmente existe
 *
 * Falso positivo: todos os bits marcados, mas o elemento
 * de fato não está na estrutura real (tabela hash).
 *
 * Dimensionamento matemático:
 *   m = -(n * ln(p)) / (ln(2))²   bits necessários
 *   k = (m/n) * ln(2)             funções hash ideais
 *   onde:
 *     n = número esperado de elementos
 *     p = taxa de falso positivo desejada (ex: 0.01 = 1%)
 * ============================================================ */

/* Número esperado máximo de elementos (cenário 100.000) */
#define BLOOM_N_MAX     100000

/* Taxa de falso positivo desejada: 1% */
#define BLOOM_FP_RATE   0.01

/* Cálculo de m (tamanho do vetor de bits):
 * m = -(100000 * ln(0.01)) / (ln(2))^2
 * m = -(100000 * (-4.60517)) / 0.480453
 * m ≈ 958506 bits ≈ 958506 / 8 ≈ 119.814 bytes
 * Usamos 1.000.000 para margem de segurança. */
#define BLOOM_TAMANHO_BITS  1000000

/* Cálculo de k (número de funções hash):
 * k = (m/n) * ln(2) = (1000000/100000) * 0.693147 ≈ 6.93 → 7
 * Arredondamos para o inteiro mais próximo. */
#define BLOOM_NUM_HASH      7

/* Estrutura do Filtro de Bloom */
typedef struct {
    unsigned char *vetor;   /* vetor de bits (armazenado em bytes) */
    int tamanho_bits;       /* total de bits no vetor */
    int num_hash;           /* quantidade de funções hash usadas */
    int quantidade;         /* elementos inseridos */
    long long consultas;    /* total de consultas realizadas */
    long long evitadas;     /* consultas à hash evitadas pelo bloom */
    long long falsos_pos;   /* falsos positivos detectados */
} FiltroBoom;

/* ---- Protótipos ---- */

/* Cria e inicializa o filtro de bloom */
FiltroBoom *criarFiltroBloom(int tamanho_bits, int num_hash);

/* Libera memória do filtro */
void destruirFiltroBloom(FiltroBoom *fb);

/* Marca os bits correspondentes à chave no vetor */
void inserirBloom(FiltroBoom *fb, const char *chave);

/* Consulta se a chave pode existir. Retorna 1 (possível) ou 0 (definitivamente não) */
int consultarBloom(FiltroBoom *fb, const char *chave);

/* Define/lê um bit individual do vetor de bits */
void setBit(FiltroBoom *fb, int pos);
int  getBit(FiltroBoom *fb, int pos);

/* Funções hash diversificadas para o Bloom */
unsigned int hashBloom1(const char *chave, int tamanho);
unsigned int hashBloom2(const char *chave, int tamanho);
unsigned int hashBloom3(const char *chave, int tamanho);
unsigned int hashBloom4(const char *chave, int tamanho);
unsigned int hashBloom5(const char *chave, int tamanho);
unsigned int hashBloom6(const char *chave, int tamanho);
unsigned int hashBloom7(const char *chave, int tamanho);

/* Exibe estatísticas do filtro */
void estatisticasBloom(FiltroBoom *fb);

#endif /* BLOOM_H */