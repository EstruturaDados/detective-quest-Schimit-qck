#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // Para tolower

// Definições de tamanho
#define TAMANHO_NOME 50
#define TAMANHO_TEXTO_PISTA 100
#define TAMANHO_TABELA_HASH 10

// --- Estruturas de Dados ---

// 🌱 Nível Novato: Mapa da Mansão com Árvore Binária
typedef struct Sala {
    char nome[TAMANHO_NOME];
    struct Sala *esquerda;
    struct Sala *direita;
    // Campo para saber se a pista dessa sala já foi coletada (para Nível Aventureiro)
    char pista_na_sala[TAMANHO_TEXTO_PISTA];
} Sala;

// 🔍 Nível Aventureiro: Armazenamento de Pistas com Árvore de Busca (BST)
typedef struct Pista {
    char texto[TAMANHO_TEXTO_PISTA];
    struct Pista *esquerda;
    struct Pista *direita;
} Pista;

// 🧠 Nível Mestre: Relacionamento de Pistas com Suspeitos via Hash
// Nó da Lista Encadeada para a Tabela Hash (trata colisões e armazena suspeito)
typedef struct AssociacaoPista {
    char pista[TAMANHO_TEXTO_PISTA];
    struct AssociacaoPista *proximo;
} AssociacaoPista;

typedef struct Suspeito {
    char nome[TAMANHO_NOME];
    int contagem_pistas;
    AssociacaoPista *pistas_associadas; // Lista encadeada de pistas
    struct Suspeito *proximo;           // Ponteiro para colisões na tabela hash
} Suspeito;

// Tabela Hash (Array de ponteiros para struct Suspeito)
Suspeito *tabelaHash[TAMANHO_TABELA_HASH];

// --- Funções Auxiliares ---

// 📌 Funções para Nível Novato (Mapa da Mansão)

Sala *criarSala(const char *nome, const char *pista) {
    Sala *novaSala = (Sala *)malloc(sizeof(Sala));
    if (novaSala == NULL) {
        perror("Erro ao alocar Sala");
        exit(EXIT_FAILURE);
    }
    strncpy(novaSala->nome, nome, TAMANHO_NOME - 1);
    novaSala->nome[TAMANHO_NOME - 1] = '\0';
    strncpy(novaSala->pista_na_sala, pista, TAMANHO_TEXTO_PISTA - 1);
    novaSala->pista_na_sala[TAMANHO_TEXTO_PISTA - 1] = '\0';
    novaSala->esquerda = NULL;
    novaSala->direita = NULL;
    return novaSala;
}

// 📌 Funções para Nível Aventureiro (Árvore de Busca de Pistas)

Pista *criarPista(const char *texto) {
    Pista *novaPista = (Pista *)malloc(sizeof(Pista));
    if (novaPista == NULL) {
        perror("Erro ao alocar Pista");
        exit(EXIT_FAILURE);
    }
    strncpy(novaPista->texto, texto, TAMANHO_TEXTO_PISTA - 1);
    novaPista->texto[TAMANHO_TEXTO_PISTA - 1] = '\0';
    novaPista->esquerda = novaPista->direita = NULL;
    return novaPista;
}

Pista *inserirPistaBST(Pista *raiz, const char *texto) {
    if (raiz == NULL) {
        return criarPista(texto);
    }

    int comparacao = strcmp(texto, raiz->texto);

    if (comparacao < 0) {
        raiz->esquerda = inserirPistaBST(raiz->esquerda, texto);
    } else if (comparacao > 0) {
        raiz->direita = inserirPistaBST(raiz->direita, texto);
    }
    // Ignora se for igual (pista duplicada)
    return raiz;
}

void listarPistasEmOrdem(Pista *raiz) {
    if (raiz != NULL) {
        listarPistasEmOrdem(raiz->esquerda);
        printf("   - %s\n", raiz->texto);
        listarPistasEmOrdem(raiz->direita);
    }
}

// 📌 Funções para Nível Mestre (Tabela Hash de Suspeitos)

