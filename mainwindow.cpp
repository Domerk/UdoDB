#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowState(Qt::WindowMaximized); // Главное окно разворачивается на весь экран
    ui->splitter->setSizes(QList <int> () << 120 << 750 << 200);

    lastSelect = new QString();
    currentTable = new QString();

    connectDialog = new ConnectionDialog();
    connect(connectDialog, SIGNAL(connectReconfig()), this, SLOT(connectReconfigSlot()));
    connect(ui->tableWidget->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(rowClicked(int)));

    dbDialog = new TableOpt();
    dbDialog->setType("tempDB");
    connect(dbDialog, SIGNAL(signalQuery(QTableWidget*,QString,bool)), this, SLOT(querySlot(QTableWidget*,QString,bool)));

    // Регулярное выражение для проверки имён, фамилий и отчеств.
    names = new QRegularExpression("^[А-ЯЁ]{1}[а-яё]*(-[А-ЯЁ]{1}[а-яё]*)?$");

    // ------------------------- Всякая красота ----------------------------

    this->setWindowTitle(tr("Тут будет содержательное название"));

    ui->alID->hide();
    ui->studID->hide();
    ui->teachID->hide();
    ui->directID->hide();
    ui->groupID->hide();

    ui->groupTeachID->hide();
    ui->groupAssID->hide();

    ui->alDirectID->hide();

    ui->treeWidget->setColumnCount(1);

    ui->addStudInGroup->setIcon(QIcon(":/icons/Icons/add"));
    ui->removeStudToGroup->setIcon(QIcon(":/icons/Icons/remove"));


    ui->groupInAl->setColumnHidden(0, true);
    ui->groupInTeach->setColumnHidden(0, true);
    ui->studsInGroupe->setColumnHidden(0, true);
    ui->alInDirect->setColumnHidden(0, true);

    // -------------------------------- Меню --------------------------------

    connect (ui->actionConnect, SIGNAL(triggered()), connectDialog, SLOT(exec()));
    connect(ui->exit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionExel, SIGNAL(triggered()), this, SLOT(exportInExel()));

    connect(ui->actionRefreshTab, SIGNAL(triggered()), this, SLOT(refreshTable()));
    connect(ui->actionRepeatLastSelect, SIGNAL(triggered()), this, SLOT(repeatLastSelect()));
    connect (ui->actionForMask, SIGNAL(triggered()), this, SLOT(changeTableMask()));

    connect(ui->actionNewStr, SIGNAL(triggered()), this, SLOT(clearMoreInfoForm()));
    connect(ui->actionDeleteStr, SIGNAL(triggered()), this, SLOT(deleteThis()));

    // --------------------------- Main ToolBar ----------------------------

    // Иконки: http://www.flaticon.com/packs/web-application-ui/4

    ui->mainToolBar->addAction(QIcon(":/icons/Icons/new.png"),tr("Новая запись"), this, SLOT(clearMoreInfoForm()));
    ui->mainToolBar->addAction(QIcon(":/icons/Icons/delete.png"),tr("Удалить запись"), this, SLOT(deleteThis()));
    ui->mainToolBar->addAction(QIcon(":/icons/Icons/import.png"),tr("Импорт из временной базы"), this, SLOT(showTempTable()));

    ui->mainToolBar->addAction(QIcon(":/icons/Icons/options.png"),tr("Скрыть/Показать поля"), this, SLOT(changeTableMask()));
    ui->mainToolBar->addAction(QIcon(":/icons/Icons/optool.png"),tr("Расширенный поиск"), this, SLOT(globalSearch()));

    //ui->mainToolBar->actions()[ToolButton::...]->setDisabled(true);


    // ------------------------- Search ToolBar ----------------------------

    searchBox = new QComboBox();
    searchEdit = new QLineEdit();
    searchEdit->setFixedWidth(250);
    searchBox->setLayoutDirection(Qt::LeftToRight);     // Поскольку сам ТулБар RingToLeft, принудительно задаём комбобоку нормальный вид
    ui->searchToolBar->addAction(QIcon(":/icons/Icons/search.png"),tr("Поиск"), this, SLOT(simpleSearch()));
    ui->searchToolBar->addWidget(searchEdit);
    ui->searchToolBar->addWidget(searchBox);
    ui->searchToolBar->actions()[SearchToolButton::Start]->setDisabled(true);

    connect(searchEdit, SIGNAL(textChanged(QString)), this, SLOT(setSearchActive()));
    connect(searchEdit, SIGNAL(returnPressed()), this, SLOT(simpleSearch()));


    // ------------------ Запросы на вывод основных таблиц -----------------

    Info inf;

    inf.query.append("SELECT `ID`, `Фамилия`, `Имя`, `Отчество`, `Тип документа`, `Номер документа`, `Пол`, `Год рождения`, `Район школы`, `Школа`, `Класс`, `Родители`, `Домашний адрес`, `Телефон`, `e-mail`, `Дата заявления`, `Форма обучения`,`Когда выбыл`, `С ослабленным здоровьем`, `Сирота`, `Инвалид`, `На учёте в полиции`, `Многодетная семья`, `Неполная семья`, `Малообеспеченная семья`, `Мигранты`, `Примечания` FROM Учащиеся;");
    inf.index = 2;
    inf.mask.append(true);
    for (int i = 1; i<11; i++)
        inf.mask.append(false);
    for (int i = 11; i<27; i++)
        inf.mask.append(true);
    infoMap.insert("Учащиеся", inf);

    inf.mask.clear();

    inf.query = "SELECT `ID`, `Фамилия`, `Имя`, `Отчество`, `Паспорт`, `Отдел` FROM Преподаватели;";
    inf.index = 1;
    inf.mask.append(true);
    for (int i = 1; i<6; i++)
        inf.mask.append(false);
    infoMap.insert("Преподаватели", inf);

    inf.mask.clear();

    inf.query = "SELECT `ID`, `ID Направленности`, `Название`, `Направленность`, `Отдел`, `Описание` FROM Объединения;";
    inf.index = 0;
    inf.mask.append(true);
    for (int i = 1; i<5; i++)
        inf.mask.append(false);
    infoMap.insert("Объединения", inf);

    inf.mask.clear();

    inf.query = "SELECT `ID`, `Название` FROM Направленности;";
    inf.index = 3;
    infoMap.insert("Направленности", inf);

    inf.mask.clear();

    inf.query = "SELECT `ID`, `ID объединения`, `ID преподавателя`, `Номер`, `Год обучения`, `Объединение`, `Фамилия преподавателя`, `Имя преподавателя`, `Отчество преподавателя` FROM Группы;";
    inf.index = 4;
    infoMap.insert("Группы", inf);


    // ----------------------------- DataBase ------------------------------

    if (connectDB("MainDB"))
    {
        drawTree();
        showTable("Учащиеся");
    }

}

// ============================================================
// ============================================================

MainWindow::~MainWindow()
{
    if (myDB.isOpen())
        myDB.close();

    if (tempDB.isOpen())
        tempDB.close();

    delete connectDialog;
    delete lastSelect;
    delete currentTable;
    delete dbDialog;
    delete ui;
}

// ============================================================
// ============== Установка соединения с базой ================
// ============================================================

