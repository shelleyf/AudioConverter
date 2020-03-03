#include "userinformation.h"

QAtomicPointer<UserInformation> UserInformation::_instance = 0;
QMutex UserInformation::_mutex;

UserInformation::UserInformation()
{
    username="";
}

void UserInformation::setPassword(QString content){
    password = content;
}

UserInformation * UserInformation::instance()
{
#ifndef Q_ATOMIC_POINTER_TEST_AND_SET_IS_ALWAYS_NATIVE
    if(!QAtomicPointer::isTestAndSetNative())//运行时检测
        qDebug() << "Error: TestAndSetNative not supported!";
#endif
    //使用双重检测。
    /*! testAndSetOrders操作保证在原子操作前和后的的内存访问
     * 不会被重新排序。
     */
    if(_instance.testAndSetOrdered(0, 0))//第一次检测
    {
        QMutexLocker locker(&_mutex);//加互斥锁。

        _instance.testAndSetOrdered(0, new UserInformation);//第二次检测。
    }
    return _instance;
}

QString UserInformation::getPassword(){
    return password;
}

