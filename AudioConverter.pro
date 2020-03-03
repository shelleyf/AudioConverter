#-------------------------------------------------
#
# Project created by QtCreator 2019-05-05T14:36:13
#
#-------------------------------------------------

QT       += core gui multimedia network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
DEFINES += HAVE_MPGLIB HAVE_CONFIG_H #audio WcVicunalPlayer depend
INCLUDEPATH += ./audio/libmp3lame ./audio/mpglib ./audio
TARGET = AudioConverter
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    audio/LameMp3Decoder.cpp \
    audio/WcVicunaPlayer.cpp \
    capturer.cpp \
    globalapp.cpp \
    keymonitor.cpp \
    main.cpp \
    mainwindow.cpp \
    mydevice.cpp \
    mytextcontent.cpp \
    audio/libmp3lame/vector/xmm_quantize_sub.c \
    audio/libmp3lame/bitstream.c \
    audio/libmp3lame/encoder.c \
    audio/libmp3lame/fft.c \
    audio/libmp3lame/gain_analysis.c \
    audio/libmp3lame/id3tag.c \
    audio/libmp3lame/lame.c \
    audio/libmp3lame/mpglib_interface.c \
    audio/libmp3lame/newmdct.c \
    audio/libmp3lame/presets.c \
    audio/libmp3lame/psymodel.c \
    audio/libmp3lame/quantize.c \
    audio/libmp3lame/quantize_pvt.c \
    audio/libmp3lame/reservoir.c \
    audio/libmp3lame/set_get.c \
    audio/libmp3lame/tables.c \
    audio/libmp3lame/takehiro.c \
    audio/libmp3lame/util.c \
    audio/libmp3lame/vbrquantize.c \
    audio/libmp3lame/VbrTag.c \
    audio/libmp3lame/version.c \
    audio/mpglib/common.c \
    audio/mpglib/dct64_i386.c \
    audio/mpglib/decode_i386.c \
    audio/mpglib/interface.c \
    audio/mpglib/layer1.c \
    audio/mpglib/layer2.c \
    audio/mpglib/layer3.c \
    audio/mpglib/tabinit.c \
    audio/sonic.c \
    loginwindow.cpp \
    userinformation.cpp

HEADERS += \
    fmod/fmod.h \
    fmod/fmod.hpp \
    fmod/fmod_codec.h \
    fmod/fmod_common.h \
    fmod/fmod_dsp.h \
    fmod/fmod_dsp_effects.h \
    fmod/fmod_errors.h \
    fmod/fmod_output.h \
    mainwindow.h \
    mydevice.h \
    globalapp.h \
    capturer.h \
    keymonitor.h \
    mytextcontent.h \
    audio/libmp3lame/i386/nasm.h \
    audio/libmp3lame/vector/lame_intrin.h \
    audio/libmp3lame/bitstream.h \
    audio/libmp3lame/encoder.h \
    audio/libmp3lame/fft.h \
    audio/libmp3lame/gain_analysis.h \
    audio/libmp3lame/id3tag.h \
    audio/libmp3lame/l3side.h \
    audio/libmp3lame/lame-analysis.h \
    audio/libmp3lame/lame.rc \
    audio/libmp3lame/lame_global_flags.h \
    audio/libmp3lame/lameerror.h \
    audio/libmp3lame/machine.h \
    audio/libmp3lame/newmdct.h \
    audio/libmp3lame/psymodel.h \
    audio/libmp3lame/quantize.h \
    audio/libmp3lame/quantize_pvt.h \
    audio/libmp3lame/reservoir.h \
    audio/libmp3lame/set_get.h \
    audio/libmp3lame/tables.h \
    audio/libmp3lame/util.h \
    audio/libmp3lame/vbrquantize.h \
    audio/libmp3lame/VbrTag.h \
    audio/libmp3lame/version.h \
    audio/mpglib/common.h \
    audio/mpglib/dct64_i386.h \
    audio/mpglib/decode_i386.h \
    audio/mpglib/huffman.h \
    audio/mpglib/interface.h \
    audio/mpglib/l2tables.h \
    audio/mpglib/layer1.h \
    audio/mpglib/layer2.h \
    audio/mpglib/layer3.h \
    audio/mpglib/mpg123.h \
    audio/mpglib/mpglib.h \
    audio/mpglib/tabinit.h \
    audio/config.h \
    audio/lame.h \
    audio/LameMp3Decoder.h \
    audio/sonic.h \
    audio/WcVicunaPlayer.h \
    loginwindow.h \
    userinformation.h

FORMS += \
        mainwindow.ui \
    loginwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

SUBDIRS += \
    AudioConverter.pro

DISTFILES += \
    audio/libmp3lame/depcomp \
    audio/mpglib/depcomp \
    audio/libmp3lame/logoe.ico \
    audio/libmp3lame/i386/choose_table.nas \
    audio/libmp3lame/i386/cpu_feat.nas \
    audio/libmp3lame/i386/fft.nas \
    audio/libmp3lame/i386/fft3dn.nas \
    audio/libmp3lame/i386/fftfpu.nas \
    audio/libmp3lame/i386/fftsse.nas \
    audio/libmp3lame/i386/ffttbl.nas \
    audio/libmp3lame/i386/scalar.nas \
    audio/mpglib/AUTHORS \
    audio/mpglib/README \ \
    fmod/fmod.cs \
    fmod/fmod_dsp.cs \
    fmod/fmod_errors.cs

LIBS += -L"D:\FMOD SoundSystem\FMOD Studio API Windows\api\core\lib\x86" -lfmod

RESOURCES += \
    res.qrc
