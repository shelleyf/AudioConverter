#include "LameMp3Decoder.h"
#include <QDebug>
#include "config.h"
#include "mpg123.h"
#include "mpglib.h"
#include "interface.h"
#include "VbrTag.h"
#ifndef JWIN
#include <sys/stat.h>
#endif

static  size_t
min_size_t(size_t a, size_t b)
{
    if (a < b) {
        return a;
    }
    return b;
}
/* Replacement for forward fseek(,,SEEK_CUR), because fseek() fails on pipes */
static int fskip(FILE * fp, long offset, int whence)
{
#ifndef PIPE_BUF
    char    buffer[4096];
#else
    char    buffer[PIPE_BUF];
#endif

    /* S_ISFIFO macro is defined on newer Linuxes */
#ifndef S_ISFIFO
# ifdef _S_IFIFO
    /* _S_IFIFO is defined on Win32 and Cygwin */
#  define S_ISFIFO(m) (((m)&_S_IFIFO) == _S_IFIFO)
# endif
#endif

#ifdef S_ISFIFO
    /* fseek is known to fail on pipes with several C-Library implementations
       workaround: 1) test for pipe
       2) for pipes, only relatvie seeking is possible
       3)            and only in forward direction!
       else fallback to old code
     */
    {
        int const fd = fileno(fp);
        struct stat file_stat;

        if (fstat(fd, &file_stat) == 0) {
            if (S_ISFIFO(file_stat.st_mode)) {
                if (whence != SEEK_CUR || offset < 0) {
                    return -1;
                }
                while (offset > 0) {
                    size_t const bytes_to_skip = min_size_t(sizeof(buffer), offset);
                    size_t const read = fread(buffer, 1, bytes_to_skip, fp);
                    if (read < 1) {
                        return -1;
                    }
                    //assert( read <= LONG_MAX );
                    offset -= (long) read;
                }
                return 0;
            }
        }
    }
#endif
    if (0 == fseek(fp, offset, whence)) {
        return 0;
    }
    if (whence != SEEK_CUR || offset < 0) {
        qInfo()<<"fskip problem: Mostly the return status of functions is not evaluate so it is more secure to polute <stderr>.";
        return -1;
    }
    while (offset > 0) {
        size_t const bytes_to_skip = min_size_t(sizeof(buffer), offset);
        size_t const read = fread(buffer, 1, bytes_to_skip, fp);
        if (read < 1) {
            return -1;
        }
        //assert( read <= LONG_MAX );
        offset -= (long) read;
    }
    return 0;
}
static int check_aid(const unsigned char *header)
{
    return 0 == memcmp(header, "AiD\1", 4);
}

/*
 * Please check this and don't kill me if there's a bug
 * This is a (nearly?) complete header analysis for a MPEG-1/2/2.5 Layer I, II or III
 * data stream
 */

static int is_syncword_mp123(const void *const headerptr)
{
    const unsigned char *const p = (const unsigned char *const)headerptr;
    static const char abl2[16] = { 0, 7, 7, 7, 0, 7, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8 };

    if ((p[0] & 0xFF) != 0xFF)
        return 0;       /* first 8 bits must be '1' */
    if ((p[1] & 0xE0) != 0xE0)
        return 0;       /* next 3 bits are also */
    if ((p[1] & 0x18) == 0x08)
        return 0;       /* no MPEG-1, -2 or -2.5 */
    switch (p[1] & 0x06) {
    default:
    case 0x00:         /* illegal Layer */
        return 0;

    case 0x02:         /* Layer3 */
        //if (global_reader.input_format != sf_mp3 && global_reader.input_format != sf_mp123) {
        //    return 0;
        //}
        //global_reader.input_format = sf_mp3;
        break;

    case 0x04:         /* Layer2 */
        //if (global_reader.input_format != sf_mp2 && global_reader.input_format != sf_mp123) {
            return 0;
        //}
        //global_reader.input_format = sf_mp2;
        break;

    case 0x06:         /* Layer1 */
        //if (global_reader.input_format != sf_mp1 && global_reader.input_format != sf_mp123) {
            return 0;
        //}
        //global_reader.input_format = sf_mp1;
        break;
    }
    if ((p[1] & 0x06) == 0x00)
        return 0;       /* no Layer I, II and III */
    if ((p[2] & 0xF0) == 0xF0)
        return 0;       /* bad bitrate */
    if ((p[2] & 0x0C) == 0x0C)
        return 0;       /* no sample frequency with (32,44.1,48)/(1,2,4)     */
    if ((p[1] & 0x18) == 0x18 && (p[1] & 0x06) == 0x04 && abl2[p[2] >> 4] & (1 << (p[3] >> 6)))
        return 0;
    if ((p[3] & 3) == 2)
        return 0;       /* reserved enphasis mode */
    return 1;
}

