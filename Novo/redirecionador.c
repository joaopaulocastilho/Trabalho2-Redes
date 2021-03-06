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
  // Parâmetros do buffer do Vetor Distância
  pacote_t *buffer_vetor_distancia;
  pthread_mutex_t *buffer_vetor_distancia_mutex;
  int *ultimo_pacote_buffer_vetor_distancia;

  pthread_mutex_t *checagens_recebidas_mutex;
  int *ultima_checagem_recebida;
  pacote_t *checagens_recebidas;

  pthread_mutex_t *respostas_checagem_vizinhos_mutex;
  char *respostas_checagem_vizinhos;
};

int adiciona_pacote_buffer_impressao(pacote_t pacote, struct argumentos_redirecionador_struct *argumentos);
int adiciona_pacote_buffer_vetor_distancia(pacote_t pacote, struct argumentos_redirecionador_struct *argumentos);
int adiciona_pacote_checagens_recebidas(pacote_t pacote, struct argumentos_redirecionador_struct *argumentos);
void adiciona_pacote_resposta_checagem_vizinho(pacote_t pacote, struct argumentos_redirecionador_struct *argumentos);

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
      int adicionou_pacote = adiciona_pacote_buffer_impressao(pacote_redirecionar,
                                                              argumentos);
      // Imprime no log se adicionou ou não o pacotu ao buffer de impressão
      if (adicionou_pacote) {
        sprintf(mensagem_log,
                "[REDIRECIONADOR] Pacote de origem [%d] e tipo [%d] adicionado ao buffer de impressão.",
                pacote_redirecionar.origem,
                pacote_redirecionar.tipo);
        grava_log(mensagem_log);
        continue;
      }

      sprintf(mensagem_log,
              "[REDIRECIONADOR] Falha ao tentar adicionar pacote de origem [%d] e tipo [%d] ao buffer de impressão.",
              pacote_redirecionar.origem,
              pacote_redirecionar.tipo);
      grava_log(mensagem_log);
      continue;
    }

    /* Mensagem de Vetor de Distâncias */
    if (pacote_redirecionar.tipo == TIPO_PACOTE_VETOR_DISTANCIA) {
      int adicionou_pacote = adiciona_pacote_buffer_vetor_distancia(pacote_redirecionar, argumentos);
      // Imprime no log se adicionou ou não o pacotu ao buffer de Vetores Distâncias Recebidos
      if (adicionou_pacote) {
        sprintf(mensagem_log,
          "[REDIRECIONADOR] Pacote de origem [%d] e tipo [%d] adicionado ao buffer de Vetores Distâncias Recebidos.",
          pacote_redirecionar.origem,
          pacote_redirecionar.tipo);
          grava_log(mensagem_log);
          continue;
        }

        sprintf(mensagem_log,
          "[REDIRECIONADOR] Falha ao tentar adicionar pacote de origem [%d] e tipo [%d] ao buffer de Vetores Distâncias Recebidos.",
          pacote_redirecionar.origem,
          pacote_redirecionar.tipo);
          grava_log(mensagem_log);
          continue;
    }

    /* Mensagem de checagem de vizinhos */
    if (pacote_redirecionar.tipo == TIPO_PACOTE_CHECA_NO_ATIVO) {
      int adicionou_pacote = adiciona_pacote_checagens_recebidas(
        pacote_redirecionar,
        argumentos
      );

      if (adicionou_pacote) {
        sprintf(mensagem_log,
                "[REDIRECIONADOR] Pacote de origem [%d] e tipo [%d] adicionado ao vetor de checagens recebidas.",
                pacote_redirecionar.origem,
                pacote_redirecionar.tipo);
        grava_log(mensagem_log);
        continue;
      }

      sprintf(mensagem_log,
              "[REDIRECIONADOR] Falha ao tentar adicionar pacote de origem [%d] e tipo [%d] ao vetor de checagens recebidas.",
              pacote_redirecionar.origem,
              pacote_redirecionar.tipo);
      grava_log(mensagem_log);
      continue;
    }

    /* Respostas de checagens de vizinhos */
    if (pacote_redirecionar.tipo == TIPO_PACOTE_CONFIRMACAO_NO_ATIVO) {
      adiciona_pacote_resposta_checagem_vizinho(
        pacote_redirecionar,
        argumentos
      );
      sprintf(mensagem_log,
              "[REDIRECIONADOR] Pacote de origem [%d] e tipo [%d] adicionado às respostas de checagens recebidas.",
              pacote_redirecionar.origem,
              pacote_redirecionar.tipo);
      grava_log(mensagem_log);
      continue;
    }
  } while (1);
}

