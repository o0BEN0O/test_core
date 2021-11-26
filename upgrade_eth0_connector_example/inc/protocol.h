#ifndef _PROTOCOL_API_H_
#define _PROTOCOL_API_H_
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef int (*json_handler)(uint8_t *data,uint32_t len);
extern void ftr_json_api_init(void);
extern void ftr_register_receive_callback(json_handler json_api_handler);
extern int ftr_send_json_data(uint8_t* format_data, uint32_t format_data_len);
extern int ftr_get_socket_id();

int send_json_data(int socket_fd,uint8_t* format_data, uint32_t format_data_len);
int json_recv_data(int sock_fd);

int ftr_socket_client_init(void);


#ifdef __cplusplus
}
#endif

#endif

