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

struct argumentos_receptor_struct {
  pthread_mutex_t *buffer_entrada_mutex;
  pacote_t *buffer_entrada;
  int tamanho_buffer_entrada;
  int ultimo_pacote_buffer_entrada;
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

void* receptor(void *args) {
  struct argumentos_receptor_struct* argumentos = (struct argumentos_receptor_struct*) args;
  while (1) {
    printf("Sou o receptor!\n");
    // pthread_mutex_lock(argumentos->buffer_entrada_mutex);

    // pthread_mutex_unlock(argumentos->buffer_entrada_mutex);
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

  // Buffer de entrada
  pacote_t buffer_entrada[TAMANHO_BUFFER_ENTRADA];
  pthread_mutex_t buffer_entrada_mutex;
  pthread_mutex_init(&buffer_entrada_mutex, NULL);
  int ultimo_pacote_buffer_entrada = 0;
  memset(buffer_entrada, 0, TAMANHO_BUFFER_ENTRADA * sizeof(pacote_t));

  /* Inicia o transmissor **************************************/
  pthread_t transmissor_thread_id;
  struct argumentos_transmissor_struct argumentos_transmissor;
  argumentos_transmissor.buffer_saida_mutex = &buffer_saida_mutex;
  argumentos_transmissor.buffer_saida = buffer_saida;
  argumentos_transmissor.tamanho_buffer_saida = TAMANHO_BUFFER_SAIDA;
  argumentos_transmissor.ultimo_pacote_buffer_saida = ultimo_pacote_buffer_saida;
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
