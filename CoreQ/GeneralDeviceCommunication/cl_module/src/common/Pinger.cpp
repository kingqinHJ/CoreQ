#include "Pinger.h"

#include "Utils.h"

#ifdef Q_OS_UNIX
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

typedef int        SOCKET;
#define SOCKET_ERROR -1
#elif defined(Q_OS_WIN)
#include <WinSock.h>
#pragma comment(lib, "Ws2_32.lib")
#include <iphlpapi.h>
#pragma comment(lib, "Iphlpapi.lib")
#endif

/*

OSI七层模型：
  应用层 -> 表示层 -> 会话层 -> 传输层 -> 网络层 -> 数据链路层 -> 物理层

TCP/IP五层模型：
  应用层 -> 传输层（TCP、UDP） -> 网络层（IP协议、ARP协议、PARP协议、ICMP协议、IGMP协议） -> 数据链路层 -> 物理层

以太网首部格式：
  6byte终端MAC地址
  6byte源端MAC地址
  2byte帧类型（ARP: 0x0806 IPv4: 0x0800）

IP数据报首部格式：
  4bit版本号 | 4bit首部长度 | 8bit服务类型TOS | 16bit总长度
  16bit标识 | 3bit标志 | 13bit片偏移
  8bit生存时间TTL | 8bit协议 | 16bit头部校验和
  32bit源端IP地址
  32bit终端IP地址
  可选字段，最大40字节

  检验和计算：
    1.把校验和字段置为0；
    2.对IP头部中的每16bit进行二进制求和；
    3.如果和的高16bit不为0，则将和的高16bit和低16bit反复相加，直到和的高16bit为0，从而获得一个16bit的值；
    4.将该16bit的值取反，存入校验和字段。

  20字节固定部分+40字节可选部分，IP头部最长60字节

ICMP报文格式：
  8bit类型 | 8bit代码 | 8bit校验和
  16bit标识 | 16bit序列号
  ICMP数据部分

  ICMP帧=以太网首部+IP数据报首部+ICMP报文

ARP报文格式：
  2byte硬件类型 | 2byte协议类型 | 1byte硬件地址长度 | 1byte协议地址长度 | 2byte OP（为1表示请求，为0表示应答）
  6byte源端MAC地址 | 4byte源端IP地址
  6byte源端MAC地址 | 4byte源端IP地址

  ARP帧=以太网首部+ARP报文

  注：
    windows好像不支持通过socket发送arp，通过winapi实现: sendARP
    https://learn.microsoft.com/zh-cn/windows/win32/api/iphlpapi/nf-iphlpapi-sendarp?redirectedfrom=MSDN

ping: 基于ICMP协议

*/

typedef struct IPHeader
{
    // 版本(0b0100)+首部长度(IP首部有多少个32bit，默认首部大小20字节，即为0b0101)
    quint8  VIHL = 0b01010100;
    quint8  ToS;            // 服务类型
    quint16 TotalLength;    // 整个IP数据报的字节大小
    quint16 ID;             // 唯一地标识主机发送地每一个数据报
    quint16 Frag_Flags;     // 标志+片偏移量
    quint8  TTL;            // 生存时间
    quint8  Protocol;       // 协议  ICMP是1，TCP是6，UDP是17
    quint16 Checksum;       // 首部校验和
    quint32 SrcIP;          // 源端地址-这是大端字节序
    quint32 DestIP;         // 终端地址
}IPHDR, *PIPHDR;

#define ICMP_ECHOREQ 8
#define ICMP_REPLY   0
// ICMP头结构体 （标准ICMP头为8字节）
typedef struct ICMPHeader
{
    quint8  Type;       // 类型 type=8表示响应请求报文，type=0表示响应应答报文
                        //   Type：8，Code：0：表示回显请求报文(ping请求)
                        //   Type：0，Code：0：表示回显回答报文(ping应答)
    quint8  Code;       // 代码
    quint16 Checksum;   // 首部校验和
    quint16 ID;         // 标识
    quint16 Seq;        // 序列号
}ICMPHDR, *PICMPHDR;

