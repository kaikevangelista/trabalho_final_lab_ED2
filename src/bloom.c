#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bloom.h"

/*
 * Cria um filtro de Bloom com 'size_bits' bits e 'num_hashes' funções hash.
 * Retorna NULL se a alocação falhar.
 */
BloomFilter* create_bloom(int size_bits, int num_hashes) {
    BloomFilter* bf = (BloomFilter*)malloc(sizeof(BloomFilter));
    if (!bf) {
        fprintf(stderr, "Erro: falha ao alocar BloomFilter.\n");
        return NULL;
    }
    bf->size_bits = size_bits;
    bf->num_hashes = num_hashes;
    int size_bytes = (size_bits + 7) / 8; // Teto da divisão
    bf->bit_vector = (unsigned char*)calloc(size_bytes, sizeof(unsigned char));
    if (!bf->bit_vector) {
        fprintf(stderr, "Erro: falha ao alocar vetor de bits do Bloom.\n");
        free(bf);
        return NULL;
    }
    return bf;
}

/*
 * Função auxiliar que gera um hash duplo (DJB2 + SDBM) para simular múltiplas
 * funções hash independentes. O parâmetro 'i' seleciona a i-ésima função.
 */
static unsigned int bloom_hash(const char* str, int i, int max_bits) {
    unsigned long hash1 = 5381;
    unsigned long hash2 = 0;
    int c;
    while ((c = *str++)) {
        hash1 = ((hash1 << 5) + hash1) + c; // DJB2
        hash2 = c + (hash2 << 6) + (hash2 << 16) - hash2; // SDBM
    }
    return (hash1 + i * hash2) % max_bits;
}

/*
 * Insere um identificador no filtro de Bloom, ativando os bits correspondentes.
 */
void insert_bloom(BloomFilter* bf, const char* id) {
    if (!bf) return;
    for (int i = 0; i < bf->num_hashes; i++) {
        unsigned int bit_pos = bloom_hash(id, i, bf->size_bits);
        bf->bit_vector[bit_pos / 8] |= (1 << (bit_pos % 8));
    }
}

/*
 * Consulta o filtro de Bloom.
 * Retorna 1 se "possivelmente existe", 0 se "definitivamente não existe".
 */
int check_bloom(BloomFilter* bf, const char* id) {
    if (!bf) return 0;
    for (int i = 0; i < bf->num_hashes; i++) {
        unsigned int bit_pos = bloom_hash(id, i, bf->size_bits);
        if (!(bf->bit_vector[bit_pos / 8] & (1 << (bit_pos % 8)))) {
            return 0; // Bit desligado → definitivamente não presente
        }
    }
    return 1; // Todos os bits ligados → possivelmente presente
}

/*
 * Libera a memória alocada pelo filtro de Bloom.
 */
void free_bloom(BloomFilter* bf) {
    if (!bf) return;
    free(bf->bit_vector);
    free(bf);
}