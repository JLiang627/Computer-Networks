#include <pthread.h>     
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>    
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#define MTU 1500
#define BUFF_LEN 10000  // buffer size
#define PACKET_SIZE 1518

#define CLIENT_IP "127.0.0.1"
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000//code
#define ROUTER_PORT 9001//code
#define CLIENT_PORT 9002//code
#define CLIENTTWO_PORT 9003//code
#define SA struct sockaddr

// ==================== 封包結構定義 ====================
typedef struct IPHeader{
    uint8_t version_ihl;
    uint8_t type_of_service;
    uint16_t total_length;
    uint16_t identification;
    uint16_t flags_fragment_offset;
    uint8_t time_to_live;
    uint8_t protocol;
    uint16_t header_checksum;
    uint32_t source_ip;
    uint32_t destination_ip;
    uint32_t options; // router 有多一個 options 欄位
}IPHeader;

typedef struct UDPHeader {
  uint32_t source_port:16,
           dest_port:16;
  uint32_t Segment_Length:16,
           Checksum:16;
}UDPHeader;

typedef struct MACHeader{
    uint8_t sour_mac[6];   // source
    uint8_t des_mac[6];    // destination
    uint16_t fram_typ;     // frame type
    uint32_t crc;          // crc
}MACHeader; // 18 bytes

typedef struct Packet {
  struct IPHeader ipheader;
  struct UDPHeader udpheader;
  struct MACHeader macheader;
  char buffer[MTU - 28];
}Packet;

// ==================== TCP 轉發 Thread ====================
void *tcp_socket(void *argu) {  

	//code
	return NULL;    
}    

// ==================== UDP 轉發 Thread ====================
void *udp_socket(void *argu) { 
		
	//code
	return NULL;    
}    

// ==================== main ====================
int main(){
	pthread_t tid1, tid2;

	// 建立 TCP 接收 thread
	if (pthread_create(&tid1, NULL, tcp_socket, NULL) != 0) {
		perror("pthread_create for TCP failed");
		return 1;
	}

	// 建立 UDP 發送 thread
	if (pthread_create(&tid2, NULL, udp_socket, NULL) != 0) {
		perror("pthread_create for UDP failed");
		return 1;
	}

	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);

	return 0;
}
