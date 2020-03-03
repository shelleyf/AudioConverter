#include "WcVicunaPlayer.h"
#include "lame.h"
#include <QAudioOutput>

#define SPEEDMAX (3.0f)
#define SPEEDMIN (0.5f)


WcVicunaPlayer::WcVicunaPlayer(QObject *parent)
    : QObject(parent)
    , speed_(1.0f)
    , output_(NULL)
    , ioWrite_(NULL)
    , decoder_(NULL)
    , bMute_(false)
    , volume_(1.0)
    , sonic_(NULL)
    , sonicOutput_(NULL)
    , sonicMaxOut_(0)
    , state_(QMediaPlayer::StoppedState)
{
    feedTimer_.setInterval(10);  //根据资料了解到一个frame固定是20ms
    feedTimer_.setSingleShot(false);
    connect(&feedTimer_,&QTimer::timeout,this,&WcVicunaPlayer::onFeedTime);
}

WcVicunaPlayer::~WcVicunaPlayer()
{
    stop();
    unload();
}

void WcVicunaPlayer::setMedia(QString localMp3)
{
    QString ntPath = QDir::toNativeSeparators(localMp3);
    if(QFile::exists(ntPath)==false){
        emit error(QMediaPlayer::ResourceError);
        return;
    }
    if(decoder_){
        stop();
        unload();
    }

    Q_ASSERT(decoder_ == NULL);
    QByteArray path = ntPath.toLocal8Bit();
    decoder_ = new LameMp3Decoder();
    if(0 != decoder_->init(path.constData()))
    {
        delete decoder_;
        decoder_ = NULL;
        emit error(QMediaPlayer::AccessDeniedError);
        return;
    }
    float spd = speed_;
    speed_ = 1.0f;
    setPlaySpeed(spd);   //如果speed_不是1.0f，会触发创建sonic.

    localMp3_ = ntPath;
}

QString WcVicunaPlayer::currentMedia()
{
    return localMp3_;
}

void WcVicunaPlayer::play()
{
    if(state_ == QMediaPlayer::PlayingState) return;
    if(decoder_==NULL){
        qInfo()<<"Can't play at this moment.";
        emit error(QMediaPlayer::ResourceError);
        return;
    }
    feedTimer_.start();
    state_ = QMediaPlayer::PlayingState;
}

void WcVicunaPlayer::pause()
{
    if(state_ == QMediaPlayer::PausedState) return;
    feedTimer_.stop();
    state_ = QMediaPlayer::PausedState;

}

void WcVicunaPlayer::stop()
{
    if(state_ == QMediaPlayer::StoppedState) return;
    feedTimer_.stop();
    setPosition(0);
    state_ = QMediaPlayer::StoppedState;
}

void WcVicunaPlayer::setPlaySpeed(float speed)
{
    Q_ASSERT(speed>=SPEEDMIN && speed<=SPEEDMAX);
    if(speed<SPEEDMIN)speed = SPEEDMIN;
    if(speed>SPEEDMAX) speed = SPEEDMAX;
    if(speed == speed_) return;
    speed_ = speed;

    //修改速度需要重建sonic_
    if(sonic_){
        sonicDestroyStream(sonic_);
        sonic_ = NULL;
    }
    //1.0的速度不创建sonic_
    if(int(speed_*10)==10) return;
    if(decoder_ == NULL){
        qInfo()<<"currently no decoder_, can't Create sonic stream.";
        return;
    }
    sonic_ = sonicCreateStream(decoder_->sampleRate(),1);
    sonicSetSpeed(sonic_, speed_);
    //int newMaxOut = 1152*speed_+10;
    int newMaxOut = 2400;
    if(newMaxOut > sonicMaxOut_)
    {
        sonicMaxOut_ = newMaxOut;
        if(sonicOutput_){
            delete[] sonicOutput_;
            sonicOutput_ = NULL;
        }
        sonicOutput_ = new short[sonicMaxOut_];
    }
    qInfo()<<"sonic created."<<sonic_<<"speed:"<<speed_;
}

