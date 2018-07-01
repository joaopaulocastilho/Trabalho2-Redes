#ifndef INCLUSAO_ARQUIVO_REDIRECIONADOR
#define INCLUSAO_ARQUIVO_REDIRECIONADOR
#include "globais.h"

/** Redirecionador
 * Thread responsável por pegar os pacotes no buffer de entrada e colocá-los
 * nos espaços das threads que vão cuidar deles. No planejamento inicial
 * presente na imagem do plano, ele passa os pacotes para um desses espaços:
 * {Nudges Recebidos | Vetores Distância Recebidos | Bitmask ativos | "Msg :)"}
 */

struct argumentos_redirecionador_struct {
  // Parâmetros do buffer de entrada
  pacote_t *buffer_entrada;
  pthread_mutex_t *buffer_entrada_mutex;
  // Parâmetros do buffer de impressão
  pacote_t *buffer_impressao;
  pthread_mutex_t *buffer_impressao_mutex;
  int *ultimo_pacote_buffer_impressao;
};

void *redirecionador(void *args) {
  struct argumentos_redirecionador_struct* argumentos = (struct argumentos_redirecionador_struct*) args;
  pacote_t pacote_redirecionar;
  pacote_t ultimo_pacote_vetor; // Variável auxiliar
  int primeiro_pacote_buffer_entrada = 0;
  int proximo_ultimo_pacote_impressao; // Variáveil auxiliar
  char mensagem_log[1000];

  do {
    pthread_mutex_lock(argumentos->buffer_entrada_mutex);
      pacote_redirecionar = argumentos->buffer_entrada[primeiro_pacote_buffer_entrada];
      if (pacote_redirecionar.tipo == TIPO_PACOTE_VAZIO) {
        // Não tem pacote recebido
        pthread_mutex_unlock(argumentos->buffer_entrada_mutex);
        continue;
      }
      argumentos->buffer_entrada[primeiro_pacote_buffer_entrada].tipo = TIPO_PACOTE_VAZIO;
    pthread_mutex_unlock(argumentos->buffer_entrada_mutex);

    sprintf(mensagem_log,
            "[REDIRECIONADOR] Pacote de origem [%d] e tipo [%d] retirado do buffer de entrada.",
            pacote_redirecionar.origem,
            pacote_redirecionar.tipo);
    grava_log(mensagem_log);

    primeiro_pacote_buffer_entrada = (primeiro_pacote_buffer_entrada + 1) % TAMANHO_BUFFER_ENTRADA;

    /* Mensagem de usuário recebida */
    if (pacote_redirecionar.tipo == TIPO_PACOTE_MENSAGEM_USUARIO) {
      pthread_mutex_lock(argumentos->buffer_impressao_mutex);
        proximo_ultimo_pacote_impressao = *(argumentos->ultimo_pacote_buffer_impressao) + 1;
        ultimo_pacote_vetor = argumentos->buffer_impressao[proximo_ultimo_pacote_impressao];
        if (ultimo_pacote_vetor.tipo != TIPO_PACOTE_VAZIO) {
          // Vetor cheio
          pthread_mutex_unlock(argumentos->buffer_impressao_mutex);
          sprintf(mensagem_log,
            "[REDIRECIONADOR] Falha ao tentar adicionar pacote de origem [%d] e tipo [%d] ao buffer de impressão.",
            pacote_redirecionar.origem,
            pacote_redirecionar.tipo);
          grava_log(mensagem_log);
          continue;
        }

        argumentos->buffer_impressao[proximo_ultimo_pacote_impressao] = pacote_redirecionar;
        (*argumentos->ultimo_pacote_buffer_impressao) = proximo_ultimo_pacote_impressao;
      pthread_mutex_unlock(argumentos->buffer_impressao_mutex);
      sprintf(mensagem_log,
        "[REDIRECIONADOR] Pacote de origem [%d] e tipo [%d] adicionado ao buffer de impressão.",
        pacote_redirecionar.origem,
        pacote_redirecionar.tipo);
      grava_log(mensagem_log);
      continue;
    }
  } while (1);
}

#endif
