#include "globalapp.h"

globalApp::globalApp(int &argc,char **argv):QApplication(argc,argv)
{

}

void globalApp::setWindowInstance(QWidget *wnd)
{
     widget = wnd;
}

bool globalApp::notify(QObject *obj, QEvent *e)
{

    if(e->type() == QEvent::KeyPress)
    {

        qDebug() << e->type();

    }
    return QApplication::notify(obj,e);
}
