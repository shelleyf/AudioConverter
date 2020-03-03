#ifndef WCVICUNAPLAYER_H
#define WCVICUNAPLAYER_H

#include <QtCore>
#include <QAudioDeviceInfo>
#include <QMediaPlayer> //使用了他的一些枚举定义.
#include "sonic.h"
#include "LameMp3Decoder.h"

//使用mp3lame库解码，使用sonic变速，使用QAudioOutput播放
class QAudioOutput;
class WcVicunaPlayer : public QObject
{
    Q_OBJECT
public:
    WcVicunaPlayer(QObject *parent = Q_NULLPTR);
    ~WcVicunaPlayer();
    void setMedia(QString localMp3);
    QString currentMedia();
    void play();
    void pause();
    void stop();
     //speed 建议范围0.5~2.0 小数点1位，太多位没效果还可能有副作用
    void setPlaySpeed(float speed);
    void setMuted(bool bMute);
    void setVolume(qreal vol);  //0~1.0
    void setPosition(qint64 position);  //position时间.
    qint64 position();  //获取播放位置.
    QMediaPlayer::State state() const;
    qint64 duration() const;
    QAudioOutput *output_;
    QAudioDeviceInfo audio_out;
protected:
    void unload();  //清理重置，可以重新再setMedia的状态.
signals:
    void finished();
    void error(QMediaPlayer::Error error);
private slots:
    void onFeedTime();
    void handleStateChanged(QAudio::State newState);
private:
    float speed_;
    QIODevice* ioWrite_;
    LameMp3Decoder *decoder_;
    QString localMp3_;
    bool bMute_;
    qreal volume_;
    sonicStream sonic_;
    short *sonicOutput_;
    int sonicMaxOut_;
    QMediaPlayer::State state_;
    QTimer feedTimer_;
};


#endif  //WCVICUNAPLAYER_H
