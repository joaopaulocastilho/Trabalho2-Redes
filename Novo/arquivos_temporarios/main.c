#include <stdio.h>
#include <string.h> // memset
#include <stdlib.h> // exit(0);
#include <sys/types.h> // fork()
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unistd.h>
#include <pthread.h>

#define DEBUG 0
#define MAX_ROUTERS 20 // Número máximo de roteadores presentes no arquivo roteador.config
#define MAX_ROUTER_ADDRESS_SIZE 20 // Tamanho máximo do endereço de ip dos roteadores
#define MESSAGE_SIZE 500 // Tamanho do buffer pra guardar a mensagem
#define PACKAGE_SIZE (MESSAGE_SIZE + 8) // 2 bytes pra guardar o destino do pacote, 2 para o remetente e 4 pra informações extras
#define INF 112345678

typedef struct { int id, w; }neighbor_t;

int node_id;
int routers_ports[MAX_ROUTERS]; //Lista das portas dos roteadores
char routers_addresses[MAX_ROUTERS][MAX_ROUTER_ADDRESS_SIZE]; // Lista dos endereços dos roteadores
int router_table[MAX_ROUTERS][MAX_ROUTERS]; //Tabela de roteamento
neighbor_t neighbors[MAX_ROUTERS]; //Lista de vizinhos do nó
int qtt_neighbors; // Quantidade de vizinhos que o nó tem
int sit_neighbors[MAX_ROUTERS]; // Os meus vizinhos estão disponíveis?
int newst[MAX_ROUTERS];
int next_nodes[MAX_ROUTERS]; //Vetor de saltos
int package_id = 0;


void wrap_message(char package_space[], char message[], int destination_node_id, int this_node_id, int confirmation, int package_id);
void unwrap_message(int *bitmask, short int *destination, short int *sender, char *message, char *buffer, int *is_confirmation, int *package_id);

void die(char *s) {
  perror(s);
  exit(1);
}

/** Função que le o arquivo roteador.config e grava os dados nos vetores bidimensionais routers_ports e routers_addresses passados por parâmetro. */
int read_routers(int routers_ports[MAX_ROUTERS], char routers_addresses[MAX_ROUTERS][MAX_ROUTER_ADDRESS_SIZE]) {
  FILE *routers_file;
  int router_id, router_port;
  char router_address[MAX_ROUTER_ADDRESS_SIZE];

  memset(routers_ports, -1, MAX_ROUTERS * sizeof(int));
  memset(routers_addresses, 0, MAX_ROUTERS * MAX_ROUTER_ADDRESS_SIZE);

  routers_file = fopen("roteador.config", "r");
  if (!routers_file) {
    return 1;
  }

  while(fscanf(routers_file, "%d %d %s\n", &router_id, &router_port, router_address) != EOF) {
    routers_ports[router_id] = router_port;
    strcpy(routers_addresses[router_id], router_address);
  }
  fclose(routers_file);
  return 0;
}

// Função que envia os dados do +package+ para o nó com id +node_id+. Tanto o cliente (para enviar) quanto o servidor (para reencaminhar) usam essa função.
void send_package(char *package, int next_node) {
  struct sockaddr_in si_other;
  int s, slen = sizeof(si_other);
  int jump_node = next_nodes[next_node];

  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) die("socket");

  memset((char *) &si_other, 0, sizeof(si_other));
  si_other.sin_family = AF_INET;

  si_other.sin_port = htons(routers_ports[jump_node]);
  if (inet_aton(routers_addresses[jump_node] , &si_other.sin_addr) == 0) {
    fprintf(stderr, "inet_aton() failed\n");
    exit(1);
  }
  //send the message
  if (sendto(s, package, 8 + strlen(package + 8), 0, (struct sockaddr *) &si_other, slen) == -1) die("sendto()");

  close(s);
}

