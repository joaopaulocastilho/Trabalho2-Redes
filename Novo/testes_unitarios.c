#include "globais.h"

/* Testa a função converte_pacote_para_char */
int testa_converte_pacote_para_char() {
  pacote_t pacote_entrada;
  char resultado[1000];
  char resultado_esperado[500];
  strcpy(resultado_esperado, "@@@@B@@@@Mensagem");
  pacote_entrada.destino = 1094795585;
  pacote_entrada.tipo = 66;
  pacote_entrada.origem = 1094795585;
  strcpy(pacote_entrada.mensagem, "Mensagem");
  converte_pacote_para_char(pacote_entrada, resultado);


  int teste = strcmp(resultado_esperado, resultado);
  return teste == -1;
};

int main() {
  printf("Testando função testa_converte_pacote_para_char: %s\n", testa_converte_pacote_para_char() ? "Passou!" : "Falhou");
  return 0;
};