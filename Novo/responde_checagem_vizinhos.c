#ifndef INCLUSAO_ARQUIVO_RESPONDE_CHECAGEM_VIZINHOS
#define INCLUSAO_ARQUIVO_RESPONDE_CHECAGEM_VIZINHOS
#include "globais.h"

/** Responde checagem de vizinhos
 * Thread responsável por enviar uma resposta à checagem dos vizinhos, para si-
 * nalizar que o nó e a aresta ainda está ativa.
 */

struct argumentos_responde_checagem_vizinhos_struct {
  pthread_mutex_t *checagens_recebidas_mutex;
  pacote_t *checagens_recebidas;
  int id_nodo_atual;
  pacote_t *buffer_saida;
  pthread_mutex_t *buffer_saida_mutex;
  int *ultimo_pacote_buffer_saida;
};

pacote_t cria_pacote_resposta_checagem(int id_nodo_atual, int id_nodo_destino);

pacote_t cria_pacote_checagem(int id_nodo_atual, int id_destino);

void* responde_checagem_vizinhos(void *args) {
  struct argumentos_responde_checagem_vizinhos_struct *argumentos = (struct argumentos_responde_checagem_vizinhos_struct*)args;
  pacote_t pacote_checagem; // Variável auxiliar
  int primeiro_pacote_checagens_recebidas = 0;
  char mensagem_log[1000];
  pacote_t pacote_resposta;
  int resultado_enfileiramento_buffer_saida; // Variável auxiliar

  do {
    pthread_mutex_lock(argumentos->checagens_recebidas_mutex);
    pacote_checagem = argumentos->checagens_recebidas[primeiro_pacote_checagens_recebidas];
    argumentos->checagens_recebidas[primeiro_pacote_checagens_recebidas].tipo = TIPO_PACOTE_VAZIO;
    pthread_mutex_unlock(argumentos->checagens_recebidas_mutex);

    // Se o pacote que pegou estava vazio, volta ao começo do loop
    if (pacote_checagem.tipo == TIPO_PACOTE_VAZIO) {
      continue;
    }

    primeiro_pacote_checagens_recebidas++;
    primeiro_pacote_checagens_recebidas %= TAMANHO_CHECAGENS_RECEBIDAS;

    pacote_resposta = cria_pacote_resposta_checagem(
      argumentos->id_nodo_atual,
      pacote_checagem.origem
    );

    /* Adiciona o pacote ao buffer de saída */
    resultado_enfileiramento_buffer_saida = enfileira_pacote_para_envio(
      pacote_resposta,
      argumentos->buffer_saida,
      argumentos->buffer_saida_mutex,
      argumentos->ultimo_pacote_buffer_saida
    );

    if (resultado_enfileiramento_buffer_saida) {
      sprintf(mensagem_log,
             "[RESPONDE CHECAGEM VIZINHOS] Pacote de resposta à checagem do vizinho [%d] adicionado ao Buffer de saída",
             pacote_checagem.origem);
    } else {
      sprintf(mensagem_log,
             "[RESPONDE CHECAGEM VIZINHOS] Falha ao tentar adicionar pacote de resposta à checagem do vizinho [%d] no Buffer de saída",
             pacote_checagem.origem);
    }
    grava_log(mensagem_log);

  } while(1);
}

pacote_t cria_pacote_resposta_checagem(int id_nodo_atual, int id_nodo_destino) {
  pacote_t pacote_retorno;
  pacote_retorno.origem = id_nodo_atual;
  pacote_retorno.destino = id_nodo_destino;
  pacote_retorno.tipo = TIPO_PACOTE_CONFIRMACAO_NO_ATIVO;
  pacote_retorno.mensagem[0] = '\0';
  return pacote_retorno;
};

#endif
