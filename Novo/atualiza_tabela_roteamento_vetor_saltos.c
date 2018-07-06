#ifndef INCLUSAO_ARQUIVO_ATRVS
#define INCLUSAO_ARQUIVO_ATRVS
#include "globais.h"
#include "atualiza_vetor_saltos.c"

/** Atualiza Tabela de Roteamento e calcula o novo Vetor de Saltos **/

 struct argumentos_atrvs_struct {
   int id_nodo_atual;
   // Informações sobre a tabela de roteamento
   int (*tabela_roteamento)[QUANTIDADE_MAXIMA_NOS];
   pthread_mutex_t *tabela_roteamento_mutex;
   // Parâmetros do Buffer de Vetor Distância
   pacote_t *buffer_vetor_distancia;
   pthread_mutex_t *buffer_vetor_distancia_mutex;
   // Vetor de Saltos
   int *vetor_saltos;
   pthread_mutex_t *vetor_saltos_mutex;
 };

 void* atualiza_tabela_roteamento_vetor_saltos(void *args) {
   struct argumentos_atrvs_struct* argumentos = (struct argumentos_atrvs_struct*) args;
   int nodo_atual = argumentos->id_nodo_atual;
   int indice_primeiro_pacote = 0;
   int i, j, nova_distancia;
   pacote_t pacote_vetor_distancia; // Pacote a ser enviado.
   char mensagem_log[5000];
   char descricao_pacote[5000]; // Variável auxiliar

   while (1) {
     /* Pega o pacote no buffer de Vetor Distância */
     pthread_mutex_lock(argumentos->buffer_vetor_distancia_mutex);
     pacote_vetor_distancia = argumentos->buffer_vetor_distancia[indice_primeiro_pacote];

     // Se não tem pacote para atualizar nada
     if (pacote_vetor_distancia.tipo == TIPO_PACOTE_VAZIO) {
       pthread_mutex_unlock(argumentos->buffer_vetor_distancia_mutex);
       continue;
     }

     informacoes_pacote(pacote_vetor_distancia, descricao_pacote);
     sprintf(mensagem_log, "[ATRVS] Retira pacote do buffer de vetor de distância. Info: %s\n", descricao_pacote);
     grava_log(mensagem_log);

     argumentos->buffer_vetor_distancia[indice_primeiro_pacote].tipo = TIPO_PACOTE_VAZIO;
     indice_primeiro_pacote = (indice_primeiro_pacote + 1) % TAMANHO_BUFFER_VETOR_DISTANCIA;
     pthread_mutex_unlock(argumentos->buffer_vetor_distancia_mutex);

     //Atualizar tabela de roteamento com o novo vetor distância do nó descrito na origem do pacote
     i = pacote_vetor_distancia.origem;
    //printf("RECEBEU VETOR DE %d: ", i); //DEBUG
     pthread_mutex_lock(argumentos->tabela_roteamento_mutex);
     for (j = 0; j < TAMANHO_MENSAGEM_PACOTE / 4; j++) {
       nova_distancia = ((int)pacote_vetor_distancia.mensagem[j * 4]) << 24;
       nova_distancia |= ((int)pacote_vetor_distancia.mensagem[j * 4 + 1]) << 16;
       nova_distancia |= ((int)pacote_vetor_distancia.mensagem[j * 4 + 2]) << 8;
       nova_distancia |= pacote_vetor_distancia.mensagem[j * 4 + 3];
       argumentos->tabela_roteamento[i][j] = nova_distancia;
      // if (j < 7) printf(" %d", nova_distancia == INFINITO ? -1 : nova_distancia); //DEBUG
     }
     //printf("\n"); //DEBUG
     pthread_mutex_unlock(argumentos->tabela_roteamento_mutex);
     // Atualizar o vetor distância do nó atual
     atualiza_vetor_distancia(nodo_atual, argumentos->tabela_roteamento, argumentos->tabela_roteamento_mutex);
     // Atualizar o vetor de saltos
     atualiza_vetor_saltos(argumentos->tabela_roteamento, argumentos->tabela_roteamento_mutex, argumentos->vetor_saltos, argumentos->vetor_saltos_mutex, nodo_atual);
   }
 }

#endif
