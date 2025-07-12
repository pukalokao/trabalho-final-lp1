#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

typedef struct {
    char nome[30];
    int custo;
    float retorno;
} Acao;


void liberarmemoria(float **matrizD, int i, int n){
    if (i >= n){
        free(matrizD);
        return;
    }
    free(matrizD[i]);
    liberarmemoria(matrizD, i + 1, n);

}
// knapsack problem (programação dinâmica)
float melhorInvestimento(Acao *listaAcoes, int num_acoes, int capital, int *selecionadas){
    float **matrizD = malloc((num_acoes + 1) * sizeof(float *));
    for(int i = 0; i <= num_acoes; i++){
        matrizD[i] = calloc((capital + 1), sizeof(float));
    }
    for(int i = 1; i <= num_acoes; i++){
        for(int j = 0; j <= capital; j++){
            if(listaAcoes[i-1].custo > j){
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
    for(int i = num_acoes; i > 0; i--){
        if(matrizD[i][capitalRestante] != matrizD[i - 1][capitalRestante]){
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

Acao *lerArquivo(const char *nomeArquivo, int *num_acoes, int *capital){
    FILE *arquivo = fopen(nomeArquivo, "r");
    if(arquivo == NULL){
        perror("erro ao abrir arquivo");
        exit(1);
    }
    char linha[150];
    *num_acoes = 0;
    int capacidade = 50;
    Acao *acoes = malloc(capacidade * sizeof(Acao));
    if(acoes == NULL) {
        perror("Erro ao alocar memória inicial para ações");
        exit(EXIT_FAILURE);
    }

    while (fgets(linha, sizeof(linha), arquivo)){
        if(strncmp(linha, "CAPITAL_DISPONIVEL_R$:", strlen("CAPITAL_DISPONIVEL_R$:")) == 0){
            float capitalFloat;
            sscanf(linha, "CAPITAL_DISPONIVEL_R$: %f", &capitalFloat);
            *capital = (int)(capitalFloat + 0.5);
            break;
        }
    }
    while (fgets(linha, sizeof(linha), arquivo)){
        if (strncmp(linha, "ACOES:", 6) == 0){
            break;
        }
    }
    char idAcao[10], nomeAcaoTemp[50];
    float custoAcaoTemp;
    float retornoAcaoTemp;

    while (fgets(linha, sizeof(linha), arquivo)){
        if(sscanf(linha, "%s %f %f %[^\n]", idAcao, &custoAcaoTemp, &retornoAcaoTemp, nomeAcaoTemp) == 4){
            if(*num_acoes >= capacidade){
                capacidade *= 2;
                acoes = realloc(acoes, capacidade * sizeof(Acao));

            }
            strncpy(acoes[*num_acoes].nome, nomeAcaoTemp, sizeof(acoes[*num_acoes].nome) - 1);
            acoes[*num_acoes].nome[sizeof(acoes[*num_acoes].nome) - 1] = '\0';
            acoes[*num_acoes].custo = custoAcaoTemp;
            acoes[*num_acoes].retorno = retornoAcaoTemp;
            (*num_acoes)++;
        }
    }
    fclose(arquivo);
    return acoes;
}

void exibirResultado(Acao *listaAcoes, int num_acoes, int capital, int melhorCombinacao, int custoTotal, float retornoMaximo ){
    printf("\n----------------------------------------\n");
    printf("Carteira de Investimentos Otimizada\n");
    printf("----------------------------------------\n");
    printf("Capital Disponível: R$ %d\n\n", capital);
    printf("Ações a Comprar:\n");

    for(int j = 0; j < num_acoes; j++){
        if(melhorCombinacao & (1 << j)){
            printf("- %s (Custo: R$ %d, Retorno: %.2f%%)\n", listaAcoes[j].nome, listaAcoes[j].custo, listaAcoes[j].retorno);
        }
    }
    printf("\nResumo da Carteira:\n");
    printf("- Custo Total: R$ %d\n", custoTotal);
    printf("- Retorno Máximo Esperado: %.2f%%\n", retornoMaximo);
    printf("----------------------------------------\n");
}

void exibirResultadoPD(Acao *listaAcoes, int num_acoes, int capital, int *selecionadas, float retornoMaximo){
    printf("\n----------------------------------------\n");
    printf("Carteira de Investimentos Otimizada\n");
    printf("----------------------------------------\n");
    printf("Capital Disponível: R$ %d\n\n", capital);
    printf("Ações a Comprar:\n");

    int custoTotal = 0;
    for(int i = 0; i < num_acoes; i++){
        if(selecionadas[i]){
            printf("- %s (Custo: R$ %d, Retorno: %.2f%%)\n", listaAcoes[i].nome, listaAcoes[i].custo, listaAcoes[i].retorno);
            custoTotal += listaAcoes[i].custo;
        }
    }

    printf("\nResumo da Carteira:\n");
    printf("- Custo Total: R$ %d\n", custoTotal);
    printf("- Retorno Máximo Esperado: %.2f%%\n", retornoMaximo);
    printf("----------------------------------------\n");
}

int main(){
    setlocale(LC_ALL, "Portuguese");
    int numeroAcoes, capital;
    char nomeArquivo[100];

    printf("Digite o nome do arquivo: ");
    scanf("%s", nomeArquivo);

    Acao *lista_Acoes = lerArquivo(nomeArquivo, &numeroAcoes, &capital);

    if(numeroAcoes == 0){
        printf("Nenhuma ação foi lida. Verifique o arquivo.\n");
        free(lista_Acoes);
        return 1;
    }

    printf("Foram lidas %d ações. Capital disponível: R$ %d\n", numeroAcoes, capital);

    printf("========================== Acoes ==========================\n");
    printf("-----------------------------------------------------------\n");
    printf("%-20s | %-10s | %-10s\n", "Nome", "Custo", "Retorno");
    for(int i = 0; i < numeroAcoes; i++){
        printf("%-20s | %10d | Retorno: %9.2f%%\n", lista_Acoes[i].nome, lista_Acoes[i].custo, lista_Acoes[i].retorno);
    }

    // Método máscara de bit (pode ser muito lento para muitas ações)
    /*
    int melhorCusto;
    float maiorRetorno;
    int melhorcomb = investimento(array, n, capital, &melhorCusto, &maiorRetorno);
    exibirResultado(array, n, capital, melhorcomb, melhorCusto, maiorRetorno);
    */

    // Mochila dinâmica
    int *selecionadas = calloc(numeroAcoes, sizeof(int));
    float retornoMaximo = melhorInvestimento(lista_Acoes, numeroAcoes, capital, selecionadas);
    exibirResultadoPD(lista_Acoes, numeroAcoes, capital, selecionadas, retornoMaximo);

    free(lista_Acoes);
    free(selecionadas);
    return 0;
}

