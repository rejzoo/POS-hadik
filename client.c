#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>

#define PORT 8080

char get_key() {
  struct termios oldt, newt;
  char ch;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;

  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

  return ch;
}

int main(int argc, char *argv[])
{
  int status, valread, client_fd;
  struct sockaddr_in serv_addr;
  char* hello = "CLIENT: Hello from client";
  char buffer[1024] = { 0 };
  if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\nCLIENT: Socket creation error \n");
    return EXIT_FAILURE;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    printf("\nCLIENT: Invalid address/ Address not supported \n");
    return EXIT_FAILURE;
  }

  if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
    printf("\nCLIENT: Connection failed. \n");
    return EXIT_FAILURE;
  }

  send(client_fd, hello, strlen(hello), 0);
  printf("CLIENT: Hello message sent\n");
  
  while (1) {
    
  }

  /*
  //valread = read(client_fd, buffer, 1024 - 1);
  read(client_fd, buffer, 1024 - 1);

  printf("%s\n", buffer);
  */

  close(client_fd);

  return EXIT_SUCCESS;
}
