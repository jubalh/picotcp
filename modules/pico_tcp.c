#include "pico_tcp.h"
#include "pico_config.h"
#include "pico_eth.h"
#include "pico_socket.h"
#include "pico_stack.h"


/* Queues */
static struct pico_queue in = {};
static struct pico_queue out = {};


/* Functions */

struct __attribute__((packed)) tcp_pseudo_hdr_ipv4
{
  struct pico_ip4 src;
  struct pico_ip4 dst;
  uint16_t tcp_len;
  uint8_t res;
  uint8_t proto;
};

int pico_tcp_checksum_ipv4(struct pico_frame *f)
{
  struct pico_tcp_hdr *hdr = (struct pico_tcp_hdr *) f->transport_hdr;
  struct pico_socket *s = f->sock;
  struct tcp_pseudo_hdr_ipv4 pseudo;
  if (!hdr || !s)
    return -1;

  pseudo.src.addr = s->local_addr.ip4.addr;
  pseudo.dst.addr = s->remote_addr.ip4.addr;
  pseudo.res = 0;
  pseudo.proto = PICO_PROTO_TCP;
  pseudo.tcp_len = short_be(f->transport_len);

  hdr->crc = 0;
  dbg("Calculating checksum of pseudo_hdr + %d bytes\n", f->transport_len);
  hdr->crc = pico_dualbuffer_checksum(&pseudo, sizeof(struct tcp_pseudo_hdr_ipv4), hdr, f->transport_len);
  dbg("TCP checksum is %04x\n", hdr->crc);
  hdr->crc = short_be(hdr->crc);
  return 0;
}

static int pico_tcp_process_out(struct pico_protocol *self, struct pico_frame *f)
{
  pico_network_send(f);
  return 0;
}


/* Interface: protocol definition */
struct pico_protocol pico_proto_tcp = {
  .name = "tcp",
  .proto_number = PICO_PROTO_TCP,
  .layer = PICO_LAYER_TRANSPORT,
  .process_in = pico_transport_process_in,
  .process_out = pico_tcp_process_out,
  .q_in = &in,
  .q_out = &out,
};

struct pico_socket_tcp {
  struct pico_socket sock;
  uint32_t snd_nxt;
  uint32_t snd_una;
  uint16_t cwnd;
  uint16_t ssthresh;
  uint32_t rcv_nxt;
  uint32_t rcv_ackd;
  uint32_t remote_timestamp;
  uint16_t rwnd;
  uint16_t avg_rtt;
};

#define TCP_SOCK(s) ((struct pico_socket_tcp *)s)

static uint32_t pico_paws(void)
{
  return 0xc0cac01a; /*XXX: implement paws */
}

static inline int seq_compare(uint32_t a, uint32_t b)
{
  uint32_t thresh = ((uint32_t)(-1))>>1;
  if (((a > thresh) && (b > thresh)) || ((a <= thresh) && (b <= thresh))) {
    if (a > b)
      return 1;
    if (b > a)
      return -1;
  } else {
    if (a > b)
      return -2;
    if (b > a)
      return 2;
  }
  return 0;
}

static void pico_add_options(struct pico_socket_tcp *ts, struct pico_frame *f)
{
  uint16_t mss = 1460;
  int option_len = PICO_SIZE_TCP_DATAHDR - PICO_SIZE_TCPHDR;
  memset(f->start, 1, option_len);

  f->start[0] = PICO_TCP_OPTION_MSS;
  f->start[1] = PICO_TCPOPTLEN_MSS;
  f->start[2] = (mss >> 8) & 0xFF;
  f->start[3] = mss & 0xFF;

  f->start[4] = PICO_TCP_OPTION_SACK;
  f->start[5] = PICO_TCPOPTLEN_SACK;

  f->start[6] = PICO_TCP_OPTION_WS;
  f->start[7] = PICO_TCPOPTLEN_WS;
  f->start[8] = 2;

  f->start[9] = PICO_TCP_OPTION_TIMESTAMP;
  f->start[10] = PICO_TCPOPTLEN_TIMESTAMP;
  memcpy(f->start + 11, &ts->remote_timestamp, 4);
  memcpy(f->start + 15, &ts->remote_timestamp, 4);


  f->start[option_len -1] = 0;

}


static int tcp_send(struct pico_socket_tcp *ts, struct pico_frame *f)
{
  struct pico_tcp_hdr *hdr= (struct pico_tcp_hdr *) f->transport_hdr;
  hdr->trans.sport = ts->sock.local_port;
  hdr->trans.dport = ts->sock.remote_port;
  hdr->seq = long_be(ts->snd_nxt);
  if (((ts->rcv_ackd == 0) && (ts->rcv_nxt != 0)) || (seq_compare(ts->rcv_ackd, ts->rcv_nxt) < 0)) {
    hdr->flags |= PICO_TCP_ACK;
    hdr->ack = long_be(ts->rcv_nxt);
  }
  f->start = f->transport_hdr + PICO_SIZE_TCPHDR;
  pico_add_options(ts,f);
  f->transport_len = PICO_SIZE_TCP_DATAHDR;
  pico_tcp_checksum_ipv4(f);
  pico_enqueue(&out, f);
  dbg("Packet enqueued for TCP transmission.\n");

  return 0;
}

