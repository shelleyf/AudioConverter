#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mydevice.h"
#include "keymonitor.h"
#include "capturer.h"
#include "fmod/fmod.h"
#define LATENCY_MS      (50) /* Some devices will require higher latency to avoid glitches */
#define DRIFT_MS        (1)
#define DEVICE_INDEX    (0)
#define ERRORCHECK(_result) if(_result != FMOD_OK){qDebug()<<FMOD_ErrorString(_result);}
#define MAX_DRIVERS  (16)
struct RECORD_STATE
{
    FMOD_SOUND *sound;
    FMOD_CHANNEL *channel;
};
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),playing(false),converting(false),m_token("0")
{
    m_volume=1;
    voice_speed=5;
    voice_tune=5;
    voice_volume=9;
    current_user=UserInformation::instance();
    audioFormat.setSampleRate(8000);
    audioFormat.setChannelCount(2);
    audioFormat.setSampleSize(16);
    audioFormat.setCodec("audio/pcm");
    hostname = QHostInfo::localHostName();
    ui->setupUi(this);
    m_vecBtn.push_back(ui->topBtn1);
    m_vecBtn.push_back(ui->topBtn2);
    m_vecBtn.push_back(ui->topBtn3);
    m_vecBtn.push_back(ui->topBtn4);
    m_vecBtn.push_back(ui->topBtn5);
    m_vecBtn.push_back(ui->topBtn6);
    for (int i = 0; i != m_vecBtn.size(); ++i)
    {
        m_vecBtn[i]->setCheckable(true);
        m_vecBtn[i]->setAutoExclusive(true);
    }
    setWindowTitle("痛快王语音助手");
    setDevice();
    getToken();
    voicetype="0";
    current_input=QAudioDeviceInfo::defaultInputDevice();
    current_output=QAudioDeviceInfo::defaultOutputDevice();
    current_output2=QAudioDeviceInfo::defaultOutputDevice();
    mp3Player = new WcVicunaPlayer();
    startHook();
    connect( KeyCapturer::instance(), &KeyCapturer::getKey, [=](int key){
        if(key==121)on_btn_play_clicked();
    });
//    ui->label_username->setText(current_user->username);
//    qDebug()<<current_user->is_activate;
//    if(current_user->is_activate==0){
//        ui->label_activate_status->setText("未激活");
//    }else{
//        ui->label_activate_status->setText("已激活");
//        ui->label_left->hide();
//        ui->label_left_times->hide();
//    }
//    ui->label_left_times->setText(QString::number(current_user->left_times));
    ui->stackedWidget->setCurrentIndex(1);
    ui->labelSpeed->setText(QString::number(voice_speed));
    ui->labelTune->setText(QString::number(voice_tune));
    ui->labelVolume->setText(QString::number(voice_volume));
    on_topBtn2_clicked();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setCurrentWidget()
{
    for (int i = 0; i != m_vecBtn.size(); ++i)
    {
        if ( m_vecBtn[i]->isChecked() )
        {
            ui->stackedWidget->setCurrentIndex(i);
            this->repaint();
        }
    }
}
void MainWindow::getToken(){
    QString token_url="http://openapi.baidu.com/oauth/2.0/token";
    QString appkey="11gR9mOWUGHcr42rHk97RUaE";
    QString appsecrete="mYVbMIHenXIlAjS8PludiBw63VaL7UUj";

    QNetworkAccessManager* manager = new QNetworkAccessManager;
    QString sign =QString("grant_type=client_credentials&client_id="+appkey+"&client_secret="+appsecrete);
    qDebug()<<sign;
    QByteArray params=sign.toUtf8();
    QNetworkRequest req;
    req.setUrl(QUrl(token_url));
    manager->post(req,params);
    QObject::connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(tokenFinishedSlot(QNetworkReply*)));
}

void MainWindow::tokenFinishedSlot(QNetworkReply *reply){
    if(reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Error:" << reply->errorString();
        return;
    }
    QByteArray buf = reply->readAll();
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(buf, &json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        if(parse_doucment.isObject())
        {
            QJsonObject obj = parse_doucment.object();
            if(obj.contains("access_token"))
            {
                QJsonValue name_value = obj.take("access_token");
                m_token=name_value.toString();
            }
        }
    }
    qDebug() <<"token:"<<m_token;
}