// Função responsável por criar a interface do client (o que manda a mensagem). Baseada no arquivo client-udp mandado pelo professor.
void* sender(void *arg) {
  char message[MESSAGE_SIZE];
  short int message_target_node;
  char package[PACKAGE_SIZE];

  while(1) {
    scanf("%hd<", &message_target_node);
    fgets(message, MESSAGE_SIZE, stdin);
    message[strlen(message) - 1] = '\0';

    //O penúltimo parâmetro da função é se é uma mensagem de confirmação. Nesse caso é 0 pois não é.
    wrap_message(package, message, message_target_node, node_id, 0, package_id);
    package_id = (package_id + 1) % 8; //Estamos dando 3 bits para id's de pacotes. Caso seja mandado mais, o id será resetado.

    printf("\n\033[1;32m⬆\033[0m ");
    printf("[\033[1;31m%d\033[0m], ", node_id);
    if (message_target_node != next_nodes[message_target_node]) {
      printf("\033[1;31m%d\033[0m, ..., ", next_nodes[message_target_node]);
    }
    printf("\033[1;31m%d\033[0m\n", message_target_node);

    send_package(package, message_target_node);
  }
};

// Função responsável por criar a interface do server (o que recebe a mensagem). Baseada no arquivo server-udp mandado pelo professor.
void* receiver(void *arg) {
  struct sockaddr_in si_me, si_other;
  int s, slen = sizeof(si_other) , recv_len;
  char buffer[PACKAGE_SIZE];
  //create a UDP socket
  if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) die("socket");
  // zero out the structure
  memset((char *) &si_me, 0, sizeof(si_me));

  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(routers_ports[node_id]);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);

  //bind socket to port
  if (bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) die("bind");

  //keep listening for data
  while(1) {
    int package_bitmask;
    short int destination_node_id, sender_node_id;
    char message[MESSAGE_SIZE];
    int is_confirmation, id_package, is_validation;

    memset(buffer, 0, PACKAGE_SIZE);

    //try to receive some data, this is a blocking call
    recv_len = recvfrom(s, buffer, PACKAGE_SIZE, 0, (struct sockaddr *) &si_other, &slen);
    if (recv_len == -1) { die("recvfrom()"); }

    unwrap_message(&package_bitmask, &destination_node_id, &sender_node_id,
                   message, buffer, &is_confirmation, &id_package);

    if (destination_node_id == node_id) {
      if (!is_confirmation) {
        char package[PACKAGE_SIZE];
        int message_target_node = sender_node_id;
        int node_id = destination_node_id;
        printf("\n\033[1;32m⬇\033[0m ");
        printf("\033[1;31m%d\033[0m, ..., [\033[1;31m%d\033[0m]", sender_node_id,
                                                                node_id
               );
        printf("\n");
        printf("\033[1m%s\033[0m\n", message);
        message[0] = '\0';
        //Enviar mensagem de confirmação que recebeu.
        wrap_message(package, message, message_target_node, node_id, 1, id_package);
        printf("\n\033[1mEnviando Corfirmacao: \033[0m");
        printf("\n\033[1;32m⬆\033[0m ");
        printf("[\033[1;31m%d\033[0m], ", node_id);
        if (message_target_node != next_nodes[message_target_node]) {
          printf("\033[1;31m%d\033[0m, ..., ", next_nodes[message_target_node]);
        }
        printf("\033[1;31m%d\033[0m\n", message_target_node);
        send_package(package, message_target_node);
      } else {
        printf("Pacote %d entregue com sucesso!\n", id_package);
      }
    } else {
      //Recebe pacote com outro destino
      if (is_confirmation) printf("\n\033[1mEnviando Corfirmacao: \033[0m");
      else printf("\n");
      printf("\033[1;34m↕\033[0m ");
      printf("\033[1;31m%d\033[0m, ..., [\033[1;31m%d\033[0m], ",
        sender_node_id,
        node_id
      );

      if (destination_node_id != next_nodes[destination_node_id]) {
        printf("\033[1;31m%d\033[0m, ..., ", next_nodes[destination_node_id]);
      }
      printf("\033[1;31m%d\033[0m", destination_node_id);
      printf("\n");

      // Reencaminha
      send_package(buffer, destination_node_id);
    }
  }
  close(s);
}

