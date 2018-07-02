#ifndef INCLUSAO_ARQUIVO_ATRVS
#define INCLUSAO_ARQUIVO_ATRVS
#include "globais.h"

/** Atualiza Tabela de Roteamento e calcula o novo Vetor de Saltos **/

 struct argumentos_atrvs_struct {
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
   
}

#endif
