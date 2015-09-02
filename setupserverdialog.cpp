#include "setupserverdialog.h"
#include "ui_setupserverdialog.h"

setupServerDialog::setupServerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::setupServerDialog)
{
    ui->setupUi(this);
    ui->ServerIPlineEdit->setText(QString("127.0.0.1"));
    ui->PortlineEdit_2->setText(QString("2333"));
}

setupServerDialog::~setupServerDialog()
{
    delete ui;
}

quint16 setupServerDialog::get_port(){
    return ui->PortlineEdit_2->text().toInt();
}

QString setupServerDialog::get_ip(){
    return ui->ServerIPlineEdit->text();
}
