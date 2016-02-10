#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    rowCount = 0;
    columnCount = 0;
    lastSelect = new QString();
    currentTable = new QString();

    // --------------------------- Main ToolBar ----------------------------
    ui->mainToolBar->addAction(tr("Удалить запись"), this, SLOT(deleteThis()));
    ui->mainToolBar->addAction(tr("Перезагрузить таблицу"), this, SLOT(refreshTable()));
    ui->mainToolBar->addAction(tr("Перезагрузить таблицу"), this, SLOT(repeatLastSelect()));
    //ui->mainToolBar->addAction(tr("Расширенный поиск"), this, SLOT(showSearchForm()));


    // ----------------------------- DataBase ------------------------------
    // Временная строчка - указан путь к базе.
    connectDB("D:/Domerk/GUAP/Diplom/KcttDB/kcttDB.sqlite");
}

// ============================================================

MainWindow::~MainWindow()
{
    delete lastSelect;
    delete currentTable;
    delete ui;
}

// ============================================================
// Установка соединения с базой
bool MainWindow::connectDB(QString pathToDB)
{
    myDB = QSqlDatabase::addDatabase("QSQLITE");
    myDB.setDatabaseName(pathToDB);

    QFileInfo checkFile(pathToDB);

    if (checkFile.isFile())
    {
        if (myDB.open())
        {
            ui->lblStatus->setText(tr("Соединение установлено"));
            return true;
        }
        else
        {
            ui->lblStatus->setText(tr("Ошибка соединения: соединение не установлено"));
        }
    }
    else
    {
        ui->lblStatus->setText(tr("Ошибка соединения: отсутсвует файл базы данных"));
    }
    return false;
}

// ============================================================
// Вывод таблицы
void MainWindow::showTable(QSqlQuery query)
{
    drawHeaders(query);
    drawRows(query);
}

// ============================================================
// Отрисовка заголовков таблицы
void MainWindow::drawHeaders(QSqlQuery query)
{
    columnCount = 0;
    QStringList qsl;
    QSqlRecord rec;
    rec = query.record();
    columnCount = rec.count();
    ui->tableWidget->setColumnCount(columnCount);

    for (int i = 0; i<columnCount; i++)
    {
        qsl.append(rec.fieldName(i));
    }

}

// ============================================================
// Отрисовка строк таблицы
void MainWindow::drawRows(QSqlQuery query)
{
    rowCount = 0;
    while (query.next())
        {
            ui->tableWidget->insertRow(rowCount);
            for (int i = 0; i<columnCount; i++)
            {
                ui->tableWidget->setItem(rowCount, i, new QTableWidgetItem(query.value(i).toString()));
            }
            rowCount ++;
        }

    ui->tableWidget->insertRow(rowCount);
}

// ============================================================
// Удаление строки
void MainWindow::deleteThis()
{

}

// ============================================================
// Обновление строки
void MainWindow::updateThis()
{

}

// ============================================================
// Добавление строки
void MainWindow::insertThis()
{

}

// ============================================================
// Обновление таблицы (Select *)
void MainWindow::refreshTable()
{

}

// ============================================================
// Повторение последнего запроса (Select)
void MainWindow::repeatLastSelect()
{
    QSqlQuery query;
    query.exec(*lastSelect);
    drawRows(query);
}

// ============================================================
