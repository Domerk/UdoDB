#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowState(Qt::WindowMaximized); // Главное окно разворачивается на весь экран
    ui->splitter->setSizes(QList <int> () << 100 << 750 << 200);

    rowCount = 0;
    columnCount = 0;
    lastSelect = new QString();
    currentTable = new QString();

    connectDialog = new ConnectionDialog();
    connect(connectDialog, SIGNAL(connectReconfig()), this, SLOT(connectReconfigSlot()));
    connect(ui->tableWidget->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(rowClicked(int)));

    // Регулярное выражение для проверки имён, фамилий и отчеств.
    names = new QRegularExpression("^[А-ЯЁ]{1}[а-яё]*(-[А-ЯЁ]{1}[а-яё]*)?$");

    // ------------------------- Всякая красота ----------------------------

    this->setWindowTitle(tr("Тут будет содержательное название"));

    ui->alID->hide();
    ui->studID->hide();
    ui->teachID->hide();
    ui->directID->hide();
    ui->groupID->hide();

    ui->treeWidget->setColumnCount(1);

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

    queryStud.append("SELECT `ID`, `Фамилия`, `Имя`, `Отчество`, `Тип документа`, `Номер документа`, `Пол`, `Год рождения`, ");
    queryStud.append("`Район школы`, `Школа`, `Класс`, `Родители`, `Домашний адрес`, `Телефон`, `e-mail`, `Дата заявления`, `Форма обучения`, ");
    queryStud.append("`Когда выбыл`, `С ослабленным здоровьем`, `Сирота`, `Инвалид`, `На учёте в полиции`, `Многодетная семья`, ");
    queryStud.append("`Неполная семья`, `Малообеспеченная семья`, `Мигранты`, `Примечания` FROM Учащиеся;");

    queryTeach.append("SELECT `ID`, `Фамилия`, `Имя`, `Отчество`, `Паспорт`, `Отдел` FROM Преподаватели;");

    // !!!
    // Вот здесь хорошо бы заменить обращения к таблицам на обращения к представлениям:
    // !!!

    queryAllians.append("SELECT `ID`, `ID Направленности`, `Название`, `Отдел`, `Описание` FROM Объединения;");

    queryDirection.append("SELECT `ID`, `Название` FROM Направленности;");

    queryGroup.append("SELECT `ID`, `ID объединения`, `ID преподавателя`, `Номер`, `Год обучения` FROM Группы;");

    // -------------------- Маски для скрытия колонок в таблицах ---------------

    studTableMask.append(true);
    for (int i = 1; i<11; i++)
        studTableMask.append(false);
    for (int i = 11; i<27; i++)
        studTableMask.append(true);

    teachTableMask.append(true);
    for (int i = 1; i<6; i++)
        teachTableMask.append(false);

    alliansTableMask.append(true);
    for (int i = 1; i<5; i++)
        alliansTableMask.append(false);

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

    delete connectDialog;
    delete lastSelect;
    delete currentTable;
    delete ui;
}

// ============================================================
// ============== Установка соединения с базой ================
// ============================================================

