# =================================================================
# Makefile for Project01_Problem (C/C++ Hybrid Project)
# =================================================================

# --- 1. 定義變數 ---

# C/C++ 編譯器
CXX = g++  # C++ Compiler (GCC/G++)
CC  = gcc  # C Compiler (GCC)

# 最終可執行檔名稱
TARGET = project

# 原始碼檔案
CPPSRCS = client.cpp server.cpp
CSRCS   = router.c

# 目標檔案 (Object files)
# 將 .cpp 轉換為 .o, 將 .c 轉換為 .o
CPPOBJS = $(CPPSRCS:.cpp=.o)
COBJS   = $(CSRCS:.c=.o)

# 所有需要連結的目標檔案
OBJS = $(CPPOBJS) $(COBJS)

# 編譯旗標 (Flags)
# -Wall: 開啟所有常見警告
# -std=c++17: 使用 C++17 標準
# -g: 包含除錯資訊 (Debugging symbols)
# -O2: 開啟優化
CXXFLAGS = -Wall -std=c++17 -g -O2
CFLAGS   = -Wall -std=c99 -g -O2 


# --- 2. 主要目標 ---

# 預設目標：編譯最終可執行檔
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@

# --- 3. 編譯規則 (隱式規則) ---

# C++ 檔案的編譯規則 (.cpp -> .o)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# C 檔案的編譯規則 (.c -> .o)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# --- 4. 輔助目標 ---

# 清理目標：刪除所有編譯產生的中間檔案和可執行檔
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)

# 顯示目標：印出重要的變數值
.PHONY: show_vars
