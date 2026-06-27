/* ============================================================
 * main.c — Sistema de Verificação de Cadastro de Usuários
 * Integra Tabela Hash + Filtro de Bloom
 * Autores: Equipe do Projeto Final
 * Data: 2026
 * ============================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "hash.h"
#include "bloom.h"

/* Tamanho máximo de um identificador de usuário */
#define MAX_ID 64

/* Tamanho máximo de um nome de arquivo */
#define MAX_ARQUIVO 256

/* ============================================================
 * FUNÇÕES AUXILIARES DE TEMPO
 * ------------------------------------------------------------
 * clock() mede o tempo de CPU em "ticks".
 * Dividindo por CLOCKS_PER_SEC obtemos segundos.
 * ============================================================ */

/* Retorna o instante atual em segundos (ponto flutuante) */
double tempo_agora() {
    return (double)clock() / CLOCKS_PER_SEC;
}

/* ============================================================
 * FUNÇÕES DE INSERÇÃO E CONSULTA COM MÉTRICAS
 * ============================================================ */

/* Estrutura para acumular métricas de desempenho */
typedef struct {
    long long total_consultas;
    long long evitadas_bloom;
    long long falsos_positivos;
    double    tempo_com_bloom;
    double    tempo_sem_bloom;
} Metricas;

/* ------------------------------------------------------------
 * inserirUsuario()
 * Insere um usuário em AMBAS as estruturas.
 * A ordem importa: inserir no bloom antes da hash.
 * ------------------------------------------------------------ */
void inserirUsuario(TabelaHash *th, FiltroBoom *fb, const char *id) {
    /* Insere no filtro de bloom */
    inserirBloom(fb, id);
    /* Insere na tabela hash */
    inserirHash(th, id);
}

/* ------------------------------------------------------------
 * consultarUsuario()
 * Fluxo completo obrigatório (RF02):
 *   1. Consulta o Bloom
 *   2. Se "definitivamente não" → retorna imediatamente
 *   3. Se "possivelmente sim" → consulta a Hash
 *   4. Verifica falso positivo
 * Retorna: 1 (existe), 0 (não existe)
 * ------------------------------------------------------------ */
int consultarUsuario(TabelaHash *th, FiltroBoom *fb,
                     const char *id, Metricas *m) {
    m->total_consultas++;

    /* Passo 1: consulta o Filtro de Bloom */
    int bloom_diz = consultarBloom(fb, id);

    if (bloom_diz == 0) {
        /* Bloom diz "definitivamente NÃO existe"
         * Economizamos a consulta à tabela hash! */
        m->evitadas_bloom++;
        return 0; /* usuário inexistente */
    }

    /* Bloom diz "possivelmente existe" → consulta a Hash */
    int hash_diz = buscarHash(th, id);

    if (hash_diz == 0) {
        /* Bloom disse SIM, mas Hash disse NÃO → FALSO POSITIVO */
        m->falsos_positivos++;
        fb->falsos_pos++; /* atualiza no filtro também */
        return 0;
    }

    return 1; /* encontrado com confirmação */
}

/* ============================================================
 * CARREGAMENTO DE ARQUIVO (RF04)
 * ============================================================ */

/* ------------------------------------------------------------
 * carregarArquivo()
 * Lê linha a linha de um arquivo texto e insere cada ID
 * nas duas estruturas.
 * Retorna o número de registros carregados.
 * ------------------------------------------------------------ */
int carregarArquivo(TabelaHash *th, FiltroBoom *fb, const char *caminho) {
    FILE *arq = fopen(caminho, "r");
    if (!arq) {
        printf("Erro: arquivo '%s' não encontrado.\n", caminho);
        return 0;
    }

    char linha[MAX_ID];
    int count = 0;

    while (fgets(linha, sizeof(linha), arq)) {
        /* Remove o '\n' do final da linha */
        linha[strcspn(linha, "\n")] = '\0';
        linha[strcspn(linha, "\r")] = '\0';

        /* Ignora linhas vazias */
        if (strlen(linha) == 0) continue;

        inserirUsuario(th, fb, linha);
        count++;
    }

    fclose(arq);
    printf("  %d registros carregados de '%s'.\n", count, caminho);
    return count;
}

/* ============================================================
 * GERAÇÃO DE DADOS PARA EXPERIMENTOS
 * ============================================================ */

/* Caracteres usados para gerar IDs aleatórios */
static const char LETRAS[] = "abcdefghijklmnopqrstuvwxyz";

