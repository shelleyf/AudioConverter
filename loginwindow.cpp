#include "loginwindow.h"
#include "ui_loginwindow.h"

LoginWindow::LoginWindow():
    ui(new Ui::LoginWindow)
{
    setWindowFlags(Qt::FramelessWindowHint);
    ui->setupUi(this);
    current_user=UserInformation::instance();
    hostname = QHostInfo::localHostName();
}
LoginWindow::~LoginWindow()
{
    delete ui;
}
//void LoginWindow::on_btn_exit2_clicked()
//{
//    app->quit();
//}
//void LoginWindow::on_btn_exit1_clicked()
//{
//    app->quit();
//}
//void LoginWindow::on_btn_register_clicked()
//{
//    this->hide();
//    if(ui->le_register_2->text()!=ui->le_register_3->text()){
//        QMessageBox::information(this,"错误","两次密码输入不一致");
//        this->show();
//        return;
//    }
//    if(ui->le_register_1->text().length()<6 || ui->le_register_1->text().length()>20||isDigitStr(ui->le_register_1->text()) == 1||isDigitStr(ui->le_register_2->text()) == 1){
//        QMessageBox::information(this,"错误","用户名/密码只能使用六位以上字母或数字");
//        this->show();
//        return;
//    }
//    if(ui->le_register_2->text().length()<6 || ui->le_register_2->text().length()>20||isDigitStr(ui->le_register_2->text()) == 1||isDigitStr(ui->le_register_2->text()) == 1){
//        QMessageBox::information(this,"错误","用户名/密码只能使用六位以上字母或数字");
//        this->show();
//        return;
//    }
//    if(ui->le_register_3->text().length()<6 || ui->le_register_3->text().length()>20||isDigitStr(ui->le_register_3->text()) == 1||isDigitStr(ui->le_register_3->text()) == 1){
//        QMessageBox::information(this,"错误","用户名/密码只能使用六位以上字母或数字");
//        this->show();
//        return;
//    }
//    if(ui->le_register_4->text().length()<6 || ui->le_register_4->text().length()>20||isDigitStr(ui->le_register_4->text()) == 1||isDigitStr(ui->le_register_4->text()) == 1){
//        QMessageBox::information(this,"错误","用户名/密码只能使用六位以上字母或数字");
//        this->show();
//        return;
//    }
//    QNetworkAccessManager* manager = new QNetworkAccessManager;
//    QByteArray params;
//    params.append("username="+ui->le_register_1->text());
//    params.append("&password="+ui->le_register_2->text());
//    params.append("&ip="+getIP());
//    params.append("&hostname="+hostname);
//    params.append("&qq="+ui->le_register_4->text());

//    QNetworkRequest req;
//    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded;charset=UTF-8");
//    req.setUrl(QUrl("http://119.3.209.144/php/register.php"));
//    manager->post(req,params);
//    QObject::connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(registerFinishedSlot(QNetworkReply*)));

//}
//void LoginWindow::registerFinishedSlot(QNetworkReply *reply){
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
//    if(jsonObject.take("blacklist").toInt()){
//        QMessageBox::information(this,"错误","每日只能注册一个账号");
//    }else{
//        if(tmp==1){
//            QMessageBox::information(this,"恭喜","注册成功");
//        }else{
//            QMessageBox::information(this,"错误","用户名已存在");
//        }
//    }
//    this->show();
//}
void LoginWindow::on_btn_login_clicked()
{
    this->hide();
    mw = new MainWindow();
    mw->app=app;
    mw->show();
//    if(ui->le_login_1->text().length()<6 || ui->le_login_1->text().length()>20 || isDigitStr(ui->le_login_1->text()) == 1||isDigitStr(ui->le_login_2->text()) == 1){
//        QMessageBox::information(this,"错误","格式错误");
//        this->show();
//        return;
//    }
//    QNetworkAccessManager* manager = new QNetworkAccessManager;
//    QByteArray params;
//    params.append("username="+ui->le_login_1->text());
//    params.append("&password="+ui->le_login_2->text());
//    params.append("&ip="+getIP());
//    params.append("&hostname="+hostname);
//    current_user->username=ui->le_login_1->text();
//    QNetworkRequest req;
//    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded;charset=UTF-8");
//    req.setUrl(QUrl("http://119.3.209.144/php/login.php"));
//    manager->post(req,params);
//    QObject::connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(authFinishedSlot(QNetworkReply*)));
}
//void LoginWindow::authFinishedSlot(QNetworkReply *reply){
//    if(reply->error() != QNetworkReply::NoError)
//    {
//        qDebug() << "Error:" << reply->errorString();
//        QMessageBox::information(this,"错误","网络错误");
//        this->show();
//        return;
//    }
//    QByteArray buf = reply->readAll();
//    qDebug()<<QString(buf).toLocal8Bit().data();
//    QJsonDocument jsonDocument = QJsonDocument::fromJson(buf);
//    QJsonObject jsonObject = jsonDocument.object();
//    int tmp=jsonObject.take("auth").toInt();
//    current_user->is_activate=jsonObject.take("activate").toInt();
//    current_user->left_times=jsonObject.take("left_times").toInt();
//    if(tmp==1){
//        mw = new MainWindow();
//        mw->app=app;
//        mw->show();
//    }else{
//        QMessageBox::information(this,"错误","用户名/密码错误");
//        this->show();
//        return;
//    }
//}

QString LoginWindow::getIP()
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
int LoginWindow::isDigitStr(QString src)
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
