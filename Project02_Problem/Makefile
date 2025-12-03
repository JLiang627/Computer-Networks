## 編譯器設定
CC = gcc
CXX = g++

## 編譯旗標
# -Wall: 開啟所有警告
# -g: 產生除錯資訊
# -pthread: 支援多執行緒編譯與連結
CFLAGS = -Wall -g -pthread
CXXFLAGS = -Wall -g -pthread -std=c++11

## 目標程式名稱
TARGETS = client router server

.PHONY: all clean

## 預設目標：編譯所有程式
all: $(TARGETS)

## ----------------------------------------------------
## 程式編譯規則
## ----------------------------------------------------

# 編譯 client (C++)
client: client.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

# 編譯 router (C)
router: router.c
	$(CC) $(CFLAGS) $< -o $@

# 編譯 server (C++)
server: server.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

## ----------------------------------------------------
## 清理目標
## ----------------------------------------------------

# 清除所有編譯出的執行檔與目的檔
clean:
	rm -f $(TARGETS) *.o