bool MainWindow::connectDB(QString nameDB)
{
    // Подключение к основной базе

    if (myDB.isOpen())
    {
        myDB.close();
    }
    else
    {
        myDB = QSqlDatabase::addDatabase("QSQLITE");    // Указываем СУБД, имени соединения нет - соединение по умолчанию
    }

    QSettings settings ("Kctt", "KcttDB");
    settings.beginGroup("mainDB");
    myDB.setHostName(settings.value("hostname", "localhost").toString());
    myDB.setDatabaseName(settings.value("dbname", "kcttDB").toString());
    myDB.setUserName(settings.value("username").toString());
    myDB.setPassword(settings.value("password").toString());
    settings.endGroup();

    // Подключение к временной базе базе
    if (tempDB.isOpen())
    {
        tempDB.close();
    }
    else
    {
        tempDB = QSqlDatabase::addDatabase("QSQLITE", "tempDB");    // Указываем СУБД
    }

    settings.beginGroup("TempDB");
    tempDB.setHostName(settings.value("hostname", "localhost").toString());
    tempDB.setDatabaseName(settings.value("dbname", "kcttTempDB").toString());
    tempDB.setUserName(settings.value("username").toString());
    tempDB.setPassword(settings.value("password").toString());
    settings.endGroup();

    // Вот это место переписать, тк соединене с основной базой критично, а со временной - нет

    if (myDB.open() && tempDB.open())                            // Открываем соединение
    {
        ui->lblStatus->setText(tr("Соединение установлено")); // Выводим сообщение
        return true;                 // Возвращаем true
    }
    else
    {
        ui->lblStatus->setText(tr("Ошибка соединения: соединение не установлено"));
    }
    return false;
}

void MainWindow::connectReconfigSlot()
{
    connectDB("MainDB");
    if (!currentTable->isEmpty())
        refreshTable();
}

// ----------------------------------------------------------------------------------------------------
// -------------------------------------- Рисование таблиц --------------------------------------------
// ----------------------------------------------------------------------------------------------------

// ============================================================
// ====================== Вывод таблицы =======================
// ============================================================

void MainWindow::showTable(QString table)
{

    if (table == "Общее" || table == *currentTable)
        return;

    QSqlQuery query;        // Создаём запрос
    lastSelect->clear();    // Удаляем данные о последнем запросе
    currentTable->clear();  // О текущей таблице
    currentMask.clear();   // И о её маске

    // Далее в зависимости от того, какую таблицу нцжно вывести
    // Выполняем соотвествующий запрос, сохраняем информацию о нём
    // И показываем соотвесттвующую страницу StackedWidget (форму)

    query.exec(infoMap.value(table).query);
    lastSelect->append(infoMap.value(table).query);
    ui->stackedWidget->setCurrentIndex(infoMap.value(table).index);
    currentMask = infoMap.value(table).mask;

    // Сохраняем инфу о текущей таблице
    currentTable->append(table);

    // Отображаем заголовки и строки таблицы
    drawHeaders(query, ui->tableWidget, true, searchBox);
    drawRows(query, ui->tableWidget, true);
    hideColumnsFromMask(currentMask);

    return;
}

// ============================================================
// ============= Отрисовка заголовков таблицы =================
// ============================================================

void MainWindow::drawHeaders(QSqlQuery query, QTableWidget *table, bool forSearch, QComboBox *serchCBox)
{
    // Удаляем столбцы, которые уже были нарисованы

    int columnCount = table->columnCount();
    for(int i = 0; i < columnCount; i++ )
             table->removeColumn(0);

    columnCount = 0;
    QStringList qsl;
    QSqlRecord rec;             // Объект данного типа содержит информацию о Select'е
    rec = query.record();       // Получаем нужную инфу от запроса
    columnCount = rec.count();  // Узнаём количество столбцов
    table->setColumnCount(columnCount);   // Задаём количество столбцов у таблицы

    for (int i = 0; i<columnCount; i++)     // Пока прочитали не все столбцы
    {
        qsl.append(rec.fieldName(i));       // Пишем их названия в стринглист
    }

    table->setHorizontalHeaderLabels(qsl);    // Устанавливаем названия столбцов в таблице

    if (forSearch)
    {
        serchCBox->clear();
        if (qsl.size() > 1 && qsl.at(0) == "ID")
            qsl.removeFirst();          // Удаляем 0й элемент (ID)
        serchCBox->addItems(qsl);       // Задаём комбобоксу поиска
    }
}

// ============================================================
// ================ Отрисовка строк таблицы ===================
// ============================================================

void MainWindow::drawRows(QSqlQuery query, QTableWidget *table, bool available)
{
    QSqlRecord rec;             // Объект данного типа содержит информацию о Select'е
    rec = query.record();       // Получаем нужную инфу от запроса
    int columnCount = rec.count();  // Узнаём количество столбцов

    // Удаляем строки, которые уже были нарисованы

    int rowCount = table->rowCount();
    for(int i = 0; i < rowCount; i++)
             table->removeRow(0);

    rowCount = 0;           // Нет ни одной строки

    table->setSortingEnabled(false); // Временно запрещаем сортировку

    while (query.next())    // Пока есть результаты запроса
        {
            table->insertRow(rowCount);   // Добавляем строку в конец
            for (int i = 0; i<columnCount; i++)     // Для всех полей таблицы
            {
                // Создаём ячейку в текущем поле текущей строки и заносим туда инфу

                if (query.value(i).toString() == "true")
                {
                    table->setItem(rowCount, i, new QTableWidgetItem("Да"));
                    continue;
                }
                if (query.value(i).toString() == "false")
                {
                    table->setItem(rowCount, i, new QTableWidgetItem("Нет"));
                    continue;
                }

                table->setItem(rowCount, i, new QTableWidgetItem(query.value(i).toString()));
            }
            rowCount ++;    // Увеличиваем количество строк
        }

    if (available)
        table->insertRow(rowCount); // В конце добавляем пустую строку

    table->setSortingEnabled(true); // Разрешаем сортировку
}

// ============================================================
// ================= Скрытие полей по маске ===================
// ============================================================


void MainWindow::hideColumnsFromMask(QVector<bool> mask)
{
    if (mask.size() == ui->tableWidget->columnCount())
    {
        for (int i = 0; i < mask.size(); i++)
            ui->tableWidget->setColumnHidden(i, mask[i]);
    }
}

// ============================================================
// ===================== Измнение маски =======================
// ============================================================

void MainWindow::changeTableMask()
{
    QVector<QCheckBox*> vct;
    QDialog *wgt = new QDialog(this);
    QVBoxLayout *layout = new QVBoxLayout();
    QGridLayout *gl = new QGridLayout();
    QDialogButtonBox *buttonBox;
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), wgt, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), wgt, SLOT(reject()));

    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Применить"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Отмена"));

    int colCount = ui->tableWidget->columnCount();
    int row = 0;
    int col = 0;

    for (int i = 0; i < colCount-1; i++)        // Запись заголовков
    {
        vct.append(new QCheckBox(ui->tableWidget->horizontalHeaderItem(i+1)->text()));
        vct[i]->setChecked(!currentMask[i+1]);
        gl->addWidget(vct[i], row, col);

        row++;
        if (row>10)
        {
            row=0;
            col++;
        }
    }

    layout->addLayout(gl);
    layout->addWidget(buttonBox);

    wgt->setLayout(layout);
    wgt->setModal(true);
    wgt->setWindowTitle(tr("Скрыть / Показать поля"));
    if (wgt->exec() == QDialog::Accepted)
    {
        for(int i = 1; i < colCount; i++)
            currentMask[i] = !vct[i-1]->isChecked();

        hideColumnsFromMask(currentMask);

        // Сохраняем инфу об изменениях
       infoMap[*currentTable].mask = currentMask;
    }
}

