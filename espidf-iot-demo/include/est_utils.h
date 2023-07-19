#ifndef H_EST_UTILS
#define H_EST_UTILS

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048

#ifdef __cplusplus
extern "C" {
#endif

bool est_cacerts(char *est_host, int est_port, char *root_cert, char *ca_cert);
bool est_simpleenroll(char *est_host, int est_port, char *root_cert, char *bauth_us, char *bauth_pw, char* csr_data, char *device_cert);
bool est_simpleenroll_certauth(char *est_host, int est_port, char *root_cert, char *auth_cert, char *auth_key, char* csr_data, char *device_cert);
bool est_convert_p7_to_pem(char *device_cert);

#ifdef __cplusplus
}
#endif

#endif