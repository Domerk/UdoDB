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
#include <QSpinBox>

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
               ImportFromTempDB,
               Delete,
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
        int index = -1;
        QVector<bool> mask;
    };

    Ui::MainWindow *ui;
    QSqlDatabase myDB;
    QSqlDatabase tempDB;

    TableOpt *dbDialog;
    ConnectionDialog *connectDialog;

    QMap<QString, Info> infoMap;

    QString *lastSelect;
    QString *currentTable;   
    QVector<bool> currentMask;

    QRegularExpression *names;

    QComboBox *searchBox;
    QLineEdit *searchEdit;

    QStringList* bases;

    bool connectDB();
    void showTable(QString table);
    void drawHeaders(QSqlQuery query, QTableWidget *table, bool forSearch, QComboBox *serchCBox);
    void drawRows(QSqlQuery query, QTableWidget *table, bool available);
    void showMoreInfo(int row);
    void hideColumnsFromMask(QVector<bool> mask);
    void drawTree();

    void getDateToTable(QString str, QComboBox* d, QComboBox* m, QSpinBox* y);
    void getDateToForm(QString* str, QComboBox* d, QComboBox* m, QSpinBox* y);

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

    void on_removeStudToGroup_clicked();

    void showQtInfo();
    void showLicense();
    void showProgramInfo();

public slots:
    void connectReconfigSlot();
    void querySlot(QString);
    void querySlot(QTableWidget* tableWidget, QString strQuery, bool mainDB);
    void queriesSlot(QStringList qsl, bool mainDB);
};

#endif // MAINWINDOW_H
