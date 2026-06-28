#include <stdio.h>
#include <assert.h>
#include "../src/bloom.h"

int main() {
    printf("=== Executando Testes Unitarios: FILTRO DE BLOOM ===\n");

    // Cria um filtro com 200 bits e 4 funções hash para fins de teste
    BloomFilter* bf = create_bloom(200, 4);

    // Teste 1: Elemento inserido deve retornar positivo (possivelmente existe)
    insert_bloom(bf, "joao123");
    assert(check_bloom(bf, "joao123") == 1);

    // Teste 2: Elemento limpo e longo deve retornar falso (definitivamente não existe)
    assert(check_bloom(bf, "inexistenteXYZ999") == 0);

    // Teste 3: Inserir outro elemento e testar a coexistência
    insert_bloom(bf, "maria098");
    assert(check_bloom(bf, "maria098") == 1);
    assert(check_bloom(bf, "joao123") == 1); // Garante que não sobrescreveu o anterior

    free_bloom(bf);

    printf("Sucesso: Todos os testes unitarios do Filtro de Bloom passaram!\n\n");
    return 0;
}