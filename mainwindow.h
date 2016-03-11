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
               Columns = 0,
               Text,
               Start
           };

    Ui::MainWindow *ui;
    QSqlDatabase myDB;

    ConnectionDialog *connectDialog;

    int rowCount;
    int columnCount;
    QString *lastSelect;
    QString *currentTable;

    // Шаблонные запросы на вывод таблиц
    // Вероятно, стоит вынести отсюда в отдельный файл

    QString queryStud;
    QString queryTeach;
    QString queryAllians;

    QVector<bool> studTableMask;
    QVector<bool> teachTableMask;
    QVector<bool> alliansTableMask;

    QVector<bool> currentMask;

    QRegularExpression *names;

    QComboBox *searchBox;
    QLineEdit *searchEdit;

    bool connectDB(QString nameDB);
    void showTable(QString table);
    void drawHeaders(QSqlQuery query);
    void drawRows(QSqlQuery query);
    void showMoreInfo(int row);
    void clearMoreInfoForm();
    void hideColumnsFromMask(QVector<bool> mask);
    void drawTree();

private slots:
    void clearFormForAdd();
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

public slots:
    void connectReconfigSlot();
};

#endif // MAINWINDOW_H
