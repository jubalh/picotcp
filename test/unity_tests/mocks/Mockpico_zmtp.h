/* AUTOGENERATED FILE. DO NOT EDIT. */
#ifndef _MOCKPICO_ZMTP_H
#define _MOCKPICO_ZMTP_H

#include "pico_zmtp.h"

void Mockpico_zmtp_Init(void);
void Mockpico_zmtp_Destroy(void);
void Mockpico_zmtp_Verify(void);


typedef void(*cmock_pico_zmtp_func_ptr1)(uint16_t ev, struct zmtp_socket* s);


#define zmtp_socket_open_IgnoreAndReturn(cmock_retval) zmtp_socket_open_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void zmtp_socket_open_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, struct zmtp_socket* cmock_to_return);
#define zmtp_socket_open_ExpectAndReturn(net, proto, type, zmq_cb, cmock_retval) zmtp_socket_open_CMockExpectAndReturn(__LINE__, net, proto, type, zmq_cb, cmock_retval)
void zmtp_socket_open_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, uint16_t net, uint16_t proto, uint8_t type, cmock_pico_zmtp_func_ptr1 zmq_cb, struct zmtp_socket* cmock_to_return);
typedef struct zmtp_socket* (* CMOCK_zmtp_socket_open_CALLBACK)(uint16_t net, uint16_t proto, uint8_t type, cmock_pico_zmtp_func_ptr1 zmq_cb, int cmock_num_calls);
void zmtp_socket_open_StubWithCallback(CMOCK_zmtp_socket_open_CALLBACK Callback);
#define zmtp_socket_accept_IgnoreAndReturn(cmock_retval) zmtp_socket_accept_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void zmtp_socket_accept_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, struct zmtp_socket* cmock_to_return);
#define zmtp_socket_accept_ExpectAndReturn(zmtp_s, cmock_retval) zmtp_socket_accept_CMockExpectAndReturn(__LINE__, zmtp_s, cmock_retval)
void zmtp_socket_accept_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, struct zmtp_socket* zmtp_s, struct zmtp_socket* cmock_to_return);
typedef struct zmtp_socket* (* CMOCK_zmtp_socket_accept_CALLBACK)(struct zmtp_socket* zmtp_s, int cmock_num_calls);
void zmtp_socket_accept_StubWithCallback(CMOCK_zmtp_socket_accept_CALLBACK Callback);
#define zmtp_socket_connect_IgnoreAndReturn(cmock_retval) zmtp_socket_connect_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void zmtp_socket_connect_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define zmtp_socket_connect_ExpectAndReturn(s, srv_addr, remote_port, cmock_retval) zmtp_socket_connect_CMockExpectAndReturn(__LINE__, s, srv_addr, remote_port, cmock_retval)
void zmtp_socket_connect_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, struct zmtp_socket* s, void* srv_addr, uint16_t remote_port, int cmock_to_return);
typedef int (* CMOCK_zmtp_socket_connect_CALLBACK)(struct zmtp_socket* s, void* srv_addr, uint16_t remote_port, int cmock_num_calls);
void zmtp_socket_connect_StubWithCallback(CMOCK_zmtp_socket_connect_CALLBACK Callback);
#define zmtp_socket_send_IgnoreAndReturn(cmock_retval) zmtp_socket_send_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void zmtp_socket_send_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define zmtp_socket_send_ExpectAndReturn(s, vec, cmock_retval) zmtp_socket_send_CMockExpectAndReturn(__LINE__, s, vec, cmock_retval)
void zmtp_socket_send_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, struct zmtp_socket* s, struct pico_vector* vec, int cmock_to_return);
typedef int (* CMOCK_zmtp_socket_send_CALLBACK)(struct zmtp_socket* s, struct pico_vector* vec, int cmock_num_calls);
void zmtp_socket_send_StubWithCallback(CMOCK_zmtp_socket_send_CALLBACK Callback);
#define zmtp_socket_bind_IgnoreAndReturn(cmock_retval) zmtp_socket_bind_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void zmtp_socket_bind_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define zmtp_socket_bind_ExpectAndReturn(s, local_addr, port, cmock_retval) zmtp_socket_bind_CMockExpectAndReturn(__LINE__, s, local_addr, port, cmock_retval)
void zmtp_socket_bind_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, struct zmtp_socket* s, void* local_addr, uint16_t port, int cmock_to_return);
typedef int (* CMOCK_zmtp_socket_bind_CALLBACK)(struct zmtp_socket* s, void* local_addr, uint16_t port, int cmock_num_calls);
void zmtp_socket_bind_StubWithCallback(CMOCK_zmtp_socket_bind_CALLBACK Callback);
#define zmtp_socket_close_IgnoreAndReturn(cmock_retval) zmtp_socket_close_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void zmtp_socket_close_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define zmtp_socket_close_ExpectAndReturn(s, cmock_retval) zmtp_socket_close_CMockExpectAndReturn(__LINE__, s, cmock_retval)
void zmtp_socket_close_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, struct zmtp_socket* s, int cmock_to_return);
typedef int (* CMOCK_zmtp_socket_close_CALLBACK)(struct zmtp_socket* s, int cmock_num_calls);
void zmtp_socket_close_StubWithCallback(CMOCK_zmtp_socket_close_CALLBACK Callback);

#endif
