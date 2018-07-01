#include "globais.h"
#include "transmissor.c"
#include "receptor.c"
#include "unpacker.c"

/** Função que le o arquivo roteador.config e grava os dados nos vetores bidimensionais portas_roteadores e enderecos_roteadores passados por parâmetro. */
int le_roteadores(int portas_roteadores[QUANTIDADE_MAXIMA_NOS], char enderecos_roteadores[QUANTIDADE_MAXIMA_NOS][TAMANHO_MAXIMO_ENDERECO]) {
  FILE *arquivo_roteadores;
  int id_roteador, router_port;
  char endereco_tmp[TAMANHO_MAXIMO_ENDERECO];

  memset(portas_roteadores, -1, QUANTIDADE_MAXIMA_NOS * sizeof(int));
  memset(enderecos_roteadores, 0, QUANTIDADE_MAXIMA_NOS * TAMANHO_MAXIMO_ENDERECO);

  arquivo_roteadores = fopen("roteador.config", "r");
  if (!arquivo_roteadores) {
    return 1;
  }

  while(fscanf(arquivo_roteadores, "%d %d %s\n", &id_roteador, &router_port, endereco_tmp) != EOF) {
    portas_roteadores[id_roteador] = router_port;
    strcpy(enderecos_roteadores[id_roteador], endereco_tmp);
  }
  fclose(arquivo_roteadores);
  return 0;
}

void inicializa_tabela_roteamento(int id_nodo_atual, int *quantidade_vizinhos, vizinho_t vizinhos[], int tabela_roteamento[QUANTIDADE_MAXIMA_NOS][QUANTIDADE_MAXIMA_NOS], int vetor_saltos[]) {
  int i, j, u, v, w;
  //Inicia a tabela de roteamento
  for (i = 0; i < QUANTIDADE_MAXIMA_NOS; i++)
    for (j = 0; j < QUANTIDADE_MAXIMA_NOS; j++) tabela_roteamento[i][j] = INFINITO;

  FILE *arquivo_enlaces = fopen("enlaces.config", "r");
  if (!arquivo_enlaces) die("Falha ao abrir o arquivo de enlaces");
  while(fscanf(arquivo_enlaces, "%d %d %d\n", &u, &v, &w) != EOF){
    if (v == id_nodo_atual) { v = u; u = id_nodo_atual; }
    if (u == id_nodo_atual) {
      vizinhos[*quantidade_vizinhos].id = v;
      vizinhos[(*quantidade_vizinhos)++].peso = w;
    }
  }
  fclose(arquivo_enlaces);

  //Seta os custos de um nó para ele mesmo, e o salto dele para ele mesmo
  tabela_roteamento[id_nodo_atual][id_nodo_atual] = 0;
  vetor_saltos[id_nodo_atual] = id_nodo_atual;

  //Preenche o seu vetor na tabela de roteamento
  for (i = 0; i < *quantidade_vizinhos; i++) {
    u = id_nodo_atual; v = vizinhos[i].id; w = vizinhos[i].peso;
    tabela_roteamento[u][v] = w;
    vetor_saltos[v] = v;
  }
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("ERRO! Passe o ID do nodo como argumento para execução.\n");
    return 0;
  }

  int nodo_processo_id = *argv[1] - '0'; // Pega o ID do nó passado por argumento na execução

  // Vetor de saltos
  int vetor_saltos[QUANTIDADE_MAXIMA_NOS];
  memset(vetor_saltos, -1, QUANTIDADE_MAXIMA_NOS * sizeof(int));

  // Inicia o vetor de endereços
  int portas_roteadores[QUANTIDADE_MAXIMA_NOS];
  char enderecos_roteadores[QUANTIDADE_MAXIMA_NOS][TAMANHO_MAXIMO_ENDERECO];
  le_roteadores(portas_roteadores, enderecos_roteadores);

  // Inicia tabela de roteamento e vetor de vizinhos
  int quantidade_vizinhos = 0;
  vizinho_t vizinhos[QUANTIDADE_MAXIMA_NOS];
  int tabela_roteamento[QUANTIDADE_MAXIMA_NOS][QUANTIDADE_MAXIMA_NOS];
  inicializa_tabela_roteamento(nodo_processo_id, &quantidade_vizinhos, vizinhos, tabela_roteamento, vetor_saltos);

  // Log
  pthread_mutex_init(&log_mutex, NULL);

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
  argumentos_transmissor.vetor_saltos = vetor_saltos;
  argumentos_transmissor.enderecos_vizinhos = enderecos_roteadores;
  argumentos_transmissor.portas_vizinhos = portas_roteadores;
  pthread_create(&transmissor_thread_id, NULL, transmissor, (void*)&argumentos_transmissor);
  /*************************************************************/

  /* Inicia o receptor *****************************************/
  pthread_t receptor_thread_id;
  struct argumentos_receptor_struct argumentos_receptor;
  argumentos_receptor.buffer_entrada_mutex = &buffer_entrada_mutex;
  argumentos_receptor.buffer_entrada = buffer_entrada;
  argumentos_receptor.tamanho_buffer_entrada = TAMANHO_BUFFER_ENTRADA;
  argumentos_receptor.portas_roteadores = portas_roteadores;
  argumentos_receptor.id_nodo_atual = nodo_processo_id;

  pthread_create(&receptor_thread_id, NULL, receptor, (void*)&argumentos_receptor);
  /*************************************************************/

  /* Encerra as THREADS ****************************************/
  pthread_join(transmissor_thread_id, NULL);
  pthread_join(receptor_thread_id, NULL);
  /*************************************************************/
  return 0;
};
