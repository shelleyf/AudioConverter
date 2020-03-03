#include "mydevice.h"

MyDevice::MyDevice()
{
    this->open(QIODevice::ReadWrite);
    len_written=0;
}
MyDevice::~MyDevice()
{
    this->close();
}

void MyDevice::setData(QByteArray array){
    data_pcm=array;
}

qint64 MyDevice::readData(char *data,qint64 maxlen)
{
    if(len_written >=data_pcm.size())
    {
        return 0;
    }
    int len;
    len = (len_written+maxlen)>data_pcm.size()?(data_pcm.size()-len_written):maxlen;
    memcpy(data,data_pcm.data()+len_written,len);
    len_written += len;
    return len;
}

qint64 MyDevice::writeData(const char *data,qint64 len){
    if(strlen(data)>0){
        return 0;
    }
    qDebug()<<"convert dev write:"<<strlen(data);
    memcpy(&data_pcm+len_written,data,len);
    len_written+=len;
    //if(data_pcm.length()>20000)
    emit update();
    return len;
}


