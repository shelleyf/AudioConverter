#ifndef USERINFORMATION_H
#define USERINFORMATION_H

#include <QString>
#include <QObject>
#include <QMutex>
#include <QMutexLocker>

class UserInformation
{
public:
    static UserInformation* instance();
    QString username;
    QString password;
    int is_activate;
    int left_times;

    QString getPassword();
    void setPassword(QString content);

private:
    explicit UserInformation();
    static QAtomicPointer<UserInformation> _instance;
    static QMutex _mutex;
};

#endif // USERINFORMATION_H
