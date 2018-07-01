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

#define INFINITO 1123456789

// Pacote: |Destino 4B|Tipo 1B|Origem 4B|Payload ?B|
typedef struct {
  int destino;
  char tipo;
  int origem;
  char mensagem[TAMANHO_MENSAGEM_PACOTE];
} pacote_t;

typedef struct {
  int id, peso;
} vizinho_t;

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

/* Converte pacote para tripa de char. Resultado deve ter tamanho de
 * TAMANHO_TOTAL_PACOTE ou mais, mas só isso será escrito.
 */
void converte_pacote_para_char(pacote_t pacote, char *resultado) {
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

/* Converte uma cadeia de caracteres para o pacote referenciado pelo ponteiro
 * pacote passado por parâmetro.
 */
void converte_char_para_pacote(char *cadeia, pacote_t *pacote) {
  pacote->destino = cadeia[0] << 24;
  pacote->destino |= cadeia[1] << 16;
  pacote->destino |= cadeia[2] << 8;
  pacote->destino |= cadeia[3];
  pacote->tipo = cadeia[4];
  pacote->origem = cadeia[5] << 24;
  pacote->origem |= cadeia[6] << 16;
  pacote->origem |= cadeia[7] << 8;
  pacote->origem |= cadeia[8];
  strncpy(pacote->mensagem, cadeia + 9, TAMANHO_MENSAGEM_PACOTE);
};

/* Função única para adicionar pacote ao buffer de saída. Retorna 0 se falhou
 * ou 1 caso deu certo
 */
int enfileira_pacote_para_envio(pacote_t pacote,
                                 pacote_t *buffer_saida,
                                 pthread_mutex_t *buffer_saida_mutex,
                                 int *indice_ultimo_pacote_buffer_saida) {
  int proximo_espaco;
  pacote_t pacote_checado;
  int retorno = 1;

  pthread_mutex_lock(buffer_saida_mutex);
  proximo_espaco = ((*indice_ultimo_pacote_buffer_saida) + 1) % TAMANHO_BUFFER_SAIDA;
  pacote_checado = buffer_saida[proximo_espaco];
  if (pacote_checado.tipo != TIPO_PACOTE_VAZIO) {
    retorno = 0;
  } else {
    buffer_saida[proximo_espaco] = pacote;
  }
  pthread_mutex_unlock(buffer_saida_mutex);

  return retorno;
}

#endif
