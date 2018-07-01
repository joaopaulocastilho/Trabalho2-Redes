#ifndef INCLUSAO_ARQUIVO_GLOBALS
#define INCLUSAO_ARQUIVO_GLOBALS

#include <stdio.h> // printf
#include <string.h>
#include <stdlib.h> // exit(0);
#include <sys/types.h> // fork()
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h> // Para usar as threads
#include <unistd.h> // Para usar mutex

#define DEBUG_PRINT_LOG 1 // Tudo que vai pro LOG também é imprimido no console

#define TIPO_PACOTE_VAZIO 0
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
#define TAMANHO_MAXIMO_ENDERECO 20
#define MAXIMO_ROTEADORES 20 // Máximo de roteadores no arquivo de roteadores

#define INFINITO 1123456789

// Pacote: |Destino 4B|Tipo 1B|Origem 4B|Payload ?B|
typedef struct {
  int destino;
  char tipo;
  int origem;
  char mensagem[TAMANHO_MENSAGEM_PACOTE];
} pacote_t;

pthread_mutex_t log_mutex;

int grava_log(char *mensagem) {
  FILE *file;
  file = fopen("./log.txt", "a");
  pthread_mutex_lock(&log_mutex);
    if (DEBUG_PRINT_LOG) {
      printf("%s\n", mensagem);
    }
    fprintf(file, "%s\n", mensagem);
  pthread_mutex_unlock(&log_mutex);
  fclose(file);
};

void die(char *s) {
  perror(s);
  exit(1);
};

/** Função que le o arquivo roteador.config e grava os dados nos vetores bidimensionais portas_roteadores e enderecos_roteadores passados por parâmetro. */
int le_roteadores(int portas_roteadores[MAXIMO_ROTEADORES], char enderecos_roteadores[MAXIMO_ROTEADORES][TAMANHO_MAXIMO_ENDERECO]) {
  FILE *arquivo_roteadores;
  int id_roteador, router_port;
  char endereco_tmp[TAMANHO_MAXIMO_ENDERECO];

  memset(portas_roteadores, -1, MAXIMO_ROTEADORES * sizeof(int));
  memset(enderecos_roteadores, 0, MAXIMO_ROTEADORES * TAMANHO_MAXIMO_ENDERECO);

  arquivo_roteadores = fopen("roteador.config", "r");
  if (!arquivo_roteadores) {
    return 1;
  }

  while(fscanf(arquivo_roteadores, "%d %d %s\n", &id_roteador, &router_port, endereco_tmp) != EOF) {
    portas_roteadores[id_roteador] = router_port;
    strcpy(enderecos_roteadores[id_roteador], endereco_tmp);
  }
  fclose(arquivo_roteadores);
  return 0;
}

/* Converte pacote para tripa de char. Resultado deve ter tamanho de
 * TAMANHO_TOTAL_PACOTE ou mais, mas só isso será escrito.
 */
int converte_pacote_para_char(pacote_t pacote, char *resultado) {
  snprintf(resultado,
          TAMANHO_TOTAL_PACOTE,
          "%c%c%c%c%c%c%c%c%c%s",
          pacote.destino >> 24,
          pacote.destino >> 16,
          pacote.destino >> 8,
          pacote.destino,
          pacote.tipo,
          pacote.origem >> 24,
          pacote.origem >> 16,
          pacote.origem >> 8,
          pacote.origem,
          pacote.mensagem);
};

#endif
