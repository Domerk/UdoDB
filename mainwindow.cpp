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


    // ------------------ Запросы на вывод основных таблиц -----------------

   /* queryStud.append("SELECT 'ID', 'Фамилия', 'Имя', 'Отчество', 'Тип документа', 'Номер документа', 'Пол', 'Год рождения', ");
    queryStud.append("'Район школы', 'Школа', 'Класс', 'Родители', 'Домашний адрес', 'Телефон', 'e-mail', 'Дата заявления', 'Форма обучения', ");
    queryStud.append("'Когда выбыл', 'С ослабленным здоровьем', 'Сирота', 'Инвалид', 'На учёте в милиции', 'Многодетная семья', ");
    queryStud.append("'Неполная семья', 'Малообеспеченная семья', 'Мигранты', 'Примечания' FROM Учащиеся;");
*/

    queryStud.append("SELECT * FROM Учащиеся");

    queryTeach.append("SELECT ID, 'Фамилия', 'Имя', 'Отчество', 'Номер паспорта', 'Отдел' FROM Преподаватели;");
    queryAllians.append("SELECT ID, 'Название', 'Направленность', 'Отдел', 'Описание' FROM Объединения;");

    // ----------------------------- DataBase ------------------------------

    // Временная строчка - указан путь к базе.
    if (connectDB("D:/Domerk/GUAP/Diplom/kcttDB.sqlite"))
    {
        showTable("Учащиеся");
    }
}

// ============================================================

MainWindow::~MainWindow()
{
    delete lastSelect;
    delete currentTable;
    delete ui;
}

// ============================================================
// ============== Установка соединения с базой ================
// ============================================================

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
// ====================== Вывод таблицы =======================
// ============================================================

void MainWindow::showTable(QString table)
{
    QSqlQuery query;
    lastSelect->clear();
    currentTable->clear();

    if (table == "Учащиеся")
    {
        query.exec(queryStud);
        lastSelect->append(queryStud);
    }

    if (table == "Преподаватели")
    {
        query.exec(queryTeach);
        lastSelect->append(queryTeach);
    }

    if (table == "Объединения")
    {
        query.exec(queryAllians);
        lastSelect->append(queryAllians);
    }

    currentTable->append(table);

    drawHeaders(query);
    drawRows(query);
}

// ============================================================
// ============= Отрисовка заголовков таблицы =================
// ============================================================

void MainWindow::drawHeaders(QSqlQuery query)
{
    // Удаляем столбцы, которые уже были нарисованы

    columnCount = ui->tableWidget->columnCount();
    for(int i = 0; i < columnCount; i++ )
             ui->tableWidget->removeColumn(0);


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

    ui->tableWidget->setHorizontalHeaderLabels(qsl);
}

// ============================================================
// ================ Отрисовка строк таблицы ===================
// ============================================================

void MainWindow::drawRows(QSqlQuery query)
{
    // Удаляем строки, которые уже были нарисованы

    rowCount = ui->tableWidget->rowCount();
    for(int i = 0; i < rowCount; i++)
             ui->tableWidget->removeRow(0);

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
// =========== Вывод подробной информации в форму =============
// ============================================================

void MainWindow::showMoreInfo()
{

}

// ============================================================
// =================== Удаление строки ========================
// ============================================================

void MainWindow::deleteThis()
{
    if(ui->tableWidget->verticalHeader()->currentIndex().row() >= 0)
        {

            QMessageBox messageBox(QMessageBox::Question,
                        tr("Удаление записи"),
                        tr("Вы действительно хотите удалить запись из базы?"),
                        QMessageBox::Yes | QMessageBox::No,
                        this);

            messageBox.setButtonText(QMessageBox::Yes, tr("Да"));
            messageBox.setButtonText(QMessageBox::No, tr("Нет"));

            if (messageBox.exec() == QMessageBox::Yes)
            {
                    QSqlQuery query;
                    QString str = "DELETE FROM " + *currentTable + "WHERE ID = " + ui->tableWidget->item(ui->tableWidget->verticalHeader()->currentIndex().row(), 0)->text() + " ;";
                    query.exec(str);
                    repeatLastSelect();
            }

        }
        else
        {
            QMessageBox messageBox(QMessageBox::Information,
                                   tr("Удаление записи"),
                                   tr("Не выбрано ни одной записи для удаления!"),
                                   QMessageBox::Yes,
                                   this);
            messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
            messageBox.exec();
        }
}

// ============================================================
// =================== Обновление строки ======================
// ============================================================

void MainWindow::updateThis()
{

}

// ============================================================
// =================== Добавление строки ======================
// ============================================================

void MainWindow::insertThis()
{

}

// ============================================================
// =============== Обновление таблицы (Select *) ==============
// ============================================================

void MainWindow::refreshTable()
{
    QSqlQuery query;
    lastSelect->clear();

    if (*currentTable == "Учащиеся")
    {
        query.exec(queryStud);
        lastSelect->append(queryStud);
    }

    if (*currentTable == "Преподаватели")
    {
        query.exec(queryTeach);
        lastSelect->append(queryTeach);
    }

    if (*currentTable == "Объединения")
    {
        query.exec(queryAllians);
        lastSelect->append(queryAllians);
    }

    drawRows(query);
}

// ============================================================
// ========== Повторение последнего запроса (Select) ==========
// ============================================================

void MainWindow::repeatLastSelect()
{
    QSqlQuery query;
    query.exec(*lastSelect);
    drawRows(query);
}

// ============================================================
// ============================================================