void inicializarHash() {
    for (int i = 0; i < TAMANHO_TABELA_HASH; i++) {
        tabelaHash[i] = NULL;
    }
}

// Função de Hashing simples: soma dos valores ASCII do primeiro caractere (depois de converter para minúsculo)
int funcaoHash(const char *nome) {
    if (nome == NULL || *nome == '\0') return 0;
    // Pega a primeira letra e normaliza para minúsculo
    char primeira_letra = tolower(nome[0]); 
    // Mapeia 'a' a 'z' para um índice. Se for um caractere não-alfabético, usa índice 0.
    if (primeira_letra >= 'a' && primeira_letra <= 'z') {
        return (primeira_letra - 'a') % TAMANHO_TABELA_HASH;
    }
    return 0; // Fallback para índice 0
}

Suspeito *buscarSuspeito(const char *nome) {
    int indice = funcaoHash(nome);
    Suspeito *atual = tabelaHash[indice];

    while (atual != NULL) {
        if (strcmp(atual->nome, nome) == 0) {
            return atual; // Suspeito encontrado
        }
        atual = atual->proximo; // Próximo na lista de colisão
    }
    return NULL; // Não encontrado
}

void inserirAssociacaoHash(const char *suspeitoNome, const char *pistaTexto) {
    Suspeito *suspeito = buscarSuspeito(suspeitoNome);

    if (suspeito == NULL) {
        // Suspeito não existe, cria um novo
        int indice = funcaoHash(suspeitoNome);
        
        Suspeito *novoSuspeito = (Suspeito *)malloc(sizeof(Suspeito));
        if (novoSuspeito == NULL) {
             perror("Erro ao alocar Suspeito");
             return;
        }

        strncpy(novoSuspeito->nome, suspeitoNome, TAMANHO_NOME - 1);
        novoSuspeito->nome[TAMANHO_NOME - 1] = '\0';
        novoSuspeito->contagem_pistas = 0;
        novoSuspeito->pistas_associadas = NULL;
        
        // Trata a colisão: insere no início da lista encadeada do índice
        novoSuspeito->proximo = tabelaHash[indice];
        tabelaHash[indice] = novoSuspeito;
        suspeito = novoSuspeito;
    }

    // Adiciona a pista à lista encadeada do suspeito
    AssociacaoPista *novaAssociacao = (AssociacaoPista *)malloc(sizeof(AssociacaoPista));
    if (novaAssociacao == NULL) {
        perror("Erro ao alocar AssociacaoPista");
        return;
    }

    strncpy(novaAssociacao->pista, pistaTexto, TAMANHO_TEXTO_PISTA - 1);
    novaAssociacao->pista[TAMANHO_TEXTO_PISTA - 1] = '\0';
    novaAssociacao->proximo = suspeito->pistas_associadas;
    suspeito->pistas_associadas = novaAssociacao;
    suspeito->contagem_pistas++; // Incrementa o contador
}

void listarAssociacoes() {
    printf("\n--- 🧠 Relatório de Suspeitos e Pistas ---\n");
    int total_suspeitos = 0;
    
    for (int i = 0; i < TAMANHO_TABELA_HASH; i++) {
        Suspeito *atual = tabelaHash[i];
        while (atual != NULL) {
            total_suspeitos++;
            printf("  ➡️ **Suspeito:** %s (Pistas Associadas: %d)\n", atual->nome, atual->contagem_pistas);
            AssociacaoPista *pistaAtual = atual->pistas_associadas;
            while (pistaAtual != NULL) {
                printf("    - \"%s\"\n", pistaAtual->pista);
                pistaAtual = pistaAtual->proximo;
            }
            atual = atual->proximo;
        }
    }
    
    if (total_suspeitos == 0) {
        printf("Nenhum suspeito registrado ainda.\n");
    }
    printf("----------------------------------------\n");
}