void MainWindow::voiceFinishedSlot(QNetworkReply *reply){
    if(reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Error:" << reply->errorString();
        return;
    }
    m_voice = reply->readAll();
    qDebug()<<"baiduvoice:"<<m_voice.length();

    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)){
        if(ui->comboBox2->currentText()==deviceInfo.deviceName()){
            current_output=deviceInfo;
        }
    }

    if (!current_output.isFormatSupported(audioFormat)) {
        qDebug()<<"default format not supported try to use nearest";
        audioFormat = current_output.nearestFormat(audioFormat);
    }
    //qDebug()<<current_output.deviceName();
    audioOutput = new QAudioOutput(current_output,audioFormat);
    connect(audioOutput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(audioStateChanged(QAudio::State)));
    playdev=new MyDevice();
    playdev->setData(m_voice);
    audioOutput->start(playdev);
}

void MainWindow::audioStateChanged(QAudio::State newState){
    switch (newState) {
        case QAudio::IdleState:
            if(ui->cb_circulate->isChecked()){
                if(playdev){
                    delete playdev;
                }
                playdev=new MyDevice();
                playdev->setData(m_voice);
                audioOutput->start(playdev);
            }else{
                playing=!playing;
                if(!playing){
                    ui->btn_play->setText("播放(F10)");
                    audioOutput->stop();
                    delete audioOutput;
                    if(mp3Player){
                        mp3Player->stop();
                    }
                }
            }
            break;
        case QAudio::StoppedState:
            // Stopped for other reasons
            if (audioOutput->error() != QAudio::NoError) {
                qDebug()<<"audio error";// Error handling
            }
            break;
        default:
            // ... other cases as appropriate
            break;
    }
}

void MainWindow::slotTimeOut()
{

}


void MainWindow::saveFinishedSlot(QNetworkReply *reply){
    if(reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Error:" << reply->errorString();
        return;
    }
    QByteArray tmp = reply->readAll();
    QString exportFileName = QFileDialog::getSaveFileName( this,
                                               ("选择要保存的文件"),
                                              "/",
                                               ("mp3音频文件 (*.mp3);; 所有文件 (*.*);; "));
    QFile f(exportFileName);
    f.open(QIODevice::WriteOnly);
    QDataStream qs(&f);
    qs<<tmp;
    f.close();
}

void MainWindow::setDevice()
{

    ui->comboBox1->addItem(QAudioDeviceInfo::defaultInputDevice().deviceName());
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
        ui->comboBox2->addItem(deviceInfo.deviceName());
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
        ui->comboBox3->addItem(deviceInfo.deviceName());
    ui->comboBox4->addItem(QAudioDeviceInfo::defaultOutputDevice().deviceName());

    ui->cb_voice->addItem("普通女声");
    ui->cb_voice->addItem("普通男声");
    ui->cb_voice->addItem("情感女声");
    ui->cb_voice->addItem("情感男声");
}

void MainWindow::on_btn_template1_clicked(){
    tmp_text=m_text.m_text1;
    ui->te_content->setText(QString(tmp_text.replace("**",ui->le_target->text())));
    current_textTemplate=1;
}
void MainWindow::on_btn_template2_clicked(){
    tmp_text=m_text.m_text2;
    ui->te_content->setText(QString(tmp_text.replace("**",ui->le_target->text())));
    current_textTemplate=2;
}
void MainWindow::on_btn_template3_clicked(){
    tmp_text=m_text.m_text3;
    ui->te_content->setText(QString(tmp_text.replace("**",ui->le_target->text())));
    current_textTemplate=3;
}
void MainWindow::on_btn_template4_clicked(){
    tmp_text=m_text.m_text4;
    ui->te_content->setText(QString(tmp_text.replace("**",ui->le_target->text())));
    current_textTemplate=4;
}
void MainWindow::on_btn_template5_clicked(){
    tmp_text=m_text.m_text5;
    ui->te_content->setText(QString(tmp_text.replace("**",ui->le_target->text())));
    current_textTemplate=5;
}
void MainWindow::on_btn_template6_clicked(){
    tmp_text=m_text.m_text6;
    ui->te_content->setText(QString(tmp_text.replace("**",ui->le_target->text())));
    current_textTemplate=6;
}
void MainWindow::on_btn_setTemplate_clicked(){

}

