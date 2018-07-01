#include "globais.h"

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
  return 0;
};