void WcVicunaPlayer::setMuted(bool bMute)
{
    if(bMute_ == bMute) return;
    bMute_ = bMute;
    if(output_){  //支持提前设置，后生效
        if(bMute_){
            output_->setVolume(0);
        }
        else{
            output_->setVolume(volume_);
        }
    }
    else{
        qInfo()<<"currently no output. setMuted delay.";
    }
}

void WcVicunaPlayer::setVolume(qreal vol)
{
    volume_ = vol;
    if(output_){  //支持提前设置，后生效
        if(bMute_ == false){
            output_->setVolume(vol);
        }
    }
    else{
        qInfo()<<"currently no output. setMuted delay.";
    }
}

void WcVicunaPlayer::setPosition(qint64 position)
{
    if(decoder_==NULL) return;
    if(position < 0 || position > decoder_->duration()){
        qInfo()<<"seek trigger stop. pos:"<<position<<"duration:"<<decoder_->duration();
        stop();
        return;
    }
    decoder_->setPosition(position);
}

qint64 WcVicunaPlayer::position()
{
    if(decoder_==NULL) return 0;
    return decoder_->position();
}

QMediaPlayer::State WcVicunaPlayer::state() const
{
    return state_;
}

qint64 WcVicunaPlayer::duration() const
{
    if(decoder_==NULL) return 0;
    return decoder_->duration();
}

void WcVicunaPlayer::unload()
{
    Q_ASSERT(state_ == QMediaPlayer::StoppedState);
    Q_ASSERT(feedTimer_.isActive()==false);
    stop();
    if(decoder_){
        delete decoder_;
        decoder_ = NULL;
    }
    if(sonic_){
        sonicDestroyStream(sonic_);
        sonic_ = NULL;
    }
    if(sonicMaxOut_>0 && sonicOutput_){
        delete []sonicOutput_;
        sonicOutput_ = NULL;
    }

    if(output_){
        delete output_;
        output_ = NULL;
    }
    bMute_ = false;
    volume_ = 1.0f;
    localMp3_.clear();
}

void WcVicunaPlayer::onFeedTime()
{
    if(state_ != QMediaPlayer::PlayingState) return;
    if(decoder_ == NULL) return;
    if(output_ == NULL){
        QAudioFormat format;
        format.setSampleRate(decoder_->sampleRate());
        //format.setChannelCount(decoder_->channelCount());
        format.setChannelCount(1); //由喂数据的方式决定了就是单通道=1.
        format.setSampleSize(decoder_->sampleSize());
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        //注意，16bit固定要用SignedInt。否则mac上不出声音播不动.!!
        format.setSampleType(QAudioFormat::SignedInt);


//        QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
        if (!audio_out.isFormatSupported(format)) {
            qWarning() << "Default format not supported - trying to use nearest";
            format = audio_out.nearestFormat(format);
        }


//        output_ = new QAudioOutput(QAudioDeviceInfo::defaultOutputDevice(),
//                                   format, this);
        output_ = new QAudioOutput(audio_out,format);
        //output_ = new QAudioOutput(format);
        setMuted(bMute_);
        setVolume(volume_);
        connect(output_, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
        ioWrite_ = output_->start();
    }

    while(output_->bytesFree()>0)
    {
        short pcm_l[1200];
        int nRet = decoder_->decode(pcm_l);
        if(nRet == -1){
            this->stop();
            emit finished();
            return;
        }
        //默认需要写入的buff和长度（short为单位）
        short* writeToDevice = pcm_l;
        int shortToWrite = nRet;
        if(sonic_){  //有变速需求.
            sonicWriteShortToStream(sonic_, pcm_l, nRet);
            nRet = sonicReadShortFromStream(sonic_,sonicOutput_,sonicMaxOut_);
            writeToDevice = sonicOutput_;
            shortToWrite = nRet;
        }
        if(shortToWrite>0){
            ioWrite_->write((const char*)writeToDevice,shortToWrite*2);
        }
    }
}

void WcVicunaPlayer::handleStateChanged(QAudio::State newState)
{
    qInfo()<<"QAudioOutput state change to:"<<newState;
}
