#include <pico_defines.h>
#include <pico_stack.h>
#include <pico_socket.h>
#include <pico_tftp.h>
#include "modules/pico_tftp.c"
#include "check.h"


/* MOCKS */
static int called_pico_socket_close = 0;
static uint16_t expected_opcode = 0;
static int called_user_cb = 0;
static int called_sendto = 0;
static struct pico_socket example_socket;

int pico_socket_close(struct pico_socket *s)
{
    fail_if(s != pico_tftp_socket);
    called_pico_socket_close++;
    return 0;
}

int pico_socket_sendto(struct pico_socket *s, const void *buf, const int len, void *dst, uint16_t remote_port)
{
    struct pico_tftp_hdr *h = (struct pico_tftp_data_hdr *)buf; 
    fail_if(s != pico_tftp_socket);
    fail_if(short_be(h->opcode) != expected_opcode);
    fail_if(len <= 0);
    (void)dst;
    (void)remote_port;
    called_sendto++;
    return 0;
}

int tftp_user_cb(uint16_t err, uint8_t *block, uint32_t len)
{
    called_user_cb++;
}

struct pico_timer *pico_timer_add(pico_time expire, void (*timer)(pico_time, void *), void *arg)
{

}

/* TESTS */
START_TEST(tc_check_opcode)
{
   /* TODO: test this: static int check_opcode(struct pico_tftp_hdr *th) */
    struct pico_tftp_hdr th;
    th.opcode = 0;
    fail_unless(check_opcode(&th) == -1);
    th.opcode = short_be(PICO_TFTP_RRQ);
    fail_unless(check_opcode(&th) == 0);
    th.opcode = short_be(0xFF);
    fail_unless(check_opcode(&th) == -1);
}
END_TEST

START_TEST(tc_tftp_finish)
{


    /* Test case: client */
    pico_tftp_socket = &example_socket;
    pico_tftp_server_on = 0;
    called_pico_socket_close = 0;
    tftp_finish();
    fail_if(pico_tftp_state != PICO_TFTP_STATE_IDLE);
    fail_if(pico_tftp_socket);
    fail_if(!called_pico_socket_close);

    /* Test eval_finish() len is 5*/
    pico_tftp_socket = &example_socket;
    pico_tftp_server_on = 0;
    called_pico_socket_close = 0;
    tftp_eval_finish(5);
    fail_if(pico_tftp_state != PICO_TFTP_STATE_IDLE);
    fail_if(pico_tftp_socket);
    fail_if(!called_pico_socket_close);

    /* Test eval_finish() len is PICO_TFTP_BLOCK_SIZE */
    pico_tftp_socket = &example_socket;
    pico_tftp_server_on = 0;
    called_pico_socket_close = 0;
    tftp_eval_finish(PICO_TFTP_BLOCK_SIZE);
    fail_if(called_pico_socket_close);


    /* Test case: server */
    pico_tftp_socket = &example_socket;
    pico_tftp_server_on = 1;
    called_pico_socket_close = 0;
    tftp_finish();
    fail_if(called_pico_socket_close);
}
END_TEST

START_TEST(tc_tftp_send_ack)
{
    pico_tftp_socket = &example_socket;
#ifdef PICO_FAULTY
    /* send_ack must not segfault when out of memory */
    pico_set_mm_failure(1);
    tftp_send_ack();
    fail_if(called_sendto > 0);
#endif
    expected_opcode = PICO_TFTP_ACK;
    tftp_send_ack();
    fail_if(called_sendto < 1);

}
END_TEST

START_TEST(tc_tftp_send_req)
{
    /* Not needed. The tftp_send_rx_req and tftp_send_tx_req cover this. */
}
END_TEST

START_TEST(tc_tftp_send_rx_req)
{
    pico_tftp_socket = &example_socket;
    called_user_cb = 0;
    called_pico_socket_close = 0;
    called_sendto = 0;
#ifdef PICO_FAULTY
    pico_tftp_user_cb = tftp_user_cb;

    /* send_req must call error cb when out of memory */
    pico_set_mm_failure(1);
    tftp_send_rx_req(NULL, 0, "some filename");
    fail_if(called_user_cb < 1);
    fail_if(called_sendto > 0);
#endif
    expected_opcode = PICO_TFTP_RRQ;
    tftp_send_rx_req(NULL, 0, NULL);
    fail_if(called_sendto > 0); /* Calling with filename = NULL: not good */

    tftp_send_rx_req(NULL, 0, "some filename");
    fail_if(called_sendto < 0);

}
END_TEST