#define REQ_DATASIZE 32
//定义 ICMP 回应请求
typedef struct ECHOREQUEST
{
    ICMPHDR icmpHdr;
    quint32 ts;
    char data[REQ_DATASIZE];
}ECHOREQUEST, *PECHOREQUEST;

//定义 ICMP 回应答复
typedef struct ECHOREPLY
{
    IPHDR ipHdr;
    ECHOREQUEST echoRequest;
    char filler[256];
}ECHOREPLY, *PECHOREPLY;

class PingerPrivate {
public:
    bool abort = false;

    quint16 checksum(char *buffer, int len);

    int SendEchoRequest(SOCKET s, struct sockaddr_in *lpstToAddr);
    quint32 RecvEchoReply(SOCKET s, sockaddr_in *lpsaFrom, quint8 *pTTL);
    int WaitForEchoReply(SOCKET s);

    bool Ping(const char *pstrHost);
};

quint16 PingerPrivate::checksum(char *buffer, int len)
{
    quint16 *w = (quint16*)buffer;
    quint16 result = 0;
    int sum = 0;

    while (len > 1) {
        sum += *w++;
        len -= 2;
    }

    if (len == 1) {
        sum += *(u_char*)w;
    }

    sum = (sum >> 16) + (sum & 0xffff);    // 高16位和低16位相加，sum & 0xffff将高16位置0
    sum += (sum >> 16);					   // 将低16位与溢出值相加
    result = ~sum;						   // 取反码
    return result;
}

int PingerPrivate::SendEchoRequest(SOCKET s, sockaddr_in *lpstToAddr)
{
    static ECHOREQUEST echoReq;
    static int nId = 1;
    static int nSeq = 1;
    int nRet;
    echoReq.icmpHdr.Type = ICMP_ECHOREQ;
    echoReq.icmpHdr.Code = 0;
    echoReq.icmpHdr.Checksum = 0;
    echoReq.icmpHdr.ID = nId++;
    echoReq.icmpHdr.Seq = nSeq++;

    // 初始化填充字节
    for (nRet = 0; nRet < REQ_DATASIZE; nRet++)
        echoReq.data[nRet] = '1';

    // 记录时间戳
    // echoReq.ts = GetTickTimeMS();
    echoReq.ts = 123456;

    // 计算校验和
    echoReq.icmpHdr.Checksum = checksum((char*)&echoReq, sizeof(ECHOREQUEST));

    nRet = sendto(s, (char*)&echoReq, sizeof(ECHOREQUEST), 0, (struct sockaddr*)lpstToAddr, sizeof(sockaddr_in));
    // if (nRet == SOCKET_ERROR)
    //     LOGC("sendto error: %d", nRet);

    return echoReq.ts;
}

quint32 PingerPrivate::RecvEchoReply(SOCKET s, sockaddr_in *lpsaFrom, quint8 *pTTL)
{
    ECHOREPLY echoReply;
    int nRet;
#ifdef Q_OS_UNIX
    socklen_t nAddrLen = sizeof(struct sockaddr_in);
#elif defined(Q_OS_WIN)
    int nAddrLen = sizeof(struct sockaddr_in);
#endif

    nRet = recvfrom(s, (char*)&echoReply, sizeof(ECHOREPLY), 0, (struct sockaddr*)lpsaFrom, &nAddrLen);
    if (nRet == SOCKET_ERROR)
        LOGC("recvfrom error: %d", nRet);

    *pTTL = echoReply.ipHdr.TTL;

    return echoReply.echoRequest.ts;
}

int PingerPrivate::WaitForEchoReply(SOCKET s)
{
    struct timeval timeout;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(s, &readfds);
    timeout.tv_sec = 0;
    timeout.tv_usec = 500*1000;

    return select(s+1, &readfds, NULL, NULL, &timeout);
}