// ----------------------------------------------------------------------------------------------------
// -------------------------------------------- Запросы -----------------------------------------------
// ----------------------------------------------------------------------------------------------------

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
                    QString strQuery;

                    for (QTableWidgetSelectionRange selectionRange : ui->tableWidget->selectedRanges())
                    {
                        if (selectionRange.rowCount() > 0)
                            for (int row = selectionRange.topRow(); row <= selectionRange.bottomRow(); row++)
                            {
                                strQuery.append("DELETE FROM " + *currentTable + " WHERE ID = " + ui->tableWidget->item(row, 0)->text() + " ;");
                                query.exec(strQuery);
                                strQuery.clear();
                            }
                    }

                    repeatLastSelect();     // Повторяем последний Select
                    clearMoreInfoForm();    // Чистим поле с подробностями
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
// =============== Обновление таблицы (Select *) ==============
// ============================================================

void MainWindow::refreshTable()
{
    QSqlQuery query;
    lastSelect->clear();
    lastSelect->append(infoMap.value(*currentTable).query);
    query.exec(*lastSelect);
    drawRows(query, ui->tableWidget, true);
}

// ============================================================
// ========== Повторение последнего запроса (Select) ==========
// ============================================================

void MainWindow::repeatLastSelect()
{
    QSqlQuery query;
    query.exec(*lastSelect);
    drawRows(query, ui->tableWidget, true);
}

// ============================================================
// ============== Добавление и изменение записей ==============
// ============================================================

