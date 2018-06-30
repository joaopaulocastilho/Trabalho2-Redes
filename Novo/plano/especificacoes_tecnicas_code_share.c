// Você escreve abaixo da linha, eu escrevo acima
// Pacote:
// |Destino 4B|Tipo 1B|Origem 4B|Payload ?B|
#define TIPO_PACOTE_MENSAGEM_USUARIO 1
#define TIPO_PACOTE_CONFIRMACAO_MENSAGEM_USUARIO 2
#define TIPO_PACOTE_VETOR_DISTANCIA 3
#define TIPO_PACOTE_CONFIRMACAO_VETOR_DISTANCIA 4
#define TIPO_PACOTE_CHECA_NO_ATIVO 5
#define TIPO_PACOTE_CONFIRMACAO_NO_ATIVO 6

#define TAMANHO_TOTAL_PACOTE 516 //Dá pra mudar
#define TAMANHO_BUFFER_ENTRADA 516
#define TAMANHO_BUFFER_SAIDA 516
#define QUANTIDADE_MAXIMA_NOS 50

#define INFINITO 1123456789

<mutex_sei_la> buffer_entrada_mutex;
pacote_t buffer_entrada[TAMANHO_TOTAL_PACOTE * TAMANHO_BUFFER_ENTRADA]; // Na imagem: "mensagens recebidas"
int ultimo_pacote_buffer_entrada;

<mutex_sei_la> buffer_saida_mutex;
pacote_t buffer_saida[TAMANHO_TOTAL_PACOTE * TAMANHO_BUFFER_SAIDA]; // Na imagem: "mensagens para enviar"
int ultimo_pacote_buffer_saida;

<mutex_sei_la> mensagens_log_mutex;
pacote_t mensagens_log[TAMANHO_TOTAL_PACOTE * 10]; // Na imagem: "msg :)"

<mutex_sei_la> vetor_saltos_mutex;
int vetor_saltos[QUANTIDADE_MAXIMA_NOS]; // Na imagem: "vetor saltos"

<mutex_sei_la> vetores_distancia_recebidos_mutex;
pacote_t vetores_distancia_recebidos[TAMANHO_BUFFER_ENTRADA]; // Na imagem: "V. D. recebidos"
int indice_nova_mensagem; // Pra guardar onde uma nova mensagem deve ser inserida no array de cima (A menos que já tenha uma mensagem lá, aí deve descartar o pacote.)
// Lembrar de aumentar esse número toda vez que uma thread inserir uma mensagem no vetores_distancia_recebidos

<mutex_sei_la> tabela_distancia_mutex;
int tabela_distancia[QUANTIDADE_MAXIMA_NOS][QUANTIDADE_MAXIMA_NOS]; // Na imagem: "tabela distância"

<mutex_sei_la> checagens_recebidas_mutex;
pacote_t checagens_recebidas[BUFFER_ENTRADA]; // Na imagem: "nudges recebidos". É as checagens que os outros nós fazem

<mutex_sei_la> resposta_checagens_bitmask_mutex;
int resposta_checagens_bitmax; // Na imagen: "bitmask ativas". Onde vai ficar guardada as respostas dos "nudges" enviados

// Acho que a parte de compartilhamento de variáveis é isso aí em cima


// Delegações
// Receiver (receptor): Recebe uma mensagem. Checa se a mensagem é para o nó do processo atual. Se for, coloca no "mensagens recebidas"(buffer_entrada).
//     se não for, vai ter que tratar pra reencaminhar. Tem que colocar na imagem como vai ser (!!!!!)
// Unpacker (Redirecionador): Agora que sabe-se que o pacote recebido é pra esse nó, ele redirecionada para a thread que vai cuidar do pacote, como tá lá na imagem.
//    Ele fica checando sempre se tem pacote no buffer de entrada
// "Nudger" (Checador): Ele vai mandar checagens para os vizinhos, esperar um tempo e ver se a resposta dessas checagens chegaram. Se não chegaram, ele coloca
//     distância infinita na tabela de distância para os vizinhos que não responderam. Se todas chegarem, ele segue o baile
// "Reply Nudger" (?Contar-Checador?): Se chegou um pacote de checagem, ele vai montar um pacote de resposta para essa checagem. Coloca esse pacote no buffer
//    de saída
// "Send vector periodically" (?Transmissor Periódico): De tempos em tempos vai mandar o vetor distância do nó atual para os vizinhos
// "Updater tabela" (Atualizar Saltos): Ao contrário do nome, ele vai atualizar o vetor de saltos de tempos em tempos.
// "Message typebox (console)" (User Message Interface): Vai cuidar da parte de ler uma mensagem do console do usuário e enviar pra onde ele quiser. Ele empacota
//    a mensagem e colocar no buffer de saída
// "Sender" (Transmissor): Pega o que está no buffer de saída e envia para o próximo nó de acordo com o vetor de saltos
// "Unpack update vetor distância" (Atualizador Vetor Distâncias): Pega os vetores distâncias recebidos e atualiza a tabela de distância
// "Printer" (User Message Receiver): Recebe mensagens (de usuário) de outros nós, printa na tela e envia confirmação de mensagem recebida