void MainWindow::on_btn_play_clicked(){
    if(!playing){
        if(!playAuth()){
            QMessageBox::information(this,"错误","免费次数已用完，请激活");
            app->quit();
        }
    }
    if(currentSheetId==1){
        playing=!playing;
        if(playing){
            ui->btn_play->setText("停止(F10)");
            if(ui->cb_background->isChecked()){
                foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)){
                    if(ui->comboBox2->currentText()==deviceInfo.deviceName()){
                        current_output=deviceInfo;
                    }
                }
                //qDebug()<<current_output.deviceName();
                mp3Player->audio_out = current_output;
                mp3Player->setMedia(ui->labelMusicPath->text());
                mp3Player->setVolume(0.5);
                mp3Player->play();
            }
            QString voice_url="http://tsn.baidu.com/text2audio";
            QString sign="lan=zh&ctp=1&cuid=daoshanhuohai444&tok="+m_token+"&tex="+ui->te_content->toPlainText().toUtf8()+"&vol="+QString::number(voice_volume)+"&per="+voicetype+"&spd="+QString::number(voice_speed)+"&pit="+QString::number(voice_tune)+"&aue=4";
            QNetworkAccessManager* manager = new QNetworkAccessManager;
            QByteArray params=sign.toUtf8();
            QNetworkRequest req;
            req.setUrl(QUrl(voice_url));
            qDebug()<<req.url();
            manager->post(req,params);
            QObject::connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(voiceFinishedSlot(QNetworkReply*)));
        }else{
            if(ui->cb_background->isChecked()){
                mp3Player->stop();
            }
            ui->btn_play->setText("播放(F10)");
            audioOutput->stop();
            disconnect(audioOutput);
            delete audioOutput;
        }
    }else if(currentSheetId==0){
        playing=!playing;
        if(playing){
            ui->btn_play->setText("停止(F10)");
            foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)){
                if(ui->comboBox2->currentText()==deviceInfo.deviceName()){
                    current_output=deviceInfo;
                }
            }
                //qDebug()<<current_output.deviceName();
            mp3Player->audio_out = current_output;
            mp3Player->setMedia(ui->labelMusicPath2->text());
            mp3Player->setVolume(m_volume);
            mp3Player->play();
        }else{
            mp3Player->stop();
            ui->btn_play->setText("播放(F10)");
        }
    }else if(currentSheetId==5){
        qDebug()<<"currentsheetid:"+QString::number(currentSheetId);
        playing=!playing;
        if(playing){
            ui->btn_play->setText("停止(F10)");

            RECORD_STATE record[MAX_DRIVERS] = { };
            fmodResult = FMOD_System_Create(&fmodSystem);
            ERRORCHECK(fmodResult);
            fmodResult = FMOD_System_Init(fmodSystem, 100, FMOD_INIT_NORMAL, 0);
            ERRORCHECK(fmodResult);
            fmodResult = FMOD_System_CreateDSPByType(fmodSystem,FMOD_DSP_TYPE_PITCHSHIFT, &dsp1);
            ERRORCHECK(fmodResult);
            //fmodResult = FMOD_System_CreateSound(fmodSystem, "D:/singing.wav", FMOD_LOOP_NORMAL, 0, &sound);
            fmodResult = FMOD_System_GetMasterChannelGroup(fmodSystem,&channelgroup);
            ERRORCHECK(fmodResult);
            //playdevice
            int playDrivers = 0;
            fmodResult = FMOD_System_GetNumDrivers(fmodSystem, &playDrivers);
            ERRORCHECK(fmodResult);
            int ix;
            char name[1000];
            int playRate = 0;
            int playChannels = 0;
            for(ix=0;ix<playDrivers;ix++){
                fmodResult = FMOD_System_GetDriverInfo(fmodSystem,ix, name, 1000, NULL, &playRate, NULL, &playChannels);
                ERRORCHECK(fmodResult);
                QString  str;
                str = QString(QLatin1String(name));
                qDebug()<<str;
                qDebug()<<ix;
            }
            fmodResult = FMOD_System_SetDriver(fmodSystem,ui->comboBox2->currentIndex());
            ERRORCHECK(fmodResult);
            //record device
            int numDrivers = 0;
            fmodResult = FMOD_System_GetRecordNumDrivers(fmodSystem, NULL, &numDrivers);
            ERRORCHECK(fmodResult);
            int nativeRate = 0;
            int nativeChannels = 0;
            fmodResult = FMOD_System_GetRecordDriverInfo(fmodSystem,DEVICE_INDEX, NULL, 0, NULL, &nativeRate, NULL, &nativeChannels, NULL);
            ERRORCHECK(fmodResult);
            unsigned int driftThreshold = (nativeRate * DRIFT_MS) / 1000;       /* The point where we start compensating for drift */
            unsigned int desiredLatency = (nativeRate * LATENCY_MS) / 1000;     /* User specified latency */
            unsigned int adjustedLatency = desiredLatency;                      /* User specified latency adjusted for driver update granularity */
            int actualLatency = desiredLatency;                                 /* Latency measured once playback begins (smoothened for jitter) */
            FMOD_CREATESOUNDEXINFO exinfo = {0};
            exinfo.cbsize           = sizeof(FMOD_CREATESOUNDEXINFO);
            exinfo.numchannels      = nativeChannels;
            exinfo.format           = FMOD_SOUND_FORMAT_PCM16;
            exinfo.defaultfrequency = nativeRate;
            exinfo.length           = nativeRate * sizeof(short) * nativeChannels; /* 1 second buffer, size here doesn't change latency */
            fmodResult = FMOD_System_CreateSound(fmodSystem, 0, FMOD_LOOP_NORMAL | FMOD_OPENUSER, &exinfo, &sound);
            ERRORCHECK(fmodResult);
            fmodResult = FMOD_System_RecordStart(fmodSystem, DEVICE_INDEX, sound, true);
            ERRORCHECK(fmodResult);
            unsigned int soundLength = 0;
            fmodResult = FMOD_Sound_GetLength(sound,&soundLength, FMOD_TIMEUNIT_PCM);
            ERRORCHECK(fmodResult);
            fmodResult = FMOD_System_PlaySound(fmodSystem, sound, 0, true, &channel);
            ERRORCHECK(fmodResult);
            fmodResult = FMOD_DSP_SetParameterFloat(dsp1,FMOD_DSP_PITCHSHIFT_PITCH, cvPitch);
            ERRORCHECK(fmodResult);
            fmodResult = FMOD_Channel_AddDSP(channel,0,dsp1);
            ERRORCHECK(fmodResult);
            fmodResult = FMOD_System_Update(fmodSystem);
            ERRORCHECK(fmodResult);
        }else{
            fmodResult = FMOD_System_RecordStop(fmodSystem,DEVICE_INDEX);
            fmodResult = FMOD_DSP_Release(dsp1);
            fmodResult = FMOD_Sound_Release(sound);
            fmodResult = FMOD_System_Release(fmodSystem);
            ui->btn_play->setText("播放(F10)");
        }
    }else{

    }
}