bool MainWindow::connectDB(QString nameDB)
{
    if (myDB.isOpen())
    {
        myDB.close();
    }
    else
    {
        myDB = QSqlDatabase::addDatabase("QSQLITE");    // Указываем СУБД
    }

    QSettings settings ("Kctt", "KcttDB");
    settings.beginGroup(nameDB);
    myDB.setHostName(settings.value("hostname", "localhost").toString());
    myDB.setDatabaseName(settings.value("dbname", "kcttDB").toString());
    myDB.setUserName(settings.value("username").toString());
    myDB.setPassword(settings.value("password").toString());
    settings.endGroup();


    if (myDB.open())                            // Открываем соединение
    {
        ui->lblStatus->setText(tr("Соединение установлено")); // Выводим сообщение
        return true;                 // Возвращаем true
    }
    else
    {
        ui->lblStatus->setText(tr("Ошибка соединения: соединение не установлено"));
    }

    return false;

    /*
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
    return false; */
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

    if (table == "Общее")
    {
        return;
    }

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
        currentMask = studTableMask;
    }

    if (table == "Преподаватели")
    {
        query.exec(queryTeach);
        lastSelect->append(queryTeach);
        ui->stackedWidget->setCurrentIndex(1);
        currentMask = teachTableMask;
    }

    if (table == "Объединения")
    {
        query.exec(queryAllians);
        lastSelect->append(queryAllians);
        ui->stackedWidget->setCurrentIndex(0);
        currentMask = alliansTableMask;
    }

    if (table == "Направленности")
    {
        query.exec(queryDirection);
        lastSelect->append(queryDirection);
        ui->stackedWidget->setCurrentIndex(3);

    }

    if (table == "Группы")
    {
        query.exec(queryGroup);
        lastSelect->append(queryGroup);
        ui->stackedWidget->setCurrentIndex(4);

    }

    // Сохраняем инфу о текущей таблице
    currentTable->append(table);

    // Отображаем заголовки и строки таблицы
    drawHeaders(query);
    drawRows(query);
    hideColumnsFromMask(currentMask);

    return;
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
    searchBox->clear();

    if (qsl.size() > 1)
        qsl.removeFirst();          // Удаляем 0й элемент (ID)
    searchBox->addItems(qsl);   // Задаём комбобоксу поиска
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

    ui->tableWidget->setSortingEnabled(false); // Временно запрещаем сортировку

    while (query.next())    // Пока есть результаты запроса
        {
            ui->tableWidget->insertRow(rowCount);   // Добавляем строку в конец
            for (int i = 0; i<columnCount; i++)     // Для всех полей таблицы
            {
                // Создаём ячейку в текущем поле текущей строки и заносим туда инфу

                if (query.value(i).toString() == "true")
                {
                    ui->tableWidget->setItem(rowCount, i, new QTableWidgetItem("Да"));
                    continue;
                }
                if (query.value(i).toString() == "false")
                {
                    ui->tableWidget->setItem(rowCount, i, new QTableWidgetItem("Нет"));
                    continue;
                }

                ui->tableWidget->setItem(rowCount, i, new QTableWidgetItem(query.value(i).toString()));
            }
            rowCount ++;    // Увеличиваем количество строк
        }

    ui->tableWidget->insertRow(rowCount); // В конце добавляем пустую строку
    ui->tableWidget->setSortingEnabled(true); // Разрешаем сортировку
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
    //vct.append(true);

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

        if (*currentTable == "Учащиеся")
        {
            studTableMask = currentMask;
        }

        if (*currentTable == "Преподаватели")
        {
            teachTableMask = studTableMask;
        }

        if (*currentTable == "Объединения")
        {
            alliansTableMask = studTableMask;
        }


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
                    QString str = "DELETE FROM " + *currentTable + " WHERE ID = " + ui->tableWidget->item(ui->tableWidget->verticalHeader()->currentIndex().row(), 0)->text() + " ;";
                    query.exec(str);        // Выполняем запрос
                    repeatLastSelect();     // Повторяеи последний Select
                    clearMoreInfoForm();    // Чистим поле с подробностями

                    ui->lblStatus->setText(str);
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
            QString direct = ui->alDirect->currentText();                  // Напавленность
            QString otd = ui->alOtd->text().simplified().replace(QRegularExpression("-{2,}"), "-");            // Отдел
            QString desc = ui->alDescript->toPlainText().replace(QRegularExpression("-{2,}"), "-");     // Описание

            if (name.isEmpty() || direct.isEmpty() || otd.isEmpty())
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
                strQuery = "INSERT INTO Объединения (Название, Направленность, Отдел, Описание) VALUES ('" + name + "', '" + direct + "', '" + otd  + "', '" + desc + "');";
                clearMoreInfoForm();
            }
            else
            {
                strQuery = "UPDATE Объединения SET Название = '" + name + "', Направленность = '" + direct + "', Отдел = '" + otd + "', Описание = '" + desc + "' WHERE ID = " + id + ";";
            }

            drawTree(); // Перерисовываем дерево
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
                strQuery = "INSERT INTO Преподаватели (Имя, Фамилия, Отчество, Паспорт, Отдел) VALUES ('" + name + "', '" + surname + "', '" + patrname + "', '" + numpass + "', '" + otd + "');";
                clearMoreInfoForm();
            }
            else
            {
                strQuery = "UPDATE Преподаватели SET Имя = '" + name + "', Фамилия = '" + surname + "', Отчество = '" + patrname + "', Паспорт = '" + numpass + "', Отдел = '" + otd + "' WHERE ID = " + id + ";";
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


            QString birthday = ui->studBirthday->text();    // Год рождения
            QString admiss = ui->admissDate->text();        // Дата подачи заявления
            QString out = ui->outDate->text();              // Когда выбыл

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
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // ВОТ ЗДЕСЬ
        // Добавить проверку на то, может ли поиск осуществляться таким образом !!!!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        // + Для учащегося вставить фиксу на логические поля

        // + Убрать регистрозависимость

        QString *newSelect = new QString();

        if (*currentTable == "Учащиеся")
        {
            if (searchBox->currentText() == "С ослабленным здоровьем" || searchBox->currentText() ==  "Сирота"
                    || searchBox->currentText() == "Инвалид" || searchBox->currentText() == "На учёте в полиции"
                    || searchBox->currentText() == "Многодетная семья" || searchBox->currentText() == "Неполная семья"
                    || searchBox->currentText() == "Малообеспеченная семья" || searchBox->currentText() == "Мигранты")
            {
                newSelect->append(queryStud.replace(";", " ") + " WHERE `" + searchBox->currentText());
                if (searchText->toLower() == "да")
                    newSelect->append("` = 'true';");
                else
                    newSelect->append("` = 'false';");
            }
            else
            {
                newSelect->append(queryStud.replace(";", " ") + " WHERE `" + searchBox->currentText() + "` LIKE '%" + *searchText + "%';");
            }
        }

        if (*currentTable == "Преподаватели")
            newSelect->append(queryTeach.replace(";", " ") + " WHERE `" + searchBox->currentText() + "` LIKE '%" + *searchText + "%';");

        if (*currentTable == "Объединения")
            newSelect->append(queryAllians.replace(";", " ") + " WHERE `" + searchBox->currentText() + "` LIKE '%" + *searchText + "%';");

        qDebug() << *newSelect;

        QSqlQuery query;
        query.exec(*newSelect);

        qDebug() << query.lastError();

        drawRows(query);
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
   ui->alDirect->addItems(*directions);

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

        ui->studBirthday->setText(ui->tableWidget->item(row, 7)->text());   // Год рождения

        ui->areaSchools->setText(ui->tableWidget->item(row, 8)->text());    // Район школы
        ui->school->setText(ui->tableWidget->item(row, 9)->text());         // Школа
        ui->grade->setText(ui->tableWidget->item(row, 10)->text());         // Класс
        ui->parents->setText(ui->tableWidget->item(row, 11)->text());       // Родители
        ui->address->setText(ui->tableWidget->item(row, 12)->text());       // Адрес
        ui->phone->setText(ui->tableWidget->item(row, 13)->text());         // Телефон
        ui->email->setText(ui->tableWidget->item(row, 14)->text());         // email

        ui->admissDate->setText(ui->tableWidget->item(row, 15)->text());    // Дата подачи заявления

        ui->eduForm->setCurrentText(ui->tableWidget->item(row, 16)->text()); // Форма обучения

        ui->outDate->setText(ui->tableWidget->item(row, 17)->text());     // Когда выбыл



        // Чекбоксы - вероятно, имеет смысл предварительно засунуть их в вектор!!!

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
    }

    if (*currentTable == "Объединения")
    {
        ui->alID->setText(ui->tableWidget->item(row, 0)->text());           // ID
        ui->alName->setText(ui->tableWidget->item(row, 1)->text());         // Описание
        ui->alDirect->setCurrentText(ui->tableWidget->item(row, 2)->text());       // Напавленность
        ui->alOtd->setText(ui->tableWidget->item(row, 3)->text());          // Отдел
        ui->alDescript->setText(ui->tableWidget->item(row, 4)->text());     // Описание

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

        ui->studBirthday->clear();  // Год рождения
        ui->admissDate->clear();    // Дата подачи заявления
        ui->outDate->clear();       // Когда выбыл

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
        ui->alDirect->setCurrentIndex(0);       // Напавленность
        ui->alOtd->clear();          // Отдел
        ui->alDescript->clear();     // Описание
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
    dialog->exec();
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
    if (row < rowCount)
    {
        showMoreInfo(row);
    }
    else
    {
        clearMoreInfoForm();
    }
}

// ============================================================
// ========= Слот для сигнала Выбрана строка таблицы ==========
// ============================================================

void MainWindow::rowClicked(int row)
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
