#include "loginwidget.h"
#include "ui_loginwidget.h"

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);//hide systemtitlebar
//    setAttribute(Qt::WA_TranslucentBackground);

}

LoginWidget::~LoginWidget()
{
    delete ui;
}

void LoginWidget::on_pb_minwindow_clicked()
{
    window()->showMinimized();
}

void LoginWidget::on_pb_closewindow_clicked()
{
    window()->close();
}
void LoginWidget::mouseMoveEvent(QMouseEvent *e)
{
    this->move(e->globalPos() - m_pointStart);
}

void LoginWidget::mousePressEvent(QMouseEvent *e)
{
    m_pointPress = e->globalPos();
    m_pointStart = m_pointPress - this->pos();

}
