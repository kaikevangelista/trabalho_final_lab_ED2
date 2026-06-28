#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hash.h"
#include "bloom.h"

/* Estrutura para acumular estatísticas do sistema */
typedef struct {
    int total_queries;
    int queries_avoided;
    int false_positives;
    double total_time;
} Statistics;

Statistics stats = {0, 0, 0, 0.0};

/*
 * RF04 - Inserção em lote a partir de um arquivo de texto.
 * Lê cada linha como um identificador e insere no Bloom e na Hash.
 * Retorna o número de elementos efetivamente inseridos (únicos).
 */
int inserir_em_lote(HashTable* ht, BloomFilter* bf, const char* caminho_arquivo) {
    FILE* file = fopen(caminho_arquivo, "r");
    if (!file) {
        printf("Erro: Nao foi possivel abrir o arquivo '%s'\n", caminho_arquivo);
        return 0;
    }

    char username[50];
    int inseridos = 0;
    while (fscanf(file, "%s", username) != EOF) {
        insert_bloom(bf, username);
        if (insert_hash(ht, username)) {
            inseridos++;
        }
    }
    fclose(file);
    printf("Sucesso: %d usuarios carregados em lote a partir de '%s'.\n", inseridos, caminho_arquivo);
    return inseridos;
}

/*
 * Função auxiliar que conta o número de linhas em um arquivo de texto.
 * Retorna -1 em caso de erro.
 */
static int contar_linhas(const char* caminho) {
    FILE* file = fopen(caminho, "r");
    if (!file) return -1;
    int linhas = 0;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file)) {
        linhas++;
    }
    fclose(file);
    return linhas;
}

/*
 * Executa o experimento automatizado para um determinado arquivo.
 * Mede o tempo de consulta sem Bloom, com Bloom e a taxa de falsos positivos.
 */
void executar_experimento(const char* caminho_arquivo) {
    printf("\n--- Executando Experimento para: %s ---\n", caminho_arquivo);

    // Conta quantas linhas (registros) o arquivo possui
    int total_registros = contar_linhas(caminho_arquivo);
    if (total_registros <= 0) {
        printf("Erro: arquivo vazio ou inexistente: %s\n", caminho_arquivo);
        return;
    }

    // Cria estruturas temporárias para o experimento
    HashTable* temp_ht = create_table(200000);
    if (!temp_ht) {
        fprintf(stderr, "Falha ao criar tabela hash temporária.\n");
        return;
    }
    BloomFilter* temp_bf = create_bloom(1000000, 7);
    if (!temp_bf) {
        fprintf(stderr, "Falha ao criar filtro Bloom temporário.\n");
        free_table(temp_ht);
        return;
    }

    // Carrega os dados do arquivo nas estruturas
    int inseridos = inserir_em_lote(temp_ht, temp_bf, caminho_arquivo);
    if (inseridos == 0) {
        printf("Nenhum dado inserido. Abortando experimento.\n");
        free_table(temp_ht);
        free_bloom(temp_bf);
        return;
    }

    // Agora, leia o arquivo novamente para obter todos os identificadores
    // em um vetor dinâmico com tamanho exato (total_registros).
    FILE* file = fopen(caminho_arquivo, "r");
    if (!file) {
        printf("Erro ao reabrir o arquivo.\n");
        free_table(temp_ht);
        free_bloom(temp_bf);
        return;
    }

    // Aloca vetor para armazenar todas as strings (cada uma com até 50 caracteres)
    char (*usuarios)[50] = (char(*)[50])malloc(total_registros * sizeof(char[50]));
    if (!usuarios) {
        fprintf(stderr, "Erro: falha ao alocar vetor de usuários.\n");
        fclose(file);
        free_table(temp_ht);
        free_bloom(temp_bf);
        return;
    }

    int idx = 0;
    while (idx < total_registros && fscanf(file, "%s", usuarios[idx]) != EOF) {
        idx++;
    }
    fclose(file);
    // idx agora deve ser igual a total_registros, mas usamos idx como quantidade lida
    int quant_lida = idx;

    // 1) Medir tempo SEM Bloom: consultar diretamente a Hash para todos os registros
    clock_t start_sem = clock();
    for (int i = 0; i < quant_lida; i++) {
        search_hash(temp_ht, usuarios[i]);
    }
    clock_t end_sem = clock();
    double tempo_sem = (double)(end_sem - start_sem) / CLOCKS_PER_SEC;

    // 2) Medir tempo COM Bloom e contar falsos positivos
    int falsos_positivos_exp = 0;
    char user_falso[60];

    clock_t start_com = clock();
    // 2a) Consultas para elementos existentes (Bloom + Hash)
    for (int i = 0; i < quant_lida; i++) {
        if (check_bloom(temp_bf, usuarios[i])) {
            search_hash(temp_ht, usuarios[i]);
        }
    }
    // 2b) Consultas para elementos inexistentes (gerados artificialmente)
    //     Modificamos o nome original para garantir que não exista.
    for (int i = 0; i < quant_lida; i++) {
        sprintf(user_falso, "falso_%s", usuarios[i]);
        if (check_bloom(temp_bf, user_falso)) {
            // Bloom disse que existe, mas verificamos na Hash
            if (!search_hash(temp_ht, user_falso)) {
                falsos_positivos_exp++;
            }
        }
    }
    clock_t end_com = clock();
    double tempo_com = (double)(end_com - start_com) / CLOCKS_PER_SEC;

    // Exibe resultados no formato solicitado
    printf("\nRESULTADO DO CENARIO:\n");
    printf("Quantidade | Tempo sem Bloom | Tempo com Bloom | Falsos Positivos\n");
    printf("%-10d | %-15.4f s | %-15.4f s | %d (%.2f%%)\n",
           quant_lida, tempo_sem, tempo_com, falsos_positivos_exp,
           (quant_lida > 0) ? ((double)falsos_positivos_exp / quant_lida) * 100 : 0.0);

    // Libera memória
    free(usuarios);
    free_table(temp_ht);
    free_bloom(temp_bf);
}