START_TEST(tc_tftp_send_tx_req)
{
    pico_tftp_socket = &example_socket;
    called_user_cb = 0;
    called_pico_socket_close = 0;
    called_sendto = 0;
#ifdef PICO_FAULTY
    pico_tftp_user_cb = tftp_user_cb;

    /* send_req must call error cb when out of memory */
    pico_set_mm_failure(1);
    tftp_send_tx_req(NULL, 0, "some filename");
    fail_if(called_user_cb < 1);
    fail_if(called_sendto > 0);
#endif
    expected_opcode = PICO_TFTP_WRQ;
    tftp_send_tx_req(NULL, 0, NULL);
    fail_if(called_sendto > 0); /* Calling with filename = NULL: not good */

    tftp_send_tx_req(NULL, 0, "some filename");
    fail_if(called_sendto < 0);
}
END_TEST

START_TEST(tc_tftp_send_error)
{
    char longtext[1024];
    pico_tftp_socket = &example_socket;
    called_user_cb = 0;
    called_pico_socket_close = 0;

    /* Sending empty msg */
    called_sendto = 0;
    expected_opcode = TFTP_ERROR;
    tftp_send_error(NULL, 0, 0, NULL);
    fail_if(called_sendto < 1); 
    /* Sending some msg */
    called_sendto = 0;
    expected_opcode = TFTP_ERROR;
    tftp_send_error(NULL, 0, 0, "some text here");
    fail_if(called_sendto < 1); 

    /* sending some very long msg */
    memset(longtext, 'a', 1023);
    longtext[1023] = (char)0;
    called_sendto = 0;
    expected_opcode = TFTP_ERROR;
    tftp_send_error(NULL, 0, 0, longtext);
    fail_if(called_sendto < 1); 
}
END_TEST

START_TEST(tc_tftp_send_data)
{
    pico_tftp_socket = &example_socket;
    called_sendto = 0;
    expected_opcode = PICO_TFTP_DATA;
    tftp_send_data("buffer", strlen("buffer"));
    fail_if(called_sendto < 1);

}
END_TEST

/* Receiving functions */

START_TEST(tc_tftp_data)
{
   /* TODO: test this: static void tftp_data(uint8_t *block, uint32_t len, union pico_address *a, uint16_t port) */
}
END_TEST
START_TEST(tc_tftp_ack)
{
   /* TODO: test this: static void tftp_ack(uint8_t *block, uint32_t len, union pico_address *a, uint16_t port) */
}
END_TEST
START_TEST(tc_tftp_timeout)
{
   /* TODO: test this: static void tftp_timeout(pico_time t) */
}
END_TEST
START_TEST(tc_tftp_req)
{
   /* TODO: test this: static void tftp_req(uint8_t *block, uint32_t len, union pico_address *a, uint16_t port) */
}
END_TEST
START_TEST(tc_tftp_data_err)
{
   /* TODO: test this: static void tftp_data_err(uint8_t *block, uint32_t len, union pico_address *a, uint16_t port) */
}
END_TEST
START_TEST(tc_tftp_fsm_receive_request)
{
   /* TODO: test this: static void tftp_fsm_receive_request(uint8_t *block, uint32_t r, union pico_address *a, uint16_t port) */
}
END_TEST
START_TEST(tc_tftp_fsm_receive)
{
   /* TODO: test this: static void tftp_fsm_receive(uint8_t *block, uint32_t r, union pico_address *a, uint16_t port) */
}
END_TEST
START_TEST(tc_tftp_fsm_error)
{
   /* TODO: test this: static void tftp_fsm_error(uint8_t *block, uint32_t r, union pico_address *a, uint16_t port) */
}
END_TEST
START_TEST(tc_tftp_fsm_timeout)
{
   /* TODO: test this: static void tftp_fsm_timeout(pico_time now, void *arg) */
}
END_TEST
START_TEST(tc_tftp_receive)
{
   /* TODO: test this: static void tftp_receive(uint8_t *block, uint32_t r, union pico_address *a, uint16_t port) */
}
END_TEST
START_TEST(tc_tftp_cb)
{
   /* TODO: test this: static void tftp_cb(uint16_t ev, struct pico_socket *s) */
}
END_TEST
START_TEST(tc_tftp_bind)
{
   /* TODO: test this: static void tftp_bind(void) */
}
END_TEST
START_TEST(tc_tftp_socket_open)
{
   /* TODO: test this: static int tftp_socket_open(uint16_t family, union pico_address *a, uint16_t port) */
}
END_TEST


