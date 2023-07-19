#ifndef H_CERTSTORE_UTIL
#define H_CERTSTORE_UTIL

#ifdef __cplusplus
extern "C" {
#endif

bool certstore_delete_all(void);
bool certstore_remove_client(void);
bool certstore_reset_material(char *default_cert, char *default_key, char *default_csr);
bool certstore_check_material(void);
bool certstore_load_material(char *device_cert_pem, char *device_key_pem, char *device_csr_pem);
bool certstore_save_file(char *file_name, char *file_content);
void certstore_dump_material(char *device_cert_pem, char *device_key_pem, char *device_csr_pem);

#ifdef __cplusplus
}
#endif

#endif