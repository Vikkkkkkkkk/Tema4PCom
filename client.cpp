#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include "helpers.h"
#include "requests.h"
#include "json.hpp"

using json = nlohmann::json;

using namespace std;

#define SERVER_IP "34.246.184.49"
#define SERVER_PORT 8080
#define SERVER_API "/api/v1/tema"
#define SERVER_REGISTER "/api/v1/tema/auth/register"
#define SERVER_LOGIN "/api/v1/tema/auth/login"
#define SERVER_LIB_ACCESS "/api/v1/tema/library/access"
#define SERVER_BOOKS "/api/v1/tema/library/books"
#define SERVER_LOGOUT "/api/v1/tema/auth/logout"
#define COOKIES_SIZE 5
#define COOKIES_LENGTH 20
#define PAYLOAD_TYPE "application/json"

bool containsSpaces(const char* str) {
    // Iterate through each character in the string
    for (const char* ptr = str; *ptr != '\0'; ++ptr) {
        if (*ptr == ' ') {
            return true;
        }
    }
    return false;
}

bool isNumber(char* str) {
    for (int i = 0; i < strlen(str) - 1; i++) {
        if (str[i] < '0' || str[i] > '9')
            return false;
    }
    return true;
}

void remove_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

int main(int argc, char *argv[])
{
    char *data[2];
    char *login;
    char *message;
    char *response;
    char **cookies;
    int nmb_cookies = 0;
    int sockfd;
    char *token_jwt = NULL;

    cookies = (char **) malloc(COOKIES_SIZE * sizeof(char *));
    for (int i = 0; i < COOKIES_SIZE; i++) {
        cookies[i] = (char *) malloc(COOKIES_LENGTH * sizeof(char));
    }

    while (1) {
        string command;
        cin >> command;

        if (command == "register") {
            bool isLogedIn = false;
            for (int i = 0; i < nmb_cookies; i++) {
                if (strstr(cookies[i], "connect.sid"))
                    isLogedIn = true;
            }
            if (isLogedIn) {
                cout << "ERROR You are already logged in." << endl;
                continue;
            }

            char username[100], password[100], newline[2];

            fgets(newline, 2, stdin);

            cout << "username = ";
            fgets(username, 100, stdin);
            cout << "password = ";
            fgets(password, 100, stdin);

            username[strcspn(username, "\n")] = 0;
            password[strcspn(password, "\n")] = 0;

            if (containsSpaces(username) || containsSpaces(password)) {
                cout << "ERROR Credentials cannot contain whitespaces." << endl;
                continue;
            }

            sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
            if(sockfd < 0) {
                error("Socket not available. It has other missions to accomplish.");
            }

            json reg_json = {{"username", username}, {"password", password}};
            string payload = reg_json.dump();

            char* message = compute_post_request(SERVER_IP, SERVER_REGISTER, PAYLOAD_TYPE, payload.c_str(), NULL, 0, NULL);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            close_connection(sockfd);

            char *json_resp = basic_extract_json_response(response);
            if (json_resp == NULL) {
                cout << "SUCCESS You have been registered successfully." << endl;
            } else {
                cout << "ERROR Username already exists." << endl;
            }
            continue;
        }

        if (command == "login") {
            bool isLogedIn = false;
            for (int i = 0; i < nmb_cookies; i++) {
                if (strstr(cookies[i], "connect.sid"))
                    isLogedIn = true;
            }
            if (isLogedIn) {
                cout << "ERROR You are already logged in." << endl;
                continue;
            }

            char username[100], password[100], newline[2];

            fgets(newline, 2, stdin);

            cout << "username = ";
            fgets(username, 100, stdin);
            cout << "password = ";
            fgets(password, 100, stdin);

            username[strcspn(username, "\n")] = 0;
            password[strcspn(password, "\n")] = 0;

            if (containsSpaces(username) || containsSpaces(password)) {
                cout << "ERROR Credentials cannot contain whitespaces." << endl;
                continue;
            }

            sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
            if(sockfd < 0) {
                error("Socket not available. It has other missions to accomplish.");
            }

            json reg_json = {{"username", username}, {"password", password}};
            string payload = reg_json.dump();

            char* message = compute_post_request(SERVER_IP, SERVER_LOGIN, PAYLOAD_TYPE, payload.c_str(), NULL, 0, NULL);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            close_connection(sockfd);

            char *pointer = strstr(response, "Set-Cookie: ");
            if (pointer) {
                char *cookie = strtok(pointer + strlen("Set-Cookie: "), "\n");
                if (strstr(cookie, "connect.sid")) {
                    strcpy(cookies[nmb_cookies++], strtok(strdup(cookie), ";"));
                    cout << "SUCCESS User logged in successfully." << endl;
                }
            } else {
                cout << "ERROR Credentials invalid." << endl;
            }
            continue;
        }

        if (command == "enter_library") {
            sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
            if(sockfd < 0) {
                error("Socket not available. It has other missions to accomplish.");
            }

            char *message = compute_get_request(SERVER_IP, SERVER_LIB_ACCESS, NULL, cookies, nmb_cookies, NULL);

            send_to_server(sockfd, message);
            char *response = receive_from_server(sockfd);
            close_connection(sockfd);

            char *json_resp = basic_extract_json_response(response);

            json parsed_json = json::parse(json_resp);
            if (parsed_json.contains("token") && !parsed_json["token"].is_null()) {
                string token = parsed_json["token"];
                token_jwt = (char *) malloc(500 * sizeof(char));
                strcpy(token_jwt, token.c_str());
                cout << "SUCCESS Access to library granted." << endl;
            } else {
                cout << "ERROR Access to library denied." << endl;
            }
            continue;
        }

        if (command == "get_books") {
            sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
            if(sockfd < 0) {
                error("Socket not available. It has other missions to accomplish.");
            }

            char *message = compute_get_request(SERVER_IP, SERVER_BOOKS, NULL, cookies, nmb_cookies, token_jwt);

            send_to_server(sockfd, message);
            char *response = receive_from_server(sockfd);
            close_connection(sockfd);

            if (!strstr(response, "[")) {
                cout << "ERROR No access to library." << endl;
                continue;
            }

            json book_list = json::parse(strstr(response, "["));
            cout << "SUCCESS Here are your books" << endl << book_list << endl;
            continue;
        }

        if (command == "add_book") {
            bool isLogedIn = false;
            for (int i = 0; i < nmb_cookies; i++) {
                if (strstr(cookies[i], "connect.sid"))
                    isLogedIn = true;
            }
            if (!isLogedIn) {
                cout << "ERROR You are not logged in." << endl;
                continue;
            }
            if (token_jwt == NULL) {
                cout << "ERROR You do not have access to the library." << endl;
                continue;
            }

            char newline[2];
            char *title = (char *)calloc(420, sizeof(char));
            char *author = (char *)calloc(420, sizeof(char));
            char *genre = (char *)calloc(420, sizeof(char));
            char *publisher = (char *)calloc(420, sizeof(char));

            fgets(newline, 2, stdin);
            cout << "title = " << flush;
            fgets(title, 420, stdin);

            cout << "author = " << flush;
            fgets(author, 420, stdin);

            cout << "genre = " << flush;
            fgets(genre, 420, stdin);

            cout << "publisher = " << flush;
            fgets(publisher, 420, stdin);

            title[strcspn(title, "\n")] = 0;
            author[strcspn(author, "\n")] = 0;
            genre[strcspn(genre, "\n")] = 0;
            publisher[strcspn(publisher, "\n")] = 0;

            char *page_count = (char *)calloc(10, sizeof(char));
            cout << "page_count = ";
            fgets(page_count, 10, stdin);
            page_count[strcspn(page_count, "\n")] = 0;

            if (!isNumber(page_count)) {
                cout << "ERROR Given page count is not a number!" << endl;
                continue;
            }

            sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
            if(sockfd < 0) {
                error("Socket not available. It has other missions to accomplish.");
            }

            json new_book = {{"title", title}, {"author", author}, {"genre", genre}, {"publisher", publisher}, {"page_count", page_count}};
            string payload = new_book.dump();

            char *message = compute_post_request(SERVER_IP, SERVER_BOOKS, PAYLOAD_TYPE, payload.c_str(), NULL, 0, token_jwt);

            send_to_server(sockfd, message);
            char *response = receive_from_server(sockfd);
            close_connection(sockfd);

            char *json_resp = basic_extract_json_response(response);
            if (!json_resp) {
                cout << "SUCCESS Your book has been added successfully." << endl;
            } else {
                cout << "ERROR Your book could not be added to the library." << endl;
            }
            continue;
        }

        if (command == "get_book") {
            char id[10];
            cout << "id = ";
            cin >> id;
            if (!isNumber(id)) {
                cout << "ERROR Given id is not a number!" << endl;
                continue;
            }

            string new_url = SERVER_BOOKS + (string)"/" + id;

            sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
            if(sockfd < 0) {
                error("Socket not available. It has other missions to accomplish.");
            }

            char *message = compute_get_request(SERVER_IP, new_url.c_str(), NULL, cookies, nmb_cookies, token_jwt);

            send_to_server(sockfd, message);
            char *response = receive_from_server(sockfd);
            close_connection(sockfd);

            char *json_resp = basic_extract_json_response(response);

            json parsed_json = json::parse(json_resp);
            if (parsed_json.contains("error") && !parsed_json["error"].is_null()) {
                cout << "ERROR Book with the provided id number does not exist." << endl;
            } else {
                cout << "SUCCESS Here is your book" << endl << parsed_json << endl;
            }
            continue;
        }

        if (command == "delete_book") {
            char id[10];
            cout << "id = ";
            cin >> id;
            if (!isNumber(id)) {
                cout << "ERROR Given id is not a number!" << endl;
                continue;
            }

            string new_url = SERVER_BOOKS + (string)"/" + id;

            sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
            if(sockfd < 0) {
                error("Socket not available. It has other missions to accomplish.");
            }

            char *message = compute_delete_request(SERVER_IP, new_url.c_str(), PAYLOAD_TYPE, token_jwt);

            send_to_server(sockfd, message);
            char *response = receive_from_server(sockfd);
            close_connection(sockfd);

            char *json_resp = basic_extract_json_response(response);

            if (!json_resp) {
                cout << "SUCCESS Your book has been deleted successfully." << endl;
            } else {
                cout << "ERROR Your book could not be deleted from the library." << endl;
            }
        }

        if (command == "logout") {
            sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
            if(sockfd < 0) {
                error("Socket not available. It has other missions to accomplish.");
            }

            char *message = compute_get_request(SERVER_IP, SERVER_LOGOUT, NULL, cookies, nmb_cookies, NULL);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            close_connection(sockfd);

            char *json_resp = basic_extract_json_response(response);
            if (json_resp == NULL) {
                cout << "SUCCESS You have logged out successfully." << endl;
                nmb_cookies = 0;
                free(token_jwt);
                token_jwt = NULL;
            } else {
                cout << "ERROR You are not logged in." << endl;
            }
            continue;
        }

        if (command == "exit") {
            break;
        }
    }
    return 0;
}

