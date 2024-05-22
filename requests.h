#ifndef _REQUESTS_
#define _REQUESTS_

// computes and returns a GET request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_get_request(char *host, const char *url, char *query_params,
							char **cookies, int cookies_count, char *token_jwt);

// computes and returns a POST request string (cookies can be NULL if not needed)
char *compute_post_request(char *host, char *url, char* content_type, const char *payload, char **cookies, int cookies_count, char *token_jwt);

char* compute_delete_request(char *host, const char *url, char *content_type, char *token_jwt);

#endif