static size_t lenOfId3v2Tag(unsigned char const* buf)
{
    unsigned int b0 = buf[0] & 127;
    unsigned int b1 = buf[1] & 127;
    unsigned int b2 = buf[2] & 127;
    unsigned int b3 = buf[3] & 127;
    return (((((b0 << 7) + b1) << 7) + b2) << 7) + b3;
}

static  off_t lame_get_file_size(FILE * fp)
{
    struct stat sb;
    int     fd = fileno(fp);

    if (0 == fstat(fd, &sb))
        return sb.st_size;
    return (off_t) - 1;
}

LameMp3Decoder::LameMp3Decoder()
    : channelCount_(1)
    , sampleRate_(32000)
    , bitRate_(0)
    //frameSize_
    , current_(0)
    , duration_(0)
    , skip_start(0)
    , skip_end(0)
    , is_vbr(false)
{
    ht_ = hip_decode_init();
}
LameMp3Decoder::~LameMp3Decoder(){
    if(fp_){
        fclose(fp_);
    }
    fp_ = NULL;
    hip_decode_exit(ht_);
}



int LameMp3Decoder::init(const char* filePath)
{
    qInfo()<<"LameMp3Decoder::init"<<filePath;
    fp_ = ::fopen(filePath,"rb+");

    if(fp_ == NULL) return false;

    //做首次解码，分析获取关键信息.
    unsigned char buf[100];
    int     ret;
    size_t  len;
    int     aid_header;
    int     freeformat = 0;

    len = 4;
    if (fread(buf, 1, len, fp_) != len)
        return -1;      /* failed */
    while (buf[0] == 'I' && buf[1] == 'D' && buf[2] == '3') {
        len = 6;
        if (fread(&buf[4], 1, len, fp_) != len)
            return -1;  /* failed */
        len = lenOfId3v2Tag(&buf[6]);
        //直接跳过不分析.
        fskip(fp_, (long) len, SEEK_CUR);
        len = 4;
        if (fread(&buf, 1, len, fp_) != len)
            return -1;  /* failed */
    }
    aid_header = check_aid(buf);
    if (aid_header) {
        if (fread(&buf, 1, 2, fp_) != 2)
            return -1;  /* failed */
        aid_header = (unsigned char) buf[0] + 256 * (unsigned char) buf[1];
        qInfo()<<"Album ID found.  length="<<aid_header;
        /* skip rest of AID, except for 6 bytes we have already read */
        fskip(fp_, aid_header - 6, SEEK_CUR);

        /* read 4 more bytes to set up buffer for MP3 header check */
        if (fread(&buf, 1, len, fp_) != len)
            return -1;  /* failed */
    }
    len = 4;
    while (!is_syncword_mp123(buf)) {
        unsigned int i;
        for (i = 0; i < len - 1; i++)
            buf[i] = buf[i + 1];
        if (fread(buf + len - 1, 1, 1, fp_) != 1)
            return -1;  /* failed */
    }

    if ((buf[2] & 0xf0) == 0) {
        qInfo()<<"Input file is freeformat.";
        freeformat = 1;
    }
    int enc_delay=0, enc_padding=0;
    short int pcm_l[1152], pcm_r[1152];
    mp3data_struct mp3data;
    memset(&mp3data, 0, sizeof(mp3data_struct));
    /* now parse the current buffer looking for MP3 headers.    */
    /* (as of 11/00: mpglib modified so that for the first frame where  */
    /* headers are parsed, no data will be decoded.   */
    /* However, for freeformat, we need to decode an entire frame, */
    /* so mp3data->bitrate will be 0 until we have decoded the first */
    /* frame.  Cannot decode first frame here because we are not */
    /* yet prepared to handle the output. */
    ret = hip_decode1_headersB(ht_, buf, len, pcm_l, pcm_r, &mp3data, &enc_delay, &enc_padding);
    if (-1 == ret)
        return -1;

    /* repeat until we decode a valid mp3 header.  */
    while (!mp3data.header_parsed) {
        len = fread(buf, 1, sizeof(buf), fp_);
        if (len != sizeof(buf))
            return -1;
        ret =
            hip_decode1_headersB(ht_, buf, len, pcm_l, pcm_r, &mp3data, &enc_delay,
                                 &enc_padding);
        if (-1 == ret)
            return -1;
    }

    if (mp3data.bitrate == 0 && !freeformat) {
        qInfo()<<"fail to sync...\n";
        //return lame_decode_initfile(fp_, &mp3data, &enc_delay, &enc_padding);
        return -1;
    }

    file_size = lame_get_file_size(fp_);
    if (mp3data.totalframes > 0) { //MP3的frame，一个frame 1152（framesize=1152)
        /* mpglib found a Xing VBR header and computed nsamp & totalframes */
        PMPSTR pm = (PMPSTR)ht_;
        memcpy(toc,pm->toc,NUMTOCENTRIES);
        is_vbr = true;
    }
    else {
        //根据文件大小和参数估算.
        double const flen = file_size; /* try to figure out num_samples */
        if (flen >= 0 && mp3data.bitrate > 0 ) {
            /* try file size, assume 2 bytes per sample */
            double  totalseconds = (flen * 8.0 / (1000.0 * mp3data.bitrate));
            mp3data.nsamp = totalseconds * mp3data.samplerate;  //num_samples
        }
        is_vbr = false;
    }

    skip_start = 0, skip_end = 0;
    if (enc_delay > -1 || enc_padding > -1) {
        if (enc_delay > -1)
            skip_start = enc_delay + 528 + 1;
        if (enc_padding > -1)
            skip_end = enc_padding - (528 + 1);
    }
    else{
        skip_start = 528 + 1;
    }
    skip_start = skip_start < 0 ? 0 : skip_start;
    skip_end = skip_end < 0 ? 0 : skip_end;
    unsigned long const discard = skip_start + skip_end;
    Q_ASSERT(mp3data.nsamp > discard);
    mp3data.nsamp -= discard;
    channelCount_ = mp3data.stereo;
    sampleRate_ = mp3data.samplerate;
    bitRate_ = mp3data.bitrate;
    duration_ = (float)mp3data.nsamp / mp3data.samplerate * 1000;
    current_ = 0;
    return 0;
}

