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

// Function that computes a GET Request that includes the host that it will
// be sent to, query_params, cookies (if any), the number of cookies and
// the token (if needed)
char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *token)
{
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    // Write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
    compute_message(message, line);

    // Add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
        sprintf(line, "Cookie:");
       for (int i = 0; i < cookies_count; i++) {
            strcat(line, " ");
            strcat(line, cookies[i]);
            strcat(line, ";");
       }
       compute_message(message, line);
    }

    // Add the token (if one is provided)
    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // Add final new line
    compute_message(message, "\n");
    return message;
}

// Function that computes a DELETE Request, the host and the url to which it will
// be sent to are added, as well as query_params, cookies (if any), the number of
// to be sent cookies and a token, if needed
// The function is identical to the GET Request function, the only difference
// is the type of the request specified in the header
char *compute_delete_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *token)
{
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    // Write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }
    compute_message(message, line);

    // Add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
        sprintf(line, "Cookie:");
       for (int i = 0; i < cookies_count; i++) {
            strcat(line, " ");
            strcat(line, cookies[i]);
            strcat(line, ";");
       }
       compute_message(message, line);
    }

    // Add the token, if one is provided
    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // Add final new line
    compute_message(message, "\n");
    return message;
}

// Function to compute a POST Request, including the host and the url of the destination
// (server) it will be sent to, as well as the type of the content (the only type used
// in the application is applciation/json), the body of the request as string, cookies
// the number of the cookies sent and the token
char *compute_post_request(char *host, char *url, char* content_type, char *body_data,
                                char **cookies, int cookies_count, char *token)
{
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    // Write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    /* Add necessary headers (Content-Type and Content-Length are mandatory)
            in order to write Content-Length you must first compute the message size
    */

    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);
    sprintf(line, "Content-Length: %ld", strlen(body_data));
    compute_message(message, line);

    // Add cookies
    if (cookies != NULL) {
       sprintf(line, "Cookie:");
       for (int i = 0; i < cookies_count; i++) {
           sprintf(line, " %s;", cookies[i]);
           compute_message(message, line);
       }
    }

    // Add the token, if one is provided
    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // Add new line at end of header
    compute_message(message, "");

    // Add the actual payload data
    memset(line, 0, LINELEN);
    compute_message(message, body_data);

    free(line);
    return message;
}
