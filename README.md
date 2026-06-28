# Sistema de Consulta Eficiente com Tabela Hash e Filtro de Bloom

## Descrição

Este projeto foi desenvolvido como trabalho da disciplina de **Estruturas de Dados**, com o objetivo de implementar manualmente duas estruturas de dados complementares para armazenamento e consulta eficiente de grandes volumes de informações:

- **Tabela Hash**: responsável pelo armazenamento exato dos identificadores de usuários.
- **Filtro de Bloom**: estrutura probabilística utilizada para acelerar consultas de existência, reduzindo acessos desnecessários à Tabela Hash.

O sistema simula um ambiente de verificação de cadastro de usuários, onde milhares de consultas são realizadas continuamente. Antes de acessar a Tabela Hash, toda consulta passa pelo Filtro de Bloom, que informa se o elemento **definitivamente não existe** ou **possivelmente existe**.

Além da implementação das estruturas, o projeto realiza experimentos comparando desempenho, consumo de memória e taxa de falsos positivos para diferentes volumes de dados.

---

# Objetivos

- Implementar manualmente uma **Tabela Hash** com tratamento de colisões por encadeamento externo;
- Implementar manualmente um **Filtro de Bloom** utilizando vetor de bits;
- Comparar o desempenho da consulta utilizando apenas Hash e utilizando Bloom + Hash;
- Medir a taxa de falsos positivos do Filtro de Bloom;
- Avaliar o impacto do dimensionamento das estruturas no desempenho do sistema.

---

# Funcionalidades

O sistema disponibiliza as seguintes funcionalidades:

- **RF01 – Inserção de usuário**
  - Insere um novo identificador no Filtro de Bloom e na Tabela Hash.

- **RF02 – Consulta de usuário**
  - Consulta primeiro o Filtro de Bloom;
  - Caso o Bloom indique que o elemento não existe, evita a consulta na Hash;
  - Caso indique que possivelmente existe, realiza a busca na Tabela Hash.

- **RF03 – Estatísticas**
  - Quantidade de elementos armazenados;
  - Número total de consultas;
  - Consultas evitadas pelo Bloom;
  - Número de falsos positivos;
  - Taxa percentual de falsos positivos;
  - Tempo médio das consultas.

- **RF04 – Inserção em lote**
  - Carrega usuários a partir de arquivos texto.

- **Experimentos Automáticos**
  - Executa testes utilizando arquivos com:
    - 1.000 registros;
    - 10.000 registros;
    - 100.000 registros.

---

# Estrutura do Projeto

```text
projeto/
│
├── src/
│   ├── bloom.c
│   ├── bloom.h
│   ├── hash.c
│   ├── hash.h
│   └── main.c
│
├── data/
│   ├── usuarios_1k.txt
│   ├── usuarios_10k.txt
│   └── usuarios_100k.txt
│
├── testes/
│   ├── gerador.c
│   ├── test_hash.c
│   └── test_bloom.c
│
├── README.md
└── relatorio.pdf
```

---

# Tecnologias Utilizadas

| Componente | Tecnologia |
|------------|------------|
| Linguagem | C |
| Compilador | GCC |
| Estruturas | Tabela Hash e Filtro de Bloom |
| Biblioteca utilizada | stdio.h, stdlib.h, string.h e time.h |

---

# Implementação

## Tabela Hash

A Tabela Hash foi implementada manualmente utilizando:

- Função hash baseada no algoritmo **DJB2**;
- Tratamento de colisões por **encadeamento externo**;
- Inserção;
- Busca;
- Liberação de memória.

Para reduzir colisões foi utilizado um fator de carga aproximado de **0,67** considerando o cenário de 100.000 elementos.

---

## Filtro de Bloom

O Filtro de Bloom foi implementado manualmente utilizando:

- Vetor de bits;
- Double Hashing (DJB2 + SDBM);
- Inserção;
- Consulta probabilística.

A configuração utilizada foi:

| Parâmetro | Valor |
|-----------|-------|
| Tamanho do vetor | 1.000.000 bits |
| Funções Hash | 5 |
| Elementos esperados | 100.000 |

---

# Compilação

## Pré-requisitos

- GCC (GNU Compiler Collection)

Linux:

```bash
sudo apt install gcc
```

Windows:

- MinGW
- MSYS2
- CodeBlocks
- Dev-C++

---

## Compilar o Sistema

```bash
gcc -o sistema src/main.c src/hash.c src/bloom.c
```

---

## Compilar os Testes

Tabela Hash

```bash
gcc src/hash.c testes/test_hash.c -o testes/run_hash
```

