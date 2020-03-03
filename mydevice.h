#ifndef MYDEVICE_H
#define MYDEVICE_H
#include <QByteArray>
#include <QIODevice>
#include <QDebug>
class MyDevice : public QIODevice
{
    Q_OBJECT
public:
    MyDevice();
    ~MyDevice();
    qint64 readData(char *data,qint64 maxlen);
    qint64 writeData(const char *data ,qint64 len);
    void setData(QByteArray pcm);
    QByteArray data_pcm;
    int len_written;

signals:
    void update();
};

#endif // MYDEVICE_H
