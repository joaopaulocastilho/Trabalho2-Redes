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

#define INFINITO 286331153
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
void copia_pacote_para(pacote_t *pacote_destino, pacote_t pacote_origem);

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
  memcpy(pacote->mensagem, cadeia + 9, TAMANHO_MENSAGEM_PACOTE);
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
    copia_pacote_para(&buffer_saida[proximo_espaco], pacote);
  }
  *indice_ultimo_pacote_buffer_saida = proximo_espaco;
  pthread_mutex_unlock(buffer_saida_mutex);

  return retorno;
}

// Recebe o nodo que está chamando a função e a tabela de roteamento para atualizar as distância de nodo_atual.
void atualiza_vetor_distancia(int id_nodo_atual,
                              int tabela_roteamento[QUANTIDADE_MAXIMA_NOS][QUANTIDADE_MAXIMA_NOS],
                              pthread_mutex_t *tabela_roteamento_mutex) {
  int i = id_nodo_atual, j, k, custo, menor = INFINITO;
  pthread_mutex_lock(tabela_roteamento_mutex);
  //printf("atualizando tabela de roteamento: ");
  for (j = 0; j < QUANTIDADE_MAXIMA_NOS; j++) {
    //if (j < 7) printf("[%d,", tabela_roteamento[i][j] == INFINITO ? -1 : tabela_roteamento[i][j]);
    for (menor = INFINITO, k = 0; k < QUANTIDADE_MAXIMA_NOS; k++) {
      custo = tabela_roteamento[i][k] + tabela_roteamento[k][j];
      if (custo < menor) menor = custo;
    }
    tabela_roteamento[i][j] = menor;
    //if (j < 7) printf("%d] ", tabela_roteamento[i][j] == INFINITO ? -1 : tabela_roteamento[i][j]);
  }
  //printf("\n");
  pthread_mutex_unlock(tabela_roteamento_mutex);
}

// Debug
void imprime_tabela_roteamento(int tabela_roteamento[QUANTIDADE_MAXIMA_NOS][QUANTIDADE_MAXIMA_NOS], int vetor_saltos[]) {
  int i, j;
  printf("----------------------------------------------------------------\n");
  for (i = 1; i <= 6; i++) {
    printf("\n");
    for (j = 1; j <= 6; j++) {
      printf("%5d ", tabela_roteamento[i][j] == INFINITO ? -1 : tabela_roteamento[i][j]);
    }
  }
  printf("\n[");
  for (i = 1; i <= 6; i++) printf("%5d, ", vetor_saltos[i]);
  printf("]\n");
  printf("\n----------------------------------------------------------------\n");
}

/* Função que coloca informações do pacote na string saida */
void informacoes_pacote(pacote_t pacote_parametro, char *saida) {
  char string_distancias[500]; // Variável auxiliar
  int distancias[10]; // Variável auxiliar
  int inicio_mensagem; // Variável auxiliar
  pacote_t pacote;

  copia_pacote_para(&pacote, pacote_parametro);

  switch (pacote.tipo) {
    case TIPO_PACOTE_VAZIO:
      sprintf(saida, "Pacote vazio");
      break;
    case TIPO_PACOTE_MENSAGEM_USUARIO:
      sprintf(saida,
              "Mensagem de usuário - Destino: [%d] - Origem: [%d] - Mensagem: [%s]",
              pacote.destino,
              pacote.origem,
              pacote.mensagem);
      break;
    case TIPO_PACOTE_CONFIRMACAO_MENSAGEM_USUARIO:
      sprintf(saida,
              "Confirmação de recebimento de mensagem - Destino: [%d] - Origem: [%d]",
              pacote.destino,
              pacote.origem);
      break;
    case TIPO_PACOTE_VETOR_DISTANCIA:
      // 10 é um número arbitrário pra não encher o log de números
      for (int i = 0; i < 10; i++) {
        distancias[i] = ((int)pacote.mensagem[i * 4]) << 24;
        distancias[i] |= ((int)pacote.mensagem[i * 4 + 1]) << 16;
        distancias[i] |= ((int)pacote.mensagem[i * 4 + 2]) << 8;
        distancias[i] |= ((int)pacote.mensagem[i * 4 + 3]);
      }
      sprintf(string_distancias,
              "[0: %d][1: %d][2: %d][3: %d][4: %d][5: %d][6: %d][7: %d][8: %d][9: %d]",
              distancias[0], distancias[1], distancias[2], distancias[3],
              distancias[4], distancias[5], distancias[6], distancias[7],
              distancias[8], distancias[9]);
      sprintf(saida,
              "Vetor distância - Destino: [%d] - Origem: [%d] - Distâncias: [%s]",
              pacote.destino,
              pacote.origem,
              string_distancias);
      break;
    case TIPO_PACOTE_CONFIRMACAO_VETOR_DISTANCIA:
      sprintf(saida,
              "Confirmação de recebimento de vetor distância - Destino: [%d] - Origem: [%d]",
              pacote.destino,
              pacote.origem);
      break;
    case TIPO_PACOTE_CHECA_NO_ATIVO:
      sprintf(saida,
              "Checagem de vizinho - Destino: [%d] - Origem: [%d]",
              pacote.destino,
              pacote.origem);
      break;
    case TIPO_PACOTE_CONFIRMACAO_NO_ATIVO:
      sprintf(saida,
              "Resposta de checagem de vizinho - Destino: [%d] - Origem: [%d]",
              pacote.destino,
              pacote.origem);
      break;
    default:
      sprintf(saida, "Pacote inválido");
  }
}

void copia_pacote_para(pacote_t *pacote_destino, pacote_t pacote_origem) {
  pacote_destino->destino = pacote_origem.destino;
  pacote_destino->tipo = pacote_origem.tipo;
  pacote_destino->origem = pacote_origem.origem;
  memcpy(pacote_destino->mensagem, pacote_origem.mensagem, TAMANHO_MENSAGEM_PACOTE);
};

#endif
