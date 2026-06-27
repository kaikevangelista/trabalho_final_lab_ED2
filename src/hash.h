#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * TABELA HASH COM ENCADEAMENTO EXTERNO
 * ------------------------------------------------------------
 * Cada posição da tabela aponta para uma lista encadeada.
 * Quando dois elementos caem no mesmo índice (colisão),
 * eles são adicionados à mesma lista.
 * ============================================================ */

/* Tamanho inicial da tabela hash.
 * Usar número primo reduz colisões por distribuição mais uniforme.
 * Fator de carga alvo: ~0.75  →  escolhemos prime próximo de N/0.75
 * Para 100.000 elementos → ~133.333 → próximo primo: 133337 */
#define TAMANHO_HASH 133337

/* Nó da lista encadeada (para resolver colisões) */
typedef struct No {
    char *chave;        /* identificador do usuário (string) */
    struct No *proximo; /* ponteiro para o próximo nó na lista */
} No;

/* Estrutura principal da Tabela Hash */
typedef struct {
    No **tabela;            /* vetor de ponteiros para listas encadeadas */
    int tamanho;            /* número de posições na tabela */
    int quantidade;         /* total de elementos inseridos */
    long long colisoes;     /* contador de colisões ocorridas */
} TabelaHash;

/* ---- Protótipos das funções ---- */

/* Cria e inicializa a tabela hash */
TabelaHash *criarTabelaHash();

/* Libera toda a memória da tabela hash */
void destruirTabelaHash(TabelaHash *th);

/* Função hash: converte a chave em índice */
unsigned int funcaoHash(const char *chave, int tamanho);

/* Insere um usuário na tabela hash */
void inserirHash(TabelaHash *th, const char *chave);

/* Busca um usuário na tabela hash. Retorna 1 se encontrar, 0 se não */
int buscarHash(TabelaHash *th, const char *chave);

/* Exibe estatísticas da tabela hash */
void estatisticasHash(TabelaHash *th);

#endif /* HASH_H */