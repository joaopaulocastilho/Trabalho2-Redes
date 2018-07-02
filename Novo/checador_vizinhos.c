#ifndef INCLUSAO_ARQUIVO_CHECADOR_VIZINHOS
#define INCLUSAO_ARQUIVO_CHECADOR_VIZINHOS
#include "globais.h"

/** Checador de vizinhos
 * Thread responsável por mandar um pacote do tipo TIPO_PACOTE_CHECA_NO_ATIVO para todos os vizinhos. A thread dorme por TEMPO_PAUSA_RESPOSTA_CHECAGEM_VIZINHOS segundos. Depois checa no vetor_checagem_vizinhos_ativos quais vizinhos responderam. Atualiza a distância dos que não responderam para INFINITO no vetor de vizinhos.
 */

struct argumentos_checador_vizinhos_struct {
  pthread_mutex_t *respostas_checagem_vizinhos_mutex;
  char *respostas_checagem_vizinhos;
  int quantidade_vizinhos;
  vizinho_t *vizinhos;
  // Parâmeteros do buffer de saída
  pacote_t *buffer_saida;
  pthread_mutex_t *buffer_saida_mutex;
  int *ultimo_pacote_buffer_saida;
  // Parâmetros da tabela de roteamento
  pthread_mutex_t *tabela_roteamento_mutex;
  int (*tabela_roteamento)[QUANTIDADE_MAXIMA_NOS];
  // Parâmetros gerais
  int id_nodo_atual;
};

pacote_t cria_pacote_checagem(int id_nodo_atual, int id_destino);

void* checador_vizinhos(void *args) {
  struct argumentos_checador_vizinhos_struct *argumentos = (struct argumentos_checador_vizinhos_struct*)args;
  pacote_t pacote_checagem;
  int vizinhos_ausentes[QUANTIDADE_MAXIMA_NOS];
  int quantidade_vizinhos_ausentes;
  char mensagem_log[1000];
  do {
    sprintf(mensagem_log, "[CHECADOR VIZINHOS] Criando pacotes de checagem.");
    grava_log(mensagem_log);

    // Limpa vetor de resposta dos vizinhos
    pthread_mutex_lock(argumentos->respostas_checagem_vizinhos_mutex);
      memset(argumentos->respostas_checagem_vizinhos, 0, QUANTIDADE_MAXIMA_NOS);
    pthread_mutex_unlock(argumentos->respostas_checagem_vizinhos_mutex);

    // Envia pacotes de checagens
    for (int i = 0; i < argumentos->quantidade_vizinhos; i++) {
      int id_vizinho = argumentos->vizinhos[i].id;
      pacote_checagem = cria_pacote_checagem(argumentos->id_nodo_atual, id_vizinho);
      enfileira_pacote_para_envio(pacote_checagem,
                                  argumentos->buffer_saida,
                                  argumentos->buffer_saida_mutex,
                                  argumentos->ultimo_pacote_buffer_saida);
    }

    // Dorme
    sleep(TEMPO_PAUSA_RESPOSTA_CHECAGEM_VIZINHOS);

    // Checa quem respondeu
    quantidade_vizinhos_ausentes = 0;
    pthread_mutex_lock(argumentos->respostas_checagem_vizinhos_mutex);
      for (int i = 0; i < argumentos->quantidade_vizinhos; i++) {
        int id_vizinho = argumentos->vizinhos[i].id;

        if (!argumentos->respostas_checagem_vizinhos[id_vizinho]) {
          vizinhos_ausentes[quantidade_vizinhos_ausentes++] = id_vizinho;
        }
      }
    pthread_mutex_unlock(argumentos->respostas_checagem_vizinhos_mutex);

    // Coloca distância infinita pra quem não respondeu
    pthread_mutex_lock(argumentos->tabela_roteamento_mutex);
      for (int i = 0; i < quantidade_vizinhos_ausentes; i++) {
        argumentos->tabela_roteamento[argumentos->id_nodo_atual][vizinhos_ausentes[i]] = INFINITO;
      }
    pthread_mutex_unlock(argumentos->tabela_roteamento_mutex);

    // Marca no LOG quem não respondeu
    for (int i = 0; i < quantidade_vizinhos_ausentes; i++) {
      sprintf(mensagem_log,
              "[CHECADOR VIZINHOS] Vizinho [%d] não respondeu. Recebeu distância infinita.",
              vizinhos_ausentes[i]);
      grava_log(mensagem_log);
    }
  } while(1);
}

pacote_t cria_pacote_checagem(int id_nodo_atual, int id_destino) {
  pacote_t pacote_retorno;
  pacote_retorno.destino = id_destino;
  pacote_retorno.origem = id_nodo_atual;
  pacote_retorno.tipo = TIPO_PACOTE_CHECA_NO_ATIVO;
  pacote_retorno.mensagem[0] = '\0';
  return pacote_retorno;
}

#endif