bool MainWindow::playAuth(){
//    QNetworkAccessManager* manager = new QNetworkAccessManager;
//    QByteArray params;
//    params.append("username="+current_user->username);
//    params.append("&ip="+getIP());
//    params.append("&hostname="+hostname);
//    QNetworkRequest req;
//    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded;charset=UTF-8");
//    req.setUrl(QUrl("http://119.3.209.144/php/playAuth.php"));
//    manager->post(req,params);
//    QObject::connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(playAuthFinishedSlot(QNetworkReply*)));
//    if(current_user->is_activate==0 && current_user->left_times<=0){
//        qDebug()<<current_user->left_times;
//        return false;
//    }
    return true;
}

void MainWindow::on_btn_play2_clicked()
{
    converting=!converting;
    if(converting){
        ui->btn_play2->setText("重置");
        if(currentSheetId==5)fmodResult = FMOD_Channel_SetPaused(channel,0);
        foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)){
            if(ui->comboBox3->currentText()==deviceInfo.deviceName()){
                current_input=deviceInfo;
                qDebug()<<deviceInfo.deviceName();
            }
        }
        if (!current_input.isFormatSupported(audioFormat)) {
            qDebug()<<"default format not supported try to use nearest";
            audioFormat = current_input.nearestFormat(audioFormat);
        }
        audioInput = new QAudioInput(current_input, audioFormat, this);
        inputDevice = audioInput->start();
        current_output2=QAudioDeviceInfo::defaultOutputDevice();
        if (!current_output2.isFormatSupported(audioFormat)) {
            qDebug()<<"default format not supported try to use nearest";
            audioFormat = current_output2.nearestFormat(audioFormat);
        }
        audioOutput2 = new QAudioOutput(current_output2,audioFormat);
        outputDevice=audioOutput2->start();
        connect(inputDevice,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
    }else{
        ui->btn_play2->setText("转换");
        if(currentSheetId==5)fmodResult = FMOD_Channel_SetPaused(channel,1);
        if(audioInput == NULL || audioOutput2 == NULL)
            return;
        audioInput->stop();
        audioOutput2->stop();
    }
}

