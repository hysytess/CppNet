#ifndef EASYTCPCLIENTWIDGET_H
#define EASYTCPCLIENTWIDGET_H

#include <QWidget>
#include <QString>
#include <QChar>
#include <QDebug>
#include <QtNetwork>
#include <QMouseEvent>
#include <WinSock2.h>

#include "LoginFrame/loginwidget.h"
#include "MSGPROC.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class EasyTcpClientWidget; }
QT_END_NAMESPACE

class EasyTcpClientWidget : public QWidget
{
    Q_OBJECT

public:
    EasyTcpClientWidget(QWidget *parent = nullptr);
    ~EasyTcpClientWidget();

protected:
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);

private slots:
    void on_pb_send_clicked();

    void on_pb_clear_clicked();

    void on_comboBox_currentIndexChanged(int index);

private:
    Ui::EasyTcpClientWidget *ui;
    SOCKET _sock;
    WORD ver;
    WSADATA dat;
    LoginWidget *loginFrame;
    QPoint m_pointStart;
    QPoint m_pointPress;

};
#endif // EASYTCPCLIENTWIDGET_H
