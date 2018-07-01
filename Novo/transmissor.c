#ifndef INCLUSAO_ARQUIVO_TRANSMISSOR
#define INCLUSAO_ARQUIVO_TRANSMISSOR
#include "globais.h"

/** Transmissor
 * Pega os pacotes que estão no buffer de saída e envia-os para o nó de desti-
 * no. Envia de acordo com o vetor de saltos
 */

struct argumentos_transmissor_struct {
  pthread_mutex_t *buffer_saida_mutex;
  pacote_t *buffer_saida;
  int tamanho_buffer_saida;
  int *vetor_saltos;
};

void *transmissor(void *args) {
  struct argumentos_transmissor_struct* argumentos = (struct argumentos_transmissor_struct*) args;
  int indice_primeiro_pacote = 0;
  pacote_t pacote_envio;
  int proximo_salto_pacote;
  char mensagem_log[500];

  do {
    /* Pega o pacote no buffer de saída */
    pthread_mutex_lock(argumentos->buffer_saida_mutex);
      pacote_envio = argumentos->buffer_saida[indice_primeiro_pacote];
      if (pacote_envio.tipo == TIPO_PACOTE_VAZIO) {
        // Ainda não tem pacote para enviar
        pthread_mutex_unlock(argumentos->buffer_saida_mutex);
        continue;
      }
      argumentos->buffer_saida[indice_primeiro_pacote].tipo = TIPO_PACOTE_VAZIO;
      indice_primeiro_pacote = (indice_primeiro_pacote + 1) % argumentos->tamanho_buffer_saida;
    pthread_mutex_unlock(argumentos->buffer_saida_mutex);

    /* Busca o próximo salto para o nó de destino do pacote */
    proximo_salto_pacote = argumentos->vetor_saltos[pacote_envio.destino];
    if (proximo_salto_pacote == -1) {
      sprintf(mensagem_log, "[TRANSMISSOR] Tentativa de envio para [%d] com pacote do tipo [%d] falhou. O próximo salto ainda não foi descoberto.", pacote_envio.destino, pacote_envio.tipo);
      grava_log(mensagem_log);
      continue;
    }

    // ... Terminar ...


  } while(0); // TODO: Mudar para 1 depois que estiver pronto
}

#endif