struct pico_socket *pico_tcp_open(void)
{
  struct pico_socket_tcp *t = pico_zalloc(sizeof(struct pico_socket_tcp));
  if (!t)
    return NULL;
  return &t->sock;
}

int pico_tcp_initconn(struct pico_socket *s)
{
  struct pico_socket_tcp *ts = TCP_SOCK(s);
  struct pico_frame *syn = s->net->alloc(s->net, PICO_SIZE_TCP_DATAHDR);
  struct pico_tcp_hdr *hdr = (struct pico_tcp_hdr *) syn->transport_hdr;

  if (!syn)
    return -1;
  ts->snd_nxt = pico_paws();
  syn->sock = s;
  hdr->seq = ts->snd_nxt;
  hdr->len = PICO_SIZE_TCP_DATAHDR << 2;
  hdr->flags = PICO_TCP_SYN;
  hdr->rwnd = short_be(4096); // XXX
  tcp_send(ts, syn);
  return 0;
}

static int tcp_send_synack(struct pico_socket *s)
{
  struct pico_socket_tcp *ts = TCP_SOCK(s);
  struct pico_frame *synack = s->net->alloc(s->net, PICO_SIZE_TCP_DATAHDR);
  struct pico_tcp_hdr *hdr = (struct pico_tcp_hdr *) synack->transport_hdr;
  synack->sock = s;
  hdr->len = PICO_SIZE_TCP_DATAHDR << 2;
  hdr->flags = PICO_TCP_SYN | PICO_TCP_ACK;
  hdr->rwnd = short_be(4096); // XXX
  return tcp_send(ts, synack);
}

static int tcp_spawn_clone(struct pico_socket *s, struct pico_frame *f)
{
  /* TODO: Check against backlog length */

  struct pico_socket_tcp *new = (struct pico_socket_tcp *)pico_socket_clone(s);
  struct pico_socket_tcp *ts = TCP_SOCK(s);
  struct pico_tcp_hdr *hdr = (struct pico_tcp_hdr *)f->transport_hdr;
  uint8_t tcp_len;
  if (!new)
    return -1;
  new->sock.remote_port = ((struct pico_trans *)f->transport_hdr)->sport;
#ifdef PICO_SUPPORT_IPV4
  if (IS_IPV4(f))
    new->sock.remote_addr.ip4.addr = ((struct pico_ipv4_hdr *)(f->net_hdr))->src.addr;
#endif
#ifdef PICO_SUPPORT_IPV6
  if (IS_IPV4(f))
    memcpy(new->sock.remote_addr.ip6.addr, ((struct pico_ipv6_hdr *)(f->net_hdr))->src, PICO_SIZE_IP6);
#endif

  tcp_len = (hdr->len >> 2);

  new->rcv_nxt = long_be(hdr->seq) + 1;
  new->snd_nxt = pico_paws();
  new->snd_una = new->snd_nxt;
  new->cwnd = short_be(2);
  new->ssthresh = short_be(0xFFFF);
  new->rwnd = short_be(hdr->rwnd);
  dbg("Ts: %02x\n", f->transport_hdr[28]);
  dbg("Ts: %02x\n", f->transport_hdr[29]);
  dbg("Ts: %02x\n", f->transport_hdr[30]);
  dbg("Ts: %02x\n", f->transport_hdr[31]);

  memcpy(&new->remote_timestamp, f->transport_hdr + 28, 4);

  tcp_send_synack(&new->sock);
  return 0;
}

int pico_tcp_input(struct pico_socket *s, struct pico_frame *f)
{
  struct pico_tcp_hdr *hdr = (struct pico_tcp_hdr *) (f->transport_hdr);
  int ret = -1;
  uint8_t flags = hdr->flags;

  if (!hdr)
    goto discard;

  dbg("[tcp input] socket: %p state: %d <-- local port:%d remote port: %d seq: %08x flags: %02x\n",
      s, s->state, short_be(hdr->trans.dport), short_be(hdr->trans.sport), long_be(hdr->seq), hdr->flags);

  if (flags & PICO_TCP_SYN) {
    switch (TCPSTATE(s)) {
      case PICO_SOCKET_STATE_TCP_LISTEN:
        dbg("In TCP Listen.\n");
        if (flags & PICO_TCP_ACK)
          goto discard;
        tcp_spawn_clone(s,f);

        break;
    }

  }


  switch (TCPSTATE(s)) {
    case PICO_SOCKET_STATE_TCP_LISTEN:
      dbg("In TCP Listen.\n");
      break;
    default:
      break;
  }

discard:
  pico_frame_discard(f);
  return ret;
}



