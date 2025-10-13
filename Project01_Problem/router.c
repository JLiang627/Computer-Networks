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
    //uint32_t options; // router 有多一個 options 欄位
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
  char buffer[MTU - 46];//原本是28
}Packet;

// ==================== TCP 轉發 Thread ====================
void *tcp_socket(void *argu) {
    int listen_fd, client_conn_fd, server_fd;
    struct sockaddr_in router_addr, server_addr, client_addr;
    socklen_t client_addr_len;
    char buf[PACKET_SIZE];
    ssize_t n;

    // --- Part 1: 作為 Server，等待 Client 連線 ---

    // 1. 建立 TCP listening socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 2. 綁定 Router 用來接收 Client 連線的位址和埠號
    memset(&router_addr, 0, sizeof(router_addr));
    router_addr.sin_family = AF_INET;
    router_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    router_addr.sin_port = htons(CLIENT_PORT); // 監聽 9002

    bind(listen_fd, (struct sockaddr*)&router_addr, sizeof(router_addr));

    // 3. 開始監聽
    listen(listen_fd, 5);

    // 4. 接受來自 Client 的連線
    client_addr_len = sizeof(client_addr);
    client_conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);


    // --- Part 2: 作為 Client，連接到 Server ---

    // 5. 建立一個新的 TCP socket 用來連線到 Server
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 6. 設定 Server 的位址資訊
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT); // Server 的埠號 9000

    // 7. 連接到 Server
    if (connect(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect to server fail");
        close(client_conn_fd);
        close(listen_fd);
        return NULL;
    }

    // --- Part 3: 在 Client 和 Server 之間轉發資料 ---

    // 8. 迴圈接收並轉發
    while ((n = recv(client_conn_fd, buf, PACKET_SIZE, 0)) > 0) {
        printf("get tcp\n");
        // 將從 client 收到的資料，轉發給 server
        if (send(server_fd, buf, n, 0) < 0) {
            perror("send to server fail");
            break;
        }
        printf("send tcp\n");
    }

    // 9. 關閉所有 sockets
    close(client_conn_fd);
    close(server_fd);
    close(listen_fd);
	return NULL;
}

// ==================== UDP 轉發 Thread ====================
void *udp_socket(void *argu) {
    int router_fd;
    struct sockaddr_in router_addr, server_addr, client_addr;
    socklen_t server_addr_len;
    char buf[BUFF_LEN];
    ssize_t n;

    // 1. 建立 UDP socket
    router_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (router_fd < 0) {
        perror("create socket fail");
        return NULL;
    }

    // 2. 綁定 Router 用來接收 Server 封包的位址和埠號
    memset(&router_addr, 0, sizeof(router_addr));
    router_addr.sin_family = AF_INET;
    router_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    router_addr.sin_port = htons(ROUTER_PORT); // 監聽 9001

    if (bind(router_fd, (struct sockaddr*)&router_addr, sizeof(router_addr)) < 0) {
        perror("bind fail");
        close(router_fd);
        return NULL;
    }

    // 3. 設定要轉發的目的地 Client 的位址資訊
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr(CLIENT_IP);
    client_addr.sin_port = htons(CLIENTTWO_PORT); // Client 的接收埠號 9003

    // 4. 迴圈接收並轉發
    int packet_count = 0;
    while (packet_count < 20) {
        packet_count++;
        server_addr_len = sizeof(server_addr);
        n = recvfrom(router_fd, buf, BUFF_LEN, 0, (struct sockaddr*)&server_addr, &server_addr_len);
        if (n > 0) {
            // ***** 修改點：在字串結尾加上 \n *****
            printf("get udp\n"); 
            
            // 將收到的封包 (存在 buf 中) 直接轉發給 client
            sendto(router_fd, buf, n, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
            
            // ***** 修改點：在字串結尾加上 \n *****
            printf("send udp\n");
        }
    }

    close(router_fd);
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
