#include <stdio.h>

#define QUANTIDADE_MAXIMA_NOS 112
#define INFINITO 112345678

void atualiza_tabela_roteamento(int nodo_atual, int tabela_roteamento[QUANTIDADE_MAXIMA_NOS][QUANTIDADE_MAXIMA_NOS]) {
  int i = nodo_atual, j, k, custo, menor = INFINITO;
  for (j = 0; j < QUANTIDADE_MAXIMA_NOS; j++) {
    for (menor = INFINITO, k = 0; k < QUANTIDADE_MAXIMA_NOS; k++) {
      custo = tabela_roteamento[i][k] + tabela_roteamento[k][j];
      if (custo < menor) menor = custo;
    }
    tabela_roteamento[i][j] = menor;
  }
}

int main(void) {
  int i, j;
  int tabela_roteamento[QUANTIDADE_MAXIMA_NOS][QUANTIDADE_MAXIMA_NOS];
  int nodo_atual = 1;

  for (i = 0; i < QUANTIDADE_MAXIMA_NOS; i++)
  for (j = 0; j < QUANTIDADE_MAXIMA_NOS; j++) tabela_roteamento[i][j] = INFINITO;

  tabela_roteamento[1][1] = 0;
  tabela_roteamento[1][2] = 5;
  tabela_roteamento[1][3] = 2;
  tabela_roteamento[1][4] = INFINITO;

  tabela_roteamento[2][1] = 5;
  tabela_roteamento[2][2] = 0;
  tabela_roteamento[2][3] = 1;
  tabela_roteamento[2][4] = 4;

  tabela_roteamento[3][1] = 2;
  tabela_roteamento[3][2] = 1;
  tabela_roteamento[3][3] = 0;
  tabela_roteamento[3][4] = 8;

  tabela_roteamento[4][1] = INFINITO;
  tabela_roteamento[4][2] = INFINITO;
  tabela_roteamento[4][3] = INFINITO;
  tabela_roteamento[4][4] = INFINITO;

  atualiza_tabela_roteamento(nodo_atual, tabela_roteamento);

  for (i = 1; i <= 4; i++) {
    for (j = 1; j <= 4; j++) {
      printf("%d ", tabela_roteamento[i][j] == INFINITO ? -1 : tabela_roteamento[i][j]);
    }
    printf("\n");
  }
  return 0;
}
