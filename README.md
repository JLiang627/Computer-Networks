# Computer Networks / 計算機網路

This repository contains all projects and programming exercises for the **Computer Networks** course (EE5310) at National Sun Yat-sen University (NSYSU), Academic Year 114, Semester 1.

> **中文版說明請見**: [README_zh-TW.md](./README_zh-TW.md)

## 📚 Course Information

- **Course Name**: Computer Networks
- **Course Code**: EE5310
- **Institution**: National Sun Yat-sen University (NSYSU)
- **Department**: Department of Electrical Engineering (Master's Program)
- **Instructor**: Prof. Hsu Tsang-Ling (許蒼嶺)
- **Academic Year**: 114-1 (Fall 2025)
- **Credits**: 3
- **Course Type**: Elective

## 📂 Repository Structure

```
Computer-Networks/
├── Project01_Problem/   # Project 1 - Client/Server/Router Implementation
│   ├── client.cpp       # TCP/UDP Client implementation
│   ├── server.cpp       # TCP/UDP Server implementation
│   ├── router.c         # Router packet forwarding
│   └── Makefile         # Build configuration
├── Project02_Problem/   # Project 2 - Advanced Networking
│   ├── client.cpp       # Enhanced client
│   ├── server.cpp       # Enhanced server
│   ├── router.c         # Advanced router features
│   └── script.sh        # Testing scripts
```

## 📝 Projects Overview

### Project 1: Basic Client-Server-Router Communication
- **Implementation**: TCP and UDP socket programming
- **Features**: 
  - MAC/IP/UDP/TCP packet structure simulation
  - Multi-threaded client and server
  - Router-based packet forwarding
- **Technologies**: C/C++, POSIX sockets, pthreads

### Project 2: Advanced Network Programming
- **Implementation**: Enhanced networking features
- **Features**: Extended protocol handling and network simulation

## 🛠️ Technologies Used

- **Primary Languages**: C/C++
- **Development Environment**: Linux/Unix
- **Topics Covered**:
  - Overview of Data Networks and Protocols
  - Wireless Transmission and Communications
  - WiFi vs 4G/5G/6G
  - Personal Area Networks (BlueTooth, Sensors, RFID)
  - Transport Protocols (TCP/UDP)
  - Internet Protocols
  - Integrated and Differentiated Services
  - Protocols for QoS Support
  - SDN, NFV, Network Slicing, Mobile Edge Computing

## 🔧 Build Instructions

```bash
# Navigate to project directory
cd Project01_Problem

# Build all executables
make

# Run the programs (in separate terminals)
./server
./router
./client
```

## 📖 Course Syllabus

1. Overview of Data Networks and Protocols
2. Wireless Transmission and Communications
3. WiFi vs 4G/5G/6G
4. Personal Area Networks (BlueTooth, Sensors, RFID)
5. Transport Protocols
6. Internet Protocols
7. Integrated and Differentiated Services
8. Protocols for QoS Support
9. SDN, NFV, Network Slicing, Mobile Edge Computing

## 📊 Evaluation

- **Midterm Exam**: 35%
- **Final Exam**: 35%
- **Projects**: 30%

## 📧 Contact

- **Author**: JLiang627
- **Email**: a0979652527@icloud.com
- **GitHub**: [@JLiang627](https://github.com/JLiang627)

## 📄 License

This repository is for educational purposes only. Please respect academic integrity policies.

---

**Note**: All assignments were completed as requirements for the NSYSU course. Code and solutions are for reference and learning purposes only.
