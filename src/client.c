#include <errno.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#define PORT 8000

struct thread_args {
  int client_socket;
};

void* read_client_messages(void* args) {
  char buffer[1024] = {0};
  struct thread_args *my_args = (struct thread_args*) args; 
  int client_socket = my_args -> client_socket; 
  while (1) {
    if (read(client_socket, buffer, 1024 - 1) < 0) {
      printf("Failed to read. Error: %s\n", strerror(errno));
    }
    printf("%s\n", buffer);
  }
}

int main(int argc, char const* argv[]) {
  int status, client_socket;
  struct sockaddr_in serv_addr;
  char message[1024] = {0};
  if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
     printf("\n Socket creation error \n"); 
     return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
     printf("\n Invalid address, Address not supported \n"); 
     return -1;
  }

  if ((status = connect(client_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0)  {
     printf("\n Connection failed \n");		  
     return -1;
  }
    
  pthread_t thread_id;
  struct thread_args args;
  args.client_socket = client_socket;
  pthread_create(&thread_id, NULL, read_client_messages, (void *) &args);  
  while (strcmp(message, "exit") != 0) {
     scanf("%s", message);
     send(client_socket, message, strlen(message), 0);
  }

  return 0;
}
