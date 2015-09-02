#ifndef SETUPSERVERDIALOG_H
#define SETUPSERVERDIALOG_H

#include <QDialog>
#include <QString>
#include <QHostAddress>

namespace Ui {
class setupServerDialog;
}

class setupServerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit setupServerDialog(QWidget *parent = 0);
    ~setupServerDialog();
    quint16 get_port();
    QString get_ip();

private:
    Ui::setupServerDialog *ui;
};

#endif // SETUPSERVERDIALOG_H