// Todas as threads terão acesso ao log. Cada thread pode escrever o que quiser no log. Daria até pra cada thread criar uma nova pra escrever no log, assim
//    ela não perde tempo esperando o log estar pronto. Essa nova thread retorna (termina) depois de conseguir escrever no log. Mas isso já é "mais avançado"
<mutex_sei_la> log_mutex;
FILE log;



// Rascunho do sender --------------------
void sender(/**...*/) {
  while(TRUE) {
    pthread_mutex_lock(&buffer_saida_mutex);
    strcpy(pacote_para_enviar, buffer_saida[ultimo_pacote_buffer_saida]);
    if (pacote_valido(pacote_para_enviar)) { // Se o pacote for válido (não-vazio), aumenta o contador e coloca null no lugar onde ele leu
      ultimo_pacote_buffer_saida = (ultimo_pacote_buffer_saida + 1) % TAMANHO_BUFFER_SAIDA;
      buffer_saida[ultimo_pacote_buffer_saida] = NULL // Vai ser com um memset ou sei lá, o que importa é a ideia
    }
    pthread_mutex_unlock(&buffer_saida_mutex);
    // ...
    // envia pacote
    // ...
  }
}
// ----------------------------



--------------------------------------------------
//Entre as linhas é lugar compartilhado. Os dois podem editar

  //O atualiza tabela já muda o vetor de saltos também, né? -- Sim. Acho que ele seria mais "atualiza_saltos" do que "atualiza_tabela", na verdade. Porque todos aqueles ligados
  // vão atualizar a tabela. Ele só vai ver o que mudou na tabela e ver se precisa mudar os saltos  Ok.

  //Eu acho que aquele infinito (-1) a gente seta pra 10^9 e passa no pacote como -1, pq depois vai ter que pegar o min no bellman-ford. -- Mas não tem bellman ford

  //Atualizar as tabelas, meu caro. -- Ah, pode ser. Fica mais fácil.

  //vetores_distancia_recebidos ==> Nesse vetor, como vou saber de qual nó é esse vetor distância? -- Vai ser um pacote salvo ali. Dá pra fazer outra estrutura também
  //    mas não vejo problema em ser um pacote. Depois, como tá tudo isolado, é fácil mudar, mas por enquanto acho que não vale a pena complicar mais.
  //    Aí só pega e repassa o pacote, cada thread que se vire pra lidar com ele --> Ah, sim, eu não tinha visto que isso é um pacote salvo...

  //Vamos deixar cada thread escrever no log por conta mesmo.
--------------------------------------------------

pthread_t id_receptor = 1, id_enviador = 2, id_unpacker = 3, id_nudger = 4, id_unpack_vetor_distancia = 5;
pthread_t id_reply_nudger = 6, id_manda_vetor_sempre = 7, id_atualiza_saltos = 8, id_console = 9, id_printer = 10;


int main(int argc, char* argv[]) {
  nodo_processo_id = *argv[1] = '0';
  pthread_mutex_init(&lock, NULL); //É ASSIM QUE INICIA UM MUTEX.

  //Vamos iniciar as threads
  pthread_create(&id_receptor, NULL, &receptor, NULL); //Cria thread que vai receber os pacotes.
  pthread_create(&id_enviador, NULL, &enviador, NULL); //Cria thread que vai enviar os pacotes.
  pthread_create(&id_unpacker, NULL, &unpacker, NULL); //Cria thread que vai desempacotar os pacotes recebidos.
  pthread_create(&id_nudger, NULL, &nudger, NULL); //Cria thread que vai ficar responsável por ver se os vizinhos estão disponíveis.
  pthread_create(&id_unpack_vetor_distancia, NULL, &unpack_vetor_distancia, NULL); //Vai abrir os vetores da tabela de roteamento dos vizinhos.
  pthread_create(&id_reply_nudger, NULL, &reply_nudger, NULL); //Responde para o vizinho que perguntou se eu estou ativo.
  pthread_create(&id_manda_vetor_sempre, NULL, &manda_vetor_sempre, NULL); //Manda o meu vetor da tabela de roteamento para meus vizinhos.
  pthread_create(&id_atualiza_saltos, NULL, &atualiza_saltos, NULL); //Atualiza a tabela de roteamento e muda o vetor de saltos.
  pthread_create(&id_console, NULL, &console, NULL); //Gerencia o negócio todo.
  pthread_create(&id_printer, NULL, &printer, NULL); //Imprime os pacotes.

  pthread_join(id_receptor, NULL);
  pthread_join(id_enviador, NULL);
  pthread_join(id_unpacker, NULL);
  pthread_join(id_nudger, NULL);
  pthread_join(id_unpack_vetor_distancia, NULL);
  pthread_join(id_reply_nudger, NULL);
  pthread_join(id_manda_vetor_sempre, NULL);
  pthread_join(id_atualiza_saltos, NULL);
  pthread_join(id_console, NULL);
  pthread_join(id_printer, NULL);
  return 0;
}