/*
 * Exibe o menu principal.
 */
void exibir_menu() {
    printf("\n=== SISTEMA DE VERIFICACAO DE CADASTRO ===\n");
    printf("1 - Inserir Usuario Unico\n");
    printf("2 - Consultar Usuario\n");
    printf("3 - Exibir Estatisticas do Sistema\n");
    printf("4 - Inserir Usuarios em Lote (Arquivo)\n");
    printf("5 - Rodar Experimentos Automaticos (1k, 10k, 100k)\n");
    printf("6 - Sair\n");
    printf("Escolha uma opcao: ");
}

int main() {
// Bloom(bits)	Hashes	FP
// 500.000		  3	   9,2%
// 700.000		  5	   3,5%
// 1.000.000	  5	   0,94%
// 1.000.000	  7	   0,82%
// 1.500.000	  7	   0,10% 

// m = bits; n = elementos; k = funções hash; a = fator de carga; FP = taxa de falso positivo

    // Dimensões iniciais escolhidas: 
    //   - Hash com 150.000 posições (fator de carga ~0.67 para 100k elementos): a = n/m = 100000/150000 = 0.6667
    //   - Bloom com 1.000.000 bits e 5 funções hash (falso positivo ~0.94% para 100k elementos): FP = (1 - e^(-k * n / m))^k = (1 - e^(-5 * 100000 / 1000000))^5 = 0.0094
    HashTable* ht = create_table(150000);
    if (!ht) {
        fprintf(stderr, "Falha ao criar tabela hash principal.\n");
        return 1;
    }
    BloomFilter* bf = create_bloom(1000000, 5);
    if (!bf) {
        fprintf(stderr, "Falha ao criar filtro Bloom principal.\n");
        free_table(ht);
        return 1;
    }

    int opcao;
    char username[50];
    char caminho_arq[100];

    do {
        exibir_menu();
        if (scanf("%d", &opcao) != 1) {
            printf("Entrada invalida.\n");
            break;
        }

        switch (opcao) {
            case 1: { // RF01 - Inserção unitária
                printf("Digite o identificador do usuario: ");
                scanf("%s", username);
                insert_bloom(bf, username);
                if (insert_hash(ht, username)) {
                    printf("- %s cadastrado com sucesso!\n", username);
                } else {
                    printf("- %s ja constava no sistema.\n", username);
                }
                break;
            }

            case 2: { // RF02 - Consulta com fluxo obrigatório
                printf("Digite o usuario para consulta: ");
                scanf("%s", username);

                stats.total_queries++;
                clock_t start = clock();

                int bloom_res = check_bloom(bf, username);
                int final_res = 0;

                if (bloom_res == 0) {
                    // Bloom diz "definitivamente não existe"
                    stats.queries_avoided++;
                    final_res = 0;
                } else {
                    // Bloom diz "possivelmente existe" → consulta a Hash
                    final_res = search_hash(ht, username);
                    if (final_res == 0) {
                        stats.false_positives++; // Falso positivo detectado
                    }
                }

                clock_t end = clock();
                stats.total_time += (double)(end - start) / CLOCKS_PER_SEC;

                printf("- %s\n", final_res ? "Usuario encontrado" : "Usuario inexistente");
                break;
            }

            case 3: { // RF03 - Estatísticas
                printf("\n--- ESTATISTICAS ATUAIS ---\n");
                printf("Elementos armazenados na Hash: %d\n", ht->count);
                printf("Consultas totais realizadas: %d\n", stats.total_queries);
                printf("Consultas poupadas pelo Bloom: %d\n", stats.queries_avoided);
                printf("Falsos positivos detectados: %d\n", stats.false_positives);

                double tx_fp = (stats.total_queries > 0) ? ((double)stats.false_positives / stats.total_queries) * 100 : 0.0;
                printf("Taxa percentual de falsos positivos: %.2f%%\n", tx_fp);

                double tempo_medio = (stats.total_queries > 0) ? (stats.total_time / stats.total_queries) * 1000 : 0.0;
                printf("Tempo medio de consulta: %.4f ms\n", tempo_medio);
                break;
            }

            case 4: { // RF04 - Inserção em lote
                printf("Digite o caminho/nome do arquivo (ex: data/usuarios_1k.txt): ");
                scanf("%s", caminho_arq);
                inserir_em_lote(ht, bf, caminho_arq);
                break;
            }

            case 5: { // Parte 3 - Experimentos automáticos
                executar_experimento("data/usuarios_1k.txt");
                executar_experimento("data/usuarios_10k.txt");
                executar_experimento("data/usuarios_100k.txt");
                break;
            }

            case 6:
                printf("Encerrando o sistema...\n");
                break;

            default:
                printf("Opcao invalida!\n");
        }
    } while (opcao != 6);

    free_table(ht);
    free_bloom(bf);
    return 0;
}