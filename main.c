#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char nome[30];
    int custo;
    float retorno;
} acao;


void liberarmemoria(float **dp, int i, int n){
    if (i >= n){
        free(dp);
        return;
    }
    free(dp[i]);
    liberarmemoria(dp, i + 1, n);

}
// knapsack problem (programação dinâmica)
float combinacao_otima(acao *acoes, int n_acoes, int capital, int *selecionadas){
    float **dp = malloc((n_acoes + 1) * sizeof(float *));
    for(int i = 0; i <= n_acoes; i++){
        dp[i] = calloc((capital + 1), sizeof(float));
    }
    for(int i = 1; i <= n_acoes; i++){
        for(int j = 0; j <= capital; j++){
            if(acoes[i-1].custo > j){
                dp[i][j] = dp[i - 1][j];
            }
            else {
                float naoInclui = dp[i - 1][j];
                float inclui = dp[i - 1][j - acoes[i - 1].custo] + acoes[i-1].retorno;
                dp[i][j] = (inclui > naoInclui) ? inclui : naoInclui;
            }
        }
    }
    int restante = capital;
    for(int i = n_acoes; i > 0; i--){
        if(dp[i][restante] != dp[i - 1][restante]){
            selecionadas[i - 1] = 1;
            restante -= acoes[i - 1].custo;
        } else {
            selecionadas[i - 1] = 0;
        }
    }
    float resultado = dp[n][capital];
    liberarmemoria(dp, 0, n);

    return resultado;
}

acao *lerArquivo(const char *nomeArquivo, int *n, int *capital){
    FILE *file = fopen(nomeArquivo, "r");
    if(file == NULL){
        perror("erro ao abrir arquivo");
        exit(1);
    }
    char linha[150];
    *n = 0;
    int capacidade = 50;
    acao *array = malloc(capacidade * sizeof(acao));

    while (fgets(linha, sizeof(linha), file)){
        if(strncmp(linha, "CAPITAL_DISPONIVEL_R$:", strlen("CAPITAL_DISPONIVEL_R$:")) == 0){
            float temp;
            sscanf(linha, "CAPITAL_DISPONIVEL_R$: %f", &temp);
            *capital = (int)(temp + 0.5);
            break;
        }
    }
    while (fgets(linha, sizeof(linha), file)){
        if (strncmp(linha, "ACOES:", 6) == 0){
            break;
        }
    }
    char id[10], nomeTemp[50];
    int custoint;
    float retornofloat;

    while (fgets(linha, sizeof(linha), file)){
        if(sscanf(linha, "%s %d %f %[^\n]", id, &custoint, &retornofloat, nomeTemp) == 4){
            if(*n >= capacidade){
                capacidade *= 2;
                array = realloc(array, capacidade * sizeof(acao));
            }
            strncpy(array[*n].nome, nomeTemp, sizeof(array[*n].nome) - 1);
            array[*n].nome[sizeof(array[*n].nome) - 1] = '\0';
            array[*n].custo = custoint;
            array[*n].retorno = retornofloat;
            (*n)++;
        }
    }
    fclose(file);
    return array;
}

void exibirResultado(acao *acoes, int n, int capital, int melhorcomb, int melhorCusto, float maiorRetorno ){
    printf("\n----------------------------------------\n");
    printf("Carteira de Investimentos Otimizada\n");
    printf("----------------------------------------\n");
    printf("Capital Disponível: R$ %d\n\n", capital);
    printf("Ações a Comprar:\n");

    for(int j = 0; j < n; j++){
        if(melhorcomb & (1 << j)){
            printf("- %s (Custo: R$ %d, Retorno: %.2f%%)\n", acoes[j].nome, acoes[j].custo, acoes[j].retorno);
        }
    }
    printf("\nResumo da Carteira:\n");
    printf("- Custo Total: R$ %d\n", melhorCusto);
    printf("- Retorno Máximo Esperado: %.2f%%\n", maiorRetorno);
    printf("----------------------------------------\n");
}

void exibirResultadoPD(acao *acoes, int n, int capital, int *selecionadas, float retornoMaximo){
    printf("\n----------------------------------------\n");
    printf("Carteira de Investimentos Otimizada\n");
    printf("----------------------------------------\n");
    printf("Capital Disponível: R$ %d\n\n", capital);
    printf("Ações a Comprar:\n");

    int custoTotal = 0;
    for(int i = 0; i < n; i++){
        if(selecionadas[i]){
            printf("- %s (Custo: R$ %d, Retorno: %.2f%%)\n", acoes[i].nome, acoes[i].custo, acoes[i].retorno);
            custoTotal += acoes[i].custo;
        }
    }

    printf("\nResumo da Carteira:\n");
    printf("- Custo Total: R$ %d\n", custoTotal);
    printf("- Retorno Máximo Esperado: %.2f%%\n", retornoMaximo);
    printf("----------------------------------------\n");
}

int main(){
    int n, capital;
    char nomeArquivo[100];

    printf("Digite o nome do arquivo: ");
    scanf("%s", nomeArquivo);

    acao *array = lerArquivo(nomeArquivo, &n, &capital);

    if(n == 0){
        printf("Nenhuma ação foi lida. Verifique o arquivo.\n");
        free(array);
        return 1;
    }

    printf("Foram lidas %d ações. Capital disponível: R$ %d\n", n, capital);

    printf("========================== Acoes ==========================\n");
    printf("-----------------------------------------------------------\n");
    printf("%-20s | %-10s | %-10s\n", "Nome", "Custo", "Retorno");
    for(int i = 0; i < n; i++){
        printf("%-20s | %10d | Retorno: %9.2f%%\n", array[i].nome, array[i].custo, array[i].retorno);
    }

    // Método máscara de bit (pode ser muito lento para muitas ações)
    /*
    int melhorCusto;
    float maiorRetorno;
    int melhorcomb = investimento(array, n, capital, &melhorCusto, &maiorRetorno);
    exibirResultado(array, n, capital, melhorcomb, melhorCusto, maiorRetorno);
    */

    // Mochila dinâmica
    int *selecionadas = calloc(n, sizeof(int));
    float retornoMaximo = combinacao_otima(array, n, capital, selecionadas);
    exibirResultadoPD(array, n, capital, selecionadas, retornoMaximo);

    free(array);
    free(selecionadas);
    return 0;
}