void MainWindow::on_saveButton_clicked()
{
    int currentIndex = ui->stackedWidget->currentIndex();
    QString strQuery;

    bool isName = true;
    bool isSurname = true;
    bool isPatrName = true;

    switch (currentIndex)
    {
        case 0:     // Объединение
        {
            QString id = ui->alID->text();                                 // ID
            QString name = ui->alName->text().simplified().replace(QRegularExpression("-{2,}"), "-");          // Название
            QString directID = ui->alDirectID->text();                  // Напавленность
            QString otd = ui->alOtd->text().simplified().replace(QRegularExpression("-{2,}"), "-");            // Отдел
            QString desc = ui->alDescript->toPlainText().replace(QRegularExpression("-{2,}"), "-");     // Описание

            if (name.isEmpty() || otd.isEmpty())
            {
                // Сообщаем пользователю, что обязательные поля не заполнены
                QMessageBox messageBox(QMessageBox::Information,
                                       tr("Добавление записи"),
                                       tr("Одно или несколько обязательных полей не заполнены!<br />Проверьте заполнение полей Название, Направленность и Отдел."),
                                       QMessageBox::Yes,
                                       this);
                messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
                messageBox.exec();
                return;
            }


            if (id.isEmpty())
            {
                strQuery = "INSERT INTO Объединение (`Название`, `ID Направленности`, `Отдел`, `Описание`) VALUES ('" + name + "', '" + directID + "', '" + otd  + "', '" + desc + "');";
                clearMoreInfoForm();
            }
            else
            {
                strQuery = "UPDATE Объединение SET `Название` = '" + name + "', `ID Направленности` = '" + directID + "', `Отдел` = '" + otd + "', `Описание` = '" + desc + "' WHERE ID = " + id + ";";
            }

            //drawTree(); // Перерисовываем дерево
            break;
        }
        case 1:     // Преподаватель
        {

            QString id = ui->teachID->text();               // ID
            QString surname = ui->teachSurname->text().simplified().replace(QRegularExpression("-{2,}"), "-");     // Фамилия
            QString name = ui->teachName->text().simplified().replace(QRegularExpression("-{2,}"), "-");           // Имя
            QString patrname = ui->teachPatr->text().simplified().replace(QRegularExpression("-{2,}"), "-");       // Отчество
            QString numpass = ui->teachNumPass->text().simplified().replace(QRegularExpression("-{2,}"), "-");     // Номер паспорта
            QString otd = ui->teachOtd->text().simplified().replace(QRegularExpression("-{2,}"), "-");             // Отдел

            if (name.isEmpty() || surname.isEmpty() || numpass.isEmpty())
            {
                // Сообщаем пользователю, что обязательные поля не заполнены
                QMessageBox messageBox(QMessageBox::Information,
                                       tr("Добавление записи"),
                                       tr("Одно или несколько обязательных полей не заполнены!<br />Проверьте заполнение полей Фамилия, Имя и Номер паспорта."),
                                       QMessageBox::Yes,
                                       this);
                messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
                messageBox.exec();
                return;
            }

            QRegularExpressionMatch match = names->match(name);
            isName = match.hasMatch();

            match = names->match(surname);
            isSurname = match.hasMatch();

            if (!patrname.isEmpty())
            {
                match = names->match(patrname);
                isPatrName = match.hasMatch();
            }

            if (!isName || !isSurname || !isPatrName)
            {
                QMessageBox messageBox(QMessageBox::Information,
                                       tr("Сохранение"),
                                       tr("Ошибка сохранения: Одно или несколько полей заполнены неверно!"),
                                       QMessageBox::Yes,
                                       this);
                messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
                messageBox.exec();
                return;
            }

            if (id.isEmpty())
            {
                strQuery = "INSERT INTO Преподаватели (`Имя`, `Фамилия`, `Отчество`, `Паспорт`, `Отдел`) VALUES ('" + name + "', '" + surname + "', '" + patrname + "', '" + numpass + "', '" + otd + "');";
                clearMoreInfoForm();
            }
            else
            {
                strQuery = "UPDATE Преподаватели SET `Имя` = '" + name + "', `Фамилия` = '" + surname + "', `Отчество` = '" + patrname + "', `Паспорт` = '" + numpass + "', `Отдел` = '" + otd + "' WHERE ID = " + id + ";";
            }

            break;
        }
        case 2:     // Учащийся
        {
            QString id = ui->studID->text();                                    // ID
            QString surname = ui->studSurname->text().simplified().replace(QRegularExpression("-{2,}"), "-");       // Фамилия
            QString name = ui->studName->text().simplified().replace(QRegularExpression("-{2,}"), "-");             // Имя

            QString docType = ui->studDoc->currentText();                       // Тип документа
            QString docNum = ui->studNumDoc->text().simplified().replace(QRegularExpression("-{2,}"), "-");         // Номер документа
            QString gender = ui->studGender->currentText();                     // Пол

            if (name.isEmpty() || surname.isEmpty() || docType == "" || docNum.isEmpty() || gender == "")
            {
                // Сообщаем пользователю, что обязательные поля не заполнены
                QMessageBox messageBox(QMessageBox::Information,
                                       tr("Добавление записи"),
                                       tr("Одно или несколько обязательных полей не заполнены!<br />Проверьте заполнение полей Фамилия, Имя, Тип документа, Номер документа и Пол."),
                                       QMessageBox::Yes,
                                       this);
                messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
                messageBox.exec();
                return;
            }

            // Если обязательные поля заполнены, можно собирать информацию об остальных полях.

            QString patr = ui->studPatr->text().simplified().replace(QRegularExpression("-{2,}"), "-");
            QString arSchool = ui->areaSchools->text().simplified().replace(QRegularExpression("-{2,}"), "-");     // Район школы
            QString school = ui->school->text().simplified().replace(QRegularExpression("-{2,}"), "-");            // Школа
            QString grad = ui->grade->text().simplified().replace(QRegularExpression("-{2,}"), "-");               // Класс
            QString phone = ui->phone->text().simplified().replace(QRegularExpression("-{2,}"), "-");              // Телефон
            QString email = ui->email->text().simplified().replace(QRegularExpression("-{2,}"), "-");              // email


            QRegularExpressionMatch match = names->match(name);
            isName = match.hasMatch();

            match = names->match(surname);
            isSurname = match.hasMatch();

            if (!patr.isEmpty())
            {
                match = names->match(patr);
                isPatrName = match.hasMatch();
            }

            if (!isName || !isSurname || !isPatrName)
            {
                QMessageBox messageBox(QMessageBox::Information,
                                       tr("Сохранение"),
                                       tr("Ошибка сохранения: Одно или несколько полей заполнены неверно!"),
                                       QMessageBox::Yes,
                                       this);
                messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
                messageBox.exec();
                return;
            }

            QString birthday;
            QString admiss;
            QString out;


            if (ui->studBDay->currentIndex() != 0 && ui->studBMon->currentIndex() != 0)
                birthday = ui->studBDay->currentText() + "." + ui->studBMon->currentText() + "." + QString::number(ui->studBYear->value());    // Год рождения

            if (ui->studInDay->currentIndex() != 0 && ui->studInMon->currentIndex() != 0)
                admiss = ui->studInDay->currentText() + "." + ui->studInMon->currentText() + "." + QString::number(ui->studInYear->value());        // Дата подачи заявления

            if (ui->studOutDay->currentIndex() != 0 && ui->studOutMon->currentIndex() != 0)
            out = ui->studOutDay->currentText() + "." + ui->studOutMon->currentText() + "." + QString::number(ui->studOutYear->value());               // Когда выбыл

            // Combo Box
            QString eduForm = ui->eduForm->currentText(); // Форма обучения

            QString comments = ui->studComments->toPlainText().replace(QRegularExpression("-{2,}"), "-");
            QString parents = ui->parents->toPlainText().replace(QRegularExpression("-{2,}"), "-");   // Родители
            QString address = ui->address->toPlainText().replace(QRegularExpression("-{2,}"), "-");   // Адрес

            // Check Box

            QString accountInPolice = "false";
            QString incompleteFamily  = "false";
            QString invalid  = "false";
            QString large  = "false";
            QString lowIncome  = "false";
            QString migrants  = "false";
            QString orphan  = "false";
            QString weackHealth  = "false";

            if (ui->accountInPolice->isChecked())
                accountInPolice = "true";

            if (ui->incompleteFamily->isChecked())
                incompleteFamily = "true";

            if (ui->invalid->isChecked())
                 invalid = "true";

            if (ui->large->isChecked())
                large = "true";

            if (ui->lowIncome->isChecked())
                lowIncome = "true";

            if (ui->migrants->isChecked())
                migrants = "true";

            if (ui->orphan->isChecked())
                orphan = "true";

            if (ui->weackHealth->isChecked())
                weackHealth = "true";

            if (id.isEmpty())
            {
                // INSERT
                strQuery = "INSERT INTO Учащиеся (";
                strQuery.append("'Фамилия', 'Имя', 'Отчество', 'Тип документа', 'Номер документа', 'Пол', 'Год рождения', ");
                strQuery.append("'Район школы', 'Школа', 'Класс', 'Родители', 'Домашний адрес', 'Телефон', 'e-mail', 'Дата заявления', 'Форма обучения', ");
                strQuery.append("'Когда выбыл', 'С ослабленным здоровьем', 'Сирота', 'Инвалид', 'На учёте в полиции', 'Многодетная семья', ");
                strQuery.append("'Неполная семья', 'Малообеспеченная семья', 'Мигранты', 'Примечания') VALUES ('");
                strQuery.append(surname + "', '" + name  + "', '" + patr  + "', '" + docType  + "', '" + docNum  + "', '" + gender  + "', '" + birthday  + "', '");
                strQuery.append(arSchool  + "', '" + school  + "', '" + grad  + "', '" + parents  + "', '" + address  + "', '" + phone  + "', '" + email  + "', '" + admiss  + "', '" + eduForm  + "', '");
                strQuery.append(out  + "', '" + weackHealth  + "', '" + orphan  + "', '" + invalid  + "', '" + accountInPolice + "', '" + large  + "', '");
                strQuery.append(incompleteFamily  + "', '" + lowIncome  + "', '" + migrants  + "', '" + comments  + "');");
                clearMoreInfoForm();

            }
            else
            {
                // UPDATE
                strQuery.append("UPDATE Учащиеся SET 'Фамилия' = '" + surname + "', 'Имя' = '" + name  + "', 'Отчество' = '" + patr  + "', 'Тип документа' = '" + docType  + "', 'Номер документа' = '" + docNum  + "', 'Пол' = '" + gender  + "', 'Год рождения' = '" + birthday  + "', ");
                strQuery.append("'Район школы' = '" + arSchool  + "', 'Школа' = '" + school  + "', 'Класс' = '" + grad  + "', 'Родители' = '" + parents  + "', 'Домашний адрес' = '" + address  + "', 'Телефон' = '" + phone  + "', 'e-mail' = '" + email  + "', 'Дата заявления' = '" + admiss  + "', 'Форма обучения' = '" + eduForm  + "', ");
                strQuery.append("'Когда выбыл' = '" + out  + "', 'С ослабленным здоровьем' = '" + weackHealth  + "', 'Сирота' = '" + orphan  + "', 'Инвалид' = '" + invalid  + "', 'На учёте в полиции' = '" + accountInPolice + "', 'Многодетная семья' = '" + large  + "', ");
                strQuery.append("'Неполная семья' = '" + incompleteFamily  + "', 'Малообеспеченная семья' = '" + lowIncome  + "', 'Мигранты' = '" + migrants  + "', 'Примечания' = '" + comments  + "' ");
                strQuery.append("WHERE ID = " + id + ";");
            }

            break;
        }

    case 3: // Направленность
    {
        QString id = ui->directID->text();
        QString name = ui->directName->text().simplified().replace(QRegularExpression("-{2,}"), "-");

        if (name.isEmpty())
        {
            // Сообщаем пользователю, что обязательные поля не заполнены
            QMessageBox messageBox(QMessageBox::Information,
                                   tr("Добавление записи"),
                                   tr("Отсутсвуют данные для сохранения!"),
                                   QMessageBox::Yes,
                                   this);
            messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
            messageBox.exec();
            return;
        }


        if (id.isEmpty())
        {
            strQuery = "INSERT INTO Направленности (`Название`) VALUES ('" + name + "');";
            clearMoreInfoForm();
        }
        else
        {
            strQuery = "UPDATE Направленности SET `Название` = '" + name + "' WHERE ID = " + id + ";";
        }

        //drawTree(); // Перерисовываем дерево
        break;

    }
    case 4: // Группа
    {
        QString id = ui->groupID->text();
        QString teachID = ui->groupTeachID->text();
        QString assID = ui->groupAssID->text();
        QString num = ui->groupNum->text();
        QString year = QString::number(ui->groupYear->value());

        // ЗДЕСЬ: Продумать, какие поля должны быть обязательными

        if (num.isEmpty())
        {
            // Сообщаем пользователю, что обязательные поля не заполнены
            QMessageBox messageBox(QMessageBox::Information,
                                   tr("Добавление записи"),
                                   tr("Одно или несколько обязательных полей не заполнены!"),
                                   QMessageBox::Yes,
                                   this);
            messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
            messageBox.exec();
            return;
        }


        if (id.isEmpty())
        {
            strQuery = "INSERT INTO Группа (`Номер`, `Год обучения`, `ID объединения`, `ID преподавателя`) VALUES ('" + num + "', " + year + ", " + assID + ", " + teachID + ");";
            clearMoreInfoForm();
        }
        else
        {
            strQuery = "UPDATE Группа SET `ID объединения` = " + assID + ", `ID преподавателя` = " + teachID + ", `Номер` = '" + num + "', `Год обучения` = " + year + " WHERE ID = " + id + ";";
        }

        //drawTree(); // Перерисовываем дерево
        break;

    }
    }

    QSqlQuery query;
    query.exec(strQuery);

    if (query.isValid())
        ui->lblStatus->setText("Ok");
    else
        ui->lblStatus->setText("Error");

    repeatLastSelect();
}

