#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8000

typedef struct {
    int *data;
    int capacity;
    int size;
} Vector;

void vector_print(Vector *v) {
   printf("[");
   for (int i = 0; i < v -> size; i++) {
     printf("%d,", v -> data[i]);  
   }
   printf("]\n");
}

void vector_init(Vector *v) {
    v->data = NULL;
    v->capacity = 0;
    v->size = 0;
}

void vector_add(Vector *v, int value) {
    if (v->size == v->capacity) {
        v->capacity = v->capacity == 0 ? 1 : v->capacity * 2;
        v->data = realloc(v->data, v->capacity * sizeof(int));
    }
    v->data[v->size++] = value;
}


void vector_remove(Vector *v, int index) {
   if (index < 0 || index >= v->size) return;
   size_t copy_bytes = (v->size - index - 1);   
   memcpy(v->data, v->data + 1, copy_bytes);
   v->size -= 1;
}

int vector_get(Vector *v, int index) {
    if (index >= 0 && index < v->size) {
        return v->data[index];
    }
    // Handle out-of-bounds access appropriately
    return -1;
}

int vector_find(Vector *v, int value) {
   for (int i = 0; i < v -> size; i++) {
      if (v -> data[i] == value) {
         return i; 
      }
   }
   return -1;
}

void vector_free(Vector *v) {
    free(v->data);
    v->capacity = 0;
    v->size = 0;
}



Vector connections;

struct thread_args {
  int server_socket; 
};

void send_messages(char *buffer) {
   for (int i = 0; i < connections.size; i++) {
      send(connections.data[i], buffer, strlen(buffer), 0);
   }  
}

void* handle_server_socket(void* args) {
    ssize_t valread;
    char buffer[1024] = {0};
    while (1) { 
       struct thread_args* my_args = (struct thread_args*) args;
       int server_socket = my_args -> server_socket; 
       valread = read(server_socket, buffer, 1024 - 1); 
       if (valread <= 0) {
         close(server_socket);
         break;
       }
       printf("%s\n", buffer);
       send_messages(buffer); 
     }
     // I'm not 100% sure what the goal was here but it needs to return something.
     return NULL;
} 



int main(int argc, char const* argv[]) {
   int server_fd;
   struct sockaddr_in address;
   int opt = 1;
   socklen_t addrlen = sizeof(address);
   vector_init(&connections);
   
   if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
       perror("socket failed");       
       exit(EXIT_FAILURE);
   } 

   if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
      perror("setsockopt");
      exit(EXIT_FAILURE);
   }

   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(PORT);

   if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0 ) {
     perror("bind failed");
     exit(EXIT_FAILURE);
   }

   if (listen(server_fd, 3) < 0) {
      perror("listen");        
      exit(EXIT_FAILURE);
   }
   
   while (1) {
      int server_socket;
      if ((server_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) > 0) {
         pthread_t thread_id;  
         struct thread_args args;
         args.server_socket = server_socket;
         pthread_create(&thread_id, NULL, handle_server_socket, (void *) &args);
	 vector_add(&connections, server_socket);
	 vector_print(&connections);
         printf("socket %d is listening on thread %ld", server_socket, thread_id);
      }
   }
       return 0;
}
