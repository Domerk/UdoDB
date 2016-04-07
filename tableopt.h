#ifndef TABLEOPT_H
#define TABLEOPT_H

#include <QDialog>
#include <QTableWidget>
#include <QRegularExpression>

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
    void setType(QString);

private slots:
    void on_lineSearch_textChanged(const QString &arg1);
    void search();

private:
    Ui::TableOpt *ui;
    QString myType;
};

#endif // TABLEOPT_H
