#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QPushButton>

namespace Ui {
class SearchDialog;
}

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(QWidget *parent = 0);
    ~SearchDialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::SearchDialog *ui;


signals:
    void signalQuery(QString);
};

#endif // SEARCHDIALOG_H
