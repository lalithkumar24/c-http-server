#include "file.h"
#include "string_operations.h"
#include <dirent.h>
#include <linux/limits.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define CRIF "\r\n"
#define SP " "
int PORT = 6979;

typedef struct {
  string method;
  string uri;
  string version;
} http_req_line;

typedef enum {
  HTTP_RES_OK = 200,
  HTTP_RES_BAD_REQUEST = 400,
  HTTP_RES_NOT_FOUND = 404,
  HTTP_RES_INTERNAL_SERVER_ERROR = 500,
} http_status;

const char *http_status_to_string(http_status status) {
  switch (status) {
  case HTTP_RES_OK:
    return "OK";
  case HTTP_RES_BAD_REQUEST:
    return "Bad Request";
  case HTTP_RES_INTERNAL_SERVER_ERROR:
    return "Internal Server Error";
  case HTTP_RES_NOT_FOUND:
    return "Not Found";
  default:
    return "Unknown";
  }
}

typedef struct {
  string version;
  http_status status;
} http_res_line;

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
  req_line->method.data = components.splits[0].data;
  req_line->method.len = components.splits[0].len;

  req_line->uri.data = components.splits[1].data;
  req_line->uri.len = components.splits[1].len;

  req_line->version.data = components.splits[2].data;
  req_line->version.len = components.splits[2].len;
  free_string_splits(&components);
  return HTTP_RES_OK;
}

string header_generater(char *buf, size_t buf_len, http_status status,
                        size_t body_len) {
  string response;
  response.len = 0;
  memset(buf, 0, buf_len);
  response.len += sprintf(buf, "%s %d %s" CRIF, "HTTP/1.0", status,
                          http_status_to_string(status));
  response.len +=
      sprintf(buf + response.len, "Content-lenght:%zu" CRIF, body_len);
  response.len += sprintf(buf + response.len, CRIF);
  response.data = buf;
  return response;
}

bool send_response(int socked_id, string header, string body) {
  ssize_t n = send(socked_id, header.data, header.len, MSG_MORE);
  if (n < 0) {
    perror("send()");
    return false;
  }
  if (n == 0) {
    fprintf(stderr, "send() returned 0\n");
    return false;
  }
  n = send(socked_id, body.data, body.len, 0);
  return true;
}
string passerFile(char *filename) {

  FILE *fileptr;
  char buffer[1024];
  char readBuffer[1000000];
  string content;
  int n = 0;
  memset(&content, 0, sizeof(string));
  fileptr = fopen(filename, "r");
  if (fileptr == NULL) {
    fprintf(stderr, "send() returned 0\n");
    exit(0);
  }
  while (fscanf(fileptr, "%s", buffer) == 1) {
    n += sprintf(readBuffer + n, "%s ", buffer);
  }
  fclose(fileptr);
  content = convert_cstr_string(readBuffer);
  return content;
}
int handle_client(int client_sockid) {
  size_t n = 0;
  char buffer[1024];
  string hello_body = convert_cstr_string("<h1>Hello from WSL Server!</h1>");
  string bye_body = convert_cstr_string("<h1>bye from wsl server!</h1>");
  string defult_body =
      convert_cstr_string("<h1>coustom server writen in c</h1>");
  string err_404 = convert_cstr_string(
      "<p>Error 404: Not Found</p><p><a href=\"/\">Back to home</a></p>");
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

    if (lines.count < 1) {
      printf("Error: empty buffer\n");
      return -1;
    }
    http_req_line req_line = http_req_line_init();
    http_status status =
        pars_req_line(&req_line, lines.splits[0].data, lines.splits[0].len);
    free_string_splits(&lines);
    if (status != HTTP_RES_OK) {
      printf("ERROR: failed to passer request");
      return -1;
    }
    string route_hello = convert_cstr_string("/hello");
    string route_bye = convert_cstr_string("/bye");
    string defult_route = convert_cstr_string("/");
    if (string_equal(&req_line.uri, &defult_route)) {
      if (!send_response(client_sockid,
                         header_generater(buffer, sizeof(buffer), HTTP_RES_OK,
                                          defult_body.len),
                         defult_body))
        return -1;
    } else if (string_equal(&req_line.uri, &route_hello)) {
      if (!send_response(client_sockid,
                         header_generater(buffer, sizeof(buffer), HTTP_RES_OK,
                                          hello_body.len),
                         hello_body))
        return -1;
    } else if (string_equal(&req_line.uri, &route_bye)) {
      if (!send_response(client_sockid,
                         header_generater(buffer, sizeof(buffer), HTTP_RES_OK,
                                          bye_body.len),
                         bye_body))
        return -1;
    } else {
      printf("ERROR: unknown route: \"%.*s\"\n", (int)req_line.uri.len,
             req_line.uri.data);
      (void)send_response(client_sockid,
                          header_generater(buffer, sizeof(buffer),
                                           HTTP_RES_NOT_FOUND, err_404.len),
                          err_404);
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
  const char *web_root = "./www";

  file_metadata metadata = file_status(convert_cstr_string(web_root));
  if (metadata.exists) {
    printf("file exits\n");
  } else {

    /* rwxr-xr-x
      first 3 for user
      second 3 for group
      last 3 is for others
     */
    mkdir(web_root,
          S_IEXEC | S_IWRITE | S_IREAD | S_IRGRP | S_IXGRP | S_IXOTH | S_IROTH);
  }

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