void MainWindow::onReadyRead(){
    video vp;
    memset(&vp,0,8000);
    vp.lens = inputDevice->read(vp.data,8000);
    //qDebug()<<"input read:"<<vp.lens<<"times"<<times;
    outputDevice->write(vp.data,vp.lens);
}


void MainWindow::on_btn_chooseRecord_clicked(){
    QString importFileName = QFileDialog::getOpenFileName( this,
                                               ("选择要读取的文件"),
                                              "/",
                                               ("mp3音频文件 (*.mp3);; 所有文件 (*.*);; "));
    //ui->labelRecordPath->setText(importFileName);
}
void MainWindow::on_btn_chooseMusic_clicked(){
    QString importFileName = QFileDialog::getOpenFileName( this,
                                               ("选择要读取的文件"),
                                              "/",
                                               ("mp3音频文件 (*.mp3);; 所有文件 (*.*);; "));
    ui->labelMusicPath->setText(importFileName);
    mp3File = new QFile(importFileName);

}

void MainWindow::on_btn_saveRecord_clicked(){
    QString voice_url="http://tsn.baidu.com/text2audio";
    QString sign="lan=zh&ctp=1&cuid=daoshanhuohai444&tok="+m_token+"&tex="+ui->te_content->toPlainText().toUtf8()+"&vol="+voice_volume+"&per="+voicetype+"&spd="+voice_speed+"&pit="+voice_tune+"&aue=3";
    QNetworkAccessManager* manager = new QNetworkAccessManager;
    QByteArray params=sign.toUtf8();
    QNetworkRequest req;
    req.setUrl(QUrl(voice_url));
    qDebug()<<req.url();
    manager->post(req,params);
    QObject::connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(saveFinishedSlot(QNetworkReply*)));
}

void MainWindow::on_comboBox1_currentIndexChanged(int index)
{

}

void MainWindow::on_comboBox2_currentIndexChanged(int index)
{

}

void MainWindow::on_comboBox3_currentIndexChanged(int index)
{

}

void MainWindow::on_comboBox4_currentIndexChanged(int index)
{

}

void MainWindow::on_cb_voice_currentIndexChanged(int index)
{
    qDebug()<<index;
    if(index == 0){
        voicetype="0";
    }
    if(index == 1){
        voicetype="1";
    }
    if(index == 2){
        voicetype="4";
    }
    if(index == 3){
        voicetype="3";
    }

}


void MainWindow::on_btn_vip_clicked()
{
    QMessageBox::information(this,"错误","已激活");
//    if(ui->le_activate->text().length()<6 || ui->le_activate->text().length()>40 || isDigitStr(ui->le_activate->text()) == 1||isDigitStr(ui->le_activate->text()) == 1){
//        QMessageBox::information(this,"错误","格式错误");
//        return;
//    }
//    if(current_user->is_activate==1){
//        QMessageBox::information(this,"错误","已激活");
//    }
//    QNetworkAccessManager* manager = new QNetworkAccessManager;
//    QByteArray params;
//    params.append("username="+current_user->username);
//    params.append("&activate_code="+ui->le_activate->text());
//    QNetworkRequest req;
//    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded;charset=UTF-8");
//    req.setUrl(QUrl("http://119.3.209.144/php/activate.php"));
//    manager->post(req,params);
//    QObject::connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(activateFinishedSlot(QNetworkReply*)));
}

