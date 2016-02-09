#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    ui->toolBar->addAction(tr("Удалить запись"), this, SLOT(deleteThis()));
    ui->toolBar->addAction(tr("Перезагрузить таблицу"), this, SLOT(refreshTable()));
    ui->toolBar->addAction(tr("Перезагрузить таблицу"), this, SLOT(repeatLastSelect()));
    //ui->toolBar->addAction(tr("Расширенный поиск"), this, SLOT(showSearchForm()));

    // Временная строчка - указан путь к базе.
    connectDB("D:/Domerk/GUAP/Diplom/KcttDB/kcttDB.sqlite");
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::connectDB(QString pathToDB)
{
    myDB = QSqlDatabase::addDatabase("QSQLITE");
    myDB.setDatabaseName(pathToDB);

    QFileInfo checkFile(pathToDB);

    if (checkFile.isFile())
    {
        if (myDB.open())
        {
            ui->lblStatus->setText(tr("Соединение установлено"));
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
}


void MainWindow::showTable()
{

}

void MainWindow::drawHeaders()
{

}

void MainWindow::drawRows()
{

}


void MainWindow::deleteThis()
{

}

void MainWindow::updateThis()
{

}

void MainWindow::insertThis()
{

}

void MainWindow::refreshTable()
{

}

void MainWindow::repeatLastSelect()
{

}
