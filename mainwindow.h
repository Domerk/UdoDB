#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QtSql>
#include <QFileInfo>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QMap>

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
    Ui::MainWindow *ui;
    QSqlDatabase myDB;

    int rowCount;
    int columnCount;
    QString *lastSelect;
    QString *currentTable;

    // Шаблонные запросы на вывод таблиц
    // Вероятно, стоит вынести отсюда в отдельный файл

    QString queryStud;
    QString queryTeach;
    QString queryAllians;

    bool connectDB(QString pathToDB);
    void showTable(QString table);
    void drawHeaders(QSqlQuery query);
    void drawRows(QSqlQuery query);
    void showMoreInfo(int row);
    void clearMoreInfoForm();

private slots:
    void clearFormForAdd();
    void deleteThis();
    void updateThis();
    void insertThis();
    void refreshTable();
    void repeatLastSelect();
    void headerClicked(int);

    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);


    void on_tableWidget_cellClicked(int row, int column);
    void on_saveButton_clicked();
};

#endif // MAINWINDOW_H
