#include "string_operations.h"
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

const char *CRIF = "\r\n";
const char *SP = " ";
const int PORT = 6979;

typedef struct {
  string method;
  string uri;
  string version;
} http_req_line;

typedef enum {
  HTTP_RES_INTERNAL_SERVER_ERROR = 500,
  HTTP_RES_BAD_REQUEST = 400,
  HTTP_RES_OK = 200,
} http_status;

http_req_line http_req_line_init(void) {
  http_req_line line;
  memset(&line, 0, sizeof(line));
  return line;
}

http_status pars_req_line(http_req_line *req_line, const char *buff,
                          size_t len) {
  if (!buff || !req_line) {
    return HTTP_RES_INTERNAL_SERVER_ERROR;
  }
  string_splits components = split_string(buff, len, SP);
  if (components.count != 3) {
    printf("ERROR:invalid request line: expected components was 3,got %zu\n",
           components.count);
    return HTTP_RES_BAD_REQUEST;
  }
  req_line->method.data = components.splits[0].start;
  req_line->method.len = components.splits[0].len;

  req_line->uri.data = components.splits[1].start;
  req_line->uri.len = components.splits[1].len;

  req_line->version.data = components.splits[2].start;
  req_line->version.len = components.splits[2].len;
  free_string_splits(&components);
  return HTTP_RES_OK;
}

int handle_client(int client_sockid) {
  size_t n = 0;
  char buffer[1024];
  const char *hello = "HTTP/1.0 200 OK\r\n\r\n<h1>Hello from WSL Server!</h1>";
  const char *bye = "http/1.0 200 ok\r\n\r\n<h1>bye from wsl server!</h1>";
  const char *defult =
      "http/1.0 200 ok\r\n\r\n<h1>coustom server writen in c</h1>";
  memset(&buffer, 0, sizeof(buffer));
  printf("\n ------------------------ \n");
  while (true) {
    n = read(client_sockid, &buffer, sizeof(buffer) - 1);
    if (n < 0) {
      perror("read()");
      return -1;
    }

    if (n == 0) {
      printf("\nread ended successfully\n");
      return 1;
    }

    printf("Request:\n%s", buffer);

    string_splits lines = split_string(buffer, n, CRIF);

    if (lines.count < 0) {
      printf("Error: empty buffer\n");
      return -1;
    }
    http_req_line req_line = http_req_line_init();
    http_status status =
        pars_req_line(&req_line, lines.splits[0].start, lines.splits[0].len);
    if (status != HTTP_RES_OK) {
      // todo retuen crt page:
      printf("ERROR: failed to passer request");
      return -1;
    }
    string route_hello = convert_cstr_string("/hello");
    string route_bye = convert_cstr_string("/bye");
    string defult_route = convert_cstr_string("/");
    if (string_equal(&req_line.uri, &defult_route)) {
      (void)write(client_sockid, defult, strlen(defult));
    } else if (string_equal(&req_line.uri, &route_hello)) {
      (void)write(client_sockid, hello, strlen(hello));
    } else if (string_equal(&req_line.uri, &route_bye)) {
      (void)write(client_sockid, bye, strlen(bye));
    } else {
      printf("ERROR:unknown route:%.*s\n", (int)req_line.uri.len,
             req_line.uri.data);
      return -1;
    }
    close(client_sockid);
    break;
  }
  printf("\n ------------------------ \n");
  return 0;
}

int main(void) {
  /* Declare*/
  int rc = 0;
  struct sockaddr_in bind_addrs;
  int tcpsockt = 0;
  int ret = 0;
  int clinet_socket = 0;
  int enable = 1;
  /* Initialize*/
  tcpsockt = socket(AF_INET,     /*IPv4*/
                    SOCK_STREAM, /*TCP*/
                    0 /*Protocall*/);
  memset(&bind_addrs, 0, sizeof(bind_addrs));

  if (tcpsockt == -1) {
    perror("Socket()");
    return 1;
  }
  printf("socket Creation succeeded\n");

  if (setsockopt(tcpsockt, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) <
      0) {
    perror("setsockopt(SO_REUSEADDR) failed");
  };

  bind_addrs.sin_family = AF_INET;
  bind_addrs.sin_port = htons(PORT);
  bind_addrs.sin_addr.s_addr = INADDR_ANY;
  rc = bind(tcpsockt, (const struct sockaddr *)&bind_addrs, sizeof(bind_addrs));
  if (rc < 0) {
    perror("Bind()");
    ret = -1;
    goto exit;
  }
  printf("bind was succeeded\n");

  rc = listen(tcpsockt, SOMAXCONN);
  if (rc < 0) {
    perror("Listen()");
    ret = -1;
    goto exit;
  }
  printf("listening on http://localhost:6979\n");
  while (true) {
    printf("waiting for client... \n");
    clinet_socket = accept(tcpsockt, NULL, NULL);
    printf("Connection established\n");
    rc = handle_client(clinet_socket);
  }

exit:
  close(tcpsockt);
  return ret;
}
