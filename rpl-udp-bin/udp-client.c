#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#include "firmware_data.h"
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL		  (2 * CLOCK_SECOND)
#define FIRMWARE_CHUNK_SIZE        48
#define FIRMWARE_PACKET_HEADER_LEN 10
#define FIRMWARE_PACKET_MAGIC      0xf17e

static struct simple_udp_connection udp_conn;
static uint32_t rx_count = 0;

static uint16_t chunk_no = 0;
static uint16_t offset = 0;
static uint8_t ack_received = 0;

enum {
  STATE_SENDING,
  STATE_WAITING_ACK
};
static uint8_t current_state = STATE_SENDING;

static uint16_t
calculate_checksum(const uint8_t *data, uint16_t len)
{
  uint16_t sum = 0;
  for(int i = 0; i < len; i++) {
    sum += data[i];
  }
  return sum;
}

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{

  LOG_INFO("Received response '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
#if LLSEC802154_CONF_ENABLED
  LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
  LOG_INFO_("\n");
  rx_count++;

  if(strncmp((char *)data, "ACK ", 4) == 0) {
    uint16_t ack_num = atoi((char *)data + 4);
    if(ack_num == chunk_no) {
      ack_received = 1;
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static uint8_t packet[FIRMWARE_PACKET_HEADER_LEN + FIRMWARE_CHUNK_SIZE];
  uip_ipaddr_t dest_ipaddr;
  static uint32_t missed_tx_count;
  uint16_t remaining;
  uint8_t payload_len;
  uint8_t total_chunks;
  uint16_t checksum;

  PROCESS_BEGIN();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() &&
        NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {

      if(offset >= FIRMWARE_PAYLOAD_LEN) {
        LOG_INFO("Firmware transfer complete: %u bytes, %" PRIu32 " replies, %" PRIu32 " missed\n",
                 FIRMWARE_PAYLOAD_LEN, rx_count, missed_tx_count);
        etimer_set(&periodic_timer, SEND_INTERVAL);
        continue;
      }

      if(current_state == STATE_SENDING) {
        remaining = FIRMWARE_PAYLOAD_LEN - offset;
        payload_len = remaining > FIRMWARE_CHUNK_SIZE ?
          FIRMWARE_CHUNK_SIZE : remaining;
        total_chunks = (FIRMWARE_PAYLOAD_LEN + FIRMWARE_CHUNK_SIZE - 1) /
          FIRMWARE_CHUNK_SIZE;

        packet[0] = (uint8_t)(FIRMWARE_PACKET_MAGIC >> 8);
        packet[1] = (uint8_t)FIRMWARE_PACKET_MAGIC;
        packet[2] = (uint8_t)(chunk_no >> 8);
        packet[3] = (uint8_t)chunk_no;
        packet[4] = (uint8_t)(offset >> 8);
        packet[5] = (uint8_t)offset;
        packet[6] = payload_len;
        packet[7] = total_chunks;
        
        checksum = calculate_checksum(&firmware_payload[offset], payload_len);
        packet[8] = (uint8_t)(checksum >> 8);
        packet[9] = (uint8_t)checksum;

        memcpy(&packet[FIRMWARE_PACKET_HEADER_LEN], &firmware_payload[offset],
               payload_len);

        LOG_INFO("Sending firmware chunk %u/%u offset %u len %u to ",
                 chunk_no + 1, total_chunks, offset, payload_len);
        LOG_INFO_6ADDR(&dest_ipaddr);
        LOG_INFO_("\n");
        simple_udp_sendto(&udp_conn, packet,
                          FIRMWARE_PACKET_HEADER_LEN + payload_len,
                          &dest_ipaddr);
        
        ack_received = 0;
        current_state = STATE_WAITING_ACK;
      } else if(current_state == STATE_WAITING_ACK) {
        if(ack_received) {
          LOG_INFO("ACK received for chunk %u\n", chunk_no);
          remaining = FIRMWARE_PAYLOAD_LEN - offset;
          payload_len = remaining > FIRMWARE_CHUNK_SIZE ? FIRMWARE_CHUNK_SIZE : remaining;
          offset += payload_len;
          chunk_no++;
          current_state = STATE_SENDING;
          /* Move to next chunk faster */
          etimer_set(&periodic_timer, CLOCK_SECOND / 10);
          continue;
        } else {
          LOG_INFO("Retransmitting chunk %u\n", chunk_no);
          missed_tx_count++;
          remaining = FIRMWARE_PAYLOAD_LEN - offset;
          payload_len = remaining > FIRMWARE_CHUNK_SIZE ? FIRMWARE_CHUNK_SIZE : remaining;
          simple_udp_sendto(&udp_conn, packet,
                            FIRMWARE_PACKET_HEADER_LEN + payload_len,
                            &dest_ipaddr);
        }
      }
    } else {
      LOG_INFO("Not reachable yet\n");
      if(offset > 0) {
        missed_tx_count++;
      }
    }

    etimer_set(&periodic_timer, SEND_INTERVAL);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
