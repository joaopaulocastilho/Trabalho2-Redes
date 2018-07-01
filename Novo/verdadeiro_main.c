#include "globais.h"
#include "transmissor.c"
#include "receptor.c"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("ERRO! Passe o ID do nodo como argumento para execução.\n");
    return 0;
  }

  int nodo_processo_id = *argv[1] - '0'; // Pega o ID do nó passado por argumento na execução

  // Buffer de saída
  pacote_t buffer_saida[TAMANHO_BUFFER_SAIDA];
  pthread_mutex_t buffer_saida_mutex;
  pthread_mutex_init(&buffer_saida_mutex, NULL);
  int ultimo_pacote_buffer_saida = 0;
  memset(buffer_saida, 0, TAMANHO_BUFFER_SAIDA * sizeof(pacote_t));

  // Buffer de entrada
  pacote_t buffer_entrada[TAMANHO_BUFFER_ENTRADA];
  pthread_mutex_t buffer_entrada_mutex;
  pthread_mutex_init(&buffer_entrada_mutex, NULL);
  int ultimo_pacote_buffer_entrada = 0;
  memset(buffer_entrada, 0, TAMANHO_BUFFER_ENTRADA * sizeof(pacote_t));

  // Vetor de saltos
  int vetor_saltos[QUANTIDADE_MAXIMA_NOS];

  /* Inicia o transmissor **************************************/
  pthread_t transmissor_thread_id;
  struct argumentos_transmissor_struct argumentos_transmissor;
  argumentos_transmissor.buffer_saida_mutex = &buffer_saida_mutex;
  argumentos_transmissor.buffer_saida = buffer_saida;
  argumentos_transmissor.tamanho_buffer_saida = TAMANHO_BUFFER_SAIDA;
  argumentos_transmissor.vetor_saltos = vetor_saltos;
  pthread_create(&transmissor_thread_id, NULL, transmissor, (void*)&argumentos_transmissor);
  /*************************************************************/

  /* Inicia o receptor *****************************************/
  pthread_t receptor_thread_id;
  struct argumentos_receptor_struct argumentos_receptor;
  argumentos_receptor.buffer_entrada_mutex = &buffer_entrada_mutex;
  argumentos_receptor.buffer_entrada = buffer_entrada;
  argumentos_receptor.tamanho_buffer_entrada = TAMANHO_BUFFER_ENTRADA;
  argumentos_receptor.ultimo_pacote_buffer_entrada = ultimo_pacote_buffer_entrada;
  pthread_create(&receptor_thread_id, NULL, receptor, (void*)&argumentos_receptor);
  /*************************************************************/

  /* Encerra as THREADS ****************************************/
  pthread_join(transmissor_thread_id, NULL);
  pthread_join(receptor_thread_id, NULL);
  /*************************************************************/
  return 0;
};