int LameMp3Decoder::sampleRate(){
    return sampleRate_;
}
int LameMp3Decoder::channelCount(){
    return channelCount_;
}
int LameMp3Decoder::sampleSize(){
    return 16;
}

qint64 LameMp3Decoder::position(){
    //返回播放的位置，单位ms
    return (qint64)current_;
}

void LameMp3Decoder::setPosition(qint64 pos){
    //设置跳转到的位置. 单位ms
    qInfo()<<"before setPosition current"<<current_<<",pos"<<pos;
//    if(pos==0){
//        pos = 0;
//    }
    //if(pos < 0 || pos > duration_) return;
    if(pos < 0) pos = 0;
    if(pos > duration_) pos = duration_;
    if(qAbs((int)current_-pos) < 1000) return;

    //remove_buf((PMPSTR)ht_);
    decode_reset((PMPSTR)ht_);
    //stop restart.
    long lpos = 0;
    if(is_vbr)
    {
        qreal fa, fb, fx;
        qreal percent = (qreal)pos*100/duration_;
        //qDebug()<<"debug percent 34.6172";
        //qreal percent = 34.6172;  /* debug */
        qDebug()<<"percent:"<<percent;
        if( percent <= 0.0f ) {
            fx = 0.0f;
        } else if( percent >= 100.0f ) {
            fx = 256.0f;
        } else {
            int a = (int)percent;
            if ( a == 0 ) {
                fa = 0.0f;
            } else {
                fa = (float)toc[a-1];
            }
            if ( a < 99 ) {
                fb = (float)toc[a];
            } else {
                fb = 256.0f;
            }
            fx = fa + (fb-fa)*(percent-a);
        }

        lpos = (long)((1.0f/256.0f)*fx*(file_size-skip_start-skip_end)) + skip_start;
    }
    else
    {
        lpos = (long)skip_start + pos *(bitRate_/8);
    }
    qInfo()<<"seek with file pos:"<<lpos;
    fskip(fp_, lpos, SEEK_SET);
    current_ = pos;
}

qint64 LameMp3Decoder::duration(){
    //返回总时长.
    return (qint64)duration_;
}


//解码返回两个通道的pcm数据，返回值为sample数。对应pcm_l、pcm_r数组有效长度.
//解码mp3 返回要么是1152 要么是576
int LameMp3Decoder::decode2(short pcm_l[], short pcm_r[])
{
    static int totalframe=0;
    int     ret = 0;
    size_t  len = 0;
    mp3data_struct mp3data;
    unsigned char buf[1024];
    /* file not open yet! */
    if(fp_ == NULL) return -2;
    /* first see if we still have data buffered in the decoder: */
    ret = hip_decode1_headers(ht_, buf, len, pcm_l, pcm_r, &mp3data);
    if (ret != 0){
        current_ += (float)ret / mp3data.samplerate * 1000;
        return ret;
    }
    totalframe += ret;
    /* read until we get a valid output frame */
    for (;;) {
        len = fread(buf, 1, 1024, fp_);
        if (len == 0) {
            /* we are done reading the file, but check for buffered data */
            ret = hip_decode1_headers(ht_, buf, len, pcm_l, pcm_r, &mp3data);
            if (ret <= 0) { return -1; /* done with file */ }
            totalframe += ret;
            break;
        }
        ret = hip_decode1_headers(ht_, buf, len, pcm_l, pcm_r, &mp3data);
        if (ret == -1) { return -1;  }
        totalframe += ret;
        if (ret > 0) break;
    }
    current_ += (float)ret / mp3data.samplerate * 1000;
    return ret;
}
//pcm_l只有两种规格576或1152
int LameMp3Decoder::decode(short pcm_l[])
{
    short pcm_r[1200];  //只有两种规格576或1152
    return decode2(pcm_l,pcm_r);
}
