#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>  
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <string>
#include <cstring>

using namespace std;

#define MTU 1500
#define BUFF_LEN 10000  // buffer size
#define PACKET_SIZE 1518

#define CLIENT_IP "127.0.0.1"
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT //code
#define ROUTER_PORT //code
#define CLIENT_PORT //code
#define CLIENTTWO_PORT //code

#define SA struct sockaddr

// ==================== 封包結構定義 ====================
// 模擬實際網路中的 MAC/IP/UDP/TCP 封包格式
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
}IPHeader; // 20 bytes

typedef struct UDPHeader {
  uint32_t source_port:16,
           dest_port:16;
  uint32_t Segment_Length:16,
           Checksum:16;
}UDPHeader; // 8 bytes

typedef struct MACHeader{
    uint8_t sour_mac[6];   // source MAC
    uint8_t des_mac[6];    // destination MAC
    uint16_t fram_typ;     // frame type
    uint32_t crc;          // crc
}MACHeader; // 18 bytes

typedef struct TCPHeader {
    uint16_t source_port;
    uint16_t destination_port;
    uint32_t sequence_number;
    uint32_t ack_number;
    uint16_t offset_reserved_flags;
    uint16_t window_size;
    uint16_t checksum;
    uint16_t urgent_pointer;
}TCPHeader; // 20 bytes

typedef struct Packet {
  struct IPHeader ipheader;
  struct UDPHeader udpheader;
  struct MACHeader macheader;
  char buffer[PACKET_SIZE - 46];
}Packet;  // UDP Packet

int count;
char last_payload[5000] = "`abc"; // 保存上一次傳輸的 payload

// ==================== TCP 封包發送 ====================
void tcp_msg_sender(int fd, struct sockaddr* dst){
    // 修改 payload：將上一次的字串每個字元 +1
    char payload[5000];
    strcpy(payload, last_payload);
    for(int i = 0; i < strlen(payload); i++) {
        payload[i]++;
    }
    strncpy(last_payload, payload, 5000); // 保存此次 payload
    size_t payload_length = strlen(payload);
	
    char buffer[PACKET_SIZE] = {0}; 
    
    // ---- MAC Header ----
    struct MACHeader macHeader;
    uint8_t des_mac[6] = {0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};
    uint8_t sour_mac[6] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
    uint16_t fram_typ = 0x0800; // IPv4
    memcpy(macHeader.des_mac , des_mac , 6);
    memcpy(macHeader.sour_mac , sour_mac, 6);
    macHeader.fram_typ = htons(fram_typ);
    uint32_t crc = 0;

    // ---- IP Header ----
    struct IPHeader ipHeader;
    uint8_t version_ihl = 0x45; // IPv4 + header length
    uint8_t type_of_service = 0x00;
    uint16_t total_length = sizeof(IPHeader) + sizeof(TCPHeader) +payload_length;
    uint16_t identification = 0xAAAA;
    uint16_t flags_fragment_offset = 0x4000;
    uint8_t time_to_live = 64; 
    uint8_t protocol = 0x06; // TCP
    uint16_t header_checksum = 0x0000;
    const char*source_ip_str = "10.17.164.10"; 
    const char* destination_ip_str ="10.17.89.69"; 
    
    inet_pton(AF_INET, source_ip_str, &ipHeader.source_ip);
    inet_pton(AF_INET, destination_ip_str, &ipHeader.destination_ip);
    
    ipHeader.version_ihl = version_ihl;
    ipHeader.type_of_service = type_of_service;
    ipHeader.total_length = htons(total_length);
    ipHeader.identification = htons(identification);
    ipHeader.flags_fragment_offset = htons(flags_fragment_offset);
    ipHeader.time_to_live = time_to_live;
    ipHeader.protocol = protocol;
    ipHeader.header_checksum = htons(header_checksum);

    // ---- TCP Header ----
    struct TCPHeader tcpHeader;
    uint16_t source_port = 12345;
    uint16_t destination_port = 80; 
    uint32_t sequence_number = 0x00000001;
    uint32_t ack_number = 0x12345678;
    uint8_t offset_reserved_flags = 0x14;
    uint16_t window_size = 0xFFFF;
    uint16_t checksum = 0x0000;
    uint16_t urgent_pointer = 0x0000;

    tcpHeader.source_port = htons(source_port);
    tcpHeader.destination_port = htons(destination_port);
    tcpHeader.sequence_number = htonl(sequence_number);
    tcpHeader.ack_number = htonl(ack_number);
    tcpHeader.offset_reserved_flags = offset_reserved_flags;
    tcpHeader.window_size = htons(window_size);
    tcpHeader.checksum = htons(checksum);
    tcpHeader.urgent_pointer = htons(urgent_pointer);
    
    // ---- 封裝封包 ---- (MAC header, IP header and TCP header複製到buffer)
    memcpy(buffer, &macHeader, sizeof(MACHeader));
    memcpy(buffer + sizeof(MACHeader), &ipHeader, sizeof(IPHeader));
    memcpy(buffer + sizeof(MACHeader) + sizeof(IPHeader), &tcpHeader, sizeof(TCPHeader));
    memcpy(buffer + sizeof(MACHeader) + sizeof(IPHeader) + sizeof(TCPHeader), payload , payload_length);

    // 傳送封包
    send(fd, buffer, sizeof(buffer) , 0 );
    printf("client send tcp packet\n");
}

// ==================== 接收 UDP 封包 ====================
void rcv_UDPpacket(int fd){
	struct IPHeader *iphdr = (struct IPHeader *)malloc(sizeof(struct IPHeader));
	struct UDPHeader *udphdr = (struct UDPHeader *)malloc(sizeof(struct UDPHeader));
	struct MACHeader *machdr = (struct MACHeader *)malloc(sizeof(struct MACHeader));
	struct Packet *packet = (struct Packet *)malloc(sizeof(struct Packet));
	socklen_t len;
	struct sockaddr_in router_addr;
	char buf[BUFF_LEN];
    int cnt = 0;

	while(cnt<20){
		cnt+=1;
    	memset(buf, 0, BUFF_LEN);
    	len = sizeof(router_addr);
    	count = recvfrom(fd, buf, BUFF_LEN, 0, (struct sockaddr*)&router_addr, &len);
    		
    	if(count == -1){
    		printf("client recieve data fail!\n");
    		return;
    	}else{
    		printf("client rcv UDP packet %d !\n", cnt);
    	}
    		
    	memcpy(packet, buf, sizeof(*packet));
    	*iphdr = packet->ipheader;
    	*udphdr = packet->udpheader;
    	*machdr = packet->macheader;
    }
    printf("end recieve\n");
}

// ==================== TCP 傳送 Thread ====================
void *tcp_socket(void *argu){
	
	//code
	return NULL;
}

// ==================== UDP 接收 Thread ====================
void *udp_socket(void *argu){
		
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

