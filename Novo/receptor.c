#ifndef INCLUSAO_ARQUIVO_RECEPTOR
#define INCLUSAO_ARQUIVO_RECEPTOR
#include "globais.h"

/** Receptor
* Fica esperando a chegada de um novo pacote. Ao chegar, o pacote é colocado
* em um buffer de entrada, onde vai ser tratado pelo unpacker.
*/

struct argumentos_receptor_struct {
  pthread_mutex_t *buffer_entrada_mutex;
  pthread_mutex_t *buffer_saida_mutex;
  pacote_t *buffer_entrada;
  pacote_t *buffer_saida;
  int *portas_roteadores;
  int *indice_ultimo_pacote_buffer_saida;
  int id_nodo_atual;
};

void* receptor(void *args) {
  struct argumentos_receptor_struct* argumentos = (struct argumentos_receptor_struct*) args;
  int ultimo_pacote_entrada = 0;
  int nodo_atual = argumentos->id_nodo_atual;
  int inseriu_buffer_saida; // Uma flag para saber se a mensagem foi inserida no buffer de saída caso a mensagem não seja para este pacote.
  char mensagem_log[5000];
  char descricao_pacote[5000]; // Variável auxiliar

  char buffer[TAMANHO_TOTAL_PACOTE]; // Buffer onde o pacote recebido vai entrar
  pacote_t pacote_recebido, prox_posicao;
  // Inicializar os socktes UDP
  struct sockaddr_in si_me, si_other;
  int s, slen = sizeof(si_other), recv_len;
  // Aqui cria o socket UDP
  if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) { die("socket"); }
  // Zera a estrutura
  memset((char *) &si_me, 0, sizeof(si_me));

  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(argumentos->portas_roteadores[nodo_atual]);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);
  // bind socket to port
  if (bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) { die("bind"); }
  // Sempre esperarndo a chegada de um pacote
  while (1) {
    // Tenta receber algum pacote
    recv_len = recvfrom(s, buffer, TAMANHO_TOTAL_PACOTE, 0, (struct sockaddr *) &si_other, &slen);

    if (recv_len == -1) { die("recvfrom()"); }

    // Coleta as informações do pacote
    converte_char_para_pacote(buffer, &pacote_recebido);

    informacoes_pacote(pacote_recebido, descricao_pacote);

    // Se for pacote para reencaminhamento
    if (pacote_recebido.destino != nodo_atual) {
      inseriu_buffer_saida = enfileira_pacote_para_envio(pacote_recebido, argumentos->buffer_saida, argumentos->buffer_saida_mutex, argumentos->indice_ultimo_pacote_buffer_saida);

      if (inseriu_buffer_saida) {
        sprintf(mensagem_log,
          "[RECEPTOR] Pacote para reencaminhamento adicionado ao buffere de saída. Info: { %s }.", descricao_pacote);
        grava_log(mensagem_log);
      } else {
        sprintf(mensagem_log,
          "[RECEPTOR] Pacote de reencaminhamento descartado por falta de espaço no buffer de saída. Info: { %s }",
          descricao_pacote
        );
      }
      continue;
    }

    // Guarda o pacote no buffer de entrada com todos os pacotes
    pthread_mutex_lock(argumentos->buffer_entrada_mutex);
      prox_posicao = argumentos->buffer_entrada[ultimo_pacote_entrada];

      if (prox_posicao.tipo == TIPO_PACOTE_VAZIO) {
        argumentos->buffer_entrada[ultimo_pacote_entrada] = pacote_recebido;
        ultimo_pacote_entrada = (ultimo_pacote_entrada + 1) % TAMANHO_BUFFER_ENTRADA;

        sprintf(mensagem_log,
                "[RECEPTOR] Pacote adicionado ao buffer de entrada. Info: { %s }",
                descricao_pacote);
        grava_log(mensagem_log);
      } else {
        sprintf(mensagem_log,
                "[RECEPTOR] Pacote  descartado por falta de espaço no buffer de entrada. Info: { %s }", descricao_pacote);
        grava_log(mensagem_log);
      }
    pthread_mutex_unlock(argumentos->buffer_entrada_mutex);
  }
}
#endif
