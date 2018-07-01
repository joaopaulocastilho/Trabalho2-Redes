#ifndef INCLUSAO_ARQUIVO_TRANSMISSOR
#define INCLUSAO_ARQUIVO_TRANSMISSOR
#include "globais.h"

/** Transmissor
 * Pega os pacotese que estão no buffer de saída e envia-os para o nó de desti-
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
  // while(1) {
    printf("Tamanho buffer saída: %d\n", argumentos->tamanho_buffer_saida);
    pthread_mutex_lock(argumentos->buffer_saida_mutex);

    pthread_mutex_unlock(argumentos->buffer_saida_mutex);
  // }
}

#endif