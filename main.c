#include <stdio.h> // printf
#include <string.h> // memset
#include <stdlib.h> // exit(0);
#include <sys/types.h> // fork()
#include <arpa/inet.h>
#include <sys/socket.h>

#define DEBUG_MODE 0
#define MAX_ROUTERS 20 // Número máximo de roteadores presentes no arquivo roteador.config
#define MAX_ROUTER_ADDRESS_SIZE 20 // Tamanho máximo do endereço de ip dos roteadores

#define MESSAGE_SIZE 500 // Tamanho do buffer pra guardar a mensagem
#define PACKAGE_SIZE (MESSAGE_SIZE + 8) // 2 bytes pra guardar o destino do pacote, 2 para o remetente e 4 pra informações extras

void wrap_message(char package_space[], char message[], int destination_node_id, int this_node_id, int confirmation, int package_id);
void unwrap_message(int *bitmask, short int *destination, short int *sender, char *message, char *buffer, int *is_confirmation, int *id_package);

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

/** Função que le o arquivo enlaces.config e cria uma matriz de adjacência no vetor bidimensional routers_links passado por parâmetro. */
int read_links(int routers_links[MAX_ROUTERS][MAX_ROUTERS]) {
  FILE *links_file;
  int router1, router2, cost;


  links_file = fopen("enlaces.config", "r");
  if (!links_file) {
    return 1;
  }

  memset(routers_links, -1, MAX_ROUTERS * MAX_ROUTERS * sizeof(int));
  for (int i = 0; i < MAX_ROUTERS; i++) routers_links[i][i] = 0;
  while(fscanf(links_file, "%d %d %d\n", &router1, &router2, &cost) != EOF) {
    routers_links[router1][router2] = cost;
    routers_links[router2][router1] = cost;
  }

  fclose(links_file);
  return 0;
}

// Função de debug
void print_routers_ports(int routers_ports[MAX_ROUTERS]) {
  printf("routers_ports:\n");
  for (int i = 0; i < MAX_ROUTERS; i++) {
    printf("    Índice %2d: %d\n", i, routers_ports[i]);
  }
};

// Função de debug
void print_array(int *array, int tamanho) {
  printf("[%d", array[0]);
  for (int i = 1; i < tamanho; i++) {
    printf(", %d", array[i]);
  }
  printf("]\n");
};

// Função de debug
void print_routers_addresses(char routers_addresses[MAX_ROUTERS][MAX_ROUTER_ADDRESS_SIZE]) {
  printf("\nrouters_addresses:\n");
  for (int i = 0; i < MAX_ROUTERS; i++) {
    printf("    %d: ", i);
    printf("%s\n", routers_addresses[i]);
  }
}

// Função de debug
void print_routers_links(int routers_links[MAX_ROUTERS][MAX_ROUTERS]) {
  printf("\nrouters_links:\n");
  printf("       ");
  for(int i = 0; i < MAX_ROUTERS; i++) { printf(" %3d", i); }
  printf("\n");
  for(int i = 0; i < MAX_ROUTERS; i++) {
    printf("    %2d: ", i);
    for (int j = 0; j < MAX_ROUTERS; j++) {
      if (i == j) {
        printf("\033[1;31m%3d\033[0m ", routers_links[i][j]);
      } else if (routers_links[i][j] == -1) {
        printf("\033[90m%3d\033[0m ", routers_links[i][j]);
      } else {
        printf("\033[1;32m%3d\033[0m ", routers_links[i][j]);
      }
    }
    printf("\n");
  }
}

void die(char *s) {
  perror(s);
  exit(1);
}

// Função que envia os dados do +package+ para o nó com id +node_id+. Tanto o cliente (para enviar) quanto o servidor (para reencaminhar) usam essa função.
void send_package(char *package,
                  int node_id,
                  char routers_addresses[MAX_ROUTERS][MAX_ROUTER_ADDRESS_SIZE],
                  int routers_ports[MAX_ROUTERS],
                  int next_nodes[MAX_ROUTERS]) {
  struct sockaddr_in si_other;
  int s, slen = sizeof(si_other);
  int jump_node = next_nodes[node_id];

  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    die("socket");
  }

  memset((char *) &si_other, 0, sizeof(si_other));
  si_other.sin_family = AF_INET;

  si_other.sin_port = htons(routers_ports[jump_node]);
  if (inet_aton(routers_addresses[jump_node] , &si_other.sin_addr) == 0) {
    fprintf(stderr, "inet_aton() failed\n");
    exit(1);
  }

  //send the message
  if (sendto(s, package, 8 + strlen(package + 8), 0, (struct sockaddr *) &si_other, slen) == -1) {
      die("sendto()");
  }

  close(s);
}

