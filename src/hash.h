#ifndef HASH_H
#define HASH_H

// Estrutura do nó da lista encadeada utilizada para tratar colisões
// por encadeamento externo. Cada posição da tabela pode armazenar
// uma lista de elementos que possuem o mesmo índice hash.
typedef struct Node {
    char id[12];       // Identificador do usuário ([8 caracteres][3 números] + '\0')
    struct Node* next; // Ponteiro para o próximo nó da lista
} Node;

// Estrutura da Tabela Hash
typedef struct {
    Node** table;  // Vetor de ponteiros para listas encadeadas (baldes)
    int size;      // Quantidade de posições da tabela hash
    int count;     // Quantidade total de elementos armazenados
} HashTable;

HashTable* create_table(int size);
unsigned int hash_function(const char* str, int table_size);
int insert_hash(HashTable* ht, const char* id);
int search_hash(HashTable* ht, const char* id);
void free_table(HashTable* ht);

#endif