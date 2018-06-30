#include <stdio.h> // printf
#include <string.h>
#include <pthread.h> // Para usar as threads
#include <unistd.h> // Para usar mutex

// Pacote:
// |Destino 4B|Tipo 1B|Origem 4B|Payload ?B|
#define TIPO_PACOTE_MENSAGEM_USUARIO 1
#define TIPO_PACOTE_CONFIRMACAO_MENSAGEM_USUARIO 2
#define TIPO_PACOTE_VETOR_DISTANCIA 3
#define TIPO_PACOTE_CONFIRMACAO_VETOR_DISTANCIA 4
#define TIPO_PACOTE_CHECA_NO_ATIVO 5
#define TIPO_PACOTE_CONFIRMACAO_NO_ATIVO 6

#define TAMANHO_TOTAL_PACOTE (TAMANHO_MENSAGEM_PACOTE + 4 + 1 + 4)
#define TAMANHO_MENSAGEM_PACOTE 512
#define TAMANHO_BUFFER_ENTRADA 516
#define TAMANHO_BUFFER_SAIDA 516
#define QUANTIDADE_MAXIMA_NOS 50

#define INFINITO 1123456789

typedef struct {
  int destino;
  char tipo;
  int origem;
  char mensagem[TAMANHO_MENSAGEM_PACOTE];
} pacote_t;

struct argumentos_transmissor_struct {
  pthread_mutex_t *buffer_saida_mutex;
  pacote_t *buffer_saida;
  int tamanho_buffer_saida;
  int ultimo_pacote_buffer_saida;
};

void *transmissor(void *args) {
  struct argumentos_transmissor_struct* argumentos = (struct argumentos_transmissor_struct*) args;
  while(1) {
    printf("Tamanho buffer saída: %d\n", argumentos->tamanho_buffer_saida);
    // pthread_mutex_lock(argumentos->buffer_saida_mutex);

    // pthread_mutex_unlock(argumentos->buffer_saida_mutex);
  return NULL;
  }
}

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


  /* Inicia o transmissor **************************************/
  pthread_t transmissor_thread_id;
  struct argumentos_transmissor_struct argumentos_transmissor;
  argumentos_transmissor.buffer_saida_mutex = &buffer_saida_mutex;
  argumentos_transmissor.buffer_saida = buffer_saida;
  argumentos_transmissor.tamanho_buffer_saida = TAMANHO_BUFFER_SAIDA;
  argumentos_transmissor.ultimo_pacote_buffer_saida = ultimo_pacote_buffer_saida;
  pthread_create(&transmissor_thread_id, NULL, transmissor, (void*)&argumentos_transmissor);
  /*************************************************************/


  /* Encerra as THREADS ****************************************/
  pthread_join(transmissor_thread_id, NULL);
  /*************************************************************/
  return 0;
};
