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

#define DEBUG_PRINT_LOG 0 // Tudo que vai pro LOG também é imprimido no console

#define TIPO_PACOTE_VAZIO 0
#define TIPO_PACOTE_MENSAGEM_USUARIO 1
#define TIPO_PACOTE_CONFIRMACAO_MENSAGEM_USUARIO 2
#define TIPO_PACOTE_VETOR_DISTANCIA 3
#define TIPO_PACOTE_CONFIRMACAO_VETOR_DISTANCIA 4
#define TIPO_PACOTE_CHECA_NO_ATIVO 5
#define TIPO_PACOTE_CONFIRMACAO_NO_ATIVO 6

#define TAMANHO_TOTAL_PACOTE (TAMANHO_MENSAGEM_PACOTE + 4 + 1 + 4)
#define TAMANHO_MENSAGEM_PACOTE 512
#define TAMANHO_BUFFER_IMPRESSAO (TAMANHO_BUFFER_ENTRADA)
#define TAMANHO_BUFFER_VETOR_DISTANCIA (TAMANHO_BUFFER_ENTRADA)
#define TAMANHO_CHECAGENS_RECEBIDAS (TAMANHO_BUFFER_ENTRADA)
#define TAMANHO_BUFFER_ENTRADA 516
#define TAMANHO_BUFFER_SAIDA 516
#define QUANTIDADE_MAXIMA_NOS 50
#define TAMANHO_MAXIMO_ENDERECO 20

#define INFINITO 112345678
#define TEMPO_PAUSA_EVD 5 // Tempo (em segundos) em que vamos mandar o vetor distâncias para os vizinhos.
#define TEMPO_PAUSA_RESPOSTA_CHECAGEM_VIZINHOS 5 // Tempo, em segundos, que o checador de vizinhos espera pela resposta dos vizinhos

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

char caminho_arquivo_log[100];

pthread_mutex_t log_mutex;

int grava_log(char *mensagem) {
  FILE *file;
  file = fopen(caminho_arquivo_log, "a");
  pthread_mutex_lock(&log_mutex);
    if (DEBUG_PRINT_LOG) {
      printf("\033[1m\033[32m[LOG]\033[0m%s\n", mensagem);
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
  int j = 0;
  snprintf(resultado,
          TAMANHO_TOTAL_PACOTE,
          "%c%c%c%c%c%c%c%c%c",
          pacote.destino >> 24,
          pacote.destino >> 16,
          pacote.destino >> 8,
          pacote.destino,
          pacote.tipo,
          pacote.origem >> 24,
          pacote.origem >> 16,
          pacote.origem >> 8,
          pacote.origem);
  for (int i = TAMANHO_TOTAL_PACOTE - TAMANHO_MENSAGEM_PACOTE; i < TAMANHO_TOTAL_PACOTE; i++, j++) {
    resultado[i] = pacote.mensagem[j];
  }
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
  *indice_ultimo_pacote_buffer_saida = proximo_espaco;
  pthread_mutex_unlock(buffer_saida_mutex);

  return retorno;
}

// Recebe o nodo que está chamando a função e a tabela de roteamento para atualizar as distância de nodo_atual.
void atualiza_tabela_roteamento(int nodo_atual, int tabela_roteamento[QUANTIDADE_MAXIMA_NOS][QUANTIDADE_MAXIMA_NOS], pthread_mutex_t *tabela_roteamento_mutex) {
  int i = nodo_atual, j, k, custo, menor = INFINITO;
  pthread_mutex_lock(tabela_roteamento_mutex);
  for (j = 0; j < QUANTIDADE_MAXIMA_NOS; j++) {
    for (menor = INFINITO, k = 0; k < QUANTIDADE_MAXIMA_NOS; k++) {
      custo = tabela_roteamento[i][k] + tabela_roteamento[k][j];
      if (custo < menor) menor = custo;
    }
    tabela_roteamento[i][j] = menor;
  }
  pthread_mutex_unlock(tabela_roteamento_mutex);
}

// Debug
void imprime_tabela_roteamento(int tabela_roteamento[QUANTIDADE_MAXIMA_NOS][QUANTIDADE_MAXIMA_NOS]) {
  int i, j;
  for (i = 1; i <= 5; i++) {
    printf("\n");
    for (j = 1; j <= 5; j++) {
      printf("%d ", tabela_roteamento[i][j]);
    }
  }
}

#endif
