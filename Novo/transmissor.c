#ifndef INCLUSAO_ARQUIVO_TRANSMISSOR
#define INCLUSAO_ARQUIVO_TRANSMISSOR
#include "globais.h"

void envia_pacote(char *cadeia_caracteres,
                  int tamanho_cadeia_caracteres,
                  char *endereco,
                  int porta);

/** Transmissor
 * Pega os pacotes que estão no buffer de saída e envia-os para o nó de desti-
 * no. Envia de acordo com o vetor de saltos
 */

struct argumentos_transmissor_struct {
  pthread_mutex_t *buffer_saida_mutex;
  pacote_t *buffer_saida;
  int *vetor_saltos;
  char (*enderecos_vizinhos)[TAMANHO_MAXIMO_ENDERECO];
  int *portas_vizinhos;
};

void *transmissor(void *args) {
  struct argumentos_transmissor_struct* argumentos = (struct argumentos_transmissor_struct*) args;
  int indice_primeiro_pacote = 0;
  pacote_t pacote_envio; // Pacote a ser enviado.
  int proximo_salto_envio; // Id do nó do próximo salto.
  char mensagem_log[5000]; // Variável auxiliar para printar coisas no log. Ignore.
  char cadeia_caracteres_pacote[TAMANHO_TOTAL_PACOTE]; // Espaço para colocar a cadeia de caracteres do pacote.
  char string_informacoes_pacote[5000]; // Variável auxiliar

  do {
    /* Pega o pacote no buffer de saída */
    pthread_mutex_lock(argumentos->buffer_saida_mutex);
    pacote_envio = argumentos->buffer_saida[indice_primeiro_pacote];
    if (pacote_envio.tipo == TIPO_PACOTE_VAZIO) {
      // Ainda não tem pacote para enviar
      pthread_mutex_unlock(argumentos->buffer_saida_mutex);
      continue;
    }
    argumentos->buffer_saida[indice_primeiro_pacote].tipo = TIPO_PACOTE_VAZIO;
    indice_primeiro_pacote = (indice_primeiro_pacote + 1) % TAMANHO_BUFFER_SAIDA;
    pthread_mutex_unlock(argumentos->buffer_saida_mutex);

    /* Busca o próximo salto para o nó de destino do pacote */
    if (pacote_envio.tipo == TIPO_PACOTE_VETOR_DISTANCIA || pacote_envio.tipo == TIPO_PACOTE_CHECA_NO_ATIVO || pacote_envio.tipo == TIPO_PACOTE_CONFIRMACAO_VETOR_DISTANCIA || pacote_envio.tipo == TIPO_PACOTE_CONFIRMACAO_NO_ATIVO) {
      proximo_salto_envio = pacote_envio.destino;
    } else {
      proximo_salto_envio = argumentos->vetor_saltos[pacote_envio.destino];
    }
    if (proximo_salto_envio == -1) {
      informacoes_pacote(pacote_envio, string_informacoes_pacote);
      sprintf(mensagem_log, "[TRANSMISSOR] Tentativa de envio de pacote falhou. O próximo salto ainda não foi descoberto. {%s}", string_informacoes_pacote);
      grava_log(mensagem_log);
      continue;
    }

    /* Envia pacote */
    converte_pacote_para_char(pacote_envio, cadeia_caracteres_pacote);
    envia_pacote(cadeia_caracteres_pacote,
                 TAMANHO_TOTAL_PACOTE,
                 argumentos->enderecos_vizinhos[proximo_salto_envio],
                 argumentos->portas_vizinhos[proximo_salto_envio]);

    /* Salva no log que o pacote foi enviado */
    informacoes_pacote(pacote_envio, string_informacoes_pacote);
    sprintf(
      mensagem_log,
      "[TRANSMISSOR] Pacote enviado para [%d]. {%s}",
      proximo_salto_envio,
      string_informacoes_pacote
    );
    grava_log(mensagem_log);
  } while(1);
}

void envia_pacote(char *cadeia_caracteres,
                  int tamanho_cadeia_caracteres,
                  char *endereco,
                  int porta) {
  struct sockaddr_in si_other;
  int s, slen = sizeof(si_other);

  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    die("socket");
  }

  memset((char *) &si_other, 0, sizeof(si_other));
  si_other.sin_family = AF_INET;

  si_other.sin_port = htons(porta);
  if (inet_aton(endereco , &si_other.sin_addr) == 0) {
    fprintf(stderr, "inet_aton() failed\n");
    exit(1);
  }

  //send the message
  if (sendto(s, cadeia_caracteres, tamanho_cadeia_caracteres, 0, (struct sockaddr *) &si_other, slen) == -1) {
      die("sendto()");
  }

  close(s);
}

#endif