// ============================================================
// ===================== Простой поиск ========================
// ============================================================

void MainWindow::simpleSearch()
{
    if (!ui->searchToolBar->actions()[SearchToolButton::Start]->isEnabled())
        return;

    ui->searchToolBar->actions()[SearchToolButton::Start]->setDisabled(true);
    QString* searchText = new QString;
    searchText->append(searchEdit->text().simplified().replace(QRegularExpression("-{2,}"), "-"));

    if(searchText->isEmpty())
    {
        refreshTable();
    }
    else
    {

        // Убрать регистрозависимость

        // И вообще этот кусок кода не торт, поэтому перепиши его

        QString *newSelect = new QString();

        if (*currentTable == "Учащиеся")
        {
            if (searchBox->currentText() == "С ослабленным здоровьем" || searchBox->currentText() ==  "Сирота"
                    || searchBox->currentText() == "Инвалид" || searchBox->currentText() == "На учёте в полиции"
                    || searchBox->currentText() == "Многодетная семья" || searchBox->currentText() == "Неполная семья"
                    || searchBox->currentText() == "Малообеспеченная семья" || searchBox->currentText() == "Мигранты")
            {
                QString str = infoMap.value(*currentTable).query;
                newSelect->append(str.replace(";", " ") + " WHERE `" + searchBox->currentText());
                if (searchText->toLower() == "да")
                    newSelect->append("` = 'true';");
                else
                    newSelect->append("` = 'false';");
            }
            else
            {
                QString str = infoMap.value(*currentTable).query;
                newSelect->append(str.replace(";", " ") + " WHERE `" + searchBox->currentText() + "` LIKE '%" + *searchText + "%';");
            }
        }

        if (*currentTable == "Преподаватели")
        {
            QString str = infoMap.value(*currentTable).query;
            newSelect->append(str.replace(";", " ") + " WHERE `" + searchBox->currentText() + "` LIKE '%" + *searchText + "%';");
        }

        if (*currentTable == "Объединения")
        {
            QString str = infoMap.value(*currentTable).query;
            newSelect->append(str.replace(";", " ") + " WHERE `" + searchBox->currentText() + "` LIKE '%" + *searchText + "%';");
        }

        QSqlQuery query;
        query.exec(*newSelect);

        drawRows(query, ui->tableWidget, true);
        lastSelect = newSelect;
    }
}

// ----------------------------------------------------------------------------------------------------
// ------------------------------------------ Интерфейс -----------------------------------------------
// ----------------------------------------------------------------------------------------------------

// ============================================================
// ===================== Рисование дерева =====================
// ============================================================

void MainWindow::drawTree()
{
   QStringList *directions = new QStringList();
   QStringList *association = new QStringList();

   QTreeWidgetItem *direct= new QTreeWidgetItem(ui->treeWidget);
   direct->setText(0, "Направленности");

   // Получаем список направленностей

   QString strQuery = "SELECT Название FROM Направленности ;";
   QSqlQuery query;
   query.exec(strQuery);
   while (query.next())    // Пока есть результаты запроса
   {
       directions->append(query.value(0).toString());
   }

   ui->alDirect->clear();

   for (QString & dir: *directions)
   {
       // Получаем список объединений данной направленности

       QTreeWidgetItem *treeDir = new QTreeWidgetItem();
       treeDir->setText(0, dir);

       strQuery.clear();
       strQuery.append("SELECT Объединения.Название FROM Объединения, Направленности  WHERE Направленности.ID = Объединения.`ID Направленности` AND Направленности.Название = `");
       strQuery.append(dir + "`;");
       query.exec(strQuery);
       while (query.next())    // Пока есть результаты запроса
       {
           association->append(query.value(0).toString());
       }

       // Получаем список групп данного объединения данной направленности

       for (QString & ass: *association)
       {
           QTreeWidgetItem *treeAss = new QTreeWidgetItem();
           treeAss->setText(0, ass);

           strQuery.clear();
           strQuery.append("SELECT Группы.Номер, Группы.ID FROM Объединения, Направленности, Группы  WHERE Направленности.ID = Объединения.`ID Направленности` AND Объединения.ID = Группы.`ID объединения` AND Направленности.Название = `");
           strQuery.append(dir + "`, Объединения.Название = `" + ass + "`;");
           query.exec(strQuery);
           while (query.next())    // Пока есть результаты запроса
           {
               QTreeWidgetItem *treeGroup = new QTreeWidgetItem();
               treeGroup->setText(0, query.value(0).toString());
               treeGroup->setText(1, query.value(1).toString());

               treeAss->addChild(treeGroup);
           }

           treeDir->addChild(treeAss);
       }

       direct->addChild(treeDir);
   }

}

// ============================================================
// ================ Активация кнопки Поиск ====================
// ============================================================

void MainWindow::setSearchActive()
{
    ui->searchToolBar->actions()[SearchToolButton::Start]->setDisabled(false);
}

// ============================================================
// ============== Вывод подробной инфы в форму ================
// ============================================================