void MainWindow::activateFinishedSlot(QNetworkReply *reply){
    QMessageBox::information(this,"错误","已激活");
//    if(reply->error() != QNetworkReply::NoError)
//    {
//        qDebug() << "Error:" << reply->errorString();
//        QMessageBox::information(this,"错误","网络错误");
//        return;
//    }
//    QByteArray buf = reply->readAll();
//    qDebug()<<QString(buf).toLocal8Bit().data();
//    QJsonDocument jsonDocument = QJsonDocument::fromJson(buf);
//    QJsonObject jsonObject = jsonDocument.object();
//    int tmp=jsonObject.take("success").toInt();
//    if(tmp==1){
//        QMessageBox::information(this,"恭喜","激活成功");
//        ui->label_activate_status->setText("已激活");
//        ui->label_left->setText("");
//        current_user->is_activate=1;
//    }else{
//        QMessageBox::information(this,"错误","激活失败");
//        return;
//    }
}

void MainWindow::playAuthFinishedSlot(QNetworkReply *reply){
    QMessageBox::information(this,"错误","已激活");
//    if(reply->error() != QNetworkReply::NoError)
//    {
//        qDebug() << "Error:" << reply->errorString();
//        QMessageBox::information(this,"错误","网络错误");
//        return;
//    }
//    QByteArray buf = reply->readAll();
//    qDebug()<<QString(buf).toLocal8Bit().data();
//    QJsonDocument jsonDocument = QJsonDocument::fromJson(buf);
//    QJsonObject jsonObject = jsonDocument.object();
//    if(jsonObject.take("success").toInt()==0){
//        QMessageBox::information(this,"错误","服务器验证不通过");
//        app->quit();
//    }
//    if(jsonObject.take("is_activate").toInt()==0 && jsonObject.take("left_times").toInt()<=0){
//        QMessageBox::information(this,"验证错误","免费次数已用完，请激活");
//        app->quit();
//    }
//    if(jsonObject.take("authIP").toInt()==0){
//        QMessageBox::information(this,"错误","用户在另一处登录");
//        app->quit();
//    }
//    QJsonObject jsonObject2 = jsonDocument.object();
//    current_user->is_activate=jsonObject2.take("is_activate").toInt();
//    current_user->left_times=jsonObject2.take("left_times").toInt();
//    ui->label_left_times->setText(QString::number(current_user->left_times));
}


QString MainWindow::getIP()
{
    QString ipAddr;
#if 0
    QList<QHostAddress> AddressList = QNetworkInterface::allAddresses();
    foreach(QHostAddress address, AddressList){
        if(address.protocol() == QAbstractSocket::IPv4Protocol &&
                address != QHostAddress::Null &&
                address != QHostAddress::LocalHost){
            if (address.toString().contains("127.0.")){
                continue;
            }
            ipAddr = address.toString();
            break;
        }
    }
#else
    QHostInfo info = QHostInfo::fromName(hostname);
    info.addresses();//QHostInfo的address函数获取本机ip地址
    //如果存在多条ip地址ipv4和ipv6：
    foreach(QHostAddress address,info.addresses())
    {
        if(address.protocol()==QAbstractSocket::IPv4Protocol){//只取ipv4协议的地址
            qDebug()<<address.toString();
            ipAddr = address.toString();
        }
    }
#endif
    return ipAddr;
}
int MainWindow::isDigitStr(QString src)
{
    QByteArray ba = src.toLatin1();//QString 转换为 char*
     const char *s = ba.data();

    while(*s ){
        if((*s>='0' && *s<='9') ||(*s>='a' && *s<='z' )|| (*s>='A' && *s<='Z')){
            s++;
        }else{
            qDebug()<<*s<<"not text";
            return 1;
        }
    }
    return 0;
}

void MainWindow::on_topBtn1_clicked()
{
    if(playing){
        QMessageBox::information(this,"错误","先停止播放，再切换界面");
        return;
    }
    currentSheetId=0;
    setCurrentWidget();
}

void MainWindow::on_topBtn2_clicked()
{
    if(playing){
        QMessageBox::information(this,"错误","先停止播放，再切换界面");
        return;
    }
    currentSheetId=1;
    setCurrentWidget();
}

void MainWindow::on_topBtn3_clicked()
{
    if(playing){
        QMessageBox::information(this,"错误","先停止播放，再切换界面");
        return;
    }
    currentSheetId=2;
    setCurrentWidget();
}

void MainWindow::on_topBtn4_clicked()
{
    if(playing){
        QMessageBox::information(this,"错误","先停止播放，再切换界面");
        return;
    }
    currentSheetId=3;
    setCurrentWidget();
}

