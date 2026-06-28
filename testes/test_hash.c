#include <stdio.h>
#include <assert.h>
#include "../src/hash.h"

int main() {
    printf("=== Executando Testes Unitarios: TABELA HASH ===\n");

    // Cria uma tabela pequena para teste unitário
    HashTable* ht = create_table(20);

    // Teste 1: Inserção bem-sucedida
    assert(insert_hash(ht, "joao123") == 1);
    assert(insert_hash(ht, "maria098") == 1);
    
    // Teste 2: Buscar elementos que foram inseridos
    assert(search_hash(ht, "joao123") == 1);
    assert(search_hash(ht, "maria098") == 1);

    // Teste 3: Buscar elemento que NÃO existe
    assert(search_hash(ht, "pedro456") == 0);

    // Teste 4: Impedir inserção duplicada
    assert(insert_hash(ht, "joao123") == 0);

    // Verifica se a contagem interna está correta (apenas 2 únicos inseridos)
    assert(ht->count == 2);

    free_table(ht);
    
    printf("Sucesso: Todos os testes unitarios da Tabela Hash passaram!\n\n");
    return 0;
}