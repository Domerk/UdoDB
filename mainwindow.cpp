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

    // ------------------------- Всякая красота ----------------------------

    this->setWindowTitle(tr("Тут будет содержательное название"));

    // --------------------------- Main ToolBar ----------------------------

    ui->mainToolBar->addAction(tr("Удалить запись"), this, SLOT(deleteThis()));
    ui->mainToolBar->addAction(tr("Перезагрузить таблицу"), this, SLOT(refreshTable()));
    ui->mainToolBar->addAction(tr("Перезагрузить таблицу"), this, SLOT(repeatLastSelect()));
    //ui->mainToolBar->addAction(tr("Расширенный поиск"), this, SLOT(showSearchForm()));


    connect(ui->tableWidget->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(headerClicked(int)));


    // ------------------ Запросы на вывод основных таблиц -----------------

    // Разобраться с ковычками. Возможно, читать запросы из файла.

   /* queryStud.append("SELECT 'ID', 'Фамилия', 'Имя', 'Отчество', 'Тип документа', 'Номер документа', 'Пол', 'Год рождения', ");
    queryStud.append("'Район школы', 'Школа', 'Класс', 'Родители', 'Домашний адрес', 'Телефон', 'e-mail', 'Дата заявления', 'Форма обучения', ");
    queryStud.append("'Когда выбыл', 'С ослабленным здоровьем', 'Сирота', 'Инвалид', 'На учёте в милиции', 'Многодетная семья', ");
    queryStud.append("'Неполная семья', 'Малообеспеченная семья', 'Мигранты', 'Примечания' FROM Учащиеся;");
*/

    queryStud.append("SELECT * FROM Учащиеся");

    //queryTeach.append("SELECT ID, 'Фамилия', 'Имя', 'Отчество', 'Номер паспорта', 'Отдел' FROM Преподаватели;");
    queryTeach.append("SELECT * FROM Преподаватели;");


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
    myDB = QSqlDatabase::addDatabase("QSQLITE");    // Указываем СУБД
    myDB.setDatabaseName(pathToDB);                 // Задаём полное имя базы

    QFileInfo checkFile(pathToDB);                  // Информация о файле базы

    if (checkFile.isFile())                         // Если такой файл существует
    {
        if (myDB.open())                            // Открываем соединение
        {
            ui->lblStatus->setText(tr("Соединение установлено")); // Выводим сообщение
            return true;                 // Возвращаем true
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
    QSqlQuery query;        // Создаём запрос
    lastSelect->clear();    // Удаляем данные о последнем запросе
    currentTable->clear();  // И о текущей таблице

    // Далее в зависимости от того, какую таблицу нцжно вывести
    // Выполняем соотвествующий запрос, сохраняем информацию о нём
    // И показываем соотвесттвующую страницу StackedWidget (форму)

    if (table == "Учащиеся")
    {
        query.exec(queryStud);
        lastSelect->append(queryStud);
        ui->stackedWidget->setCurrentIndex(2);
    }

    if (table == "Преподаватели")
    {
        query.exec(queryTeach);
        lastSelect->append(queryTeach);
        ui->stackedWidget->setCurrentIndex(1);
    }

    if (table == "Объединения")
    {
        query.exec(queryAllians);
        lastSelect->append(queryAllians);
        ui->stackedWidget->setCurrentIndex(0);
    }

    // Сохраняем инфу о текущей таблице
    currentTable->append(table);

    // Отображаем заголовки и строки таблицы
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
    QSqlRecord rec;             // Объект данного типа содержит информацию о Select'е
    rec = query.record();       // Получаем нужную инфу от запроса
    columnCount = rec.count();  // Узнаём количество столбцов
    ui->tableWidget->setColumnCount(columnCount);   // Задаём количество столбцов у таблицы

    for (int i = 0; i<columnCount; i++)     // Пока прочитали не все столбцы
    {
        qsl.append(rec.fieldName(i));       // Пишем их названия в стринглист
    }

    ui->tableWidget->setHorizontalHeaderLabels(qsl);    // Устанавливаем названия столбцов в таблице
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

    rowCount = 0;           // Нет ни одной строки
    while (query.next())    // Пока есть результаты запроса
        {
            ui->tableWidget->insertRow(rowCount);   // Добавляем строку в конец
            for (int i = 0; i<columnCount; i++)     // Для всех полей таблицы
            {
                // Создаём ячейку в текущем поле текущей строки и заносим туда инфу
                ui->tableWidget->setItem(rowCount, i, new QTableWidgetItem(query.value(i).toString()));
            }
            rowCount ++;    // Увеличиваем количество строк
        }

    ui->tableWidget->insertRow(rowCount); // В конце добавляем пустую строку
}

// ============================================================
// =================== Удаление строки ========================
// ============================================================

void MainWindow::deleteThis()
{
    if(ui->tableWidget->verticalHeader()->currentIndex().row() >= 0) // Если выбрана строка
        {
            // Создаём окно, запрашивающее подтверждение действия
            QMessageBox messageBox(QMessageBox::Question,
                        tr("Удаление записи"),
                        tr("Вы действительно хотите удалить запись из базы?"),
                        QMessageBox::Yes | QMessageBox::No,
                        this);

            messageBox.setButtonText(QMessageBox::Yes, tr("Да"));
            messageBox.setButtonText(QMessageBox::No, tr("Нет"));

            // Если действие подтверждено
            if (messageBox.exec() == QMessageBox::Yes)
            {
                    QSqlQuery query;        // Создаём и формируем запрос
                    QString str = "DELETE FROM " + *currentTable + "WHERE ID = " + ui->tableWidget->item(ui->tableWidget->verticalHeader()->currentIndex().row(), 0)->text() + " ;";
                    query.exec(str);        // Выполняем запрос
                    repeatLastSelect();     // Повторяеи последний Select
            }

        }
        else
        {
            // Иначе сообщаем пользователю, что ни одной записи не выбрано
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

void MainWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    QString str = item->text(column);
    showTable(str);
}

// ============================================================
// ============================================================

void MainWindow::headerClicked(int index)
{
    ui->tableWidget->sortByColumn(index);
}


// ============================================================
// ============== Вывод подробной инфы в форму ================
// ============================================================
void MainWindow::showMoreInfo(int row)
{
    if (*currentTable == "Учащиеся")
    {
        ui->studID->setText(ui->tableWidget->item(row, 0)->text());
        ui->studSurname->setText(ui->tableWidget->item(row, 1)->text());
        ui->studName->setText(ui->tableWidget->item(row, 2)->text());
        ui->studPatr->setText(ui->tableWidget->item(row, 3)->text());

        if (ui->tableWidget->item(row, 4)->text() == "Паспорт")
            ui->studDoc->setCurrentIndex(1);
        else
            ui->studDoc->setCurrentIndex(0);

        ui->studNumDoc->setText(ui->tableWidget->item(row, 5)->text());
    }

    if (*currentTable == "Преподаватели")
    {

    }

    if (*currentTable == "Объединения")
    {

    }
}

// ============================================================
// ============================================================

void MainWindow::clearMoreInfoForm()
{
    if (*currentTable == "Учащиеся")
    {
        ui->studID->clear();
        ui->studSurname->clear();
        ui->studName->clear();
        ui->studPatr->clear();

        // ui->studDoc-> ??

        ui->studNumDoc->clear();
    }

    if (*currentTable == "Преподаватели")
    {

    }

    if (*currentTable == "Объединения")
    {

    }
}

// ============================================================
// ============================================================

void MainWindow::on_tableWidget_cellClicked(int row, int column)
{
    if (row < rowCount)
    {
        showMoreInfo(row);
    }
    else
    {
        clearMoreInfoForm();
    }
}
