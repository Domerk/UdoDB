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
               Home = 0,
               Repeat,
               New,
               Delete
           };

    enum SearchToolButton : int {
               Start = 0,
               Text,
               Columns
           };

    Ui::MainWindow *ui;
    QSqlDatabase myDB;

    ConnectionDialog *connectDialog;

    QString *lastSelect;
    QString *currentTable;

    // Шаблонные запросы на вывод таблиц
    // Вероятно, стоит вынести отсюда в отдельный файл

    QString queryStud;
    QString queryTeach;
    QString queryAllians;
    QString queryDirection;
    QString queryGroup;

    QVector<bool> studTableMask;
    QVector<bool> teachTableMask;
    QVector<bool> alliansTableMask;

    QVector<bool> currentMask;

    QRegularExpression *names;

    QComboBox *searchBox;
    QLineEdit *searchEdit;

    bool connectDB(QString nameDB);
    void showTable(QString table);
    void drawHeaders(QSqlQuery query, QTableWidget *table, bool isMainTable);
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

public slots:
    void connectReconfigSlot();
    void querySlot(QString);
};

#endif // MAINWINDOW_H