void MainWindow::showMoreInfo(int row)
{
    clearMoreInfoForm(); // Чистим форму

    if (*currentTable == "Учащиеся")
    {
        ui->studID->setText(ui->tableWidget->item(row, 0)->text());         // ID
        ui->studSurname->setText(ui->tableWidget->item(row, 1)->text());    // Фамилия
        ui->studName->setText(ui->tableWidget->item(row, 2)->text());       // Имя
        ui->studPatr->setText(ui->tableWidget->item(row, 3)->text());       // Отчество

        if (ui->tableWidget->item(row, 4)->text() == "Паспорт")     // Тип документа
            ui->studDoc->setCurrentIndex(2);                        // Паспорт
        else
            ui->studDoc->setCurrentIndex(1);                        // Свидетельство о рождении

        ui->studNumDoc->setText(ui->tableWidget->item(row, 5)->text()); // Номер документа

        if (ui->tableWidget->item(row, 6)->text() == "Жен")     // Пол
            ui->studGender->setCurrentIndex(2);                 // Жен
        else
            ui->studGender->setCurrentIndex(1);                 // Муж

        QStringList qsl = ui->tableWidget->item(row, 7)->text().split("."); // Год рождения
        if (qsl.length() == 3)
        {
            ui->studBDay->setCurrentText(qsl[0]);
            ui->studBMon->setCurrentText(qsl[1]);
            ui->studBYear->setValue(qsl[2].toInt());
        }
        else
        {
            ui->studBDay->setCurrentIndex(0);
            ui->studBMon->setCurrentIndex(0);
            ui->studBYear->setValue(2000);
        }

        ui->areaSchools->setText(ui->tableWidget->item(row, 8)->text());    // Район школы
        ui->school->setText(ui->tableWidget->item(row, 9)->text());         // Школа
        ui->grade->setText(ui->tableWidget->item(row, 10)->text());         // Класс
        ui->parents->setText(ui->tableWidget->item(row, 11)->text());       // Родители
        ui->address->setText(ui->tableWidget->item(row, 12)->text());       // Адрес
        ui->phone->setText(ui->tableWidget->item(row, 13)->text());         // Телефон
        ui->email->setText(ui->tableWidget->item(row, 14)->text());         // email

        // Дата подачи заявления
        qsl.clear();
        qsl = ui->tableWidget->item(row, 15)->text().split(".");
        if (qsl.length() == 3)
        {
            ui->studInDay->setCurrentText(qsl[0]);
            ui->studInMon->setCurrentText(qsl[1]);
            ui->studInYear->setValue(qsl[2].toInt());
        }
        else
        {
            ui->studInDay->setCurrentIndex(0);
            ui->studInMon->setCurrentIndex(0);
            ui->studInYear->setValue(2000);
        }

        ui->eduForm->setCurrentText(ui->tableWidget->item(row, 16)->text()); // Форма обучения

        // Когда выбыл
        qsl.clear();
        qsl = ui->tableWidget->item(row, 17)->text().split(".");
        if (qsl.length() == 3)
        {
            ui->studOutDay->setCurrentText(qsl[0]);
            ui->studOutMon->setCurrentText(qsl[1]);
            ui->studOutYear->setValue(qsl[2].toInt());
        }
        else
        {
            ui->studOutDay->setCurrentIndex(0);
            ui->studOutMon->setCurrentIndex(0);
            ui->studOutYear->setValue(2000);
        }


        // Чекбоксы

        if (ui->tableWidget->item(row, 18)->text() == "Да")       // С ослабленным здоровьем
            ui->weackHealth->setChecked(true);
        else
            ui->weackHealth->setChecked(false);

        if (ui->tableWidget->item(row, 19)->text() == "Да")       // Сирота
            ui->orphan->setChecked(true);
        else
            ui->orphan->setChecked(false);

        if (ui->tableWidget->item(row, 20)->text() == "Да")       // Инвалид
            ui->invalid->setChecked(true);
        else
            ui->invalid->setChecked(false);

        if (ui->tableWidget->item(row, 21)->text() == "Да")       // На учёте в полиции
            ui->accountInPolice->setChecked(true);
        else
            ui->accountInPolice->setChecked(false);

        if (ui->tableWidget->item(row, 22)->text() == "Да")       // Многодетные
            ui->large->setChecked(true);
        else
            ui->large->setChecked(false);

        if (ui->tableWidget->item(row, 23)->text() == "Да")       // Неполная семья
            ui->incompleteFamily->setChecked(true);
        else
            ui->incompleteFamily->setChecked(false);

        if (ui->tableWidget->item(row, 24)->text() == "Да")       // Малообеспеченная семья
            ui->lowIncome->setChecked(true);
        else
            ui->lowIncome->setChecked(false);

        if (ui->tableWidget->item(row, 25)->text() == "Да")       // Мигранты
            ui->migrants->setChecked(true);
        else
            ui->migrants->setChecked(false);

        // Дополнительные сведенья
        ui->studComments->setPlainText(ui->tableWidget->item(row, 26)->text());
    }

    if (*currentTable == "Преподаватели")
    {

        ui->teachID->setText(ui->tableWidget->item(row, 0)->text());        // ID
        ui->teachSurname->setText(ui->tableWidget->item(row, 1)->text());   // Фамилия
        ui->teachName->setText(ui->tableWidget->item(row, 2)->text());      // Имя
        ui->teachPatr->setText(ui->tableWidget->item(row, 3)->text());      // Отчество
        ui->teachNumPass->setText(ui->tableWidget->item(row, 4)->text());   // Номер паспорта
        ui->teachOtd->setText(ui->tableWidget->item(row, 5)->text());       // Отдел

        // Отрисовка таблички!!!
        // Объединение + Номер + Год обучения

        QString str = "SELECT `ID`, `Объединение`, `Номер`, `Год обучения` FROM Группы WHERE `ID преподавателя` = " + ui->teachID->text() + ";";

        QSqlQuery query;
        query.exec(str);
        drawRows(query, ui->groupInTeach, false);
    }

    if (*currentTable == "Объединения")
    {
        // "SELECT `ID`, `ID Направленности`, `Название`, `Направленность`, `Отдел`, `Описание` FROM Объединения;"
        ui->alID->setText(ui->tableWidget->item(row, 0)->text());           // ID
        ui->alDirectID->setText(ui->tableWidget->item(row, 1)->text());
        ui->alName->setText(ui->tableWidget->item(row, 2)->text());         // Описание
        ui->alDirect->setText(ui->tableWidget->item(row, 3)->text());       // Напавленность
        ui->alOtd->setText(ui->tableWidget->item(row, 4)->text());          // Отдел
        ui->alDescript->setText(ui->tableWidget->item(row, 5)->text());     // Описание

        // Отрисовка таблички!!!
        // Номер + Год обучения

        QString str = "SELECT `ID`, `Номер`, `Год обучения` FROM Группы WHERE `ID объединения` = " + ui->alID->text() + ";";

        QSqlQuery query;
        query.exec(str);
        drawRows(query, ui->groupInAl, false);
    }

    if (*currentTable == "Группы")
    {
        // SELECT `ID`, `ID объединения`, `ID преподавателя`, `Номер`, `Год обучения`, `Объединение`,
        // `Фамилия преподавателя`, `Имя преподавателя`, `Отчество преподавателя` FROM Группы;
        ui->groupID->setText(ui->tableWidget->item(row, 0)->text());    // ID
        ui->groupAssID->setText(ui->tableWidget->item(row, 1)->text());
        ui->groupTeachID->setText(ui->tableWidget->item(row, 2)->text());
        ui->groupNum->setText(ui->tableWidget->item(row, 3)->text());
        ui->groupYear->setValue(ui->tableWidget->item(row, 4)->text().toInt());
        ui->groupAss->setText(ui->tableWidget->item(row, 5)->text());
        ui->groupTeach->setText(ui->tableWidget->item(row, 6)->text() + " " + ui->tableWidget->item(row, 7)->text()[0] + "." + ui->tableWidget->item(row, 8)->text()[0] + ".");

        ui->addStudInGroup->setEnabled(true);
        ui->removeStudToGroup->setEnabled(true);

        // Отрисовка таблички!!!
        QString str = "SELECT `ID Учащегося`, `Фамилия`, `Имя`, `Отчество`, `Телефон`, `e-mail` FROM Состав_групп WHERE `ID Группы` = " + ui->groupID->text() + ";";

        QSqlQuery query;
        query.exec(str);
        drawRows(query, ui->studsInGroupe, false);
    }

    if (*currentTable == "Направленности")
    {
        ui->directID->setText(ui->tableWidget->item(row, 0)->text());    // ID
        ui->directName->setText(ui->tableWidget->item(row, 1)->text());    // Название

        // Отрисовка таблички!!! - Названия объединений

        QString str = "SELECT `ID`, `Название` FROM Объединение WHERE `ID Направленности` = " + ui->directID->text() + ";";

        QSqlQuery query;
        query.exec(str);
        drawRows(query, ui->alInDirect, false);
    }

}

// ============================================================
// ============= Очистка формы с подробной инфой ==============
// ============================================================