bool PingerPrivate::Ping(const char *pstrHost)
{
    SOCKET rawSocket;
    struct sockaddr_in destIP;
    struct sockaddr_in srcIP;
    quint32 dwTimeSend;
    quint32 dwTimeRecv;
    // quint32 minimum = 100000, maximum = 0;
    quint8 cTTL;
    int nRet;

    // 创建原始套接字 ,ICMP 类型
    // 仅支持AF_INET格式，也就是说ARPA Internet地址格式； SOCK_RAW 为原始套接字，可自定义ip
    rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (rawSocket == SOCKET_ERROR) {
        // LOGC("socket error: %d", rawSocket);
        return false;
    }

    std::shared_ptr<void> guard(nullptr, [rawSocket](void*){
#ifdef Q_OS_UNIX
        close(rawSocket);
#elif defined(Q_OS_WIN)
        closesocket(rawSocket);
#endif
    });

    destIP.sin_family = AF_INET; //地址规格
    destIP.sin_port = 0;

    unsigned long inaddr = inet_addr(pstrHost);
    // 判断用户输入的是否为IP地址还是域名
    if(inaddr == INADDR_NONE) {
        hostent *host = gethostbyname(pstrHost);
        if(host == NULL) {
            LOGC("host not found: %s", pstrHost);
            return false;
        }

        memcpy((char*)&destIP.sin_addr, host->h_addr, host->h_length);
    }
    else {
        memcpy((char*)&destIP.sin_addr, &inaddr, sizeof(inaddr));
    }

    // inaddr = destIP.sin_addr.s_addr;
    // LOGD("PING %s, (%d.%d.%d.%d) 56(84) bytes of data: %d", pstrHost,
    //         (inaddr&0x000000ff), (inaddr&0x0000ff00)>>8,
    //         (inaddr&0x00ff0000)>>16, (inaddr&0xff000000)>>24, rawSocket);

    int n = 5;
    bool state = false;
    while (n > 0) {
        n--;
        dwTimeSend = SendEchoRequest(rawSocket, &destIP);

        nRet = WaitForEchoReply(rawSocket);
        if (nRet == SOCKET_ERROR) {
            LOGC("select error: %d", nRet);
            break;
        }
        else if (!nRet) {
            // LOGC("request time out: %s", qUtf8Printable(pstrHost));
            if (abort) break;
            else continue;
        }

        dwTimeRecv = RecvEchoReply(rawSocket, &srcIP, &cTTL);

        // 计算花费的时间
        // quint32 dwElapsed = GetTickTimeMS() - dwTimeSent;
        // if (dwElapsed > maximum) maximum = dwElapsed;
        // if (dwElapsed < minimum) minimum = dwElapsed;
        // LOGC("Reply from %s: bytes = %d time = %ldms TTL = %d\n",
        //           inet_ntoa(srcIP.sin_addr), REQ_DATASIZE, dwElapsed, cTTL);

        state = dwTimeSend == dwTimeRecv;
        if (state)
            break;
    }

    return state;
}

Pinger::Pinger()
{
    d.reset(new PingerPrivate);

#ifdef Q_OS_WIN
    WSADATA WSAData;
    int ret = WSAStartup(MAKEWORD(2, 2), &WSAData);
    if (ret != 0) {
        switch (ret) {
        case WSASYSNOTREADY:
            fprintf(stderr, "重启电脑试试，或者检查网络库.\n");
            break;
        case WSAVERNOTSUPPORTED:
            fprintf(stderr, "请更新网络库.\n");
            break;
        case WSAEINPROGRESS:
            fprintf(stderr, "请重新启动.\n");
            break;
        case WSAEPROCLIM:
            fprintf(stderr, "请尝试关闭不必要的软件，为当前网络运行提供充足资源.\n");
            break;
        default:
            break;
        }
    }
#endif
}

Pinger::~Pinger()
{
    abort();
}

void Pinger::abort()
{
    d->abort = true;
}

bool Pinger::ping(const QString &host)
{
    return d->Ping(qUtf8Printable(host));
}