void exibirSuspeitoMaisProvavel() {
    Suspeito *suspeitoMaisProvavel = NULL;
    int maxPistas = -1;
    
    for (int i = 0; i < TAMANHO_TABELA_HASH; i++) {
        Suspeito *atual = tabelaHash[i];
        while (atual != NULL) {
            if (atual->contagem_pistas > maxPistas) {
                maxPistas = atual->contagem_pistas;
                suspeitoMaisProvavel = atual;
            }
            atual = atual->proximo;
        }
    }

    if (suspeitoMaisProvavel != NULL && maxPistas > 0) {
        printf("\n✅ **SUSPEITO MAIS PROVÁVEL:** %s (Com %d pistas associadas)\n", 
               suspeitoMaisProvavel->nome, maxPistas);
    } else {
        printf("\n🤔 Ainda não há pistas suficientes para determinar o suspeito mais provável.\n");
    }
}

// 📌 Funções de Liberação de Memória

void liberarPistasBST(Pista *raiz) {
    if (raiz != NULL) {
        liberarPistasBST(raiz->esquerda);
        liberarPistasBST(raiz->direita);
        free(raiz);
    }
}

void liberarSalas(Sala *raiz) {
    if (raiz != NULL) {
        liberarSalas(raiz->esquerda);
        liberarSalas(raiz->direita);
        free(raiz);
    }
}

void liberarHash() {
    for (int i = 0; i < TAMANHO_TABELA_HASH; i++) {
        Suspeito *suspeitoAtual = tabelaHash[i];
        while (suspeitoAtual != NULL) {
            Suspeito *proximoSuspeito = suspeitoAtual->proximo;
            
            // Libera a lista de pistas associadas
            AssociacaoPista *pistaAtual = suspeitoAtual->pistas_associadas;
            while (pistaAtual != NULL) {
                AssociacaoPista *proximaPista = pistaAtual->proximo;
                free(pistaAtual);
                pistaAtual = proximaPista;
            }
            
            free(suspeitoAtual);
            suspeitoAtual = proximoSuspeito;
        }
        tabelaHash[i] = NULL;
    }
}

// 🗺️ Implementação do Mapa e Exploração

Sala *montarMapaMansao() {
    // Nível 1
    Sala *hall = criarSala("Hall de Entrada", "A chave prateada está faltando.");
    
    // Nível 2
    hall->esquerda = criarSala("Biblioteca", "Um bilhete rasgado menciona 'Srta. Scarlett'.");
    hall->direita = criarSala("Cozinha", "Há um copo de vinho quebrado no chão.");
    
    // Nível 3
    hall->esquerda->esquerda = criarSala("Quarto Principal", "Um anel com as iniciais 'J.B.' estava debaixo do travesseiro.");
    hall->esquerda->direita = criarSala("Sala de Jantar", "O retrato de um dos suspeitos foi virado para a parede.");
    hall->direita->esquerda = criarSala("Porão", "Nenhuma pista importante aqui, apenas poeira.");
    hall->direita->direita = criarSala("Sótão", "Um tecido azul foi preso na janela.");
    
    return hall;
}

