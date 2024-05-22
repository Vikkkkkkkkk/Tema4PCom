#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, const char *url, char *query_params,
                            char **cookies, int cookies_count, char *token_jwt)
{
    char *message = (char *) calloc(BUFLEN, sizeof(char));
    char *line = (char *) calloc(LINELEN, sizeof(char));

    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (cookies != NULL && cookies_count > 0) {
        sprintf(line, "Cookie: ");
        strcat(line, cookies[0]);

        for(int i = 1; i < cookies_count; i++){
            strcat(line, "; ");
            strcat(line, cookies[i]);
        }

        compute_message(message, line);
    }

    sprintf(line, "Authorization: Bearer %s", token_jwt);
    compute_message(message, line);

    compute_message(message, "");
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, const char *payload, char **cookies, int cookies_count, char *token_jwt)
{
    char *message = (char *) calloc(BUFLEN, sizeof(char));
    char *line = (char *) calloc(LINELEN, sizeof(char));

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    sprintf(line, "Content-Length: %ld", strlen(payload));
    compute_message(message, line);
    
    if (cookies != NULL && cookies_count > 0) {
        sprintf(line, "Cookie: ");
        strcat(line, cookies[0]);

        for(int i = 1; i < cookies_count; i++){
            strcat(line, "; ");
            strcat(line, cookies[i]);
        }

        compute_message(message, line);
    }
    sprintf(line, "Authorization: Bearer %s", token_jwt);
    compute_message(message, line);

    compute_message(message, "");

    memset(line, 0, LINELEN);
    strcat(message, payload);

    free(line);
    return message;
}

char* compute_delete_request(char *host, const char *url, char *content_type, char *token_jwt)
{
    char *message = (char *) calloc(BUFLEN, sizeof(char));
    char *line = (char *) calloc(LINELEN, sizeof(char));

    sprintf(line, "DELETE %s HTTP/1.1", url);
    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    sprintf(line, "Authorization: Bearer %s", token_jwt);
    compute_message(message, line);

    compute_message(message, "");

    free(line);
    return message;
}

