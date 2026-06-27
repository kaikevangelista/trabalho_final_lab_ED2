# Sistema de Verificação de Cadastro com Hash + Bloom Filter

## Descrição
Sistema em C que combina **Tabela Hash** e **Filtro de Bloom** para
consulta eficiente de grandes volumes de usuários.

---

## Estrutura do Projeto
```
projeto/
├── src/
│   ├── hash.c / hash.h    — Tabela Hash com encadeamento externo
│   ├── bloom.c / bloom.h  — Filtro de Bloom com 7 funções hash
│   └── main.c             — Menu, métricas e experimentos
├── data/
│   └── usuarios.txt       — Arquivo de entrada de exemplo
├── Makefile
└── README.md
```

---

## Compilação

### Pré-requisitos
- GCC (versão 9 ou superior)
- Make
- Biblioteca libm (matemática) — geralmente já presente no Linux/macOS

### Compilar
```
 cd "caminho do projeto"

 gcc -o sistema_cadastro src/main.c src/hash.c src/bloom.c -lm

 sistema_cadastro.exe
```

## Formato do Arquivo de Entrada

Um identificador por linha, no formato `[8letras][3dígitos]`:
```
islaifda122
djskalsa297
fjkldsaf881
```

---

## Opções do Menu

| Opção | Função                         |
|-------|--------------------------------|
| 1     | Inserir usuário manualmente    |
| 2     | Consultar existência           |
| 3     | Carregar arquivo em lote       |
| 4     | Exibir estatísticas            |
| 5     | Executar experimentos completos|
| 6     | Gerar arquivo de teste         |
| 0     | Sair                           |

---

## Exemplos de Uso

```
INSERIR joao123
CONSULTAR joao123
→ Usuário encontrado

CONSULTAR ana777
→ Usuário inexistente
```

---

## Decisões de Projeto

### Tabela Hash
- **Algoritmo:** boa distribuição, baixa taxa de colisão
- **Colisão:** Encadeamento externo (listas ligadas)
- **Tamanho:** 133.337 posições (primo próximo de 100.000 / 0,75)
- **Fator de carga alvo:** ≤ 0,75

### Filtro de Bloom
- **Tamanho do vetor:** 1.000.000 bits (~122 KB)
- **Funções hash (k):** 7
- **Taxa de falso positivo esperada:** ~1%
- **Cálculo:** `m = -(n·ln(p)) / ln(2)²`

### Por que 7 funções hash?
`k = (m/n)·ln(2) = (1000000/100000)·0,693 ≈ 6,93 → 7`

---

## Divisão da Equipe

| Integrante | Responsabilidade         | Arquivo       |
|------------|--------------------------|---------------|
| 1          | Tabela Hash              | hash.c / .h   |
| 2          | Filtro de Bloom          | bloom.c / .h  |
| 3          | Integração e experimentos| main.c        |

---

## Referências
- Bloom, B. H. (1970). *Space/time trade-offs in hash coding with allowable errors.*
- Knuth, D. E. (1998). *The Art of Computer Programming, Vol. 3.*
- Bernstein, D. J. — Algoritmo djb2.
- Fowler, G.; Noll, L. C.; Vo, P. — Algoritmo FNV-1a.