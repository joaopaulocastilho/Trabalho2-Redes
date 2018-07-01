#ifndef INCLUSAO_ARQUIVO_RECEPTOR
#define INCLUSAO_ARQUIVO_RECEPTOR
#include "globais.h"

/** Receptor
 * Fica esperando a chegada de um novo pacote. Ao chegar, o pacote é colocado
 * em um buffer de entrada, onde vai ser tratado pelo unpacker.
 */

 struct argumentos_receptor_struct {
   pthread_mutex_t *buffer_entrada_mutex;
   pacote_t *buffer_entrada;
   int tamanho_buffer_entrada;
   int ultimo_pacote_buffer_entrada;
 };

 void* receptor(void *args) {
   struct argumentos_receptor_struct* argumentos = (struct argumentos_receptor_struct*) args;

   char buffer[TAMANHO_TOTAL_PACOTE]; // Buffer onde o pacote recebido vai entrar
   // Inicializar os socktes UDP
   struct sockaddr_in si_me, si_other;
   int s, slen = sizeof(si_other), recv_len;
   // Aqui cria o socket UDP
   if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) die("socket");
   // Zera a estrutura
   memset((char *) &si_me, 0, sizeof(si_me));

   si_me.sin_family = AF_INET;
   si_me.sin_port = htons(routers_ports[node_id]);
   si_me.sin_addr.s_addr = htonl(INADDR_ANY);
   // bind socket to port
   if (bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) die("bind");
   // Sempre esperarndo a chegada de um pacote
   while (1) {
     // Tenta receber algum pacote
     recv_len = recvfrom(s, buffer, PACKAGE_SIZE, 0, (struct sockaddr *) &si_other, &slen);
     if (recv_len == -1) { die("recvfrom()"); }
     // Guarda o pacote no buffer de entrada com todos os pacotes
     pthread_mutex_lock(argumentos->buffer_entrada_mutex);



     pthread_mutex_unlock(argumentos->buffer_entrada_mutex);
     return NULL;
   }
 }

#endif
