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

   pacote_t pacote_vetor_distancia;
   while (1) {
     // Vamos crirar um pacote para cada vizinho
     for (i = 0; i < quantidade_vizinhos; i++) {
       pacote_vetor_distancia.destino = argumentos->vizinhos[i].id;
       pacote_vetor_distancia.tipo = TIPO_PACOTE_VETOR_DISTANCIA;
       pacote_vetor_distancia.origem = nodo_atual;
       pthread_mutex_lock(argumentos->tabela_roteamento_mutex);
       for (j = 0; j < QUANTIDADE_MAXIMA_NOS; j++) {
         pacote_vetor_distancia.mensagem[j] = argumentos->tabela_roteamento[nodo_atual][j];
       }
       pthread_mutex_unlock(argumentos->tabela_roteamento_mutex);
       // Conseguiu encaminhar o pacote?
       inseriu_buffer_saida = enfileira_pacote_para_envio(pacote_vetor_distancia, argumentos->buffer_saida, argumentos->buffer_saida_mutex, argumentos->ultimo_pacote_buffer_saida);
       if (inseriu_buffer_saida) {
         sprintf( // Conseguiu Conseguiu guardar a mensagem no buffer de saída para o próximo salto até o destino.
           mensagem_log,
           "[Envia Vetor Distâncias] Pacote do tipo [%d] colocado no buffer de saída com origem [%d], destino [%d].",
           pacote_vetor_distancia.tipo,
           pacote_vetor_distancia.origem,
           pacote_vetor_distancia.destino
        );
        grava_log(mensagem_log);
      } else {
        sprintf( // Não conseguiu Conseguiu guardar a mensagem no buffer de saída para o próximo salto até o destino.
          mensagem_log,
          "[Envia Vetor Distâncias] Pacote do tipo [%d] com origem [%d], destino [%d] descartado por falta de espaço no buffer de saída.",
          pacote_vetor_distancia.tipo,
          pacote_vetor_distancia.origem,
          pacote_vetor_distancia.destino
        );
      }
    }
    // Vamos esperar um tempo até mandar novamente!
    sleep(TEMPO_PAUSA_EVD);
  }
}

#endif
