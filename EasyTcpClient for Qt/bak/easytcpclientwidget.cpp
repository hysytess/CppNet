#include "easytcpclientwidget.h"
#include "ui_easytcpclientwidget.h"

EasyTcpClientWidget::EasyTcpClientWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EasyTcpClientWidget)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);//hide systemtitlebar
    window()->setWindowTitle(QString("EasyTcpClient v1.2"));
    ver = MAKEWORD(2, 2);
    WSAStartup(ver, &dat);
    _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == _sock)
    {
        qDebug() << "Build socket fail!" << endl;
    }
    else
        qDebug() << "Building socket success." << endl;

    sockaddr_in _sin{};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(233);
    _sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    int ret = ::connect(_sock, (sockaddr *)&_sin, sizeof(sockaddr_in));
    if (ret == SOCKET_ERROR)
    {
        qDebug() << "Connect to server faild!" << endl;
    }
    else
        qDebug() << "Connect to server sucess." << endl;
    loginFrame = new LoginWidget();
    loginFrame->show();
    window()->hide();
}

EasyTcpClientWidget::~EasyTcpClientWidget()
{
    closesocket(_sock);
    WSACleanup();
    delete ui;
}

void EasyTcpClientWidget::on_pb_send_clicked()
{
    QString QSmsg = ui->textEdit->toPlainText();
    QString str;
    QByteArray QBymsg = QSmsg.toLatin1();

    Message info;
    strcpy(info.msg,QBymsg.data());
    ::send(_sock,(char*)&info,sizeof(Message),0);
    ::recv(_sock,(char*)&info,sizeof(Message),0);

    str+=QString("<font style='font-size:16px;color:blue;'>%1</font>").arg(QSmsg);
    ui->textBrowser->append(str);

    QSmsg.clear();
    QSmsg+=QString("<font style='font-size:16px;'>Server: %1</font>").arg(info.msg);
    ui->textBrowser->append(QSmsg);

    ui->textEdit->clear();
    QSmsg.clear();

    //    ui->textBrowser->setAlignment(Qt::AlignLeft);// 使文本居左对其
    //    ui->textBrowser->setAlignment(Qt::AlignRight);// 使文本居右对其
}

void EasyTcpClientWidget::on_pb_clear_clicked()
{
    ui->textEdit->clear();
}

void EasyTcpClientWidget::on_comboBox_currentIndexChanged(int index)
{
    switch (index) {
    case 0:{
        Loginout logout;
        strcpy(logout.UserName,"User123");
        ::send(_sock,(char*)&logout,sizeof(Loginout),0);
        LoginoutResult logoutret;
        ::recv(_sock,(char*)&logoutret,sizeof(LoginoutResult),0);
    }
    break;
    case 1:{
        closesocket(_sock);
        WSACleanup();
        window()->close();
    }
    break;
    }
}

void EasyTcpClientWidget::mouseMoveEvent(QMouseEvent *e)
{
    this->move(e->globalPos() - m_pointStart);
}

void EasyTcpClientWidget::mousePressEvent(QMouseEvent *e)
{
    m_pointPress = e->globalPos();
    m_pointStart = m_pointPress - this->pos();

}
