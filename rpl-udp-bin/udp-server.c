/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include <stdint.h>
#include <stdio.h>
#include "cfs/cfs.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678
#define FIRMWARE_PACKET_HEADER_LEN 10
#define FIRMWARE_PACKET_MAGIC      0xf17e

static struct simple_udp_connection udp_conn;
static uint16_t received_bytes;
static uint8_t received_chunks;
static uint16_t expected_chunk_no = 0;

static uint16_t
calculate_checksum(const uint8_t *data, uint16_t len)
{
  uint16_t sum = 0;
  for(int i = 0; i < len; i++) {
    sum += data[i];
  }
  return sum;
}

PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);
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
  uint16_t magic;
  uint16_t chunk_no;
  uint16_t offset;
  uint8_t payload_len;
  uint8_t total_chunks;
  uint16_t checksum_rx;
  uint16_t checksum_calc;
  char ack[16];
  int ack_len;

  if(datalen < FIRMWARE_PACKET_HEADER_LEN) {
    LOG_INFO("Ignoring short packet len %u from ", datalen);
    LOG_INFO_6ADDR(sender_addr);
    LOG_INFO_("\n");
    return;
  }

  magic = ((uint16_t)data[0] << 8) | data[1];
  if(magic != FIRMWARE_PACKET_MAGIC) {
    LOG_INFO("Ignoring packet with wrong magic 0x%04x from ", magic);
    LOG_INFO_6ADDR(sender_addr);
    LOG_INFO_("\n");
    return;
  }

  chunk_no = ((uint16_t)data[2] << 8) | data[3];
  offset = ((uint16_t)data[4] << 8) | data[5];
  payload_len = data[6];
  total_chunks = data[7];
  checksum_rx = ((uint16_t)data[8] << 8) | data[9];

  if(datalen != FIRMWARE_PACKET_HEADER_LEN + payload_len) {
    LOG_INFO("Ignoring malformed chunk %u: datalen %u payload_len %u\n",
             chunk_no, datalen, payload_len);
    return;
  }

  checksum_calc = calculate_checksum(&data[FIRMWARE_PACKET_HEADER_LEN], payload_len);
  if (checksum_rx != checksum_calc) {
    LOG_INFO("Checksum mismatch for chunk %u (rx %04x, calc %04x)\n", chunk_no, checksum_rx, checksum_calc);
    return;
  }

  if (chunk_no == expected_chunk_no) {
    int fd;
    if (chunk_no == 0) {
      cfs_remove("firmware.bin");
      fd = cfs_open("firmware.bin", CFS_WRITE);
    } else {
      fd = cfs_open("firmware.bin", CFS_WRITE | CFS_APPEND);
    }
    
    if (fd >= 0) {
      cfs_write(fd, &data[FIRMWARE_PACKET_HEADER_LEN], payload_len);
      cfs_close(fd);
      expected_chunk_no++;
      received_chunks++;
      received_bytes += payload_len;
      LOG_INFO("Saved firmware chunk %u/%u offset %u len %u to CFS\n",
             chunk_no + 1, total_chunks, offset, payload_len);
    } else {
      LOG_INFO("Failed to open CFS file!\n");
      return;
    }
  } else if (chunk_no < expected_chunk_no) {
    LOG_INFO("Duplicate chunk %u, re-ACKing\n", chunk_no);
  } else {
    LOG_INFO("Out of order chunk %u (expected %u), ignoring\n", chunk_no, expected_chunk_no);
    return;
  }

  if(received_chunks == total_chunks && chunk_no == total_chunks - 1) {
    LOG_INFO("Yüklenmeye hazır yeni firmware alımı tamamlandı.\n");
    LOG_INFO("Firmware sample received: %u bytes in %u chunks\n",
             received_bytes, received_chunks);
  }

#if WITH_SERVER_REPLY
  ack_len = snprintf(ack, sizeof(ack), "ACK %u", chunk_no);
  simple_udp_sendto(&udp_conn, ack, ack_len, sender_addr);
#endif /* WITH_SERVER_REPLY */
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  PROCESS_BEGIN();

  /* Initialize DAG root */
  NETSTACK_ROUTING.root_start();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
