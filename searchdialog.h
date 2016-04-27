#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLineEdit>

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

    QString *columns;
    QString *from;
    QString *where;
    QString *query;

    QStringList *fromStud;
    QStringList *fromTeach;
    QStringList *fromAss;
    QStringList *fromGroup;

    QStringList *whereStud;
    QStringList *whereTeach;
    QStringList *whereAss;
    QStringList *whereGroup;

    void getDataFromLineEdit(QStringList *slist, QLineEdit *ledit, QString text);


signals:
    void signalQuery(QString);
};

#endif // SEARCHDIALOG_H
