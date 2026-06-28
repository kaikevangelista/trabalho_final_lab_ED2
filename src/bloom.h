#ifndef BLOOM_H
#define BLOOM_H

// Estrutura do Filtro de Bloom.
// Armazena um vetor de bits e os parâmetros necessários para
// realizar inserções e consultas probabilísticas.
typedef struct {
    unsigned char* bit_vector; // Vetor de bits dinâmico
    int size_bits;             // Tamanho do vetor em bits (m)
    int num_hashes;            // Quantidade de funções hash (k)
} BloomFilter;

BloomFilter* create_bloom(int size_bits, int num_hashes);
void insert_bloom(BloomFilter* bf, const char* id);
int check_bloom(BloomFilter* bf, const char* id);
void free_bloom(BloomFilter* bf);

#endif