#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAudioDeviceInfo>
#include <QFileDialog>
#include <QFile>
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHostAddress>
#include <QHostInfo>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QDebug>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QAudioInput>
#include <QScopedPointer>
#include <QTimer>
#include "mydevice.h"
#include "mytextcontent.h"
#include "audio/WcVicunaPlayer.h"
#include "userinformation.h"
#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    UserInformation *current_user;
    bool playAuth();
    int left_times;
    QApplication *app;
    QVector <QPushButton* > m_vecBtn;
    int currentSheetId=0;
    void setCurrentWidget();

public slots:
    void on_btn_template1_clicked();
    void on_btn_template2_clicked();
    void on_btn_template3_clicked();
    void on_btn_template4_clicked();
    void on_btn_template5_clicked();
    void on_btn_template6_clicked();
    void on_btn_setTemplate_clicked();
    void on_btn_play_clicked();
    void on_btn_chooseRecord_clicked();
    void on_btn_chooseMusic_clicked();
    void on_btn_saveRecord_clicked();
    void tokenFinishedSlot(QNetworkReply *reply);
    void voiceFinishedSlot(QNetworkReply *reply);
    void activateFinishedSlot(QNetworkReply *reply);
    void playAuthFinishedSlot(QNetworkReply *reply);
    void saveFinishedSlot(QNetworkReply *reply);
    void slotTimeOut();
    void onReadyRead();
    void audioStateChanged(QAudio::State state);


private slots:
    void on_comboBox1_currentIndexChanged(int index);
    void on_comboBox2_currentIndexChanged(int index);
    void on_comboBox3_currentIndexChanged(int index);
    void on_comboBox4_currentIndexChanged(int index);
    void on_cb_voice_currentIndexChanged(int index);
    void on_btn_play2_clicked();
    void on_btn_vip_clicked();
    void on_topBtn1_clicked();
    void on_topBtn2_clicked();
    void on_topBtn3_clicked();
    void on_topBtn4_clicked();
    void on_topBtn5_clicked();
    void on_btn_chooseMusic2_clicked();
    void on_settingChange_clicked();
    void on_settingDefault_clicked();
    void on_btn_setMusic_clicked();
    void on_topBtn6_clicked();
    void on_btn_b2g_clicked();
    void on_btn_g2b_clicked();
    void on_btn_cv_default_clicked();

    void on_btn_confirm_change_voice_clicked();

private:
    Ui::MainWindow *ui;
    QString hostname;
    //audio
    qreal m_volume;
    int voice_speed;
    int voice_tune;
    int voice_volume;
    void setDevice();
    QAudioDeviceInfo m_deviceInfo;
    QAudioFormat audioFormat;
    bool playing;
    bool converting;
    void getToken();
    QString m_token;
    QString voicetype;
    QByteArray m_voice;
    QAudioOutput *audioOutput;
    QAudioDeviceInfo current_output;
    MyDevice *playdev;
    QAudioInput *audioInput;
    QAudioDeviceInfo current_input;
    QAudioOutput *audioOutput2;
    QAudioDeviceInfo current_output2;
    QIODevice *inputDevice;
    QIODevice *outputDevice;
    struct video{
        char data[8000];
        int lens;
    };
    //music
    QFile *mp3File;
    WcVicunaPlayer *mp3Player;

    //text
    MyTextContent m_text;
    int current_textTemplate;
    QString tmp_text;

    //
    QString getIP();
    int isDigitStr(QString src);

    //fmod
    FMOD_RESULT fmodResult;
    FMOD_SYSTEM *fmodSystem;
    FMOD_SOUND  *sound;
    FMOD_CHANNEL *channel;
    FMOD_CHANNELGROUP *channelgroup;
    FMOD_DSP *dsp1,*dsp2;
    bool dspEnabled = false;
    int changeVoiceMode=0;
    float cvPitch=1;
    unsigned int samplesRecorded = 0;
    unsigned int samplesPlayed = 0;
};

#endif // MAINWINDOW_H
