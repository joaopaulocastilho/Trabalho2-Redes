#ifndef INCLUSAO_ARQUIVO_INTERFACE_ENVIO_MENSAGEM
#define INCLUSAO_ARQUIVO_INTERFACE_ENVIO_MENSAGEM
#include "globais.h"

/** Interface de envio de mensagem
 * Thread que cuida da parte de enviar mensagem que o usuário escreve no termi-
 * nal. Lê a mensagem, empacote e coloca no buffer de saída
 */

struct argumentos_iem_struct {
  pthread_mutex_t *buffer_saida_mutex;
  pacote_t *buffer_saida;
  int *ultimo_pacote_buffer_saida;
  int id_nodo_atual;
};

void *interface_envio_mensagem(void *args) {
  struct argumentos_iem_struct* argumentos = (struct argumentos_iem_struct*) args;

  /* Embora o tamanho do pacote seja um decimo desse número, esse vetor é
   * criado pro caso de o usuário quiser colocar uma mensagem gigante. A
   * mensagem só é enviada com no máximo TAMANHO_MENSAGEM_PACOTE,
   * porém.
   */
  int tamanho_maximo_mensagem_entrada = TAMANHO_MENSAGEM_PACOTE * 10;
  char mensagem_envio[tamanho_maximo_mensagem_entrada];

  int roteador_destino;
  pacote_t pacote_envio;
  int resultado_enfileiramento_buffer_saida;
  char mensagem_gravar_log[1000];

  do {
    printf("Envio de mensagem. Envie uma mensagem no formato: 1<Mensagem, onde 1 é o nó destino e \"Mensagem\" é a mensagem que quer enviar.\nDigite: ");
    scanf("%d<", &roteador_destino);
    fgets(mensagem_envio, tamanho_maximo_mensagem_entrada, stdin);
    mensagem_envio[TAMANHO_MENSAGEM_PACOTE - 1] = '\0';

    /* Prepara o pacote */
    pacote_envio.destino = roteador_destino;
    pacote_envio.tipo = TIPO_PACOTE_MENSAGEM_USUARIO;
    pacote_envio.origem = argumentos->id_nodo_atual;
    strcpy(pacote_envio.mensagem, mensagem_envio);

    /* Adiciona o pacote ao buffer de saída */
    resultado_enfileiramento_buffer_saida = enfileira_pacote_para_envio(
      pacote_envio,
      argumentos->buffer_saida,
      argumentos->buffer_saida_mutex,
      argumentos->ultimo_pacote_buffer_saida
    );

    /* Dá feedback pro usuário e grava status no log */
    if (resultado_enfileiramento_buffer_saida) {
      printf("Pacote adicionado ao buffer de saída. Esperando para ser enviado.\n");
      sprintf(mensagem_gravar_log,
             "[INTERFACE DE ENVIO DE MENSAGEM] Pacote de origem [%d], destino [%d], tipo [%d] e mensagem [%s] adicionado ao buffer de saída",
             pacote_envio.origem,
             pacote_envio.destino,
             pacote_envio.tipo,
             pacote_envio.mensagem);
    } else {
      printf("Falha ao tentar adicionar pacote adicionado ao buffer de saída.\n");
      sprintf(mensagem_gravar_log,
       "[INTERFACE DE ENVIO DE MENSAGEM] Falha ao tentar adicionar pacote de origem [%d], destino [%d], tipo [%d] e mensagem [%s] ao buffer de saída",
       pacote_envio.origem,
       pacote_envio.destino,
       pacote_envio.tipo,
       pacote_envio.mensagem);
    }
    grava_log(mensagem_gravar_log);
  } while (1);
}

#endif