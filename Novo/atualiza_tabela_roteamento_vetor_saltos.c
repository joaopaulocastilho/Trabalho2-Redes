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
   int i, j;
   pacote_t pacote_vetor_distancia; // Pacote a ser enviado.
   char mensagem_log[5000];

   while (1) {
     /* Pega o pacote no buffer de Vetor Distância */
     pthread_mutex_lock(argumentos->buffer_vetor_distancia_mutex);
     pacote_vetor_distancia = argumentos->buffer_vetor_distancia[indice_primeiro_pacote];
     if (pacote_vetor_distancia.tipo == TIPO_PACOTE_VAZIO) {
       // Não tem pacote para atualizar nada
       pthread_mutex_unlock(argumentos->buffer_vetor_distancia_mutex);
       continue;
     }
     argumentos->buffer_vetor_distancia[indice_primeiro_pacote].tipo = TIPO_PACOTE_VAZIO;
     indice_primeiro_pacote = (indice_primeiro_pacote + 1) % TAMANHO_BUFFER_VETOR_DISTANCIA;
     pthread_mutex_unlock(argumentos->buffer_vetor_distancia_mutex);

     //Atualizar tabela de roteamento com o novo vetor distância no nó origem que mandou esse pacote
     i = pacote_vetor_distancia.origem; // Linha da tabela de roteamento que vai atualizar na tabela de roteamento
     pthread_mutex_lock(argumentos->tabela_roteamento_mutex);
     for (j = 0; j < QUANTIDADE_MAXIMA_NOS; j++) {
       argumentos->tabela_roteamento[i][j] = pacote_vetor_distancia.mensagem[j];
     }
     pthread_mutex_unlock(argumentos->tabela_roteamento_mutex);
     // Atualizar o vetor distância do nó atual
     atualiza_tabela_roteamento(nodo_atual, argumentos->tabela_roteamento, argumentos->tabela_roteamento_mutex);
     // Atualizar o vetor de saltos
     atualiza_vetor_saltos(argumentos->tabela_roteamento, argumentos->tabela_roteamento_mutex, argumentos->vetor_saltos, argumentos->vetor_saltos_mutex, nodo_atual);
   }
 }

#endif
