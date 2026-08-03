#include "PingerSystem.h"

#include <QProcess>
#include <QTextCodec>

/*

  win10成功
    C:\Users\Administrator>ping www.baidu1.com -n 1

    正在 Ping baidu0.com [103.233.82.15] 具有 32 字节的数据:
    来自 103.233.82.15 的回复: 字节=32 时间=186ms TTL=224

    103.233.82.15 的 Ping 统计信息:
        数据包: 已发送 = 1，已接收 = 1，丢失 = 0 (0% 丢失)，
    往返行程的估计时间(以毫秒为单位):
        最短 = 186ms，最长 = 186ms，平均 = 186ms

  win10失败
    C:\Users\Administrator>ping 192.168.1.199 -n 1

    正在 Ping 192.168.1.199 具有 32 字节的数据:
    来自 192.168.1.210 的回复: 无法访问目标主机。

    192.168.1.199 的 Ping 统计信息:
        数据包: 已发送 = 1，已接收 = 1，丢失 = 0 (0% 丢失)，

  win7失败
    C:\Users\Administrator\Desktop\x64>ping 192.168.1.110 -n 1

    Pinging 192.168.1.110 with 32 bytes of data:
    Reply from 192.168.1.109: Destination host unreachable.

    Ping statistics for 192.168.1.110:
        Packets: Sent = 1, Received = 1, Lost = 0 (0% loss),

  win7成功
    C:\Users\Administrator\Desktop\x64>ping 192.168.1.210 -n 1

    Pinging 192.168.1.210 with 32 bytes of data:
    Reply from 192.168.1.210: bytes=32 time<1ms TTL=128

    Ping statistics for 192.168.1.210:
        Packets: Sent = 1, Received = 1, Lost = 0 (0% loss),
    Approximate round trip times in milli-seconds:
        Minimum = 0ms, Maximum = 0ms, Average = 0ms

*/

PingerSystem::PingerSystem(QObject *parent) : QObject(parent)
{

}

bool PingerSystem::ping(const QString &host)
{
    QString cmd = QString("ping %1 -n 1").arg(host);

    QProcess process;
    process.start(cmd);
    process.waitForFinished();

    QByteArray data = process.readAllStandardOutput();

    auto gbk = QTextCodec::codecForName("gbk");
    QString msg = gbk->toUnicode(data);
    return msg.contains("TTL=");
}