void MainWindow::clearMoreInfoForm()
{
    if (*currentTable == "Учащиеся")
    {
        // Line Edit
        ui->studID->clear();
        ui->studSurname->clear();
        ui->studName->clear();
        ui->studPatr->clear();
        ui->studNumDoc->clear();
        ui->areaSchools->clear();   // Район школы
        ui->school->clear();        // Школа
        ui->grade->clear();         // Класс
        ui->phone->clear();         // Телефон
        ui->email->clear();         // email

        ui->studBDay->setCurrentIndex(0);
        ui->studBMon->setCurrentIndex(0);
        ui->studBYear->setValue(2000);

        ui->studInDay->setCurrentIndex(0);
        ui->studInMon->setCurrentIndex(0);
        ui->studInYear->setValue(2000);

        ui->studInDay->setCurrentIndex(0);
        ui->studInMon->setCurrentIndex(0);
        ui->studInYear->setValue(2000);


        // Combo Box
        ui->studDoc->setCurrentIndex(0);
        ui->studGender->setCurrentIndex(0);
        ui->eduForm->setCurrentIndex(0); // Форма обучения

        // Text Edit
        ui->parents->clear();       // Родители
        ui->address->clear();       // Адрес
        ui->studComments->clear();

        // Check Box
        ui->accountInPolice->setChecked(false);
        ui->incompleteFamily->setChecked(false);
        ui->invalid->setChecked(false);
        ui->large->setChecked(false);
        ui->lowIncome->setChecked(false);
        ui->migrants->setChecked(false);
        ui->orphan->setChecked(false);
        ui->weackHealth->setChecked(false);

    }

    if (*currentTable == "Преподаватели")
    {
        ui->teachID->clear();        // ID
        ui->teachSurname->clear();   // Фамилия
        ui->teachName->clear();      // Имя
        ui->teachPatr->clear();      // Отчество
        ui->teachNumPass->clear();   // Номер паспорта
        ui->teachOtd->clear();       // Отдел
    }

    if (*currentTable == "Объединения")
    {
        ui->alID->clear();           // ID
        ui->alName->clear();         // Название
        ui->alDirect->clear();      // Напавленность
        ui->alDirectID->clear();
        ui->alOtd->clear();          // Отдел
        ui->alDescript->clear();     // Описание
    }

    if (*currentTable == "Группы")
    {
        ui->groupID->clear();
        ui->groupAss->clear();
        ui->groupTeach->clear();
        ui->groupNum->clear();
        ui->groupTeachID->clear();
        ui->groupAssID->clear();
        ui->groupYear->setValue(0);

        ui->addStudInGroup->setEnabled(false);
        ui->removeStudToGroup->setEnabled(false);

        int rowCount = ui->studsInGroupe->rowCount();
        for(int i = 0; i < rowCount; i++)
                 ui->studsInGroupe->removeRow(0);


    }

    if (*currentTable == "Направленности")
    {

        ui->directID->clear();
        ui->directName->clear();

        int rowCount = ui->alInDirect->rowCount();
        for(int i = 0; i < rowCount; i++)
                 ui->alInDirect->removeRow(0);
    }
}


// ----------------------------------------------------------------------------------------------------
// ------------------------------- Реакция на действия пользователя -----------------------------------
// ----------------------------------------------------------------------------------------------------

// ============================================================
// ================ Запуск глобального поиска =================
// ============================================================

void MainWindow::globalSearch()
{
    SearchDialog *dialog;
    dialog = new SearchDialog();
    connect (dialog, SIGNAL(signalQuery(QString)), this, SLOT(querySlot(QString)));
    dialog->exec();
}

void MainWindow::querySlot(QString textQuery)
{
    currentTable->clear();
    currentTable->append("GlobalSearch");

    lastSelect->clear();
    lastSelect->append(textQuery);

    QSqlQuery query;
    query.exec(textQuery);
    drawHeaders(query, ui->tableWidget, true, searchBox);
    drawRows(query, ui->tableWidget, true);
}

// ============================================================
// =============== Выбрана таблица из дерева ==================
// ============================================================

void MainWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    QString str = item->text(column);
    showTable(str);
}

// ============================================================
// ========= Слот для сигнала Выбрана ячейка таблицы ==========
// ============================================================

void MainWindow::on_tableWidget_cellClicked(int row, int column)
{
    ui->tableWidget->setSortingEnabled(false); // Временно запрещаем сортировку
    if (row < ui->tableWidget->rowCount()-1)
        showMoreInfo(row);
    else
        clearMoreInfoForm();
    ui->tableWidget->setSortingEnabled(true);
}

// ============================================================
// ========= Слот для сигнала Выбрана строка таблицы ==========
// ============================================================

void MainWindow::rowClicked(int row)
{
    ui->tableWidget->setSortingEnabled(false); // Временно запрещаем сортировку
    if (row < ui->tableWidget->rowCount()-1)
        showMoreInfo(row);
    else
        clearMoreInfoForm();
    ui->tableWidget->setSortingEnabled(true);
}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------- Экспорт --------------------------------------------
// ----------------------------------------------------------------------------------------------------

// ============================================================
// ==================== Экспорт в Exсel =======================
// ============================================================

void MainWindow::exportInExel()
{
    // Открываем QFileDialog
    QFileDialog fileDialog;
    QString fileName = fileDialog.getOpenFileName(0, tr("Экспортировать в..."), "", "*.xls *.xlsx");
    if (!fileName.isEmpty())
    {
        ui->lblStatus->setText(tr("Экспорт..."));

        // https://wiki.qt.io/Using_ActiveX_Object_in_QT
        // http://wiki.crossplatform.ru/index.php/%D0%A0%D0%B0%D0%B1%D0%BE%D1%82%D0%B0_%D1%81_MS_Office_%D1%81_%D0%BF%D0%BE%D0%BC%D0%BE%D1%89%D1%8C%D1%8E_ActiveQt

        QAxObject* excel = new QAxObject("Excel.Application", 0);
        QAxObject* workbooks = excel->querySubObject("Workbooks");
        QAxObject* workbook = workbooks->querySubObject("Open(const QString&)", fileName);
        QAxObject* sheets = workbook->querySubObject("Worksheets");

        // Вставка значения в отдельную ячейку

        QAxObject* StatSheet = sheets->querySubObject("Item( int )", 1);

       for (int col = 1; col < ui->tableWidget->columnCount(); col++)        // Запись заголовков
        {
            // получение указателя на ячейку [row][col] ((!)нумерация с единицы)
            QAxObject* cell = StatSheet->querySubObject("Cells(QVariant,QVariant)", 1, col);
            // вставка значения переменной в полученную ячейку
            cell->setProperty("Value", QVariant(ui->tableWidget->horizontalHeaderItem(col)->text()));
            // освобождение памяти
            delete cell;
        }

        for (int row = 1; row < ui->tableWidget->rowCount(); row++)
        {
            for (int col = 1; col < ui->tableWidget->columnCount(); col++)
            {
                // получение указателя на ячейку [row][col] ((!)нумерация с единицы)
                QAxObject* cell = StatSheet->querySubObject("Cells(QVariant,QVariant)", row+1, col);
                // вставка значения переменной в полученную ячейку
                cell->setProperty("Value", QVariant(ui->tableWidget->item(row-1, col)->text()));
                // освобождение памяти
                delete cell;
            }
        }

        excel->dynamicCall("Save()");       // Сохраняем - в примерах почему-то этого нет, но надо
        workbook->dynamicCall("Close()");   // Закрываем
        excel->dynamicCall("Quit()");       // Выходим

        delete StatSheet;
        delete sheets;
        delete workbook;
        delete workbooks;
        delete excel;

        ui->lblStatus->setText(tr("Экспорт завершён"));

    }
}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

