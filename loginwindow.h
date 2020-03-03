#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QApplication>
#include <QHostAddress>
#include <QHostInfo>
#include <QMessageBox>
#include "userinformation.h"
#include "mainwindow.h"
namespace Ui {
class LoginWindow;
}
class LoginWindow : public QMainWindow
{
    Q_OBJECT
public:
    LoginWindow();
    ~LoginWindow();
    UserInformation *current_user;
    QApplication *app;
    QString hostname;
    MainWindow *mw;
public slots:
    void on_btn_exit2_clicked();
    void on_btn_exit1_clicked();
    void on_btn_register_clicked();
    void on_btn_login_clicked();
    void authFinishedSlot(QNetworkReply *reply);
    void registerFinishedSlot(QNetworkReply *reply);
private:
    Ui::LoginWindow *ui;
    QString getUser();
    QString getIP();
    int isDigitStr(QString src);
};

#endif // LOGINWINDOW_H