Suite *pico_suite(void)                       
{
    Suite *s = suite_create("PicoTCP");             

    TCase *TCase_check_opcode = tcase_create("Unit test for check_opcode");
    TCase *TCase_tftp_finish = tcase_create("Unit test for tftp_finish");
    TCase *TCase_tftp_send_ack = tcase_create("Unit test for tftp_send_ack");
    TCase *TCase_tftp_send_req = tcase_create("Unit test for tftp_send_req");
    TCase *TCase_tftp_send_rx_req = tcase_create("Unit test for tftp_send_rx_req");
    TCase *TCase_tftp_send_tx_req = tcase_create("Unit test for tftp_send_tx_req");
    TCase *TCase_tftp_send_error = tcase_create("Unit test for tftp_send_error");
    TCase *TCase_tftp_send_data = tcase_create("Unit test for tftp_send_data");
    TCase *TCase_tftp_data = tcase_create("Unit test for tftp_data");
    TCase *TCase_tftp_ack = tcase_create("Unit test for tftp_ack");
    TCase *TCase_tftp_timeout = tcase_create("Unit test for tftp_timeout");
    TCase *TCase_tftp_req = tcase_create("Unit test for tftp_req");
    TCase *TCase_tftp_data_err = tcase_create("Unit test for tftp_data_err");
    TCase *TCase_tftp_fsm_receive_request = tcase_create("Unit test for tftp_fsm_receive_request");
    TCase *TCase_tftp_fsm_receive = tcase_create("Unit test for tftp_fsm_receive");
    TCase *TCase_tftp_fsm_error = tcase_create("Unit test for tftp_fsm_error");
    TCase *TCase_tftp_fsm_timeout = tcase_create("Unit test for tftp_fsm_timeout");
    TCase *TCase_tftp_receive = tcase_create("Unit test for tftp_receive");
    TCase *TCase_tftp_cb = tcase_create("Unit test for tftp_cb");
    TCase *TCase_tftp_bind = tcase_create("Unit test for tftp_bind");
    TCase *TCase_tftp_socket_open = tcase_create("Unit test for tftp_socket_open");


    tcase_add_test(TCase_check_opcode, tc_check_opcode);
    suite_add_tcase(s, TCase_check_opcode);
    tcase_add_test(TCase_tftp_finish, tc_tftp_finish);
    suite_add_tcase(s, TCase_tftp_finish);
    tcase_add_test(TCase_tftp_send_ack, tc_tftp_send_ack);
    suite_add_tcase(s, TCase_tftp_send_ack);
    tcase_add_test(TCase_tftp_send_req, tc_tftp_send_req);
    suite_add_tcase(s, TCase_tftp_send_req);
    tcase_add_test(TCase_tftp_send_rx_req, tc_tftp_send_rx_req);
    suite_add_tcase(s, TCase_tftp_send_rx_req);
    tcase_add_test(TCase_tftp_send_tx_req, tc_tftp_send_tx_req);
    suite_add_tcase(s, TCase_tftp_send_tx_req);
    tcase_add_test(TCase_tftp_send_error, tc_tftp_send_error);
    suite_add_tcase(s, TCase_tftp_send_error);
    tcase_add_test(TCase_tftp_send_data, tc_tftp_send_data);
    suite_add_tcase(s, TCase_tftp_send_data);
    tcase_add_test(TCase_tftp_data, tc_tftp_data);
    suite_add_tcase(s, TCase_tftp_data);
    tcase_add_test(TCase_tftp_ack, tc_tftp_ack);
    suite_add_tcase(s, TCase_tftp_ack);
    tcase_add_test(TCase_tftp_timeout, tc_tftp_timeout);
    suite_add_tcase(s, TCase_tftp_timeout);
    tcase_add_test(TCase_tftp_req, tc_tftp_req);
    suite_add_tcase(s, TCase_tftp_req);
    tcase_add_test(TCase_tftp_data_err, tc_tftp_data_err);
    suite_add_tcase(s, TCase_tftp_data_err);
    tcase_add_test(TCase_tftp_fsm_receive_request, tc_tftp_fsm_receive_request);
    suite_add_tcase(s, TCase_tftp_fsm_receive_request);
    tcase_add_test(TCase_tftp_fsm_receive, tc_tftp_fsm_receive);
    suite_add_tcase(s, TCase_tftp_fsm_receive);
    tcase_add_test(TCase_tftp_fsm_error, tc_tftp_fsm_error);
    suite_add_tcase(s, TCase_tftp_fsm_error);
    tcase_add_test(TCase_tftp_fsm_timeout, tc_tftp_fsm_timeout);
    suite_add_tcase(s, TCase_tftp_fsm_timeout);
    tcase_add_test(TCase_tftp_receive, tc_tftp_receive);
    suite_add_tcase(s, TCase_tftp_receive);
    tcase_add_test(TCase_tftp_cb, tc_tftp_cb);
    suite_add_tcase(s, TCase_tftp_cb);
    tcase_add_test(TCase_tftp_bind, tc_tftp_bind);
    suite_add_tcase(s, TCase_tftp_bind);
    tcase_add_test(TCase_tftp_socket_open, tc_tftp_socket_open);
    suite_add_tcase(s, TCase_tftp_socket_open);
return s;
}
                      
int main(void)                      
{                       
    int fails;                      
    Suite *s = pico_suite();                        
    SRunner *sr = srunner_create(s);                        
    srunner_run_all(sr, CK_NORMAL);                     
    fails = srunner_ntests_failed(sr);                      
    srunner_free(sr);                       
    return fails;                       
}