/* ------------------------------------------------------------
 * gerarID()
 * Gera um identificador no formato [8letras][3dígitos].
 * Exemplo: "islaifda122"
 * ------------------------------------------------------------ */
void gerarID(char *dest) {
    for (int i = 0; i < 8; i++)
        dest[i] = LETRAS[rand() % 26];
    for (int i = 8; i < 11; i++)
        dest[i] = '0' + (rand() % 10);
    dest[11] = '\0';
}

/* ------------------------------------------------------------
 * gerarArquivo()
 * Cria um arquivo texto com 'n' IDs aleatórios no formato
 * [8letras][3dígitos], sem duplicatas garantidas por conjunto.
 * ------------------------------------------------------------ */
void gerarArquivo(const char *caminho, int n) {
    FILE *arq = fopen(caminho, "w");
    if (!arq) {
        printf("Erro ao criar arquivo '%s'.\n", caminho);
        return;
    }

    /* Usamos uma hash temporária para evitar duplicatas */
    TabelaHash *temp = criarTabelaHash();
    char id[12];
    int gerados = 0;

    while (gerados < n) {
        gerarID(id);
        /* Só adiciona se ainda não gerou este ID */
        if (!buscarHash(temp, id)) {
            inserirHash(temp, id);
            fprintf(arq, "%s\n", id);
            gerados++;
        }
    }

    fclose(arq);
    destruirTabelaHash(temp);
    printf("  Arquivo '%s' gerado com %d registros.\n", caminho, n);
}

/* ============================================================
 * EXPERIMENTOS (Parte 3)
 * ============================================================ */

/* ------------------------------------------------------------
 * executarExperimento()
 * Para um dado arquivo de usuários:
 *   - Mede tempo de busca COM bloom
 *   - Mede tempo de busca SEM bloom (direto na hash)
 *   - Coleta taxa de falsos positivos
 *
 * Estratégia: buscamos todos os elementos do arquivo
 * (que existem) + gera IDs aleatórios extras (que não existem)
 * para forçar a detecção de "não existe".
 * ------------------------------------------------------------ */
void executarExperimento(const char *arquivo, int n, const char *label) {
    printf("\n========================================\n");
    printf("  EXPERIMENTO: %s (%d registros)\n", label, n);
    printf("========================================\n");

    /* Cria estruturas */
    TabelaHash *th = criarTabelaHash();
    FiltroBoom  *fb = criarFiltroBloom(BLOOM_TAMANHO_BITS, BLOOM_NUM_HASH);
    Metricas m = {0, 0, 0, 0.0, 0.0};

    /* Carrega o arquivo nas estruturas */
    printf("\nCarregando dados...\n");
    carregarArquivo(th, fb, arquivo);

    /* Lê os IDs do arquivo para um array (para repetir buscas) */
    char **ids = (char **)malloc(n * sizeof(char *));
    int lidos = 0;

    FILE *arq = fopen(arquivo, "r");
    if (arq) {
        char linha[MAX_ID];
        while (fgets(linha, sizeof(linha), arq) && lidos < n) {
            linha[strcspn(linha, "\n")] = '\0';
            if (strlen(linha) > 0) {
                ids[lidos] = strdup(linha);
                lidos++;
            }
        }
        fclose(arq);
    }

    /* Também gera IDs "inexistentes" (metade do total de buscas) */
    int n_inexistentes = n / 2;
    char **inexistentes = (char **)malloc(n_inexistentes * sizeof(char *));
    {
        TabelaHash *temp = criarTabelaHash();
        char id[12];
        for (int i = 0; i < n_inexistentes; i++) {
            do { gerarID(id); }
            while (buscarHash(th, id) || buscarHash(temp, id));
            inexistentes[i] = strdup(id);
            inserirHash(temp, id);
        }
        destruirTabelaHash(temp);
    }

    /* ---- MEDIÇÃO SEM BLOOM (busca direta na hash) ---- */
    printf("\nMedindo tempo SEM Bloom...\n");
    double t_ini_sem = tempo_agora();

    for (int i = 0; i < lidos; i++)
        buscarHash(th, ids[i]);
    for (int i = 0; i < n_inexistentes; i++)
        buscarHash(th, inexistentes[i]);

    double t_fim_sem = tempo_agora();
    m.tempo_sem_bloom = t_fim_sem - t_ini_sem;

    /* ---- MEDIÇÃO COM BLOOM ---- */
    printf("Medindo tempo COM Bloom...\n");
    double t_ini_com = tempo_agora();

    for (int i = 0; i < lidos; i++)
        consultarUsuario(th, fb, ids[i], &m);
    for (int i = 0; i < n_inexistentes; i++)
        consultarUsuario(th, fb, inexistentes[i], &m);

    double t_fim_com = tempo_agora();
    m.tempo_com_bloom = t_fim_com - t_ini_com;

    /* ---- RESULTADOS ---- */
    int total_buscas = lidos + n_inexistentes;
    double taxa_fp = (m.total_consultas > 0)
                     ? (double)m.falsos_positivos / m.total_consultas * 100.0
                     : 0.0;

    printf("\n--- Resultados do Experimento ---\n");
    printf("  Total de buscas     : %d\n", total_buscas);
    printf("  Tempo SEM Bloom     : %.6f s\n", m.tempo_sem_bloom);
    printf("  Tempo COM Bloom     : %.6f s\n", m.tempo_com_bloom);
    printf("  Ganho de velocidade : %.2fx\n",
           (m.tempo_sem_bloom > 0)
           ? m.tempo_sem_bloom / m.tempo_com_bloom : 0.0);
    printf("  Consultas evitadas  : %lld / %lld (%.1f%%)\n",
           m.evitadas_bloom, (long long)total_buscas,
           (double)m.evitadas_bloom / total_buscas * 100.0);
    printf("  Falsos positivos    : %lld (%.4f%%)\n",
           m.falsos_positivos, taxa_fp);

    estatisticasHash(th);
    estatisticasBloom(fb);

    /* Libera memória */
    for (int i = 0; i < lidos; i++) free(ids[i]);
    for (int i = 0; i < n_inexistentes; i++) free(inexistentes[i]);
    free(ids);
    free(inexistentes);
    destruirTabelaHash(th);
    destruirFiltroBloom(fb);
}

