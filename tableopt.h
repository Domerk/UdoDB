#ifndef TABLEOPT_H
#define TABLEOPT_H

#include <QDialog>
#include <QTableWidget>
#include <QRegularExpression>
#include <QVector>
#include <QComboBox>
#include <QDate>
#include <QStringList>
#include <QString>

namespace Ui {
class TableOpt;
}

class TableOpt : public QDialog
{
    Q_OBJECT

public:
    explicit TableOpt(QWidget *parent = 0);
    ~TableOpt();

    QTableWidget* letTable();
    QComboBox* letSearchBox();
    void setType(QString);

private slots:
    void on_lineSearch_textChanged(const QString &arg1);
    void search();
    void on_insertButton_clicked();
    void on_deleteButton_clicked();

private:
    Ui::TableOpt *ui;
    QString myType;
    QString baseQuery;
    QString forSearch;
    QVector <int> id;
    bool mainDB;
    QString getDateToTable(QString str);

signals:
    void signalQuery(QTableWidget* tableWidget, QString strQuery, bool mainDB);
    void signalQueries(QStringList qsl, bool mainDB);
};

#endif // TABLEOPT_H
