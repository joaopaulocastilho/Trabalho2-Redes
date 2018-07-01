#include "globais.h"
#include "atualiza_vetor_saltos.c"

/* Testa a função converte_pacote_para_char */
int testa_converte_pacote_para_char() {
  pacote_t pacote_entrada;
  char resultado[1000];
  char resultado_esperado[500];
  strcpy(resultado_esperado, "AAAABAAAAMensagem");
  pacote_entrada.destino = 1094795585;
  pacote_entrada.tipo = 66;
  pacote_entrada.origem = 1094795585;
  strcpy(pacote_entrada.mensagem, "Mensagem");
  converte_pacote_para_char(pacote_entrada, resultado);


  int resultado_comparacao = strcmp(resultado_esperado, resultado);
  return resultado_comparacao == 0;
};

/* Testa a função converte_char_para_pacote */
int testa_converte_char_para_pacote() {
  pacote_t pacote;
  char cadeia_entrada[500];
  strcpy(cadeia_entrada, "AAAABAAAAMensagem");
  converte_char_para_pacote(cadeia_entrada, &pacote);

  if (pacote.destino != 1094795585) return 0;
  if (pacote.tipo != 66) return 0;
  if (pacote.origem != 1094795585) return 0;
  int resultado_comparacao = strcmp(pacote.mensagem, "Mensagem");
  return resultado_comparacao == 0;
};

int testa_atualiza_vetor_saltos() {
  int vetor_saltos[QUANTIDADE_MAXIMA_NOS] = {0, 0, 0, 0};
  pthread_mutex_t vetor_saltos_mutex;
  int tabela_roteamento[QUANTIDADE_MAXIMA_NOS][QUANTIDADE_MAXIMA_NOS];
  for(int i = 0; i < QUANTIDADE_MAXIMA_NOS; i++) {
    for (int j = 0; j < QUANTIDADE_MAXIMA_NOS; j++) {
      tabela_roteamento[i][j] = INFINITO;
    }
  }

  // {0, 5, 3, 6},
  // {5, 0, 2, 1},
  // {3, 2, 0, 1},
  // {4, 1, 1, 0}
  tabela_roteamento[0][0] = 0;
  tabela_roteamento[0][1] = 5;
  tabela_roteamento[0][2] = 3;
  tabela_roteamento[0][3] = 6;
  tabela_roteamento[1][0] = 5;
  tabela_roteamento[1][1] = 0;
  tabela_roteamento[1][2] = 2;
  tabela_roteamento[1][3] = 1;
  tabela_roteamento[2][0] = 3;
  tabela_roteamento[2][1] = 2;
  tabela_roteamento[2][2] = 0;
  tabela_roteamento[2][3] = 1;
  tabela_roteamento[3][0] = 4;
  tabela_roteamento[3][1] = 1;
  tabela_roteamento[3][2] = 1;
  tabela_roteamento[3][3] = 0;

  pthread_mutex_t tabela_roteamento_mutex;

  // printf("vetor saltos: "); for (int i = 0; i < 4; i++) printf("%d ", vetor_saltos[i]);
  atualiza_vetor_saltos(tabela_roteamento,
                        &tabela_roteamento_mutex,
                        vetor_saltos,
                        &vetor_saltos_mutex,
                        0);

  //debug
  // printf("\n");
  // for(int i = 0; i < 10; i++) {
  //   for (int j = 0; j < 10; j++) {
  //     printf("%10d ", tabela_roteamento[i][j]);
  //   }
  //   printf("\n");
  // }
  // printf("vetor saltos: "); for (int i = 0; i < 4; i++) printf("%d ", vetor_saltos[i]);

  int vetor_saltos_esperado[4] = {0, 1, 2, 2};
  for (int i = 0; i < 4; i++) {
    if (vetor_saltos[i] != vetor_saltos_esperado[i]) {
      printf("testa_atualiza_vetor_saltos falha em %d\n", i);
      return 0;
    }
  }
  return 1;
}

void passou() {
  printf("Passou!\n");
}

void falhou() {
  printf("Falhou!\n");
}

int main() {
  printf("Testando função converte_pacote_para_char: ");
  testa_converte_pacote_para_char() ? passou() : falhou();
  printf("Testando função converte_char_para_pacote: ");
  testa_converte_char_para_pacote() ? passou() : falhou();
  printf("Testando função testa_atualiza_vetor_saltos: ");
  testa_atualiza_vetor_saltos() ? passou() : falhou();
  return 0;
};