// ============================================================
// =============== Добавление учащихся в группы ===============
// ============================================================
void MainWindow::on_addStudInGroup_clicked()
{
    dbDialog->setType("Stud");

    QTableWidget *wgt = dbDialog->letTable();
    QSqlQuery query;
    query.exec("SELECT `ID`, `Фамилия`, `Имя`, `Отчество`, `Тип документа`, `Номер документа`, `Пол`, `Год рождения` FROM Учащиеся;");
    drawHeaders(query, wgt, true, dbDialog->letSearchBox());
    drawRows(query, wgt, false);
    wgt->setColumnHidden(0, true);

    if (dbDialog->exec() == QDialog::Accepted)
    {
        // Здесь - может быть выделено несколько строк!
        int row = wgt->currentRow();
        if (row > -1) // Если есть выделенные строки
        {
            QString strQuery;
            QSqlQuery query;
            for (QTableWidgetSelectionRange selectionRange : wgt->selectedRanges())
            {
                if (selectionRange.rowCount() > 0)
                    for (int row = selectionRange.topRow(); row <= selectionRange.bottomRow(); row++)
                    {
                        strQuery.append("INSERT INTO Нагрузка(`ID учащегося`, `ID группы`) VALUES(" + wgt->item(row, 0)->text() + ", " + ui->groupID->text() + "); ");
                        qDebug() << wgt->item(row, 0)->text();
                        query.exec(strQuery);
                        qDebug() << strQuery;
                        qDebug() << query.lastError().text();
                        strQuery.clear();
                    }
            }
            strQuery.append("SELECT `ID Учащегося`, `Фамилия`, `Имя`, `Отчество`, `Телефон`, `e-mail` FROM Состав_групп WHERE `ID Группы` = " + ui->groupID->text() + ";");
            qDebug() << strQuery;
            query.exec(strQuery);
            drawRows(query, ui->studsInGroupe, false);

        }
    }
}

// ============================================================
// ============ Связь между группой и объединением ============
// ============================================================

void MainWindow::on_addAssForGroup_clicked()
{
    dbDialog->setType("Ass");

    QTableWidget *wgt = dbDialog->letTable();
    QSqlQuery query;
    query.exec("SELECT `ID`, `Название`, `Отдел`, `Описание` FROM Объединения;");
    drawHeaders(query, wgt, true, dbDialog->letSearchBox());
    drawRows(query, wgt, false);
    wgt->setColumnHidden(0, true);

    if (dbDialog->exec() == QDialog::Accepted)
    {
        int row = wgt->currentRow();
        if (row > -1)
        {
            ui->groupAssID->setText(wgt->item(row, 0)->text());
            ui->groupAss->setText(wgt->item(row, 1)->text());
        }
    }
}


// ============================================================
// ========== Связь между группой и преподавателем ============
// ============================================================
void MainWindow::on_addTeachForGroup_clicked()
{
    dbDialog->setType("Teach");

    QTableWidget *wgt = dbDialog->letTable();
    QSqlQuery query;
    query.exec("SELECT `ID`, `Фамилия`, `Имя`, `Отчество`, `Отдел` FROM Преподаватели;");
    drawHeaders(query, wgt, true, dbDialog->letSearchBox());
    drawRows(query, wgt, false);
    wgt->setColumnHidden(0, true);

    if (dbDialog->exec() == QDialog::Accepted)
    {
        int row = wgt->currentRow();
        if (row > -1)
        {
            ui->groupTeachID->setText(wgt->item(row, 0)->text());
            ui->groupTeach->setText(wgt->item(row, 1)->text() + " " + wgt->item(row, 2)->text()[0] + "." + wgt->item(row, 3)->text()[0] + ".");
        }
    }
}

void MainWindow::on_addDirectInAl_clicked()
{
    dbDialog->setType("Direct");

    QTableWidget *wgt = dbDialog->letTable();
    QSqlQuery query;
    query.exec("SELECT `ID`, `Название` FROM Направленности;");
    drawHeaders(query, wgt, true, dbDialog->letSearchBox());
    drawRows(query, wgt, false);
    wgt->setColumnHidden(0, true);

    if (dbDialog->exec() == QDialog::Accepted)
    {
        int row = wgt->currentRow();
        if (row > -1)
        {
            ui->alDirectID->setText(wgt->item(row, 0)->text());
            ui->alDirect->setText(wgt->item(row, 1)->text());
        }
    }
}



void MainWindow::showTempTable()
{
    dbDialog->setType("tempDB");
    QTableWidget *wgt = dbDialog->letTable();
    QSqlQuery query(tempDB);
    query.exec("SELECT Запись.`Объединение` `Объединение`, Учащийся.`Фамилия` `Фамилия`, Учащийся.`Имя` `Имя`, Учащийся.`Отчество` `Отчество`, Учащийся.`Тип документа` `Тип документа`, Учащийся.`Номер документа` `Номер документа`, Учащийся.`Пол` `Пол`, Учащийся.`Год рождения` `Год рождения`, Учащийся.`Район школы` `Район школы`, Учащийся.`Школа` `Школа`, `Класс` `Класс`, Учащийся.`Родители` `Родители`, Учащийся.`Домашний адрес` `Домашний адрес`, Учащийся.`Телефон` `Телефон`, Учащийся.`e-mail` `e-mail` FROM Учащийся, Запись WHERE Учащийся.`Тип документа` = Запись.`Тип документа` AND Учащийся.`Номер документа` = Запись.`Номер документа`;");
    qDebug()<<query.lastError().text();
    drawHeaders(query, wgt, true, dbDialog->letSearchBox());
    drawRows(query, wgt, false);
    dbDialog->show();
}


void MainWindow::querySlot(QTableWidget* tableWidget, QString strQuery, bool mainDB)
{
    QSqlQuery query;
    if (!mainDB)
    {
        query = QSqlQuery(tempDB);
    }

    query.exec(strQuery);
    drawRows(query, tableWidget, false);
}

void MainWindow::on_removeStudToGroup_clicked()
{
    int row = ui->studsInGroupe->currentRow();
    if (row > -1) // Если есть выделенные строки
    {
        QString strQuery;
        QSqlQuery query;
        for (QTableWidgetSelectionRange selectionRange : ui->studsInGroupe->selectedRanges())
        {
            if (selectionRange.rowCount() > 0)
                for (int row = selectionRange.topRow(); row <= selectionRange.bottomRow(); row++)
                {
                    strQuery.append("DELETE FROM Нагрузка WHERE `ID учащегося` = " + ui->studsInGroupe->item(row, 0)->text() + " AND `ID группы` = "  + ui->groupID->text() + ";");
                    qDebug() << ui->studsInGroupe->item(row, 0)->text();
                    query.exec(strQuery);
                    qDebug() << strQuery;
                    qDebug() << query.lastError().text();
                    strQuery.clear();
                }
        }
        strQuery.append("SELECT `ID Учащегося`, `Фамилия`, `Имя`, `Отчество`, `Телефон`, `e-mail` FROM Состав_групп WHERE `ID Группы` = " + ui->groupID->text() + ";");
        qDebug() << strQuery;
        query.exec(strQuery);
        drawRows(query, ui->studsInGroupe, false);
    }
}