/* ============================================================
 * MENU INTERATIVO
 * ============================================================ */

/* ------------------------------------------------------------
 * exibirEstatisticas()
 * RF03 — exibe todas as métricas do sistema.
 * ------------------------------------------------------------ */
void exibirEstatisticas(TabelaHash *th, FiltroBoom *fb, Metricas *m) {
    printf("\n========================================\n");
    printf("  ESTATÍSTICAS DO SISTEMA\n");
    printf("========================================\n");

    double taxa_fp = (m->total_consultas > 0)
                     ? (double)m->falsos_positivos / m->total_consultas * 100.0
                     : 0.0;

    printf("\n--- Métricas Gerais ---\n");
    printf("  Elementos armazenados : %d\n", th->quantidade);
    printf("  Consultas realizadas  : %lld\n", m->total_consultas);
    printf("  Consultas evitadas    : %lld\n", m->evitadas_bloom);
    printf("  Falsos positivos      : %lld (%.4f%%)\n",
           m->falsos_positivos, taxa_fp);

    estatisticasHash(th);
    estatisticasBloom(fb);
}

/* ------------------------------------------------------------
 * menuPrincipal()
 * Exibe e processa o menu interativo do sistema.
 * ------------------------------------------------------------ */
void menuPrincipal() {
    TabelaHash *th = criarTabelaHash();
    FiltroBoom  *fb = criarFiltroBloom(BLOOM_TAMANHO_BITS, BLOOM_NUM_HASH);
    Metricas     m  = {0, 0, 0, 0.0, 0.0};

    char opcao[MAX_ID];
    char id[MAX_ID];
    char arquivo[MAX_ARQUIVO];

    printf("\n================================================\n");
    printf("  SISTEMA DE VERIFICAÇÃO DE CADASTRO\n");
    printf("  Tabela Hash + Filtro de Bloom\n");
    printf("================================================\n");

    int rodando = 1;
    while (rodando) {
        printf("\n--- MENU ---\n");
        printf("  1. Inserir usuario\n");
        printf("  2. Consultar usuario\n");
        printf("  3. Carregar arquivo (lote)\n");
        printf("  4. Exibir estatisticas\n");
        printf("  5. Executar experimentos\n");
        printf("  6. Gerar arquivo de teste\n");
        printf("  0. Sair\n");
        printf("Opcao: ");

        if (!fgets(opcao, sizeof(opcao), stdin)) break;
        opcao[strcspn(opcao, "\n")] = '\0';

        if (strcmp(opcao, "1") == 0) {
            /* ---- RF01: Inserção ---- */
            printf("Digite o ID do usuario: ");
            if (fgets(id, sizeof(id), stdin)) {
                id[strcspn(id, "\n")] = '\0';
                if (strlen(id) == 0) {
                    printf("ID invalido.\n");
                } else {
                    inserirUsuario(th, fb, id);
                    printf("Usuário '%s' cadastrado com sucesso.\n", id);
                }
            }

        } else if (strcmp(opcao, "2") == 0) {
            /* ---- RF02: Consulta ---- */
            printf("Digite o ID do usuario: ");
            if (fgets(id, sizeof(id), stdin)) {
                id[strcspn(id, "\n")] = '\0';
                if (strlen(id) == 0) {
                    printf("ID invalido.\n");
                } else {
                    int resultado = consultarUsuario(th, fb, id, &m);
                    if (resultado)
                        printf("  → Usuario '%s' encontrado.\n", id);
                    else
                        printf("  → Usuario '%s' inexistente.\n", id);
                }
            }

        } else if (strcmp(opcao, "3") == 0) {
            /* ---- RF04: Carga em lote ---- */
            printf("Nome do arquivo: ");
            if (fgets(arquivo, sizeof(arquivo), stdin)) {
                arquivo[strcspn(arquivo, "\n")] = '\0';
                carregarArquivo(th, fb, arquivo);
            }

        } else if (strcmp(opcao, "4") == 0) {
            /* ---- RF03: Estatísticas ---- */
            exibirEstatisticas(th, fb, &m);

        } else if (strcmp(opcao, "5") == 0) {
            /* ---- Experimentos ---- */
            printf("\nGerando arquivos e executando experimentos...\n");

            /* Gera os arquivos de teste */
            gerarArquivo("data/usuarios_1000.txt",   1000);
            gerarArquivo("data/usuarios_10000.txt",  10000);
            gerarArquivo("data/usuarios_100000.txt", 100000);

            /* Executa os três cenários */
            executarExperimento("data/usuarios_1000.txt",   1000,   "1.000 registros");
            executarExperimento("data/usuarios_10000.txt",  10000,  "10.000 registros");
            executarExperimento("data/usuarios_100000.txt", 100000, "100.000 registros");

            printf("\n========================================\n");
            printf("  TABELA COMPARATIVA\n");
            printf("========================================\n");
            printf("  Os resultados acima mostram:\n");
            printf("  - Quantas consultas a hash foram evitadas\n");
            printf("  - Impacto do bloom no tempo de busca\n");
            printf("  - Taxa de falsos positivos (~1%% esperado)\n");

        } else if (strcmp(opcao, "6") == 0) {
            /* ---- Geração de arquivo de teste ---- */
            printf("Nome do arquivo: ");
            if (fgets(arquivo, sizeof(arquivo), stdin)) {
                arquivo[strcspn(arquivo, "\n")] = '\0';
            }
            printf("Quantidade de registros: ");
            int qtd = 0;
            scanf("%d", &qtd);
            /* consome o '\n' residual */
            while (getchar() != '\n');
            if (qtd > 0) {
                gerarArquivo(arquivo, qtd);
            } else {
                printf("Quantidade invalida.\n");
            }

        } else if (strcmp(opcao, "0") == 0) {
            rodando = 0;
            printf("Encerrando o sistema. Ate logo!\n");

        } else {
            printf("Opcao invalida. Tente novamente.\n");
        }
    }

    destruirTabelaHash(th);
    destruirFiltroBloom(fb);
}

/* ============================================================
 * PONTO DE ENTRADA
 * ============================================================ */
int main(int argc, char *argv[]) {
    /* Inicializa o gerador de números aleatórios com semente de tempo */
    srand((unsigned int)time(NULL));

    /* Cria o diretório de dados se não existir */
    system("mkdir -p data");

    /* Se um arquivo for passado como argumento, carrega direto */
    if (argc == 2) {
        printf("Carregando arquivo inicial: %s\n", argv[1]);
        TabelaHash *th = criarTabelaHash();
        FiltroBoom  *fb = criarFiltroBloom(BLOOM_TAMANHO_BITS, BLOOM_NUM_HASH);
        carregarArquivo(th, fb, argv[1]);
        estatisticasHash(th);
        estatisticasBloom(fb);
        destruirTabelaHash(th);
        destruirFiltroBloom(fb);
        return 0;
    }

    /* Menu interativo principal */
    menuPrincipal();

    return 0;
}