#include "globais.h"
#include "transmissor.c"
#include "receptor.c"
#include "redirecionador.c"
#include "interface_envio_mensagem.c"
#include "recebimento_impressao_mensagem.c"
#include "envia_vetor_distancias.c"
#include "atualiza_tabela_roteamento_vetor_saltos.c"
#include "checador_vizinhos.c"
#include "responde_checagem_vizinhos.c"

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
  printf("Iniciando processo...\n");

  int id_nodo_atual = *argv[1] - '0'; // Pega o ID do nó passado por argumento na execução

  // Vetor de saltos
  printf("Inicializando vetor de saltos...\n");
  int vetor_saltos[QUANTIDADE_MAXIMA_NOS];
  pthread_mutex_t vetor_saltos_mutex;
  pthread_mutex_init(&vetor_saltos_mutex, NULL);
  memset(vetor_saltos, -1, QUANTIDADE_MAXIMA_NOS * sizeof(int));

  // Inicia o vetor de endereços
  printf("Lendo endereços...\n");
  int portas_roteadores[QUANTIDADE_MAXIMA_NOS];
  char enderecos_roteadores[QUANTIDADE_MAXIMA_NOS][TAMANHO_MAXIMO_ENDERECO];
  le_roteadores(portas_roteadores, enderecos_roteadores);

  // Inicia tabela de roteamento e vetor de vizinhos
  printf("Inicializando tabela de roteamento e vetor de vizinhos...\n");
  int quantidade_vizinhos = 0;
  vizinho_t vizinhos[QUANTIDADE_MAXIMA_NOS];
  pthread_mutex_t tabela_roteamento_mutex;
  pthread_mutex_init(&tabela_roteamento_mutex, NULL);
  int tabela_roteamento[QUANTIDADE_MAXIMA_NOS][QUANTIDADE_MAXIMA_NOS];
  inicializa_tabela_roteamento(id_nodo_atual, &quantidade_vizinhos, vizinhos, tabela_roteamento, vetor_saltos);

  // Log
  sprintf(caminho_arquivo_log, "./log%d.txt", id_nodo_atual);
  pthread_mutex_init(&log_mutex, NULL);

  // Buffer de saída
  printf("Inicializando buffer de saída...\n");
  pacote_t buffer_saida[TAMANHO_BUFFER_SAIDA];
  pthread_mutex_t buffer_saida_mutex;
  pthread_mutex_init(&buffer_saida_mutex, NULL);
  int ultimo_pacote_buffer_saida = -1;
  for (int i = 0; i < TAMANHO_BUFFER_SAIDA; i++) {
    buffer_saida[i].tipo = TIPO_PACOTE_VAZIO;
  }

  // Buffer de impressão
  printf("Inicializando buffer de impressão...\n");
  pacote_t buffer_impressao[TAMANHO_BUFFER_IMPRESSAO];
  pthread_mutex_t buffer_impressao_mutex;
  pthread_mutex_init(&buffer_impressao_mutex, NULL);
  int ultimo_pacote_buffer_impressao = -1;
  for (int i = 0; i < TAMANHO_BUFFER_IMPRESSAO; i++) {
    buffer_impressao[i].tipo = TIPO_PACOTE_VAZIO;
  }

  // Buffer de Vetor Distância
  printf("Inicializando buffer de Vetor Distância...\n");
  pacote_t buffer_vetor_distancia[TAMANHO_BUFFER_VETOR_DISTANCIA];
  pthread_mutex_t buffer_vetor_distancia_mutex;
  pthread_mutex_init(&buffer_vetor_distancia_mutex, NULL);
  int ultimo_pacote_buffer_vetor_distancia = -1;
  for (int i = 0; i < TAMANHO_BUFFER_VETOR_DISTANCIA; i++) {
    buffer_vetor_distancia[i].tipo = TIPO_PACOTE_VAZIO;
  }

  // Buffer de entrada
  printf("Inicializando buffer de entrada...\n");
  pacote_t buffer_entrada[TAMANHO_BUFFER_ENTRADA];
  pthread_mutex_t buffer_entrada_mutex;
  pthread_mutex_init(&buffer_entrada_mutex, NULL);
  int ultimo_pacote_buffer_entrada = 0;
  memset(buffer_entrada, 0, TAMANHO_BUFFER_ENTRADA * sizeof(pacote_t));

  // Vetor de respostas de checagens de vizinnhos
  printf("Inicializando vetor de resposta de checagem dos vizinhos...\n");
  pthread_mutex_t respostas_checagem_vizinhos_mutex;
  pthread_mutex_init(&respostas_checagem_vizinhos_mutex, NULL);
  char respostas_checagem_vizinhos[QUANTIDADE_MAXIMA_NOS];

  // Vetor de checagens recebidas
  printf("Inicializando o vetor de checagens recebidas...\n");
  pthread_mutex_t checagens_recebidas_mutex;
  pthread_mutex_init(&checagens_recebidas_mutex, NULL);
  pacote_t checagens_recebidas[TAMANHO_CHECAGENS_RECEBIDAS];
  int ultima_checagem_recebida = -1;

  printf("Iniciando threads...\n");

  /* Inicia o transmissor **************************************/
  pthread_t transmissor_thread_id;
  struct argumentos_transmissor_struct argumentos_transmissor;
  argumentos_transmissor.buffer_saida_mutex = &buffer_saida_mutex;
  argumentos_transmissor.buffer_saida = buffer_saida;
  argumentos_transmissor.vetor_saltos = vetor_saltos;
  argumentos_transmissor.enderecos_vizinhos = enderecos_roteadores;
  argumentos_transmissor.portas_vizinhos = portas_roteadores;
  pthread_create(&transmissor_thread_id, NULL, transmissor, (void*)&argumentos_transmissor);
  /*************************************************************/

  /* Inicia o receptor *****************************************/
  pthread_t receptor_thread_id;
  struct argumentos_receptor_struct argumentos_receptor;
  argumentos_receptor.buffer_entrada_mutex = &buffer_entrada_mutex;
  argumentos_receptor.buffer_saida_mutex = &buffer_saida_mutex;
  argumentos_receptor.buffer_entrada = buffer_entrada;
  argumentos_receptor.buffer_saida = buffer_saida;
  argumentos_receptor.portas_roteadores = portas_roteadores;
  argumentos_receptor.id_nodo_atual = id_nodo_atual;
  argumentos_receptor.indice_ultimo_pacote_buffer_saida = &ultimo_pacote_buffer_saida;
  pthread_create(&receptor_thread_id, NULL, receptor, (void*)&argumentos_receptor);
  /*************************************************************/

  /* Interface de Envio de Mensagem (IEM) **********************/
  pthread_t iem_thread_id;
  // Parâmetros do buffer de saída
  struct argumentos_iem_struct argumentos_interface_envio_mensagem;
  argumentos_interface_envio_mensagem.buffer_saida = buffer_saida;
  argumentos_interface_envio_mensagem.buffer_saida_mutex = &buffer_saida_mutex;
  argumentos_interface_envio_mensagem.ultimo_pacote_buffer_saida = &ultimo_pacote_buffer_saida;
  // Parâmetros gerais
  argumentos_interface_envio_mensagem.id_nodo_atual = id_nodo_atual;

  pthread_create(&iem_thread_id,
                 NULL,
                 interface_envio_mensagem,
                 (void*)&argumentos_interface_envio_mensagem);
  /**************************** Interface de Envio de Mensagem */

  /* Redirecionador ********************************************/
  pthread_t redirecionador_thread_id;
  // Parâmetros do buffer de entrada
  struct argumentos_redirecionador_struct argumentos_redirecionador;
  argumentos_redirecionador.buffer_entrada = buffer_entrada;
  argumentos_redirecionador.buffer_entrada_mutex = &buffer_entrada_mutex;
  //Parâmetros do buffer de impressão
  argumentos_redirecionador.buffer_impressao = buffer_impressao;
  argumentos_redirecionador.buffer_impressao_mutex = &buffer_impressao_mutex;
  argumentos_redirecionador.ultimo_pacote_buffer_impressao = &ultimo_pacote_buffer_impressao;
  // Parâmetros do buffer de Vetor Distância
  argumentos_redirecionador.buffer_vetor_distancia = buffer_vetor_distancia;
  argumentos_redirecionador.buffer_vetor_distancia_mutex = &buffer_vetor_distancia_mutex;
  argumentos_redirecionador.ultimo_pacote_buffer_vetor_distancia = &ultimo_pacote_buffer_vetor_distancia;
  // Parâmetros do vetor de checagens recebidas
  argumentos_redirecionador.checagens_recebidas_mutex = &checagens_recebidas_mutex;
  argumentos_redirecionador.ultima_checagem_recebida = &ultima_checagem_recebida;
  argumentos_redirecionador.checagens_recebidas = checagens_recebidas;
  // Parâmetros do vetor de respostas de checagens
  argumentos_redirecionador.respostas_checagem_vizinhos_mutex = &respostas_checagem_vizinhos_mutex;
  argumentos_redirecionador.respostas_checagem_vizinhos = respostas_checagem_vizinhos;

  pthread_create(&redirecionador_thread_id,
                 NULL,
                 redirecionador,
                 (void*)&argumentos_redirecionador);
  /******************************************** Redirecionador */

  /* Recebimento e Impressão de Mensagens **********************/
  pthread_t recebimento_impressao_mensagem_thread_id;
  struct argumentos_rim_struct argumentos_rim;
  argumentos_rim.buffer_impressao = buffer_impressao;
  argumentos_rim.buffer_impressao_mutex = &buffer_impressao_mutex;
  argumentos_rim.buffer_saida = buffer_saida;
  argumentos_rim.buffer_saida_mutex = &buffer_saida_mutex;
  argumentos_rim.ultimo_pacote_buffer_saida = &ultimo_pacote_buffer_saida;
  argumentos_rim.id_nodo_atual = id_nodo_atual;

  pthread_create(&recebimento_impressao_mensagem_thread_id,
                 NULL,
                 recebimento_impressao_mensagem,
                 (void*)&argumentos_rim);
  /********************** Recebimento e Impressão de Mensagens */

  /* Envia o vetor de distâncias periódicamente (evd) **********/
  pthread_t envia_vetor_distancias_id;
  struct argumentos_evd_struct argumentos_evd;
  argumentos_evd.tabela_roteamento = tabela_roteamento;
  argumentos_evd.tabela_roteamento_mutex = &tabela_roteamento_mutex;
  argumentos_evd.buffer_saida = buffer_saida;
  argumentos_evd.buffer_saida_mutex = &buffer_saida_mutex;
  argumentos_evd.ultimo_pacote_buffer_saida = &ultimo_pacote_buffer_saida;
  argumentos_evd.id_nodo_atual = id_nodo_atual;
  argumentos_evd.vizinhos = vizinhos;
  argumentos_evd.quantidade_vizinhos = quantidade_vizinhos;
  argumentos_evd.vetor_saltos = vetor_saltos;

  pthread_create(&envia_vetor_distancias_id,
                 NULL,
                 envia_vetor_distancias,
                 (void*)&argumentos_evd);
  /********** Envia o vetor de distâncias periódicamente (evd) */

  /* Atualiza a Tabela de Roteamento (ATR) *********************/
  pthread_t atualiza_tabela_roteamento_vetor_saltos_id;
  struct argumentos_atrvs_struct argumentos_atrvs;

  argumentos_atrvs.id_nodo_atual = id_nodo_atual;
  // Informações sobre a tabela de roteamento
  argumentos_atrvs.tabela_roteamento = tabela_roteamento;
  argumentos_atrvs.tabela_roteamento_mutex = &tabela_roteamento_mutex;
  // Parâmetros do Buffer de Vetor Distância
  argumentos_atrvs.buffer_vetor_distancia = buffer_vetor_distancia;
  argumentos_atrvs.buffer_vetor_distancia_mutex = &buffer_vetor_distancia_mutex;
  // Vetor de Saltos
  argumentos_atrvs.vetor_saltos = vetor_saltos;
  argumentos_atrvs.vetor_saltos_mutex = &vetor_saltos_mutex;

  pthread_create(&atualiza_tabela_roteamento_vetor_saltos_id,
                 NULL,
                 atualiza_tabela_roteamento_vetor_saltos,
                 (void*)&argumentos_atrvs);
  /********************** Atualiza a Tabela de Roteamento (ATR) */

  /* Checador de Vizinhos **************************************/
  pthread_t checador_vizinhos_thread_id;
  struct argumentos_checador_vizinhos_struct argumentos_checador_vizinhos;

  // Parâmetros referentes ao respostas_checagem_vizinhos
  argumentos_checador_vizinhos.respostas_checagem_vizinhos_mutex = &respostas_checagem_vizinhos_mutex;
  argumentos_checador_vizinhos.respostas_checagem_vizinhos = respostas_checagem_vizinhos;
  argumentos_checador_vizinhos.quantidade_vizinhos = quantidade_vizinhos;
  argumentos_checador_vizinhos.vizinhos = vizinhos;
  // Parâmeteros do buffer de saída
  argumentos_checador_vizinhos.buffer_saida = buffer_saida;
  argumentos_checador_vizinhos.buffer_saida_mutex = &buffer_saida_mutex;
  argumentos_checador_vizinhos.ultimo_pacote_buffer_saida = &ultimo_pacote_buffer_saida;
  // Parâmetros da tabela de roteamento
  argumentos_checador_vizinhos.tabela_roteamento_mutex = &tabela_roteamento_mutex;
  argumentos_checador_vizinhos.tabela_roteamento = tabela_roteamento;
  // Parâmetros gerais
  argumentos_checador_vizinhos.id_nodo_atual = id_nodo_atual;

  pthread_create(&checador_vizinhos_thread_id,
                 NULL,
                 checador_vizinhos,
                 (void*)&argumentos_checador_vizinhos);
  /************************************** Checador de Vizinhos */

  /* Responde Checagem de Vizinhos *****************************/
  pthread_t responde_checagem_vizinhos_thread_id;
  struct argumentos_responde_checagem_vizinhos_struct argumentos_responde_checagem_vizinhos;

  argumentos_responde_checagem_vizinhos.checagens_recebidas_mutex = &checagens_recebidas_mutex;
  argumentos_responde_checagem_vizinhos.checagens_recebidas = checagens_recebidas;
  argumentos_responde_checagem_vizinhos.id_nodo_atual = id_nodo_atual;
  argumentos_responde_checagem_vizinhos.buffer_saida = buffer_saida;
  argumentos_responde_checagem_vizinhos.buffer_saida_mutex = &buffer_saida_mutex;
  argumentos_responde_checagem_vizinhos.ultimo_pacote_buffer_saida = &ultimo_pacote_buffer_saida;

  pthread_create(&responde_checagem_vizinhos_thread_id,
                 NULL,
                 responde_checagem_vizinhos,
                 (void*)&argumentos_responde_checagem_vizinhos);
  /***************************** Responde Checagem de Vizinhos */

  /* Encerra as THREADS ****************************************/
  pthread_join(transmissor_thread_id, NULL);
  pthread_join(receptor_thread_id, NULL);
  pthread_join(iem_thread_id, NULL);
  pthread_join(redirecionador_thread_id, NULL);
  pthread_join(recebimento_impressao_mensagem_thread_id, NULL);
  pthread_join(envia_vetor_distancias_id, NULL);
  pthread_join(atualiza_tabela_roteamento_vetor_saltos_id, NULL);
  pthread_join(checador_vizinhos_thread_id, NULL);
  pthread_join(responde_checagem_vizinhos_thread_id, NULL);
  /*************************************************************/
  return 0;
};
