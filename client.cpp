#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

#include <iostream>
#include "nlohmann/json.hpp"
#include <string>

using namespace std;
using json = nlohmann::json;

int main(int argc, char *argv[])
{
    char *message, *response, command[255];
    int sockfd;

    // Setting the server address, the type of content sent and the routes
    // to the apis that will be used
    char adress[] = "34.118.48.238";
    char application_json[] = "application/json";
    char register_route[] = "/api/v1/tema/auth/register";
    char login_route[] = "/api/v1/tema/auth/login";
    char enter_library_route[] = "/api/v1/tema/library/access";
    char books_route[] = "/api/v1/tema/library/books";
    char logout_route[] = "/api/v1/tema/auth/logout";

    // Variables to keep credentials and authorisation for the app
    string login_cookie;
    string login_token;

    // Event loop
    while(true) {
        // Reading the command from STDIN
        scanf("%s", command);

        // If the recieved command is exit, close the app
        if (!strcmp(command, "exit")) {
            break;
        }

        // The register command
        if (!strcmp(command, "register")) {
            char username[255], password[255];
            // Prompt for the username and password require to register a user
            printf("username=");
            scanf("%s", username);
            printf("password=");
            scanf("%s", password);

            // Create and populate the json object that will be used as the HTTP Request's body
            json json_register;

            json_register["username"] = username;
            json_register["password"] = password;

            // The json object dumped to string
            string json_payload_string = json_register.dump();

            // Turning the json object to a char* to be used by the compute_post_request function
            char *json_payload_chars = (char *)calloc(json_payload_string.length() + 1, sizeof(char));
            strcpy(json_payload_chars, json_payload_string.c_str());

            // Open the connection to the server
            sockfd = open_connection(adress, 8080, AF_INET, SOCK_STREAM, 0);

            // Compute the message with no cookie or token and print it
            message = compute_post_request(adress, register_route, application_json, json_payload_chars, NULL, 0, NULL);
            cout << message << endl;

            // Send the HTTP Request to the server
            send_to_server(sockfd, message);

            // Recieve the server's response and print it
            response = receive_from_server(sockfd);
            cout << "Message from server:" << endl << response << endl;

            // Depeding on the server's response print a success / fail message
            if (strstr(response, "Created") != NULL) {
                cout << "The user has been created succesfully" << endl << endl;
            }
            else {
                cout << "An error has occurred, please try again" << endl << endl;
            }

            // Close the connection and free the dyncamically allocated memory
            close_connection(sockfd);
            free(json_payload_chars);
            free(message);
        }

        // Login command
        else if (!strcmp(command, "login")) {
            char username[255], password[255];
            // Prompt the user for the username and password required
            printf("username=");
            scanf("%s", username);
            printf("password=");
            scanf("%s", password);

            // Create and populate the json object that will be used as the HTTP Request's body
            json json_register;

            json_register["username"] = username;
            json_register["password"] = password;

            // The json object dumped to string
            string json_payload_string = json_register.dump();

            // Turning the json object to a char* to be used by the compute_post_request function
            char *json_payload_chars = (char *)calloc(json_payload_string.length() + 1, sizeof(char));
            strcpy(json_payload_chars, json_payload_string.c_str());

            // Open the connection to the server
            sockfd = open_connection(adress, 8080, AF_INET, SOCK_STREAM, 0);

            // Compute the message with no cookie or token and print it
            message = compute_post_request(adress, login_route, application_json, json_payload_chars, NULL, 0, NULL);
            cout << message << endl;

            // Send the HTTP Request to the server
            send_to_server(sockfd, message);

            // Recieve the server's response and print it
            response = receive_from_server(sockfd);
            cout << "Message from server:" << endl << response << endl;

            // Casting the response to string to parse it easier
            string response_string = response;

            // Finding the positions of the login cookie and use substr to extract it and store it
            int pozitie_initiala = response_string.find("connect.sid=");
            int pozitie_finala = response_string.find("; Path=", pozitie_initiala + 1); 

            // If the positions have been found, extract the cookie, store it and print it
            if (pozitie_initiala != -1 && pozitie_finala != -1) {
                login_cookie = response_string.substr(pozitie_initiala, pozitie_finala - pozitie_initiala);
                cout << "Cookie:" << endl << login_cookie << endl << endl;
                cout << "Logged in successfully" << endl << endl;
            }
            else {
                cout << "An error has occurred, please try again" << endl << endl;
            }
            
            // Close the connection and free the memory
            close_connection(sockfd);
            free(json_payload_chars);
            free(message);
        }

        // Enter library command
        else if (!strcmp(command, "enter_library")) {
            // Open the connection
            sockfd = open_connection(adress, 8080, AF_INET, SOCK_STREAM, 0);

            // If the user has logged in, the cookie must be valid
            if (login_cookie.length() != 0) {
                // Create ("cast") a char **cookie from the stored string login_cookie to be
                // used in the compute_get_request function
                char **cookies = (char **)malloc(sizeof(char *));
                cookies[0] = (char *)calloc(login_cookie.length() + 1, sizeof(char));
                strcpy(cookies[0], login_cookie.c_str());

                // Compute the message with the login cookie and print it
                message = compute_get_request(adress, enter_library_route, NULL, cookies, 1, NULL);
                cout << message << endl;

                // Send the HTTP Request to the server
                send_to_server(sockfd, message);

                // Recieve the server's response and print it
                response = receive_from_server(sockfd);
                cout << "Message from server:" << endl << response << endl;
            
                // Casting the response to string to parse it easier
                string response_string = response;

                // Finding the positions of the access token and use substr to extract it and store it
                int pozitie_initiala = response_string.find("{\"token\":\"");
                int pozitie_finala = response_string.find("\"}", pozitie_initiala + 1);

                // If the positions have been found, extract the token, store it and print it
                if (pozitie_initiala != -1 && pozitie_finala != -1) {
                    login_token = response_string.substr(pozitie_initiala + 10, pozitie_finala - pozitie_initiala - 10);
                    cout << endl << "Token:" << endl << login_token << endl << endl;
                    cout << "Welcome to the library... so tell me, what is it you truly desire?" << endl << endl;
                }
                else {
                    cout << "An error has occurred, please try again" << endl << endl;
                }

                // Free the memory
                free(cookies[0]);
                free(cookies);
                free(message);
            }
            else {
                cout << "Please log in first" << endl;
            }

            // Close the connection
            close_connection(sockfd);
        }

        // The get books command
        else if (!strcmp(command, "get_books")) {
            // Open the connection
            sockfd = open_connection(adress, 8080, AF_INET, SOCK_STREAM, 0);

            // If the user has access to the library, the token must be valid
            if (login_token.length() != 0) {
                // Copy the login token into a char * so that it can be used in the 
                // compute_get_request function
                char *token = (char *)calloc(login_token.length() + 1, sizeof(char));
                strcpy(token, login_token.c_str());

                // Compute the message with the login token and print it
                message = compute_get_request(adress, books_route, NULL, NULL, 0, token);
                cout << message << endl;

                // Send the HTTP Request to the server
                send_to_server(sockfd, message);

                // Recieve the server's response and print it
                response = receive_from_server(sockfd);
                cout << "Message from server:" << endl << response << endl;

                // Casting the response to string to parse it easier
                string response_string = response;

                // Finding the positions of the response body
                int pozitie_initiala = response_string.find("[");
                int pozitie_finala = response_string.find("]", pozitie_initiala + 1);

                // If the body of the response, a json array is found, use substr to extract
                // the books array and print it
                if (pozitie_initiala != -1 && pozitie_finala != -1) {
                    cout << endl << "Here are the recorded books:" << endl << response_string.substr(pozitie_initiala, pozitie_finala - pozitie_initiala + 1) << endl << endl;
                }
                else {
                    cout << "An error has occurred, please try again" << endl << endl;
                }

                // Free the memory
                free(token);
                free(message);
            }
            else {
                cout << "No access to the library" << endl << endl;
            }

            // Close the connection
            close_connection(sockfd);
        }

        // The add book command
        else if (!strcmp(command, "add_book")) {
            // Open the connection
            sockfd = open_connection(adress, 8080, AF_INET, SOCK_STREAM, 0);

            // If the user has access to the library, the token must be valid
            if (login_token.length() != 0) {
                char title[255], author[255], genre[255], publisher[255], page_count[255];
                // Prompt the user for the needed input of the add_book command
                printf("title=");
                scanf("%s", title);
                printf("author=");
                scanf("%s", author);
                printf("genre=");
                scanf("%s", genre);
                printf("publisher=");
                scanf("%s", publisher);
                printf("page_count=");
                scanf("%s", page_count);

                // "Cast" the token to a char* for it to be used in the compute_post_request function
                char *token = (char *)calloc(login_token.length() + 1, sizeof(char));
                strcpy(token, login_token.c_str());

                // Create and populate the json object that will be used as the HTTP Request's body
                json json_add_book;

                json_add_book["title"] = title;
                json_add_book["author"] = author;
                json_add_book["genre"] = genre;
                json_add_book["publisher"] = publisher;
                json_add_book["page_count"] = page_count;

                // The json object dumped to string
                string json_payload_string = json_add_book.dump();

                // Turning the json object to a char* to be used by the compute_post_request function
                char *json_payload_chars = (char *)calloc(json_payload_string.length() + 1, sizeof(char));
                strcpy(json_payload_chars, json_payload_string.c_str());

                // Compute the message with the login token and print it
                message = compute_post_request(adress, books_route, application_json, json_payload_chars, NULL, 0, token);
                cout << message << endl;

                // Send the HTTP Request to the server
                send_to_server(sockfd, message);

                // Recieve the server's response and print it
                response = receive_from_server(sockfd);
                cout << "Message from server:" << endl << response << endl;

                // Depending on whether the response was successful or not print an adequate message
                if (strstr(response, "OK") != NULL) {
                    cout << "Book " << json_payload_string << " added" << endl << endl;
                }
                else {
                    cout << "An error has occurred, please try again" << endl << endl;
                }

                // Free the memory
                free(token);
                free(json_payload_chars);
                free(message);
            }
            else {
                cout << "No access to the library" << endl << endl;
            }

            // Close the connection
            close_connection(sockfd);
        }
        
        // The get book command
        else if (!strcmp(command, "get_book")) {
            // Open the connection
            sockfd = open_connection(adress, 8080, AF_INET, SOCK_STREAM, 0);

            // If the user has access to the library, the token must be valid
            if (login_token.length() != 0) {
                char id[255];
                // Prompt the user for an id
                printf("id=");
                scanf("%s", id);

                // "Cast" the token to a char* for it to be used in the compute_post_request function
                char *token = (char *)calloc(login_token.length() + 1, sizeof(char));
                strcpy(token, login_token.c_str());

                // Build the specific route for the book
                char book_route[255];
                strcpy(book_route, books_route);
                strcat(book_route, "/");
                strcat(book_route, id);

                // Compute the message with the login token and print it
                message = compute_get_request(adress, book_route, NULL, NULL, 0, token);
                cout << message << endl;

                // Send the HTTP Request to the server
                send_to_server(sockfd, message);

                // Recieve the server's response and print it
                response = receive_from_server(sockfd);
                cout << "Message from server:" << endl << response << endl;

                // Casting the response to string to parse it easier
                string response_string = response;

                // Finding the positions of the response body
                int pozitie_initiala = response_string.find("[{");
                int pozitie_finala = response_string.find("}]", pozitie_initiala + 1);

                // If the body of the response, a json array is found, use substr to extract
                // the books array and print it
                if (pozitie_initiala != -1 && pozitie_finala != -1) {
                    cout << endl << "Here is your book:" << endl << response_string.substr(pozitie_initiala, pozitie_finala - pozitie_initiala + 2) << endl << endl;
                }
                else {
                    cout << "An error has occurred, please try again" << endl << endl;
                }

                // Free the memory
                free(token);
                free(message);
            }
            else {
                cout << "No access to the library" << endl << endl;
            }

            // Close the connection
            close_connection(sockfd);
        }

        // The delete book command
        else if (!strcmp(command, "delete_book")) {
            // Open the connection
            sockfd = open_connection(adress, 8080, AF_INET, SOCK_STREAM, 0);

            // If the user has access to the library, the token must be valid
            if (login_token.length() != 0) {
                char id[255];
                // Prompt the user for an id
                printf("id=");
                scanf("%s", id);

                // "Cast" the token to a char* for it to be used in the compute_post_request function
                char *token = (char *)calloc(login_token.length() + 1, sizeof(char));
                strcpy(token, login_token.c_str());

                // Build the specific route for the book
                char book_route[255];
                strcpy(book_route, books_route);
                strcat(book_route, "/");
                strcat(book_route, id);

                // Compute the message with the login token and print it
                message = compute_delete_request(adress, book_route, NULL, NULL, 0, token);
                cout << message << endl;

                // Send the HTTP Request to the server
                send_to_server(sockfd, message);

                // Recieve the server's response and print it
                response = receive_from_server(sockfd);
                cout << "Message from server:" << endl << response << endl;

                // Depending on whether the response was successful or not print an adequate message
                if (strstr(response, "OK") != NULL) {
                    cout << "Book " << id << " deleted" << endl << endl;
                }
                else {
                    cout << "An error has occurred, please try again" << endl << endl;
                }

                // Free the memory
                free(token);
                free(message);
            }
            else {
                cout << "No access to the library" << endl << endl;
            }

            // Close the connection
            close_connection(sockfd);
        }

        // The logout command
        else if (!strcmp(command, "logout")) {
            // Open the connection
            sockfd = open_connection(adress, 8080, AF_INET, SOCK_STREAM, 0);

            // If the user has access to the library, the token must be valid
            if (login_cookie.length() != 0) {
                // Create ("cast") a char **cookie from the stored string login_cookie to be
                // used in the compute_get_request function
                char **cookies = (char **)malloc(sizeof(char *));
                cookies[0] = (char *)calloc(login_cookie.length() + 1, sizeof(char));
                strcpy(cookies[0], login_cookie.c_str());

                // Compute the message with the login cookie and print it
                message = compute_get_request(adress, logout_route, NULL, cookies, 1, NULL);
                cout << message << endl;

                // Send the HTTP Request to the server
                send_to_server(sockfd, message);

                // Recieve the server's response and print it
                response = receive_from_server(sockfd);
                cout << "Message from server:" << endl << response << endl;

                // In order for the client to no longer have access due to the previous connection,
                //clear the variables that store the login cookie and the login token
                login_cookie.clear();
                login_token.clear();

                // Depending on whether the response was successful or not print an adequate message
                if (strstr(response, "OK") != NULL) {
                    cout << "Logged out successfully" << endl << endl;
                }
                else {
                    cout << "An error has occurred, please try again" << endl << endl;
                }

                // Free the memory
                free(cookies[0]);
                free(cookies);
                free(message);
            }
            else {
                cout << "Not logged in to the library" << endl << endl;
            }

            // Close the connection
            close_connection(sockfd);
        }

        // If a command that isn't already implemented is issued, print an error message and
        // wait for a valid command
        else {
            cout << "No such command found" << endl << endl;
        }
    }

    return 0;
}
