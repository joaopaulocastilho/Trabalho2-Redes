#ifndef INCLUSAO_ARQUIVO_GLOBALS
#define INCLUSAO_ARQUIVO_GLOBALS

#include <stdio.h> // printf
#include <string.h>
#include <pthread.h> // Para usar as threads
#include <unistd.h> // Para usar mutex

// Pacote:
// |Destino 4B|Tipo 1B|Origem 4B|Payload ?B|
#define TIPO_PACOTE_MENSAGEM_USUARIO 1
#define TIPO_PACOTE_CONFIRMACAO_MENSAGEM_USUARIO 2
#define TIPO_PACOTE_VETOR_DISTANCIA 3
#define TIPO_PACOTE_CONFIRMACAO_VETOR_DISTANCIA 4
#define TIPO_PACOTE_CHECA_NO_ATIVO 5
#define TIPO_PACOTE_CONFIRMACAO_NO_ATIVO 6

#define TAMANHO_TOTAL_PACOTE (TAMANHO_MENSAGEM_PACOTE + 4 + 1 + 4)
#define TAMANHO_MENSAGEM_PACOTE 512
#define TAMANHO_BUFFER_ENTRADA 516
#define TAMANHO_BUFFER_SAIDA 516
#define QUANTIDADE_MAXIMA_NOS 50

#define INFINITO 1123456789

typedef struct {
  int destino;
  char tipo;
  int origem;
  char mensagem[TAMANHO_MENSAGEM_PACOTE];
} pacote_t;

#endif