int adiciona_pacote_buffer_impressao(
  pacote_t pacote,
  struct argumentos_redirecionador_struct *argumentos) {

  pacote_t ultimo_pacote_vetor; // variável auxiliar
  int proximo_ultimo_pacote_impressao;

  pthread_mutex_lock(argumentos->buffer_impressao_mutex);
    // Pega índice do próximo espaço para por
    proximo_ultimo_pacote_impressao = *(argumentos->ultimo_pacote_buffer_impressao) + 1;
    proximo_ultimo_pacote_impressao %= TAMANHO_BUFFER_IMPRESSAO;

    // Pega pacote que está na posição desejada
    ultimo_pacote_vetor = argumentos->buffer_impressao[proximo_ultimo_pacote_impressao];

    if (ultimo_pacote_vetor.tipo != TIPO_PACOTE_VAZIO) {
      // Vetor cheio
      pthread_mutex_unlock(argumentos->buffer_impressao_mutex);
      return 0;
    }

    argumentos->buffer_impressao[proximo_ultimo_pacote_impressao] = pacote;
    (*argumentos->ultimo_pacote_buffer_impressao) = proximo_ultimo_pacote_impressao;
  pthread_mutex_unlock(argumentos->buffer_impressao_mutex);
  return 1;
};

int adiciona_pacote_buffer_vetor_distancia(pacote_t pacote, struct argumentos_redirecionador_struct *argumentos) {
  pacote_t ultimo_pacote_vetor; // variável auxiliar
  int proximo_ultimo_pacote_vetor_distancia;

  pthread_mutex_lock(argumentos->buffer_vetor_distancia_mutex);
  // Pega índice do próximo espaço para por
  proximo_ultimo_pacote_vetor_distancia = *(argumentos->ultimo_pacote_buffer_vetor_distancia) + 1;
  proximo_ultimo_pacote_vetor_distancia %= TAMANHO_BUFFER_VETOR_DISTANCIA;

  // Pega pacote que está na posição desejada
  ultimo_pacote_vetor = argumentos->buffer_vetor_distancia[proximo_ultimo_pacote_vetor_distancia];

  if (ultimo_pacote_vetor.tipo != TIPO_PACOTE_VAZIO) {
    // Vetor cheio
    pthread_mutex_unlock(argumentos->buffer_vetor_distancia_mutex);
    return 0;
  }

  argumentos->buffer_vetor_distancia[proximo_ultimo_pacote_vetor_distancia] = pacote;
  (*argumentos->ultimo_pacote_buffer_vetor_distancia) = proximo_ultimo_pacote_vetor_distancia;
  pthread_mutex_unlock(argumentos->buffer_vetor_distancia_mutex);
  return 1;
}

int adiciona_pacote_checagens_recebidas(
  pacote_t pacote,
  struct argumentos_redirecionador_struct *argumentos) {

  pacote_t ultimo_pacote_vetor; // variável auxiliar
  int proximo_ultima_checagem_recebida;

  pthread_mutex_lock(argumentos->checagens_recebidas_mutex);
    // Pega índice do próximo espaço para por
    proximo_ultima_checagem_recebida = *(argumentos->ultima_checagem_recebida) + 1;
    proximo_ultima_checagem_recebida %= TAMANHO_CHECAGENS_RECEBIDAS;

    // Pega pacote que está na posição desejada
    ultimo_pacote_vetor = argumentos->checagens_recebidas[proximo_ultima_checagem_recebida];

    if (ultimo_pacote_vetor.tipo != TIPO_PACOTE_VAZIO) {
      // Vetor cheio
      pthread_mutex_unlock(argumentos->checagens_recebidas_mutex);
      return 0;
    }

    argumentos->checagens_recebidas[proximo_ultima_checagem_recebida] = pacote;
    (*argumentos->ultima_checagem_recebida) = proximo_ultima_checagem_recebida;
  pthread_mutex_unlock(argumentos->checagens_recebidas_mutex);
  return 1;
};

void adiciona_pacote_resposta_checagem_vizinho(
  pacote_t pacote,
  struct argumentos_redirecionador_struct *argumentos) {

  pthread_mutex_lock(argumentos->respostas_checagem_vizinhos_mutex);
    argumentos->respostas_checagem_vizinhos[pacote.origem] = 1;
  pthread_mutex_unlock(argumentos->respostas_checagem_vizinhos_mutex);
};

#endif
