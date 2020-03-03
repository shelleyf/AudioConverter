#ifndef LAMEMP3DECODER_H
#define LAMEMP3DECODER_H

#include "lame.h"
#include <QtCore>
#ifndef NUMTOCENTRIES
#define NUMTOCENTRIES 100
#endif

class LameMp3Decoder
{
public:
    LameMp3Decoder();
    ~LameMp3Decoder();
    int init(const char* filePath);
    int sampleRate();
    int channelCount();
    int sampleSize();
    qint64 position();
    void setPosition(qint64 pos);
    qint64 duration();

    //解码返回两个通道的pcm数据，返回值为sample数。对应pcm_l、pcm_r数组有效长度.
    //解码mp3 返回要么是1152 要么是576
    int decode(short pcm_l[]);
    int decode2(short pcm_l[], short pcm_r[]);
private:
    hip_t ht_;
    FILE* fp_;

    int channelCount_;
    int sampleRate_;
    int bitRate_;
    //int frameSize_;
    qreal current_;
    qreal duration_;

private:
    unsigned char toc[NUMTOCENTRIES];
    int  skip_start;      /* number samples to ignore at the beginning */
    int  skip_end;        /* number samples to ignore at the end */
    bool is_vbr;
    quint32 file_size;
};

#endif // LAMEMP3DECODER_H