Filtro de Bloom

```bash
gcc src/bloom.c testes/test_bloom.c -o testes/run_bloom
```

---

## Gerar Arquivos de Teste

```bash
gcc testes/gerador.c -o testes/gerador  
```

Executar:

```bash
./testes/gerador
```

Serão criados automaticamente os arquivos:

- usuarios_1k.txt
- usuarios_10k.txt
- usuarios_100k.txt

---

# Execução

Após compilar:

```bash
./sistema.exe
```

Será exibido o menu:

```text
=== SISTEMA DE VERIFICACAO DE CADASTRO ===

1 - Inserir Usuário
2 - Consultar Usuário
3 - Exibir Estatísticas
4 - Inserção em Lote
5 - Rodar Experimentos
6 - Sair
```

---

# Exemplos de Uso

## Inserção

```text
Digite o identificador:

joao123

- Usuário cadastrado com sucesso.
```

---

## Consulta

```text
Digite o usuário:

joao123

- Usuário encontrado.
```

---

```text
Digite o usuário:

usuario999

- Usuário inexistente.
```

---

## Inserção em Lote

```text
Digite o caminho do arquivo:

data/usuarios_100k.txt
```

---

## Experimentos

O sistema executa automaticamente testes utilizando três cenários:

| Registros  |
|-----------:|
| 1.000      |
| 10.000     |
| 100.000    |

Para cada cenário são medidos:

- Tempo de consulta utilizando apenas a Tabela Hash;
- Tempo utilizando Bloom + Hash;
- Número de falsos positivos;
- Taxa percentual de falsos positivos.

---

# Justificativa dos Dimensionamentos

Os parâmetros utilizados neste projeto foram definidos com base em análises teóricas e experimentais, buscando um equilíbrio entre desempenho, consumo de memória e taxa de falsos positivos.

## Tabela Hash

A Tabela Hash foi dimensionada com **150.000 posições** para armazenar aproximadamente **100.000 usuários**.

O fator de carga é dado por:

```math
α = n / m = 100000 / 150000 ≈ 0,67
```

onde:

- **n** = número esperado de elementos;
- **m** = tamanho da tabela.

Um fator de carga de aproximadamente **0,67** reduz significativamente a quantidade de colisões, mantendo as operações de inserção e busca próximas de **O(1)** na média.

---

## Filtro de Bloom

O Filtro de Bloom foi configurado utilizando:

| Parâmetro | Valor |
|-----------|------:|
| Tamanho do vetor (m) | **1.000.000 bits** |
| Número de funções hash (k) | **5** |
| Número esperado de elementos (n) | **100.000** |

A taxa teórica de falsos positivos é calculada por:

```math
FP = (1 - e^{-kn/m})^k
```

Substituindo os valores utilizados:

```math
FP = (1 - e^{-(5 × 100000 / 1000000)})^5 ≈ 0,94%
```

Essa configuração apresenta uma taxa de falsos positivos inferior a **1%**, mantendo um baixo consumo de memória (aproximadamente **122 KB** para o vetor de bits).

---

## Considerações

Os valores adotados representam um compromisso entre:

- baixo consumo de memória;
- reduzida taxa de falsos positivos;
- boa distribuição dos elementos na Tabela Hash;
- desempenho eficiente nas operações de inserção e consulta.

Durante o desenvolvimento também foram avaliadas outras configurações para o Filtro de Bloom e para a Tabela Hash. A comparação completa dos experimentos e as justificativas para a escolha final encontram-se descritas no relatório do projeto.

---

# Complexidade

## Tabela Hash

| Operação  | Complexidade Média |
|-----------|--------------------|
| Inserção  | O(1)               |
| Busca     | O(1)               |
| Pior Caso | O(n)               |

---

## Filtro de Bloom

| Operação | Complexidade |
|----------|--------------|
| Inserção | O(k)         |
| Consulta | O(k)         |

Onde **k** representa a quantidade de funções hash utilizadas.

---

# Relatório Experimental

O relatório apresenta:

- Justificativa para o dimensionamento da Tabela Hash;
- Justificativa para o dimensionamento do Filtro de Bloom;
- Comparação entre consultas com e sem Bloom;
- Taxa de falsos positivos;
- Análise do fator de carga;
- Resultados experimentais para 1.000, 10.000 e 100.000 registros.

---

# Autores

Projeto desenvolvido para a disciplina de **Algoritmo e Estruturas de Dados II**.

Equipe:

- Integrante 1 — Implementação da Tabela Hash
- Integrante 2 — Implementação do Filtro de Bloom
- Integrante 3 — Integração, experimentos e relatório