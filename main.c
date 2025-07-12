#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
    char nome[30];
    int custo;
    float retorno;
} Acao;

void liberarmemoria(float **matrizD, int i, int n) {
    if (i >= n) {
        free(matrizD);
        return;
    }
    free(matrizD[i]);
    liberarmemoria(matrizD, i + 1, n);
}

float melhorInvestimento(Acao *listaAcoes, int num_acoes, int capital, int *selecionadas) {
    float **matrizD = malloc((num_acoes + 1) * sizeof(float *));
    for(int i = 0; i <= num_acoes; i++) {
        matrizD[i] = calloc((capital + 1), sizeof(float));
    }

    for(int i = 1; i <= num_acoes; i++) {
        for(int j = 0; j <= capital; j++) {
            if(listaAcoes[i-1].custo > j) {
                matrizD[i][j] = matrizD[i - 1][j];
            }
            else {
                float naoInclui = matrizD[i - 1][j];
                float inclui = matrizD[i - 1][j - listaAcoes[i - 1].custo] + listaAcoes[i-1].retorno;
                matrizD[i][j] = (inclui > naoInclui) ? inclui : naoInclui;
            }
        }
    }

    int capitalRestante = capital;
    for(int i = num_acoes; i > 0; i--) {
        if(matrizD[i][capitalRestante] != matrizD[i - 1][capitalRestante]) {
            selecionadas[i - 1] = 1;
            capitalRestante -= listaAcoes[i - 1].custo;
        } else {
            selecionadas[i - 1] = 0;
        }
    }

    float resultado = matrizD[num_acoes][capital];
    liberarmemoria(matrizD, 0, num_acoes);
    return resultado;
}

Acao *lerArquivo(const char *nomeArquivo, int *num_acoes, int *capital) {
    FILE *arquivo = fopen(nomeArquivo, "r");
    if(arquivo == NULL) {
        perror("Erro ao abrir arquivo");
        exit(1);
    }

    char linha[150];
    *num_acoes = 0;
    int capacidade = 50;
    Acao *acoes = malloc(capacidade * sizeof(Acao));
    if(acoes == NULL) {
        perror("Erro ao alocar memória inicial para acoes");
        exit(EXIT_FAILURE);
    }

    // Ler capital disponível
    while(fgets(linha, sizeof(linha), arquivo)) {
        if(strstr(linha, "CAPITAL_DISPONIVEL_R$:") != NULL) {
            float capitalFloat;
            sscanf(linha, "CAPITAL_DISPONIVEL_R$: %f", &capitalFloat);
            *capital = (int)(capitalFloat + 0.5); // Arredonda para inteiro
            break;
        }
    }

    // Encontrar seção de ações
    while(fgets(linha, sizeof(linha), arquivo)) {
        if(strstr(linha, "ACOES:") != NULL) {
            break;
        }
    }

    // Ler ações
    char idAcao[10];
    float custoAcaoTemp;
    float retornoAcaoTemp;
    char nomeAcaoTemp[50];

    while(fgets(linha, sizeof(linha), arquivo)){
        // Pular linhas de solução ótima
        if(strstr(linha, "Solução Ótima de Referência:") != NULL) {
            break;
        }

        // Pular linhas vazias ou comentários
        if(strlen(linha) == 0 || linha[0] == '#') {
            continue;
        }

        if(sscanf(linha, "%s %f %f %49[^\n]", idAcao, &custoAcaoTemp, &retornoAcaoTemp, nomeAcaoTemp) == 4) {
            if(*num_acoes >= capacidade) {
                capacidade *= 2;
                Acao *temp = realloc(acoes, capacidade * sizeof(Acao));
                if(temp == NULL) {
                    perror("Erro ao realocar memoria para acoes");
                    free(acoes);
                    exit(EXIT_FAILURE);
                }
                acoes = temp;
            }

            // Copiar dados para a estrutura
            strncpy(acoes[*num_acoes].nome, nomeAcaoTemp, sizeof(acoes[*num_acoes].nome) - 1);
            acoes[*num_acoes].nome[sizeof(acoes[*num_acoes].nome) - 1] = '\0';
            acoes[*num_acoes].custo = (int)(custoAcaoTemp + 0.5); // Arredonda para inteiro
            acoes[*num_acoes].retorno = retornoAcaoTemp;
            (*num_acoes)++;
        }
    }

    fclose(arquivo);
    return acoes;
}

void exibirResultadoPD(Acao *listaAcoes, int num_acoes, int capital, int *selecionadas, float retornoMaximo) {
    printf("\n----------------------------------------\n");
    printf("Carteira de Investimentos Otimizada\n");
    printf("----------------------------------------\n");
    printf("Capital Disponivel: R$ %d\n\n", capital);
    printf("Acoes a Comprar:\n");

    int custoTotal = 0;
    for(int i = 0; i < num_acoes; i++) {
        if(selecionadas[i]) {
            printf("- %s (Custo: R$ %d, Retorno: %.2f%%)\n", listaAcoes[i].nome, listaAcoes[i].custo, listaAcoes[i].retorno);
            custoTotal += listaAcoes[i].custo;
        }
    }

    printf("\nResumo da Carteira:\n");
    printf("- Custo Total: R$ %d\n", custoTotal);
    printf("- Retorno Maximo Esperado: %.2f%%\n", retornoMaximo);
    printf("----------------------------------------\n");
}

int main() {

    int numeroAcoes, capital;
    char nomeArquivo[100];

    printf("Digite o nome do arquivo: ");
    scanf("%99s", nomeArquivo);

    Acao *lista_Acoes = lerArquivo(nomeArquivo, &numeroAcoes, &capital);

    if(numeroAcoes == 0) {
        printf("Nenhuma acao foi lida. Verifique o arquivo.\n");
        free(lista_Acoes);
        return 1;
    }

    printf("Foram lidas %d acoes. Capital disponivel: R$ %d\n", numeroAcoes, capital);

    printf("========================== Acoes ==========================\n");
    printf("-----------------------------------------------------------\n");
    printf("%-20s | %-10s | %-10s\n", "Nome", "Custo", "Retorno");
    for(int i = 0; i < numeroAcoes; i++) {
        printf("%-20s | %10d | Retorno: %9.2f%%\n", lista_Acoes[i].nome, lista_Acoes[i].custo, lista_Acoes[i].retorno);
    }

    int *selecionadas = calloc(numeroAcoes, sizeof(int));
    float retornoMaximo = melhorInvestimento(lista_Acoes, numeroAcoes, capital, selecionadas);
    exibirResultadoPD(lista_Acoes, numeroAcoes, capital, selecionadas, retornoMaximo);

    free(lista_Acoes);
    free(selecionadas);
    return 0;
}