void MainWindow::on_topBtn5_clicked()
{
    if(playing){
        QMessageBox::information(this,"错误","先停止播放，再切换界面");
        return;
    }
    currentSheetId=4;
    setCurrentWidget();
}

void MainWindow::on_topBtn6_clicked()
{
    if(playing){
        QMessageBox::information(this,"错误","先停止播放，再切换界面");
        return;
    }
    currentSheetId=5;
    setCurrentWidget();
}


void MainWindow::on_btn_chooseMusic2_clicked()
{
    QString importFileName = QFileDialog::getOpenFileName( this,
                                               ("选择要读取的文件"),
                                              "/",
                                               ("mp3音频文件 (*.mp3);; 所有文件 (*.*);; "));
    ui->labelMusicPath2->setText(importFileName);
}

void MainWindow::on_settingChange_clicked()
{
    voice_speed=ui->le_set_voice_speed->text().toInt();
    if(voice_speed>15)voice_speed=15;
    if(voice_speed<0)voice_speed=0;
    voice_tune=ui->le_set_voice_tune->text().toInt();
    if(voice_tune>15)voice_tune=15;
    if(voice_tune<0)voice_tune=0;
    voice_volume=ui->le_set_voice_volume->text().toInt();
    if(voice_volume>15)voice_volume=15;
    if(voice_volume<0)voice_volume=0;
    //m_volume=voice_volume/15;
    ui->labelSpeed->setText(QString::number(voice_speed));
    ui->labelTune->setText(QString::number(voice_tune));
    ui->labelVolume->setText(QString::number(voice_volume));
}

void MainWindow::on_settingDefault_clicked()
{
    voice_speed=5;
    voice_tune=5;
    voice_volume=9;
    ui->labelSpeed->setText(QString::number(voice_speed));
    ui->labelTune->setText(QString::number(voice_tune));
    ui->labelVolume->setText(QString::number(voice_volume));
}

void MainWindow::on_btn_setMusic_clicked()
{

    m_volume=ui->le_music_volumn->text().toFloat();
    ui->labelMusicVolum->setText("当前："+QString::number(m_volume));

}


void MainWindow::on_btn_b2g_clicked()
{
    ui->label_cv->setText("男变女");
    changeVoiceMode=1;
    cvPitch=3.0f;
    ui->label_pitch->setText(QString::number(cvPitch));
    fmodResult = FMOD_DSP_SetParameterFloat(dsp1,FMOD_DSP_PITCHSHIFT_PITCH, cvPitch);
    ERRORCHECK(fmodResult);
    fmodResult = FMOD_System_Update(fmodSystem);
    ERRORCHECK(fmodResult);
}

void MainWindow::on_btn_g2b_clicked()
{
    ui->label_cv->setText("女变男");
    changeVoiceMode=2;
    cvPitch=0.5f;
    ui->label_pitch->setText(QString::number(cvPitch));
    fmodResult = FMOD_DSP_SetParameterFloat(dsp1,FMOD_DSP_PITCHSHIFT_PITCH, cvPitch);
    ERRORCHECK(fmodResult);
    fmodResult = FMOD_System_Update(fmodSystem);
    ERRORCHECK(fmodResult);
}

void MainWindow::on_btn_cv_default_clicked()
{
    ui->label_cv->setText("默认");
    changeVoiceMode=0;
    cvPitch=1;
    ui->label_pitch->setText(QString::number(cvPitch));
    fmodResult = FMOD_DSP_SetParameterFloat(dsp1,FMOD_DSP_PITCHSHIFT_PITCH, cvPitch);
    ERRORCHECK(fmodResult);
    fmodResult = FMOD_System_Update(fmodSystem);
    ERRORCHECK(fmodResult);
}

void MainWindow::on_btn_confirm_change_voice_clicked()
{
    cvPitch=ui->cv_le->text().toFloat();
    ui->label_pitch->setText(QString::number(cvPitch));
    fmodResult = FMOD_DSP_SetParameterFloat(dsp1,FMOD_DSP_PITCHSHIFT_PITCH, cvPitch);
    ERRORCHECK(fmodResult);
    fmodResult = FMOD_System_Update(fmodSystem);
    ERRORCHECK(fmodResult);
}
