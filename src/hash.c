#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

/*
 * Função hash usando o algoritmo clássico DJB2.
 * Retorna um índice no intervalo [0, table_size-1].
 */
unsigned int hash_function(const char* str, int table_size) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash % table_size;
}

/*
 * Cria uma tabela hash com o tamanho especificado.
 * Retorna NULL se a alocação falhar.
 */
HashTable* create_table(int size) {
    HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
    if (!ht) {
        fprintf(stderr, "Erro: falha ao alocar HashTable.\n");
        return NULL;
    }
    ht->size = size;
    ht->count = 0;
    ht->table = (Node**)calloc(size, sizeof(Node*));
    if (!ht->table) {
        fprintf(stderr, "Erro: falha ao alocar vetor de tabela hash.\n");
        free(ht);
        return NULL;
    }
    return ht;
}

/*
 * Insere um identificador na tabela hash.
 * Retorna 1 se inserido com sucesso, 0 se já existia ou falha.
 */
int insert_hash(HashTable* ht, const char* id) {
    if (!ht) return 0;
    if (search_hash(ht, id)) return 0; // Evita duplicados

    unsigned int index = hash_function(id, ht->size);
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) {
        fprintf(stderr, "Erro: falha ao alocar nó para inserção.\n");
        return 0;
    }
    strcpy(new_node->id, id);
    // Insere no início da lista (encadeamento externo)
    new_node->next = ht->table[index];
    ht->table[index] = new_node;
    ht->count++;
    return 1;
}

/*
 * Busca um identificador na tabela hash.
 * Retorna 1 se encontrado, 0 caso contrário.
 */
int search_hash(HashTable* ht, const char* id) {
    if (!ht) return 0;
    unsigned int index = hash_function(id, ht->size);
    Node* current = ht->table[index];
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

/*
 * Libera toda a memória alocada pela tabela hash.
 */
void free_table(HashTable* ht) {
    if (!ht) return;
    for (int i = 0; i < ht->size; i++) {
        Node* current = ht->table[i];
        while (current != NULL) {
            Node* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(ht->table);
    free(ht);
}