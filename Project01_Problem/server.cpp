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
#define SERVER_PORT 9000
#define ROUTER_PORT 9001
#define CLIENT_PORT 9002
#define CLIENTTWO_PORT 9003
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
  char buffer[MTU - 46];//原本設定40
}Packet;

int count = 0;

// ==================== UDP 封包發送 ====================
void udp_msg_sender(int fd, struct sockaddr* dst){
    char buffer[PACKET_SIZE] = {0}; // 用一個 buffer 來組裝整個封包
    socklen_t len = sizeof(*dst);
    int cnt = 0;

    while(cnt <= 20){
        cnt++;
        memset(buffer, 0, PACKET_SIZE); // 每次發送前清空 buffer

        // 準備 payload
        char payload[100];
        sprintf(payload, "This is UDP packet number %d", cnt);
        size_t payload_length = strlen(payload);

        // ---- Header 設定 ----
        MACHeader macHeader;
        // ... (MAC header 內容設定可以保留您原有的)
        uint8_t des_mac[] = {0x21,0x43,0x65,0x87,0x90,0x89}; 
        uint8_t sour_mac[] = {0x12,0x34,0x56,0x78,0x90,0x98};
        memcpy(macHeader.des_mac, des_mac, 6);
        memcpy(macHeader.sour_mac, sour_mac, 6);
        macHeader.fram_typ = htons(0x0800); // IPv4
        macHeader.crc = 0;

        IPHeader ipHeader;
        ipHeader.version_ihl = 0x45;
        ipHeader.type_of_service = 0;
        ipHeader.total_length = htons(sizeof(IPHeader) + sizeof(UDPHeader) + payload_length);
        ipHeader.identification = htons(cnt); // 用 cnt 作為識別
        ipHeader.flags_fragment_offset = htons(0x4000);
        ipHeader.time_to_live = 64;
        ipHeader.protocol = IPPROTO_UDP; // Protocol is UDP (17)
        ipHeader.header_checksum = 0;
        inet_pton(AF_INET, SERVER_IP, &ipHeader.source_ip);
        inet_pton(AF_INET, CLIENT_IP, &ipHeader.destination_ip);

        UDPHeader udpHeader;
        udpHeader.source_port = htons(10000);
        udpHeader.dest_port = htons(10010);
        udpHeader.Segment_Length = htons(sizeof(UDPHeader) + payload_length);
        udpHeader.Checksum = 0;

        // ---- 依序將 Header 和 Payload 複製到 buffer 中 ----
        int offset = 0;
        memcpy(buffer + offset, &macHeader, sizeof(MACHeader));
        offset += sizeof(MACHeader);
        memcpy(buffer + offset, &ipHeader, sizeof(IPHeader));
        offset += sizeof(IPHeader);
        memcpy(buffer + offset, &udpHeader, sizeof(UDPHeader));
        offset += sizeof(UDPHeader);
        memcpy(buffer + offset, payload, payload_length);
        // 計算總長度並發送
        int total_len = offset + payload_length;
        sendto(fd, buffer, total_len, 0, dst, len);
        if (cnt == 21)break;
        printf("server send UDP packet %d !\n", cnt);
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
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("create socket fail");
        return NULL;
    }

    // 2. 綁定 Server 的位址和埠號
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind fail");
        close(listen_fd);
        return NULL;
    }

    // 3. 開始監聽
    if (listen(listen_fd, 5) < 0) {
        perror("listen fail");
        close(listen_fd);
        return NULL;
    }
    
    // 4. 接受來自 client (經 router) 的連線
    client_addr_len = sizeof(client_addr);
    conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    if (conn_fd < 0) {
        perror("accept fail");
        close(listen_fd);
        return NULL;
    }
    
    // 5. 迴圈接收資料並解析
    while ((n = recv(conn_fd, buf, PACKET_SIZE, 0)) > 0) {
        // 計算 Header 的總長度
        // MAC (18) + IP (20) + TCP (20) = 58 bytes
        const int header_total_size = sizeof(MACHeader) + sizeof(IPHeader) + sizeof(TCPHeader);
        
        // Payload 的長度是總接收長度 n 減去 Header 長度
        int payload_len = n - header_total_size;

        if (payload_len > 0) {
            // 從 buffer 中 header 結束的地方開始，就是 payload
            char *payload_start = buf + header_total_size;

            // 為了安全地印出字串，我們複製 payload 到一個新的、有結尾符的 buffer
            char payload_str[payload_len + 1];
            memcpy(payload_str, payload_start, payload_len);
            payload_str[payload_len] = '\0'; // 加上 C 字串結束符號

            // 依照你的範例格式輸出
            printf("---server receive---\n");
            printf("payload : %s\n", payload_str);
            printf("%d\n", ++count);
        }
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
