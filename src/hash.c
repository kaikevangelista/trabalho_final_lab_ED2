/* ============================================================
 * hash.c — Implementação da Tabela Hash com Encadeamento Externo
 * Autores: Equipe do Projeto Final
 * Data: 2026
 * ============================================================ */

#include "hash.h"

/* ------------------------------------------------------------
 * criarTabelaHash()
 * Aloca memória para a estrutura da tabela hash e inicializa
 * todos os ponteiros das listas encadeadas como NULL.
 * Retorna um ponteiro para a tabela criada.
 * ------------------------------------------------------------ */
TabelaHash *criarTabelaHash() {
    /* Aloca a estrutura principal */
    TabelaHash *th = (TabelaHash *)malloc(sizeof(TabelaHash));
    if (!th) {
        fprintf(stderr, "Erro: sem memoria para criar tabela hash.\n");
        exit(EXIT_FAILURE);
    }

    th->tamanho   = TAMANHO_HASH;
    th->quantidade = 0;
    th->colisoes  = 0;

    /* Aloca o vetor de listas e inicializa com NULL */
    th->tabela = (No **)calloc(th->tamanho, sizeof(No *));
    if (!th->tabela) {
        fprintf(stderr, "Erro: sem memoria para o vetor da tabela hash.\n");
        free(th);
        exit(EXIT_FAILURE);
    }

    return th;
}

/* ------------------------------------------------------------
 * destruirTabelaHash()
 * Libera toda a memória alocada: percorre cada lista encadeada
 * e libera cada nó e sua chave.
 * ------------------------------------------------------------ */
void destruirTabelaHash(TabelaHash *th) {
    if (!th) return;

    for (int i = 0; i < th->tamanho; i++) {
        No *atual = th->tabela[i];
        while (atual) {
            No *prox = atual->proximo;
            free(atual->chave);  /* libera a string copiada */
            free(atual);         /* libera o nó */
            atual = prox;
        }
    }

    free(th->tabela); /* libera o vetor de ponteiros */
    free(th);         /* libera a estrutura principal */
}

/* ------------------------------------------------------------
 * funcaoHash()
 * Converte uma string (chave) em um índice da tabela.
 *
 * Algoritmo: djb2 (Daniel J. Bernstein)
 * - Amplamente usado por boa distribuição e velocidade.
 * - hash = hash * 33 + c  (equivalente a hash*31 + hash + c)
 * - O módulo garante que o índice cabe na tabela.
 *
 * Por que djb2?
 * - Baixa taxa de colisão para strings de comprimento variado.
 * - Simples e rápido de implementar em C.
 * ------------------------------------------------------------ */
unsigned int funcaoHash(const char *chave, int tamanho) {
    unsigned long hash = 5381; /* valor inicial recomendado pelo autor */

    int c;
    /* Processa cada caractere da string */
    while ((c = (unsigned char)*chave++)) {
        /* hash * 33 + c: multiplicação por 33 espalha bem os bits */
        hash = ((hash << 5) + hash) + c;
    }

    return (unsigned int)(hash % tamanho);
}

/* ------------------------------------------------------------
 * inserirHash()
 * Insere a chave na tabela hash.
 *
 * Passos:
 * 1. Calcula o índice com funcaoHash().
 * 2. Verifica se já existe (evita duplicatas).
 * 3. Cria um novo nó e insere no início da lista (O(1)).
 * 4. Incrementa contadores.
 * ------------------------------------------------------------ */
void inserirHash(TabelaHash *th, const char *chave) {
    unsigned int indice = funcaoHash(chave, th->tamanho);

    /* Verifica duplicata percorrendo a lista naquela posição */
    No *atual = th->tabela[indice];
    while (atual) {
        if (strcmp(atual->chave, chave) == 0) {
            return; /* já existe, não insere duplicata */
        }
        atual = atual->proximo;
    }

    /* Cria novo nó */
    No *novo = (No *)malloc(sizeof(No));
    if (!novo) {
        fprintf(stderr, "Erro: sem memoria para novo no.\n");
        return;
    }

    /* Copia a chave (strdup aloca e copia) */
    novo->chave = strdup(chave);
    if (!novo->chave) {
        fprintf(stderr, "Erro: sem memoria para a chave.\n");
        free(novo);
        return;
    }

    /* Insere no início da lista encadeada desta posição */
    novo->proximo = th->tabela[indice];

    /* Conta colisão se a posição já tinha algum elemento */
    if (th->tabela[indice] != NULL) {
        th->colisoes++;
    }

    th->tabela[indice] = novo;
    th->quantidade++;
}

/* ------------------------------------------------------------
 * buscarHash()
 * Busca a chave na tabela hash.
 *
 * Retorna:
 *   1 — encontrado
 *   0 — não encontrado
 *
 * Complexidade média: O(1)
 * Pior caso (todas as chaves no mesmo índice): O(n)
 * ------------------------------------------------------------ */
int buscarHash(TabelaHash *th, const char *chave) {
    unsigned int indice = funcaoHash(chave, th->tamanho);

    No *atual = th->tabela[indice];
    while (atual) {
        if (strcmp(atual->chave, chave) == 0) {
            return 1; /* encontrado */
        }
        atual = atual->proximo;
    }

    return 0; /* não encontrado */
}

/* ------------------------------------------------------------
 * estatisticasHash()
 * Exibe informações sobre a tabela hash.
 * ------------------------------------------------------------ */
void estatisticasHash(TabelaHash *th) {
    /* Conta posições ocupadas para calcular fator de carga */
    int posicoes_ocupadas = 0;
    int maior_lista = 0;

    for (int i = 0; i < th->tamanho; i++) {
        if (th->tabela[i]) {
            posicoes_ocupadas++;
            /* mede o comprimento da lista nesta posição */
            int len = 0;
            No *n = th->tabela[i];
            while (n) { len++; n = n->proximo; }
            if (len > maior_lista) maior_lista = len;
        }
    }

    double fator_carga = (double)th->quantidade / th->tamanho;

    printf("\n--- Estatisticas da Tabela Hash ---\n");
    printf("  Tamanho da tabela  : %d posicoes\n", th->tamanho);
    printf("  Elementos inseridos: %d\n", th->quantidade);
    printf("  Fator de carga     : %.4f\n", fator_carga);
    printf("  Posicoes ocupadas  : %d\n", posicoes_ocupadas);
    printf("  Colisoes ocorridas : %lld\n", th->colisoes);
    printf("  Maior lista        : %d elementos\n", maior_lista);
}