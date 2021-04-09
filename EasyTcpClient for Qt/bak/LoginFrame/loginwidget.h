#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QMouseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class LoginWidget; }
QT_END_NAMESPACE

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

protected:
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);

private slots:
    void on_pb_minwindow_clicked();

    void on_pb_closewindow_clicked();

private:
    Ui::LoginWidget *ui;
    QPoint m_pointStart;
    QPoint m_pointPress;

};
#endif // LOGINWIDGET_H
