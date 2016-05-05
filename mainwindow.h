#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QtSql>
#include <ActiveQt/QAxObject>
#include <ActiveQt/QAxBase>
#include <QFileInfo>
#include <QFileDialog>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QRegularExpression>
#include <QMap>

#include "searchdialog.h"
#include "connectiondialog.h"
#include "tableopt.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    enum MainToolButton : int {
               New = 0,
               Delete,
               ImportFromTempDB,
               Show,
               Search
           };

    enum SearchToolButton : int {
               Start = 0,
               Text,
               Columns
           };

    struct Info
    {
        QString query;
        int index;
        QVector<bool> mask;
    };

    Ui::MainWindow *ui;
    QSqlDatabase myDB;
    QSqlDatabase tempDB;

    TableOpt *tempDbDialog;
    ConnectionDialog *connectDialog;

    QMap<QString, Info> infoMap;

    QString *lastSelect;
    QString *currentTable;   
    QVector<bool> currentMask;

    QRegularExpression *names;

    QComboBox *searchBox;
    QLineEdit *searchEdit;

    bool connectDB(QString nameDB);
    void showTable(QString table);
    void drawHeaders(QSqlQuery query, QTableWidget *table, bool forSearch, QComboBox *serchCBox);
    void drawRows(QSqlQuery query, QTableWidget *table, bool available);
    void showMoreInfo(int row);
    void hideColumnsFromMask(QVector<bool> mask);
    void drawTree();

private slots:
    void clearMoreInfoForm();
    void deleteThis();
    void refreshTable();
    void repeatLastSelect();
    void rowClicked(int);
    void changeTableMask();
    void globalSearch();
    void simpleSearch();

    void setSearchActive();

    void exportInExel();

    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void on_tableWidget_cellClicked(int row, int column);
    void on_saveButton_clicked();

    void on_addStudInGroup_clicked();
    void on_addAssForGroup_clicked();
    void on_addTeachForGroup_clicked();

    void on_addDirectInAl_clicked();

    void showTempTable();

public slots:
    void connectReconfigSlot();
    void querySlot(QString);
};

#endif // MAINWINDOW_H