void explorarSalas(Sala *mapaRaiz, Pista **pistasColetadas) {
    Sala *atual = mapaRaiz;
    char comando;

    printf("\n--- 🌱 Exploração da Mansão (Árvore Binária) ---\n");
    printf("Você está no **%s**.\n", atual->nome);

    while (1) {
        printf("\nOpções: **E** (Esquerda) | **D** (Direita) | **S** (Sair/Revisar Pistas)\n");
        printf("Comando: ");
        if (scanf(" %c", &comando) != 1) {
            // Lidar com erro de input
            while (getchar() != '\n'); // Limpar buffer
            continue;
        }
        while (getchar() != '\n'); // Limpar buffer
        
        comando = tolower(comando);

        if (comando == 's') {
            printf("\nVoltando ao menu principal.\n");
            break;
        }
        
        Sala *proximaSala = NULL;
        if (comando == 'e' && atual->esquerda != NULL) {
            proximaSala = atual->esquerda;
        } else if (comando == 'd' && atual->direita != NULL) {
            proximaSala = atual->direita;
        } else if ((comando == 'e' && atual->esquerda == NULL) || (comando == 'd' && atual->direita == NULL)) {
            printf("❌ Não há passagem nessa direção. Tente outra.\n");
            continue;
        } else {
            printf("❌ Comando inválido. Use E, D ou S.\n");
            continue;
        }

        atual = proximaSala;
        printf("Você se moveu para o(a) **%s**.\n", atual->nome);

        // Lógica de coleta de pista (Nível Aventureiro)
        if (strlen(atual->pista_na_sala) > 0) {
            printf("🌟 **PISTA ENCONTRADA!** \"%s\"\n", atual->pista_na_sala);
            
            // Adiciona na BST de pistas (Nível Aventureiro)
            *pistasColetadas = inserirPistaBST(*pistasColetadas, atual->pista_na_sala);
            
            // Adiciona associações de pistas e suspeitos (Nível Mestre)
            // Simulação de relação pista-suspeito
            if (strstr(atual->pista_na_sala, "Scarlett") != NULL) {
                inserirAssociacaoHash("Srta. Scarlett", atual->pista_na_sala);
            } else if (strstr(atual->pista_na_sala, "J.B.") != NULL) {
                inserirAssociacaoHash("Sr. Brown", atual->pista_na_sala);
            } else if (strstr(atual->pista_na_sala, "tecido azul") != NULL) {
                inserirAssociacaoHash("Sra. Peacock", atual->pista_na_sala);
            } else if (strstr(atual->pista_na_sala, "chave prateada") != NULL) {
                inserirAssociacaoHash("Sr. Green", atual->pista_na_sala);
            } else {
                 // Pistas que podem se associar a mais de um suspeito
                 inserirAssociacaoHash("Sr. Brown", atual->pista_na_sala);
                 inserirAssociacaoHash("Srta. Scarlett", atual->pista_na_sala);
            }
            
            // Remove a pista da sala para não ser coletada novamente
            atual->pista_na_sala[0] = '\0'; 
        }
    }
}

// --- Função Principal ---

int main() {
    // Variáveis globais para as estruturas
    Sala *mapaDaMansao = NULL;
    Pista *pistasColetadas = NULL;
    
    // Inicializa a Tabela Hash
    inicializarHash();
    
    // Monta o mapa da mansão
    mapaDaMansao = montarMapaMansao();
    
    int opcao;
    
    do {
        printf("\n\n=============== DETECTIVE QUEST ===============\n");
        printf("1. 🗺️ Explorar a Mansão (Nível Novato)\n");
        printf("2. 🔍 Revisar Pistas (Nível Aventureiro)\n");
        printf("3. 🧠 Analisar Suspeitos (Nível Mestre)\n");
        printf("4. 🚪 Sair do Jogo\n");
        printf("Escolha uma opção: ");
        
        if (scanf("%d", &opcao) != 1) {
            // Lidar com erro de input
            printf("Entrada inválida. Por favor, digite um número.\n");
            while (getchar() != '\n'); // Limpar buffer
            opcao = 0;
            continue;
        }
        while (getchar() != '\n'); // Limpar buffer

        printf("\n");

        switch (opcao) {
            case 1:
                explorarSalas(mapaDaMansao, &pistasColetadas);
                break;
            case 2:
                printf("\n--- 🔍 Pistas Coletadas (Árvore de Busca) ---\n");
                if (pistasColetadas == NULL) {
                    printf("Nenhuma pista coletada ainda.\n");
                } else {
                    // Exibe as pistas em ordem alfabética (emOrdem)
                    listarPistasEmOrdem(pistasColetadas);
                }
                printf("--------------------------------------------\n");
                break;
            case 3:
                listarAssociacoes();
                exibirSuspeitoMaisProvavel();
                break;
            case 4:
                printf("Obrigado por jogar Detective Quest! Até a próxima.\n");
                break;
            default:
                printf("Opção inválida. Tente novamente.\n");
                break;
        }
    } while (opcao != 4);

    // Limpeza de memória
    liberarSalas(mapaDaMansao);
    liberarPistasBST(pistasColetadas);
    liberarHash();

    return 0;
}