void wrap_message(char *package_space,
                  char *message,
                  int destination_node_id,
                  int this_node_id,
                  int confirmation,
                  int package_id) {
  memset(package_space, 0, 4);
  if (confirmation) package_space[0] = (1 << 7);
  package_space[0] |= (unsigned char)(package_id << 4);
  package_space[4] = (unsigned char)(destination_node_id >> 8);
  package_space[5] = (unsigned char)destination_node_id;
  package_space[6] = (unsigned char)(this_node_id >> 8);
  package_space[7] = (unsigned char)this_node_id;
  strncpy(package_space + 8, message, MESSAGE_SIZE);
  if (DEBUG) {
    printf("\n\n[DEBUG]Wrapping the message:\n");
    printf("[DEBUG]Message: %s\n", message);
    printf("[DEBUG]destination: %d → %hd\n", destination_node_id, ((package_space[4] << 8) | package_space[5]));
    printf("[DEBUG]package: ");
    for(int i = 0; i < PACKAGE_SIZE; i++) { printf("%c", package_space[i]); }
    printf("\n");
    printf("[DEBUG]message_size: %d\n", MESSAGE_SIZE);
  }
}

void unwrap_message(int *bitmask,
                    short int *destination,
                    short int *sender,
                    char *message,
                    char *buffer,
                    int *is_confirmation,
                    int *id_package) {
  int bit = 0;
  *is_confirmation = (buffer[0] & (1 << 7));
  for (int i = 0; i < 3; i++) {
    int bit_tmp = (buffer[0] & (1 << 6 - i));
    bit |= bit_tmp;
  }
  *id_package = (bit >> 4);
  *bitmask = buffer[0] << 24;
  *bitmask |= buffer[1] << 16;
  *bitmask |= buffer[2] << 8;
  *bitmask |= buffer[3];
  *destination = buffer[4] << 8;
  *destination |= buffer[5];
  *sender = buffer[6] << 8;
  *sender |= buffer[7];
  strncpy(message, buffer + 8, MESSAGE_SIZE);
  if (DEBUG) {
    printf("[DEBUG]Unwrapping the message:\n");
    printf("[DEBUG]Buffer: ");
    for(int i = 0; i < PACKAGE_SIZE; i++) { printf("%c", buffer[i]); }
    printf("\n");
    printf("[DEBUG]Message: %s\n", message);
    printf("[DEBUG]bitmask: %d\n", *bitmask);
    printf("[DEBUG]destination: %hd\n", *destination);
  }
}

void initialize(void) {
  int i, j, u, v, w;
  qtt_neighbors = 0;
  //Inicia a tabela de roteamento
  for (i = 0; i < MAX_ROUTERS; i++) {
    next_nodes[i] = -1;
    for (j = 0; j < MAX_ROUTERS; j++)
      router_table[i][j] = INF;
  }

  FILE *links_file = fopen("enlaces.config", "r");
  if (!links_file) die("Falha ao abrir arquivo de enlaces");
  while(fscanf(links_file, "%d %d %d\n", &u, &v, &w) != EOF){
    if (v == node_id) { v = u; u = node_id; }
    if (u == node_id) {
      neighbors[qtt_neighbors].id = v;
      neighbors[qtt_neighbors++].w = w;
    }
  }
  fclose(links_file);

  //Seta os custos de um nó para ele mesmo, e o salto dele para ele mesmo
  router_table[node_id][node_id] = 0;
  next_nodes[node_id] = node_id;

  //Preenche o seu vetor na tabela de roteamento
  for (i = 0; i < qtt_neighbors; i++) {
    u = node_id; v = neighbors[i].id; w = neighbors[i].w;
    router_table[u][v] = w;
    next_nodes[v] = v;
    sit_neighbors[i] = 1;
  }
}

int main(int argc, char* argv[]) {
  pthread_t sender_id = 1, receiver_id = 2;
  node_id = *argv[1] - '0'; //ID do nó deste processo

  printf("Digite <Nodo_Destino><Mensagem para enviar uma mensagem\n");

  read_routers(routers_ports, routers_addresses);

  initialize();

  pthread_create(&sender_id, NULL, &sender, NULL); //Cria thread enviadora
  pthread_create(&receiver_id, NULL, &receiver, NULL); //Cria thread receptora

  pthread_join(sender_id, NULL);
  pthread_join(receiver_id, NULL);
  return 0;
}
