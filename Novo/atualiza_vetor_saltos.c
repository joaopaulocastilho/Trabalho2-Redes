#ifndef INCLUSAO_ARQUIVO_ATUALIZA_VETOR_SALTOS
#define INCLUSAO_ARQUIVO_ATUALIZA_VETOR_SALTOS
#include "globais.h"

/** Atualizador do vetor de saltos.
 * Função responsável por atualizar o vetor de saltos, que é usado pelo transmissor.
 * A função deve ser chamada após atualização da tabela de roteamento.
 * Retorna 1 se houve alguma alteração no vetor de status ou 0 caso não.
 */
int atualiza_vetor_saltos(int tabela_roteamento[][QUANTIDADE_MAXIMA_NOS],
                          pthread_mutex_t *tabela_roteamento_mutex,
                          int vetor_saltos[],
                          pthread_mutex_t *vetor_saltos_mutex,
                          int id_nodo_atual) {
  int vetor_saltos_local[QUANTIDADE_MAXIMA_NOS];
  int menor_distancia; // Variável auxiliar
  int alterou = 0; // Variáveil auxiliar para marcar que teve alteração no vetor de saltos

  memset(vetor_saltos_local, -1, sizeof(vetor_saltos_local));

  pthread_mutex_lock(tabela_roteamento_mutex);
    for (int no_destino = 0; no_destino < QUANTIDADE_MAXIMA_NOS; no_destino++) {
      menor_distancia = INFINITO;

      // Se for pra si mesmo, o próximo salto é o próprio nó
      if (no_destino == id_nodo_atual) {
        vetor_saltos_local[id_nodo_atual] = id_nodo_atual;
        continue;
      }

      for (int intermediario = 0; intermediario < QUANTIDADE_MAXIMA_NOS; intermediario++) {
        int distancia_pelo_no_linha = tabela_roteamento[id_nodo_atual][intermediario] + tabela_roteamento[intermediario][no_destino];

        // Se a distância for menor e o próximo salto não for o próprio nó
        if (distancia_pelo_no_linha < menor_distancia && intermediario != id_nodo_atual) {
          vetor_saltos_local[no_destino] = intermediario;
          menor_distancia = distancia_pelo_no_linha;
        }
      }
    }
  pthread_mutex_unlock(tabela_roteamento_mutex);

  pthread_mutex_lock(vetor_saltos_mutex);
    for(int i = 0; i < QUANTIDADE_MAXIMA_NOS; i++) {
      if (vetor_saltos[i] != vetor_saltos_local[i]) {
        alterou = 1;
        vetor_saltos[i] = vetor_saltos_local[i];
      }
    }
  pthread_mutex_unlock(vetor_saltos_mutex);
  return alterou;
};

#endif