// Função responsável por criar a interface do client (o que manda a mensagem). Baseada no arquivo client-udp mandado pelo professor.
void start_client(int node_id,
                  char routers_addresses[MAX_ROUTERS][MAX_ROUTER_ADDRESS_SIZE],
                  int routers_ports[MAX_ROUTERS],
                  int next_nodes[MAX_ROUTERS],
                  int *package_id) {
  char message[MESSAGE_SIZE];
  short int message_target_node;
  char package[PACKAGE_SIZE];

  while(1) {
    scanf("%hd<", &message_target_node);
    fgets(message, MESSAGE_SIZE, stdin);
    message[strlen(message) - 1] = '\0';

    //O penúltimo parâmetro da função é se é uma mensagem de confirmação. Nesse caso é 0 pois não é.
    wrap_message(package, message, message_target_node, node_id, 0, *package_id);
    *package_id = (*package_id + 1) % 8; //Estamos dando 3 bits para id's de pacotes. Caso seja mandado mais, o id será resetado.

    printf("\n\033[1;32m⬆\033[0m ");
    printf("[\033[1;31m%d\033[0m], ", node_id);
    if (message_target_node != next_nodes[message_target_node]) {
      printf("\033[1;31m%d\033[0m, ..., ", next_nodes[message_target_node]);
    }
    printf("\033[1;31m%d\033[0m\n", message_target_node);

    send_package(package, message_target_node, routers_addresses, routers_ports, next_nodes);
  }
};

// Função responsável por criar a interface do server (o que recebe a mensagem). Baseada no arquivo server-udp mandado pelo professor.
void start_server(int node_id,
                  char routers_addresses[MAX_ROUTERS][MAX_ROUTER_ADDRESS_SIZE],
                  int routers_ports[MAX_ROUTERS],
                  int next_nodes[MAX_ROUTERS]) {
  struct sockaddr_in si_me, si_other;
  int s, slen = sizeof(si_other) , recv_len;
  char buffer[PACKAGE_SIZE];

  //create a UDP socket
  if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
      die("socket");
  }

  // zero out the structure
  memset((char *) &si_me, 0, sizeof(si_me));

  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(routers_ports[node_id]);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);

  //bind socket to port
  if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) {
      die("bind");
  }


  //keep listening for data
  while(1) {
    int package_bitmask;
    short int destination_node_id, sender_node_id;
    char message[MESSAGE_SIZE];
    int is_confirmation, id_package;

    memset(buffer, 0, PACKAGE_SIZE);

    //try to receive some data, this is a blocking call
    recv_len = recvfrom(s, buffer, PACKAGE_SIZE, 0, (struct sockaddr *) &si_other, &slen);
    if (recv_len == -1) { die("recvfrom()"); }

    unwrap_message(&package_bitmask, &destination_node_id, &sender_node_id, message, buffer, &is_confirmation, &id_package);

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
        send_package(package, message_target_node, routers_addresses, routers_ports, next_nodes);
      } else {
        printf("Pacote %d entregue com sucesso!\n", id_package);
      }
    } else {
      // Recebe pacote com outro destino
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
      send_package(buffer, destination_node_id, routers_addresses, routers_ports, next_nodes);
    }
  }

  close(s);
}

void wrap_message(char *package_space, char *message, int destination_node_id, int this_node_id, int confirmation, int package_id) {
  memset(package_space, 0, 4);
  if (confirmation) package_space[0] = (1 << 7);
  package_space[0] |= (unsigned char)(package_id << 4);
  package_space[4] = (unsigned char)(destination_node_id >> 8);
  package_space[5] = (unsigned char)destination_node_id;
  package_space[6] = (unsigned char)(this_node_id >> 8);
  package_space[7] = (unsigned char)this_node_id;
  strncpy(package_space + 8, message, MESSAGE_SIZE);
  if (DEBUG_MODE) {
    printf("\n\n[DEBUG]Wrapping the message:\n");
    printf("[DEBUG]Message: %s\n", message);
    printf("[DEBUG]destination: %d → %hd\n", destination_node_id, ((package_space[4] << 8) | package_space[5]));
    printf("[DEBUG]package: ");
    for(int i = 0; i < PACKAGE_SIZE; i++) { printf("%c", package_space[i]); }
    printf("\n");
    printf("[DEBUG]message_size: %d\n", MESSAGE_SIZE);
  }
}

