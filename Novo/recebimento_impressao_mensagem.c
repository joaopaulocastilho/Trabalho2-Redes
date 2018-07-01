#ifndef INCLUSAO_ARQUIVO_RECEBIMENTO_IMPRESSAO_MENSAGEM
#define INCLUSAO_ARQUIVO_RECEBIMENTO_IMPRESSAO_MENSAGEM
#include "globais.h"

/** Recebimento e Impressão de Mensagem (RIM)
 * Thread responsável por imprimir uma mensagem enviada de um usuário a partir
 * de outro roteador e também de responder ao roteador com uma confirmação de
 * recebimento. Tem acesso ao Buffer de Impressão para pegar os pacotes que de-
 * vem ser impressos no console e ao Buffer de Saída, para adicionar o pacote
 * de confirmação.
 */

struct argumentos_rim_struct {
  pacote_t *buffer_impressao;
  pthread_mutex_t *buffer_impressao_mutex;
  pacote_t *buffer_saida;
  pthread_mutex_t *buffer_saida_mutex;
  int *ultimo_pacote_buffer_saida;
  int id_nodo_atual;
};

pacote_t cria_pacote_resposta(pacote_t pacote_recebimento, int id_nodo_atual);
void imprime_pacote(pacote_t pacote_recebido);

void *recebimento_impressao_mensagem(void *args) {
  struct argumentos_rim_struct* argumentos = (struct argumentos_rim_struct*) args;
  int primeiro_pacote_buffer_impressao = 0;
  pacote_t pacote_impressao; // Variável auxiliar
  pacote_t pacote_resposta; // Variável auxiliar
  char mensagem_log[1000];
  int retorno_enfileiramento_saida; // Variável auxiliar

  do {
    //lock no buffer de impressao
    pthread_mutex_lock(argumentos->buffer_impressao_mutex);
      pacote_impressao = argumentos->buffer_impressao[primeiro_pacote_buffer_impressao];

      if (pacote_impressao.tipo == TIPO_PACOTE_VAZIO) {
        pthread_mutex_unlock(argumentos->buffer_impressao_mutex);
        continue;
      }

      argumentos->buffer_impressao[primeiro_pacote_buffer_impressao].tipo = TIPO_PACOTE_VAZIO;

    pthread_mutex_unlock(argumentos->buffer_impressao_mutex);

    primeiro_pacote_buffer_impressao++;
    primeiro_pacote_buffer_impressao %= TAMANHO_BUFFER_IMPRESSAO;

    sprintf(mensagem_log,
      "[RECEBIMENTO E IMPRESSÃO DE MENSAGEM] Pacote de origem [%d] e mensagem [%s] retirado do buffer de impressão",
      pacote_impressao.origem,
      pacote_impressao.mensagem);
    grava_log(mensagem_log);

    imprime_pacote(pacote_impressao);

    pacote_resposta = cria_pacote_resposta(pacote_impressao, argumentos->id_nodo_atual);

    retorno_enfileiramento_saida = enfileira_pacote_para_envio(
      pacote_resposta,
      argumentos->buffer_saida,
      argumentos->buffer_saida_mutex,
      argumentos->ultimo_pacote_buffer_saida
    );
    if (retorno_enfileiramento_saida) {
      sprintf(mensagem_log,
             "[RECEBIMENTO E IMPRESSÃO DE MENSAGEM] Pacote de confirmação de recebimento de mensagem com destino [%d] adicionado ao buffer de saída",
             pacote_resposta.destino);
    } else {
      sprintf(mensagem_log,
       "[RECEBIMENTO E IMPRESSÃO DE MENSAGEM] Falha ao tentar adicionar pacote de confirmação de recebimento de mensagem com destino [%d]",
       pacote_resposta.destino);
    }
    grava_log(mensagem_log);

  } while(1);
}

pacote_t cria_pacote_resposta(pacote_t pacote_recebimento, int id_nodo_atual) {
  pacote_t pacote_retorno;
  pacote_retorno.origem = id_nodo_atual;
  pacote_retorno.destino = pacote_recebimento.origem;
  pacote_retorno.tipo = TIPO_PACOTE_CONFIRMACAO_MENSAGEM_USUARIO;
  pacote_retorno.mensagem[0] = '\0';
  return pacote_retorno;
};

void imprime_pacote(pacote_t pacote_recebido) {
  printf("Pacote recebido: Origem [%d] - Mensagem: [%s]\n",
         pacote_recebido.origem,
         pacote_recebido.mensagem);
};

#endif
