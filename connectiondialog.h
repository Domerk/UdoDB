#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>
#include <QDataStream>
#include <QMap>
#include <QString>
#include <QPushButton>
#include <QSettings>
#include <QMessageBox>

namespace Ui {
class ConnectionDialog;
}

class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionDialog(QWidget *parent = 0);
    ~ConnectionDialog();


private slots:
    void on_buttonBox_accepted();

private:
    Ui::ConnectionDialog *ui;
    QMap<QString, QString> mainBase;
    QMap<QString, QString> tempBase;

signals:
    void connectReconfig();
};

#endif // CONNECTIONDIALOG_H
