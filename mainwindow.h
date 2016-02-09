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

    void connectDB(QString pathToDB);
    void showTable();
    void drawHeaders();
    void drawRows();

private slots:
    void deleteThis();
    void updateThis();
    void insertThis();
    void refreshTable();
    void repeatLastSelect();



};

#endif // MAINWINDOW_H
