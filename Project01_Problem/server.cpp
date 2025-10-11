#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>  
#include <iostream>
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
  char buffer[MTU - 40];
}Packet;

int count = 0;

// ==================== UDP 封包發送 ====================
void udp_msg_sender(int fd, struct sockaddr* dst){
    int payload_size;
    
    // ---- MAC Header ----
    struct MACHeader *machdr = (struct MACHeader *)malloc(sizeof(struct MACHeader));
    unsigned char sour[] = {0x12,0x34,0x56,0x78,0x90,0x98};
    unsigned char des[] = {0x21,0x43,0x65,0x87,0x90,0x89}; 
    memcpy(machdr->sour_mac, sour , sizeof(sour));
    memcpy(machdr->des_mac, des , sizeof(des));  	
    machdr->fram_typ = 0x0000; 
    machdr->crc = 0x00000000;
     
    // ---- IP Header ----
	struct IPHeader *iphdr = (struct IPHeader *)malloc(sizeof(struct IPHeader));
	iphdr->version_ihl = 0x45;
	iphdr->total_length = MTU;
	iphdr->identification = 0xAAAA;
	iphdr->flags_fragment_offset = 0x4000;
	iphdr->time_to_live = 100;
	iphdr->protocol = 0x11; // UDP
	iphdr->header_checksum = 0;
	iphdr->source_ip = 0x0A115945;
	iphdr->destination_ip = 0x0A000301;
	
    // ---- UDP Header ----
	struct UDPHeader *udphdr = (struct UDPHeader *)malloc(sizeof(struct UDPHeader));
	udphdr->source_port = 10000;
	udphdr->dest_port = 10010;
	udphdr->Segment_Length = MTU - 38;
	udphdr->Checksum = 0; 
    
    payload_size = MTU - sizeof(*iphdr) - sizeof(*udphdr) - sizeof(*machdr);
    
    char buf[payload_size];
    for(int i=0 ; i<payload_size ; i++){
    	buf[i] = 1;
    }
    
    struct Packet *packet = (struct Packet *)malloc(sizeof(struct Packet));
    packet->ipheader = *iphdr;
    packet->udpheader = *udphdr;
    packet->macheader = *machdr;
       
    socklen_t len;
    int cnt=0;

    while(cnt<20){     
    	cnt+=1;   
        packet->ipheader = *iphdr;
	    packet->udpheader = *udphdr;
	    packet->macheader = *machdr;
        len = sizeof(*dst);
        sendto(fd, packet, sizeof(*packet), 0, dst, len);
	    printf("server send UDP packet %d !\n", cnt );
	    sleep(1);
    }
}


// ==================== UDP 發送 Thread ====================
void *udp_socket(void *argu){
	int server_fd;
    struct sockaddr_in router_addr;

    // 1. 建立 UDP socket
    // AF_INET: 使用 IPv4
    // SOCK_DGRAM: 使用 UDP
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(server_fd < 0){
        perror("create socket fail");
        return NULL;
    }

    // 2. 設定 Router 的位址資訊
    memset(&router_addr, 0, sizeof(router_addr)); // 清空結構體
    router_addr.sin_family = AF_INET;
    router_addr.sin_port = htons(ROUTER_PORT); // 指定 Router 的埠號
    router_addr.sin_addr.s_addr = inet_addr(SERVER_IP); // 指定 Router 的 IP

    // 3. 呼叫發送函式
    udp_msg_sender(server_fd, (struct sockaddr*)&router_addr);
    
    // 4. 關閉 socket
    close(server_fd);
	
	return NULL;
}

// ==================== TCP 接收 Thread ====================
void *tcp_socket(void *argu){
	int listen_fd, conn_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    char buf[PACKET_SIZE];
    int n;

    // 1. 建立 TCP socket
    // AF_INET: 使用 IPv4
    // SOCK_STREAM: 使用 TCP
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("create socket fail");
        return NULL;
    }

    // 2. 綁定 Server 的位址和埠號
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 監聽任何來源的 IP
    server_addr.sin_port = htons(SERVER_PORT); // 監聽指定的埠號

    if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind fail");
        close(listen_fd);
        return NULL;
    }

    // 3. 開始監聽
    // backlog 設為 5，表示最多允許 5 個連線請求排隊
    if (listen(listen_fd, 5) < 0) {
        perror("listen fail");
        close(listen_fd);
        return NULL;
    }
    
    printf("Server listening on port %d\n", SERVER_PORT);

    // 4. 接受來自 client (經 router) 的連線
    client_addr_len = sizeof(client_addr);
    conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    if (conn_fd < 0) {
        perror("accept fail");
        close(listen_fd);
        return NULL;
    }
    printf("...server receive ----\n");

    // 5. 迴圈接收資料
    while ((n = recv(conn_fd, buf, PACKET_SIZE, 0)) > 0) {
        // 這裡可以加入解開並印出封包內容的程式碼
        // 為了簡化，我們先只印出接收到的訊息
        printf("server receive tcp packet %d\n", ++count);
    }
    
    // 6. 關閉 socket
    close(conn_fd);
    close(listen_fd);
	
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
