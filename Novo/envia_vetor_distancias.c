#ifndef INCLUSAO_ARQUIVO_EVD
#define INCLUSAO_ARQUIVO_EVD
#include "globais.h"

/** Envia o SEU Vetor Distâncias para todos os seus vizinhos **/

 struct argumentos_evd_struct {
   pacote_t *buffer_saida;
   pthread_mutex_t *buffer_saida_mutex;
   int *ultimo_pacote_buffer_saida;
   int id_nodo_atual;
   vizinho_t *vizinhos;
   int quantidade_vizinhos;
   int (*tabela_roteamento)[QUANTIDADE_MAXIMA_NOS];
   pthread_mutex_t *tabela_roteamento_mutex;
 };

 void* envia_vetor_distancias(void *args) {
   struct argumentos_evd_struct* argumentos = (struct argumentos_evd_struct*) args;
   int nodo_atual = argumentos->id_nodo_atual;
   int quantidade_vizinhos = argumentos->quantidade_vizinhos;
   int i, j;
   int inseriu_buffer_saida; // Uma flag para saber se a mensagem foi inserida no buffer de saída.
   char mensagem_log[5000];
   char string_descricao_pacote[500]; // Variável auxiliar

   pacote_t pacote_vetor_distancia;
   while (1) {
     // Vamos crirar um pacote para cada vizinho
     for (i = 0; i < quantidade_vizinhos; i++) {
       pacote_vetor_distancia.destino = argumentos->vizinhos[i].id;
       pacote_vetor_distancia.tipo = TIPO_PACOTE_VETOR_DISTANCIA;
       pacote_vetor_distancia.origem = nodo_atual;

       pthread_mutex_lock(argumentos->tabela_roteamento_mutex);
       // Coloca a distância até os outros nós na mensagem do pacote
       for (j = 0; j < TAMANHO_MENSAGEM_PACOTE / 4; j++) {
         pacote_vetor_distancia.mensagem[4 * j] = (char)argumentos->tabela_roteamento[nodo_atual][j] >> 24;
         pacote_vetor_distancia.mensagem[4 * j + 1] |= (char)argumentos->tabela_roteamento[nodo_atual][j] >> 16;
         pacote_vetor_distancia.mensagem[4 * j + 2] |= (char)argumentos->tabela_roteamento[nodo_atual][j] >> 8;
         pacote_vetor_distancia.mensagem[4 * j + 3] |= (char)argumentos->tabela_roteamento[nodo_atual][j];
       }
       pthread_mutex_unlock(argumentos->tabela_roteamento_mutex);

       // Conseguiu encaminhar o pacote?
       inseriu_buffer_saida = enfileira_pacote_para_envio(pacote_vetor_distancia, argumentos->buffer_saida, argumentos->buffer_saida_mutex, argumentos->ultimo_pacote_buffer_saida);


      /* Grava no log o resultado */
      informacoes_pacote(pacote_vetor_distancia, string_descricao_pacote);
      if (inseriu_buffer_saida) {
        sprintf(
          mensagem_log,
          "[ENVIA VETOR DISTÂNCIA] Pacote colocado no buffer de saída  Info: { %s }.",
          string_descricao_pacote
        );
      } else { // if (inseriu_buffer_saida)
        sprintf(
          mensagem_log,
          "[ENVIA VETOR DISTÂNCIA] Pacote descartado por falta de espaço no buffer de saída. Info: { %s }",
          string_descricao_pacote
        );
      }
      grava_log(mensagem_log);
    }
    //imprime_tabela_roteamento(argumentos->tabela_roteamento); // DEBUG
    // Vamos esperar um tempo até mandar novamente!
    sleep(TEMPO_PAUSA_EVD);
  }
}

#endif