void unwrap_message(int *bitmask, short int *destination, short int *sender, char *message, char *buffer, int *is_confirmation, int *id_package) {
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
  strncpy(message, &buffer[8], MESSAGE_SIZE);
  if (DEBUG_MODE) {
    printf("[DEBUG]Unwrapping the message:\n");
    printf("[DEBUG]Buffer: ");
    for(int i = 0; i < PACKAGE_SIZE; i++) { printf("%c", buffer[i]); }
    printf("\n");
    printf("[DEBUG]Message: %s\n", message);
    printf("[DEBUG]bitmask: %d\n", *bitmask);
    printf("[DEBUG]destination: %hd\n", *destination);
  }
}

int build_next_nodes_list(int next_nodes[MAX_ROUTERS],
                          int routers_links[][MAX_ROUTERS],
                          int origin_node) {
  int distance[MAX_ROUTERS];
  int predecessors[MAX_ROUTERS];
  int node, i, neighbour; // Variáveis auxiliares

  // Coloca -1 em todos os ancestrais
  memset(predecessors, -1, MAX_ROUTERS * sizeof(int));
  predecessors[origin_node] = origin_node;

  // Coloca infinito nas distâncias
  for(i = 0; i < MAX_ROUTERS; i++) { distance[i] = 0x7FFFFFFF / 2; }
  distance[origin_node] = 0;

  // Bellman-Ford
  for (i = 0; i < MAX_ROUTERS; i++) {
    for (node = 1; node < MAX_ROUTERS; node++) {
      for (neighbour = 1; neighbour < MAX_ROUTERS; neighbour++) {
        if (routers_links[node][neighbour] != -1 &&
            distance[node] + routers_links[node][neighbour] < distance[neighbour]) {

          distance[neighbour] = distance[node] + routers_links[node][neighbour];
          predecessors[neighbour] = node;
        }
      }
    }
  }

  // Constrói o caminho de ancestrais de todos os nós até o nó origem
  for (node = 1; node < MAX_ROUTERS; node++) {
    int node_predecessor = predecessors[node];

    if (node == origin_node) { continue; }
    if (node_predecessor == -1) { continue; }

    while (predecessors[node_predecessor] != origin_node) {
      node_predecessor = predecessors[node_predecessor];
      predecessors[node] = node_predecessor;
    }
  }

  // Os nós vizinhos do origin_node não podem tê-lo como next_node, se não o node_origin fica mandando mensagem pra ele mesmo e entra em loop infinito
  for (node = 1; node < MAX_ROUTERS; node++) {
    if (node != origin_node && predecessors[node] == origin_node) {
      predecessors[node] = node;
    }
  }

  /** Constrói o array de próximos nós.
  Repare que eu poderia ter feito isso no loop anterior, mas decidi deixar isolado pra se caso o código acima for alterado, essa parte de baixo não precisa ser. */
  for (int i = 0; i < MAX_ROUTERS; i++) {
    next_nodes[i] = predecessors[i];
  }
}

int main(int argc, char* argv[]) {
  int node_id = *argv[1] - '0'; // ID do nó deste processo
  int routers_ports[MAX_ROUTERS]; // Lista das portas dos roteadores
  char routers_addresses[MAX_ROUTERS][MAX_ROUTER_ADDRESS_SIZE]; // Lista dos endereços dos roteadores
  int routers_links[MAX_ROUTERS][MAX_ROUTERS]; // Matriz de adjacência do grafo dos roteadores
  int pid;
  int next_nodes[MAX_ROUTERS];
  int package_id = 0;

  printf("Para mandar uma mensagem, digite-a no formato \"(nó)<(mensagem)\":\n");
  printf("Exemplo: \"\033[1;31m1<Laranja\033[0m\" vai mandar \"\033[1;31mLaranja\033[0m\" para o nó com id \033[1;31m1\033[0m.\n\n\n");

  if (read_routers(routers_ports, routers_addresses)) {
    printf("\033\[31mErro ao abrir o arquivo de roteadores.\033[0m\n");
    return 0;
  }

  if (read_links(routers_links)) {
    printf("\033\[31mErro ao abrir o arquivo de enlaces.\033[0m\n");
    return 0;
  }

  build_next_nodes_list(next_nodes, routers_links, node_id);

  if (DEBUG_MODE) { // Printa algumas coisas no console pra debugar mais fácil
    printf("\nID do nó: \033[1;31m%d\033[0m\n", node_id);
    print_routers_ports(routers_ports);
    print_routers_addresses(routers_addresses);
    print_routers_links(routers_links);
    for (int i = 0; i < MAX_ROUTERS; i++) {
      printf("Para ir pra %d, precisa começar em %d\n", i, next_nodes[i]);
    }
  }

  pid = fork();
  if (pid) {
    start_client(node_id, routers_addresses, routers_ports, next_nodes, &package_id);
  } else {
    start_server(node_id, routers_addresses, routers_ports, next_nodes);
  }

  return 0;
};
