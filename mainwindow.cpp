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
    simpleSearchDisplayed = false;

    connectDialog = new ConnectionDialog(this);
    connect(connectDialog, SIGNAL(connectReconfig()), this, SLOT(connectReconfigSlot()));
    connect(ui->tableWidget->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(selectedRowChanged(QModelIndex,QModelIndex)));

    dbDialog = new TableOpt(this);
    dbDialog->setType("tempDB");
    connect(dbDialog, SIGNAL(signalQuery(QTableWidget*,QString,bool)), this, SLOT(querySlot(QTableWidget*,QString,bool)));
    connect(dbDialog, SIGNAL(signalQueries(QStringList,bool)), this, SLOT(queriesSlot(QStringList,bool)));

    helpDialog = new Help(this);

    bases = new QStringList();
    bases->append("Учащиеся");
    bases->append("Преподаватели");
    bases->append("Направленности");
    bases->append("Объединения");
    bases->append("Группы");

    // Регулярное выражение для проверки имён, фамилий и отчеств.
    names = new QRegularExpression("^[А-ЯЁ]{1}[а-яё]*(-[А-ЯЁ]{1}[а-яё]*)?$");

    // ------------------------- Всякая красота ----------------------------

    // Скрываем служебные поля формы, содержащие ID
    ui->alID->hide();
    ui->studID->hide();
    ui->teachID->hide();
    ui->directID->hide();
    ui->groupID->hide();
    ui->groupTeachID->hide();
    ui->groupAssID->hide();
    ui->alDirectID->hide();

    // Задаём количество столбцов в дереве
    ui->treeWidget->setColumnCount(1);

    // Задаём иконки для конопок "Добавить в группу" и "Удалить из группы"
    ui->addStudInGroup->setIcon(QIcon(":/icons/Icons/add"));
    ui->removeStudToGroup->setIcon(QIcon(":/icons/Icons/remove"));

    // Скрываем служебные поля вспомогательных таблиц
    ui->groupInAl->setColumnHidden(0, true);
    ui->groupInTeach->setColumnHidden(0, true);
    ui->studsInGroupe->setColumnHidden(0, true);
    ui->alInDirect->setColumnHidden(0, true);

    ui->studGroupTable->setColumnHidden(0, true);
    ui->studGroupTable->setColumnHidden(1, true);

    // Получаем и устанавливаем заголовок главного окна
    QSettings settings ("Other/config.ini", QSettings::IniFormat);
    settings.beginGroup("Settings");
    QString windowTitle = QString::fromUtf8(settings.value("windowtitle", QString()).toString().toLatin1());
    if (windowTitle.isNull())
        windowTitle = QString("База данных учащихся УДО");
    this->setWindowTitle("UdoDB - " + windowTitle);
    settings.endGroup();

    this->setWindowIcon(QIcon(":/icons/Icons/udod"));

    // -------------------------------- Меню --------------------------------

    // Файл
    connect (ui->actionConnect, SIGNAL(triggered()), connectDialog, SLOT(exec()));
    connect (ui->actionHtml, SIGNAL(triggered()), this, SLOT(exportInHtml()));
    connect(ui->exit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionExcel, SIGNAL(triggered()), this, SLOT(exportInExcel()));

    // Таблица
    connect(ui->actionRefreshTab, SIGNAL(triggered()), this, SLOT(refreshTable()));
    connect(ui->actionRepeatLastSelect, SIGNAL(triggered()), this, SLOT(repeatLastSelect()));
    connect(ui->actionForMask, SIGNAL(triggered()), this, SLOT(changeTableMask()));
    connect(ui->actionSearch, SIGNAL(triggered()), this, SLOT(globalSearch()));

    ui->actionForMask->setIcon(QIcon(":/icons/Icons/options.png"));
    ui->actionSearch->setIcon(QIcon(":/icons/Icons/optool.png"));

    // Запись
    connect(ui->actionNewStr, SIGNAL(triggered()), this, SLOT(clearMoreInfoForm()));
    connect(ui->actionImport, SIGNAL(triggered()), this, SLOT(showTempTable()));
    connect(ui->actionDeleteStr, SIGNAL(triggered()), this, SLOT(deleteThis()));

    ui->actionNewStr->setIcon(QIcon(":/icons/Icons/new.png"));
    ui->actionImport->setIcon(QIcon(":/icons/Icons/import.png"));
    ui->actionDeleteStr->setIcon(QIcon(":/icons/Icons/delete.png"));

    // Справка
    connect(ui->actionHelp, SIGNAL(triggered()), helpDialog, SLOT(show()));
    connect(ui->actionQt, SIGNAL(triggered()), this, SLOT(showQtInfo()));
    connect(ui->actionLicense, SIGNAL(triggered()), this, SLOT(showLicense()));
    connect(ui->actionProgram, SIGNAL(triggered()), this, SLOT(showProgramInfo()));

    // --------------------------- Main ToolBar ----------------------------

    // Иконки: http://www.flaticon.com/packs/web-application-ui/4

    ui->mainToolBar->addAction(QIcon(":/icons/Icons/new.png"),tr("Новая запись"), this, SLOT(clearMoreInfoForm()));
    ui->mainToolBar->addAction(QIcon(":/icons/Icons/import.png"),tr("Импорт из временной базы"), this, SLOT(showTempTable()));
    ui->mainToolBar->addAction(QIcon(":/icons/Icons/delete.png"),tr("Удалить запись"), this, SLOT(deleteThis()));

    ui->mainToolBar->addAction(QIcon(":/icons/Icons/options.png"),tr("Скрыть/Показать поля"), this, SLOT(changeTableMask()));
    ui->mainToolBar->addAction(QIcon(":/icons/Icons/optool.png"),tr("Расширенный поиск"), this, SLOT(globalSearch()));

    // ------------------------- Search ToolBar ----------------------------

    searchBox = new QComboBox();
    searchEdit = new QLineEdit();
    searchEdit->setFixedWidth(250);
    searchBox->setLayoutDirection(Qt::LeftToRight);     // Поскольку сам ТулБар RigthToLeft, принудительно задаём комбобоку нормальный вид
    ui->searchToolBar->addAction(QIcon(":/icons/Icons/search.png"),tr("Поиск"), this, SLOT(simpleSearch()));
    ui->searchToolBar->addWidget(searchEdit);
    ui->searchToolBar->addWidget(searchBox);
    ui->searchToolBar->actions()[SearchToolButton::Start]->setDisabled(true);

    connect(searchEdit, SIGNAL(textChanged(QString)), this, SLOT(setSearchActive()));
    connect(searchEdit, SIGNAL(returnPressed()), this, SLOT(simpleSearch()));

    // ------------------ Запросы на вывод основных таблиц -----------------

    Info inf;

    inf.query.append("SELECT `ID`, `Фамилия`, `Имя`, `Отчество`, `Тип документа`, `Номер документа`, `Пол`, `Дата рождения`, `Район школы`, `Школа`, `Класс`, `Родители`, `Домашний адрес`, `Телефон`, `e-mail`, `Дата заявления`, `Форма обучения`,`Когда выбыл`, `С ослабленным здоровьем`, `Сирота`, `Инвалид`, `На учёте в полиции`, `Многодетная семья`, `Неполная семья`, `Малообеспеченная семья`, `Мигранты`, `Примечания` FROM Учащиеся;");
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

    inf.query = "SELECT `ID`, `ID направленности`, `Название`, `Направленность`, `Отдел`, `Описание` FROM Объединения;";
    inf.index = 0;
    inf.mask.append(true);
    inf.mask.append(true);
    for (int i = 2; i<6; i++)
        inf.mask.append(false);
    infoMap.insert("Объединения", inf);

    inf.mask.clear();

    inf.query = "SELECT `ID`, `Название` FROM Направленности;";
    inf.index = 3;
    inf.mask.append(true);
    inf.mask.append(false);
    infoMap.insert("Направленности", inf);

    inf.mask.clear();

    inf.query = "SELECT `ID`, `ID объединения`, `ID преподавателя`, `Номер`, `Год обучения`, `Объединение`, `Фамилия преподавателя`, `Имя преподавателя`, `Отчество преподавателя` FROM Группы;";
    inf.index = 4;
    for (int i = 0; i<3; i++)
        inf.mask.append(true);
    for (int i = 3; i<9; i++)
        inf.mask.append(false);
    infoMap.insert("Группы", inf);

    // ----------------------------- DataBase ------------------------------

    if (connectDB())
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

    delete bases;
    delete connectDialog;
    delete lastSelect;
    delete currentTable;
    delete dbDialog;
    delete helpDialog;
    delete ui;
}

// ============================================================
// ============== Установка соединения с базой ================
// ============================================================

bool MainWindow::connectDB()
{
    ui->lblStatus->setText(tr("Соединение..."));
    qApp->processEvents();

    // Подключение к основной базе

    if (myDB.isOpen())
        myDB.close();
    else
        // QSQLITE || QMYSQL
        myDB = QSqlDatabase::addDatabase("QMYSQL");    // Указываем СУБД, имени соединения нет - соединение по умолчанию

    QSettings settings ("Other/config.ini", QSettings::IniFormat);
    settings.beginGroup("MainDB");
    myDB.setHostName(settings.value("hostname", "localhost").toString());
    myDB.setDatabaseName(settings.value("dbname", "kcttDB").toString());
    myDB.setPort(settings.value("port").toInt());
    myDB.setUserName(settings.value("username").toString());
    myDB.setPassword(settings.value("password").toString());
    settings.endGroup();

    // Подключение к временной базе базе
    if (tempDB.isOpen())
        tempDB.close();
    else
        tempDB = QSqlDatabase::addDatabase("QMYSQL", "tempDB");    // Указываем СУБД

    settings.beginGroup("TempDB");
    tempDB.setHostName(settings.value("hostname", "localhost").toString());
    tempDB.setDatabaseName(settings.value("dbname", "kcttTempDB").toString());
    tempDB.setPort(settings.value("port").toInt());
    tempDB.setUserName(settings.value("username").toString());
    tempDB.setPassword(settings.value("password").toString());
    settings.endGroup();

    QString status;
    bool myDBIsOpen = myDB.open(), tempDBIsOpen = tempDB.open(); // Открываем соединение

    if (myDBIsOpen)
        status = tr("Соединение с основной базой установлено.");
    else
        status = tr("Ошибка соединения с основной базой: ") + myDB.lastError().text();

    if (tempDBIsOpen)
        status += tr("\nСоединение с базой самозаписи установлено.");
    else
        status += tr("\nОшибка соединения с базой самозаписи: ")+ tempDB.lastError().text();

    if (myDBIsOpen && tempDBIsOpen)
        status = tr("Соединение установлено.");

    if (!myDBIsOpen && !tempDBIsOpen)
        status = tr("Ошибка: Соединение не установлено.") + '\n' +
                 tr("Основная база: ") + myDB.lastError().text() + '\n' +
                 tr("База самозаписи: ") + tempDB.lastError().text();

    ui->lblStatus->setText(status);

    if (myDBIsOpen)
        return true;
    else
        return false;
}

// ============================================================
// ============================================================

void MainWindow::connectReconfigSlot()
{
    if (connectDB())
    {
        drawTree();
        if (!currentTable->isEmpty())
            refreshTable();
        else
            showTable("Учащиеся");
    }
    else
    {
        ui->treeWidget->clear();
        ui->tableWidget->clear();
    }
    clearMoreInfoForm();
}

// ----------------------------------------------------------------------------------------------------
// -------------------------------------- Рисование таблиц --------------------------------------------
// ----------------------------------------------------------------------------------------------------

// ============================================================
// ====================== Вывод таблицы =======================
// ============================================================

void MainWindow::showTable(QString table)
{

    if (table == "Общее" || (table == *currentTable && !simpleSearchDisplayed))
        return;

    if (simpleSearchDisplayed)
        simpleSearchDisplayed = false;

    setSearchActive();
    ui->mainToolBar->actions()[MainToolButton::Delete]->setDisabled(true);
    ui->actionDeleteStr->setDisabled(true);

    QSqlQuery query;        // Создаём запрос
    lastSelect->clear();    // Удаляем данные о последнем запросе
    currentTable->clear();  // О текущей таблице
    currentMask.clear();   // И о её маске

    // Далее в зависимости от того, какую таблицу нужно вывести
    // Выполняем соотвествующий запрос, сохраняем информацию о нём
    // И показываем соотвесттвующую страницу StackedWidget (форму)

    if (infoMap.contains(table))
    {
        query.exec(infoMap.value(table).query);
        lastSelect->append(infoMap.value(table).query);
        if (infoMap.value(table).index != -1)
            ui->stackedWidget->setCurrentIndex(infoMap.value(table).index);
        currentMask = infoMap.value(table).mask;
    }

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
        qsl.append(rec.fieldName(i));       // Пишем их названия в стринглист

    table->setHorizontalHeaderLabels(qsl);    // Устанавливаем названия столбцов в таблице

    if (forSearch)
    {
        serchCBox->clear();
        while (qsl.size() > 1 && qsl.at(0).contains("ID", Qt::CaseInsensitive))
            qsl.removeFirst();          // Удаляем 0й элемент (ID)
        serchCBox->addItems(qsl);       // Задаём комбобоксу поиска
    }
    else
    {
        serchCBox->clear();
        QVector<bool> tempMask;
        for (QString & name : qsl)
        {
            if(name.contains("ID"))
                tempMask.append(true);
            else
                tempMask.append(false);
        }
        currentMask = tempMask;
        hideColumnsFromMask(tempMask);
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

    QStringList* ColumnsBool = new QStringList();
    ColumnsBool->append("С ослабленным здоровьем");
    ColumnsBool->append("Сирота");
    ColumnsBool->append("Инвалид");
    ColumnsBool->append("На учёте в полиции");
    ColumnsBool->append("Многодетная семья");
    ColumnsBool->append("Неполная семья");
    ColumnsBool->append("Малообеспеченная семья");
    ColumnsBool->append("Мигранты");
    ColumnsBool->append("Учащийся: С ослабленным здоровьем");
    ColumnsBool->append("Учащийся: Сирота");
    ColumnsBool->append("Учащийся: Инвалид");
    ColumnsBool->append("Учащийся: На учёте в полиции");
    ColumnsBool->append("Учащийся: Многодетная семья");
    ColumnsBool->append("Учащийся: Неполная семья");
    ColumnsBool->append("Учащийся: Малообеспеченная семья");
    ColumnsBool->append("Учащийся: Мигранты");

    while (query.next())    // Пока есть результаты запроса
        {
            table->insertRow(rowCount);   // Добавляем строку в конец
            for (int i = 0; i<columnCount; i++)     // Для всех полей таблицы
            {
                // Создаём ячейку в текущем поле текущей строки и заносим туда инфу

                if (query.value(i).isNull())
                {
                    table->setItem(rowCount, i, new QTableWidgetItem(""));
                    continue;
                }

                // Если дата, то она может прийти в каком-нибудь диком формате, а отобразить её надо в ДД.ММ.ГГГГ
                if (query.value(i).userType() == QMetaType::QDate)
                {
                    QString sdate;
                    sdate.append(query.value(i).toDate().toString("dd.MM.yyyy")); // получаем дату в нужном формате
                    table->setItem(rowCount, i, new QTableWidgetItem(sdate)); // Отправляем её в таблицу
                    continue;
                }

                // qDebug() << query.value(i).userType();

                // ВАЖНО
                // MySQL возвращает вместо true и false словами 1 и 0, и отличить их от int нельзя
                if (ColumnsBool->contains(table->horizontalHeaderItem(i)->text()))
                {
                    if (query.value(i).toBool())
                        table->setItem(rowCount, i, new QTableWidgetItem("Да"));
                    else
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
    wgt->setWindowFlags(Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
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

    for (int i = 0; i < colCount; i++)        // Запись заголовков
    {
        vct.append(new QCheckBox(ui->tableWidget->horizontalHeaderItem(i)->text()));
        vct[i]->setChecked(!currentMask[i]);
        gl->addWidget(vct[i], row, col);

        if (ui->tableWidget->horizontalHeaderItem(i)->text().contains("ID", Qt::CaseInsensitive))
            vct[i]->hide();     // Если название поля содержит ID, то оно скрыто
        else
            row++;  // Иначе увеличиваем количество строк

        if (row>10) // Если количество строк больше 10
        {
            row=0;      // Сбрасываем счётчик строк
            col++;      // Увеличиваем счётчик столбцов на 1
        }
    }

    layout->addLayout(gl);
    layout->addWidget(buttonBox);

    wgt->setLayout(layout);
    wgt->setModal(true);
    wgt->setWindowTitle(tr("Скрыть / Показать поля"));
    if (wgt->exec() == QDialog::Accepted)
    {
        for(int i = 0; i < colCount; i++)
            currentMask[i] = !vct[i]->isChecked();

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
            // Запрещаем удаление группы "Ьез группы"
            if (*currentTable == "Группы")
            {
                int column = 0;
                for (int i = 0; i < ui->tableWidget->columnCount(); i++)
                {
                    if (ui->tableWidget->horizontalHeaderItem(i)->text() == "Номер")
                    {
                        column = i;
                        break;
                    }
                }

                if (ui->tableWidget->item(ui->tableWidget->currentRow(), column)->text() == "Без группы")
                {
                    QMessageBox messageBox(QMessageBox::Information,
                                           tr("Удаление записи невозможно"),
                                           tr("Группа 'Без группы' является служебной и не может быть удалена."),
                                           QMessageBox::Yes,
                                           this);
                    messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
                    messageBox.exec();
                    return;
                }
            }


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
                    QString table;

                    // Заменяем имена представлений именами таблиц
                    // Представления Объединения и Группы - на Объединение и Группа
                    // Остальные имена оставляем без изменений
                    if (*currentTable == "Объединения")
                    {
                        table = "Объединение";
                    }
                    else
                    {
                        if (*currentTable == "Группы")
                            table = "Группа";
                        else
                            table = *currentTable;
                    }


                    for (QTableWidgetSelectionRange selectionRange : ui->tableWidget->selectedRanges())
                    {
                        if (selectionRange.rowCount() > 0)
                            for (int row = selectionRange.topRow(); row <= selectionRange.bottomRow(); row++)
                            {
                                strQuery.append("DELETE FROM " + table + " WHERE `ID` = " + ui->tableWidget->item(row, 0)->text() + " ;");
                                if (query.exec(strQuery)) // Если запрос выполнен
                                {
                                    strQuery.clear();
                                }
                                else
                                {
                                    QMessageBox messageBox(QMessageBox::Warning,
                                                           tr("Ошибка выполнения запроса"),
                                                           tr("В ходе выполнения запроса возникла ошибка: ") + query.lastError().text(),
                                                           QMessageBox::Yes,
                                                           this);
                                    messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
                                    messageBox.exec();
                                    return;
                                }
                            }
                    }

                    repeatLastSelect();     // Повторяем последний Select
                    clearMoreInfoForm();    // Чистим поле с подробностями

                    ui->mainToolBar->actions()[MainToolButton::Delete]->setDisabled(true);
                    ui->actionDeleteStr->setDisabled(true);

                    if (*currentTable == "Объединения" || *currentTable == "Направленности" || *currentTable == "Группы")
                        drawTree();
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
    if (query.exec(*lastSelect))
    {
        drawRows(query, ui->tableWidget, true);
    }
    else
    {
        QMessageBox messageBox(QMessageBox::Warning,
                               tr("Ошибка выполнения запроса"),
                               tr("В ходе выполнения запроса на обсновление таблицы возникла ошибка: ") + query.lastError().text(),
                               QMessageBox::Yes,
                               this);
        messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
        messageBox.exec();
    }
}

// ============================================================
// ========== Повторение последнего запроса (Select) ==========
// ============================================================

void MainWindow::repeatLastSelect()
{
    QSqlQuery query;
    if (query.exec(*lastSelect))
    {
        drawRows(query, ui->tableWidget, true);
    }
    else
    {
        QMessageBox messageBox(QMessageBox::Warning,
                               tr("Ошибка выполнения запроса"),
                               tr("В ходе выполнения запроса возникла ошибка: ") + query.lastError().text(),
                               QMessageBox::Yes,
                               this);
        messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
        messageBox.exec();
    }
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

            if (name.isEmpty())
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

            if (bases->contains(name))
            {
                // Сообщаем пользователю, что он ввёл что-то не то
                QMessageBox messageBox(QMessageBox::Information,
                                       tr("Добавление записи"),
                                       tr("Сохранение невозможно: введены некорректные данные."),
                                       QMessageBox::Yes,
                                       this);
                messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
                messageBox.exec();
                return;
            }

            if (id.isEmpty())
            {
                strQuery = "INSERT INTO Объединение (`Название`, `ID направленности`, `Отдел`, `Описание`) VALUES ('" + name + "', '" + directID + "', '" + otd  + "', '" + desc + "');";
                clearMoreInfoForm();
            }
            else
            {
                strQuery = "UPDATE Объединение SET `Название` = '" + name + "', `ID направленности` = '" + directID + "', `Отдел` = '" + otd + "', `Описание` = '" + desc + "' WHERE `ID` = " + id + ";";
            }
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
                strQuery = "UPDATE Преподаватели SET `Имя` = '" + name + "', `Фамилия` = '" + surname + "', `Отчество` = '" + patrname + "', `Паспорт` = '" + numpass + "', `Отдел` = '" + otd + "' WHERE `ID` = " + id + ";";
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

            // Дата рождения
            getDateToForm(&birthday, ui->studBDay, ui->studBMon, ui->studBYear);

            // Дата подачи заявления
            getDateToForm(&admiss, ui->studInDay, ui->studInMon, ui->studInYear);

            // Когда выбыл
            getDateToForm(&out, ui->studOutDay, ui->studOutMon, ui->studOutYear);

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
                strQuery.append("`Фамилия`, `Имя`, `Отчество`, `Тип документа`, `Номер документа`, `Пол`, `Дата рождения`, ");
                strQuery.append("`Район школы`, `Школа`, `Класс`, `Родители`, `Домашний адрес`, `Телефон`, `e-mail`, `Дата заявления`, `Форма обучения`, ");
                strQuery.append("`Когда выбыл`, `С ослабленным здоровьем`, `Сирота`, `Инвалид`, `На учёте в полиции`, `Многодетная семья`, ");
                strQuery.append("`Неполная семья`, `Малообеспеченная семья`, `Мигранты`, `Примечания`) VALUES ('");
                strQuery.append(surname + "', '" + name  + "', '" + patr  + "', '" + docType  + "', '" + docNum  + "', '" + gender  + "', " + birthday  + ", '");
                strQuery.append(arSchool  + "', '" + school  + "', '" + grad  + "', '" + parents  + "', '" + address  + "', '" + phone  + "', '" + email  + "', " + admiss  + ", '" + eduForm  + "', ");
                strQuery.append(out  + ", " + weackHealth  + ", " + orphan  + ", " + invalid  + ", " + accountInPolice + ", " + large  + ", ");
                strQuery.append(incompleteFamily  + ", " + lowIncome  + ", " + migrants  + ", '" + comments  + "');");
                clearMoreInfoForm();

            }
            else
            {
                // UPDATE
                strQuery.append("UPDATE Учащиеся SET `Фамилия` = '" + surname + "', `Имя` = '" + name  + "', `Отчество` = '" + patr  + "', `Тип документа` = '" + docType  + "', `Номер документа` = '" + docNum  + "', `Пол` = '" + gender  + "', `Дата рождения` = " + birthday  + ", ");
                strQuery.append("`Район школы` = '" + arSchool  + "', `Школа` = '" + school  + "', `Класс` = '" + grad  + "', `Родители` = '" + parents  + "', `Домашний адрес` = '" + address  + "', `Телефон` = '" + phone  + "', `e-mail` = '" + email  + "', `Дата заявления` = " + admiss  + ", `Форма обучения` = '" + eduForm  + "', ");
                strQuery.append("`Когда выбыл` = " + out  + ", `С ослабленным здоровьем` = " + weackHealth  + ", `Сирота` = " + orphan  + ", `Инвалид` = " + invalid  + ", `На учёте в полиции` = " + accountInPolice + ", `Многодетная семья` = " + large  + ", ");
                strQuery.append("`Неполная семья` = " + incompleteFamily  + ", `Малообеспеченная семья` = " + lowIncome  + ", `Мигранты` = " + migrants  + ", `Примечания` = '" + comments  + "' ");
                strQuery.append("WHERE `ID` = " + id + ";");
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

        if (bases->contains(name))
        {
            // Сообщаем пользователю, что он ввёл что-то не то
            QMessageBox messageBox(QMessageBox::Information,
                                   tr("Добавление записи"),
                                   tr("Сохранение невозможно: введены некорректные данные."),
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
            strQuery = "UPDATE Направленности SET `Название` = '" + name + "' WHERE `ID` = " + id + ";";
        }
        break;

    }
    case 4: // Группа
    {
        QString id = ui->groupID->text();
        QString teachID = ui->groupTeachID->text();
        QString assID = ui->groupAssID->text();
        QString num = ui->groupNum->text();
        QString year = QString::number(ui->groupYear->value());


        if ((!id.isEmpty() || !id.isNull()) && num == "Без группы")
        {
            QMessageBox messageBox(QMessageBox::Information,
                                   tr("Изменение записи"),
                                   tr("Группа Без группы является служебной и не может быть изменена!"),
                                   QMessageBox::Yes,
                                   this);
            messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
            messageBox.exec();
            return;
        }

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

        if (bases->contains(num))
        {
            // Сообщаем пользователю, что он ввёл что-то не то
            QMessageBox messageBox(QMessageBox::Information,
                                   tr("Добавление записи"),
                                   tr("Сохранение невозможно: введены некорректные данные."),
                                   QMessageBox::Yes,
                                   this);
            messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
            messageBox.exec();
            return;
        }

        qDebug() << teachID;
        if (teachID.isEmpty() || teachID.isNull())
            teachID.append("NULL");

        if (id.isEmpty())
        {
            strQuery = "INSERT INTO Группа (`Номер`, `Год обучения`, `ID объединения`, `ID преподавателя`) VALUES ('" + num + "', " + year + ", " + assID + ", " + teachID + ");";
            clearMoreInfoForm();
        }
        else
        {
            strQuery = "UPDATE Группа SET `ID объединения` = " + assID + ", `ID преподавателя` = " + teachID + ", `Номер` = '" + num + "', `Год обучения` = " + year + " WHERE `ID` = " + id + ";";
        }
        break;

    }
    }
    qDebug() << strQuery;
    QSqlQuery query;
    if (query.exec(strQuery))
    {
        repeatLastSelect();

        if (currentIndex == 4 || currentIndex == 3 || currentIndex == 0)
            drawTree();
    }
    else
    {
        QMessageBox messageBox(QMessageBox::Warning,
                               tr("Ошибка выполнения запроса"),
                               tr("В ходе выполнения запроса на изменение данных возникла ошибка: ") + query.lastError().text(),
                               QMessageBox::Yes,
                               this);
        messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
        messageBox.exec();
    }
}

// ============================================================
// ============================================================

void MainWindow::getDateToForm(QString* str, QComboBox* d, QComboBox* m, QSpinBox* y)
{
    if (d->currentIndex() != 0 && m->currentIndex() != 0)
    {
        QDate date;
        date.setDate(y->value(), m->currentText().toInt(), d->currentText().toInt());
        //str->append("'" + date.toString(Qt::SystemLocaleShortDate) + "'");
        str->append("'" + date.toString(Qt::ISODate) + "'");
    }
    else
        str->append("NULL");
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

        if (searchBox->currentText() == "С ослабленным здоровьем" || searchBox->currentText() ==  "Сирота"
                || searchBox->currentText() == "Инвалид" || searchBox->currentText() == "На учёте в полиции"
                || searchBox->currentText() == "Многодетная семья" || searchBox->currentText() == "Неполная семья"
                || searchBox->currentText() == "Малообеспеченная семья" || searchBox->currentText() == "Мигранты")
        {
            QString str = infoMap.value(*currentTable).query;
            newSelect->append(str.replace(";", " "));
            if (newSelect->contains("WHERE", Qt::CaseSensitive))
                newSelect->append("AND ");
            else
                newSelect->append("WHERE ");

            newSelect->append("`" + searchBox->currentText());

            if (searchText->toLower() == "да")
                newSelect->append("` = 'true';");
            else
                newSelect->append("` = 'false';");
        }
        else
        {
            QString str = infoMap.value(*currentTable).query;
            newSelect->append(str.replace(";", " "));
            if (newSelect->contains("WHERE", Qt::CaseSensitive))
                newSelect->append("AND ");
            else
                newSelect->append("WHERE ");

            newSelect->append("`" + searchBox->currentText() + "` LIKE '%" + *searchText + "%';");
        }

        QSqlQuery query;
        if (query.exec(*newSelect))
        {
            drawRows(query, ui->tableWidget, true);
            lastSelect = newSelect;
            simpleSearchDisplayed = true;
        }
        else
        {
            QMessageBox messageBox(QMessageBox::Warning,
                                   tr("Ошибка выполнения запроса"),
                                   tr("В ходе выполнения запроса возникла ошибка: ") + query.lastError().text(),
                                   QMessageBox::Yes,
                                   this);
            messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
            messageBox.exec();
        }
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
    ui->treeWidget->clear();
    QTreeWidgetItem *base = new QTreeWidgetItem(ui->treeWidget);
    base->setText(0, "Общее");
    for (int i = 0; i<5; i++)
    {
        QTreeWidgetItem *newItem = new QTreeWidgetItem();
        newItem->setText(0, bases->at(i));
        base->addChild(newItem);
    }

   QStringList *directions = new QStringList();
   QStringList *association = new QStringList();

   QTreeWidgetItem *direct= new QTreeWidgetItem(ui->treeWidget);
   direct->setText(0, "Направленности");

   // Получаем список направленностей

   QString strQuery = "SELECT `Название` FROM Направленности ORDER BY `Название`;";
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

       infoMap.insert(dir, infoMap.value("Объединения"));
       infoMap[dir].query.replace(";", " ");
       infoMap[dir].query.append("WHERE `Направленность` = '" + dir + "';");

       strQuery.clear();
       strQuery.append("SELECT `Название` FROM Объединения WHERE `Направленность` = '" + dir + "' ORDER BY `Название`;");
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

           infoMap.insert(ass, infoMap.value("Группы"));
           infoMap[ass].query.clear();
           infoMap[ass].query.append("SELECT Группа.`ID` `ID`, Группа.`ID преподавателя` `ID преподавателя`, Объединение.`ID` `ID объединения`, Группа.`Номер` `Номер`, Группа.`Год обучения` `Год обучения`, Объединение.`Название` `Объединение`, Преподаватели.`Фамилия` `Фамилия преподавателя`, Преподаватели.`Имя` `Имя преподавателя`, Преподаватели.`Отчество` `Отчество преподавателя` FROM Группа, Объединение, Преподаватели, Направленности WHERE Группа.`ID объединения` = Объединение.`ID` AND Группа.`ID преподавателя` = Преподаватели.`ID` AND Направленности.`ID` = Объединение.`ID направленности` AND Направленности.`Название` = '");
           infoMap[ass].query.append(dir + "' AND Объединение.Название = '" + ass + "';");

           strQuery.clear();
           strQuery.append("SELECT  Группа.`Номер`, Группа.`ID` FROM Объединение, Направленности, Группа  WHERE Направленности.`ID` = Объединение.`ID направленности` AND Объединение.`ID` = Группа.`ID объединения` AND Направленности.`Название` = '");
           strQuery.append(dir + "' AND Объединение.Название = '" + ass + "' ORDER BY `Номер`;");
           query.exec(strQuery);
           while (query.next())    // Пока есть результаты запроса
           {
               QTreeWidgetItem *treeGroup = new QTreeWidgetItem();
               QString id = query.value(1).toString();
               treeGroup->setText(0, query.value(0).toString());
               treeGroup->setText(1, id);

               treeAss->addChild(treeGroup);

               infoMap.insert(id, infoMap.value("Учащиеся"));
               infoMap[id].query.clear();
               infoMap[id].query.append("SELECT Учащиеся.`ID` `ID`, Учащиеся.`Фамилия` `Фамилия`, Учащиеся.`Имя` `Имя`, Учащиеся.`Отчество` `Отчество`, Учащиеся.`Тип документа` `Тип документа`, Учащиеся.`Номер документа` `Номер документа`, Учащиеся.`Пол` `Пол`, Учащиеся.`Дата рождения` `Дата рождения`, Учащиеся.`Район школы` `Район школы`, Учащиеся.`Школа` `Школа`, Учащиеся.`Класс` `Класс`, Учащиеся.`Родители` `Родители`, Учащиеся.`Домашний адрес` `Домашний адрес`, Учащиеся.`Телефон` `Телефон`, Учащиеся.`e-mail` `e-mail`, Учащиеся.`Дата заявления` `Дата заявления`, Учащиеся.`Форма обучения` `Форма обучения`, Учащиеся.`Когда выбыл` `Когда выбыл`, Учащиеся.`С ослабленным здоровьем` `С ослабленным здоровьем`, Учащиеся.`Сирота` `Сирота`, Учащиеся.`Инвалид` `Инвалид`, Учащиеся.`На учёте в полиции` `На учёте в полиции`, Учащиеся.`Многодетная семья` `Многодетная семья`, Учащиеся.`Неполная семья` `Неполная семья`, Учащиеся.`Малообеспеченная семья` `Малообеспеченная семья`, Учащиеся.`Мигранты` `Мигранты`, Учащиеся.`Примечания` `Примечания` FROM Учащиеся, Нагрузка WHERE Учащиеся.`ID` = Нагрузка.`ID учащегося` AND Нагрузка.`ID группы` = '");
               infoMap[id].query.append(id + "';");
           }

           treeDir->addChild(treeAss);
       }

       direct->addChild(treeDir);
       association->clear();
   }
}

// ============================================================
// ================ Активация кнопки Поиск ====================
// ============================================================

void MainWindow::setSearchActive()
{
    if (*currentTable != "GlobalSearch") // Если текущая таблица - не результат глобального поиска
        ui->searchToolBar->actions()[SearchToolButton::Start]->setDisabled(false);
}

// ============================================================
// ============== Вывод подробной инфы в форму ================
// ============================================================

void MainWindow::showMoreInfo(int row)
{
    clearMoreInfoForm(); // Чистим форму
    int currentIndex = ui->stackedWidget->currentIndex();
    switch (currentIndex)
    {
        case -1: return;
        case 0:     // Объединение
        {
        // "SELECT `ID`, `ID направленности`, `Название`, `Направленность`, `Отдел`, `Описание` FROM Объединения;"
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
            break;
        }
        case 1:     // Преподаватель
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
            break;
        }
        case 2:     // Учащийся
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

            // Дата рождения
            getDateToTable(ui->tableWidget->item(row, 7)->text(), ui->studBDay, ui->studBMon, ui->studBYear);

            ui->areaSchools->setText(ui->tableWidget->item(row, 8)->text());    // Район школы
            ui->school->setText(ui->tableWidget->item(row, 9)->text());         // Школа
            ui->grade->setText(ui->tableWidget->item(row, 10)->text());         // Класс
            ui->parents->setText(ui->tableWidget->item(row, 11)->text());       // Родители
            ui->address->setText(ui->tableWidget->item(row, 12)->text());       // Адрес
            ui->phone->setText(ui->tableWidget->item(row, 13)->text());         // Телефон
            ui->email->setText(ui->tableWidget->item(row, 14)->text());         // email

            // Дата подачи заявления
            getDateToTable(ui->tableWidget->item(row, 15)->text(), ui->studInDay, ui->studInMon, ui->studInYear);

            ui->eduForm->setCurrentText(ui->tableWidget->item(row, 16)->text()); // Форма обучения

            // Когда выбыл
            getDateToTable(ui->tableWidget->item(row, 17)->text(), ui->studOutDay, ui->studOutMon, ui->studOutYear);

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


            QString strQuery = "SELECT `ID объединения`, `ID группы`, `Объединение`, `Номер группы` FROM Нагрузка_Учащегося WHERE `ID учащегося` = '";
            strQuery.append(ui->studID->text() + "';");
            QSqlQuery query;
            query.exec(strQuery);
            drawRows(query, ui->studGroupTable, false);
            break;
        }

        case 3: // Направленность
        {
            ui->directID->setText(ui->tableWidget->item(row, 0)->text());    // ID
            ui->directName->setText(ui->tableWidget->item(row, 1)->text());    // Название

            // Отрисовка таблички!!! - Названия объединений

            QString str = "SELECT `ID`, `Название` FROM Объединение WHERE `ID направленности` = " + ui->directID->text() + ";";

            QSqlQuery query;
            query.exec(str);
            drawRows(query, ui->alInDirect, false);
            break;

        }
        case 4: // Группа
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
            QString str = "SELECT `ID учащегося`, `Фамилия`, `Имя`, `Отчество`, `Телефон`, `e-mail` FROM Состав_групп WHERE `ID группы` = " + ui->groupID->text() + ";";

            QSqlQuery query;
            query.exec(str);
            drawRows(query, ui->studsInGroupe, false);
            break;
        }
    }
}

// ============================================================
// ============================================================

void MainWindow::getDateToTable(QString str, QComboBox* d, QComboBox* m, QSpinBox* y)
{
    QStringList qsl = str.split(QRegularExpression("\\D"));
    if (qsl.length() == 3)
    {
        m->setCurrentText(qsl[1]);
        if (qsl[0].length() == 2)
        {
            d->setCurrentText(qsl[0]);
            y->setValue(qsl[2].toInt());
        }
        else
        {
            d->setCurrentText(qsl[2]);
            y->setValue(qsl[0].toInt());
        }
    }
    else
    {
        d->setCurrentIndex(0);
        m->setCurrentIndex(0);
        y->setValue(2000);
    }
}

// ============================================================
// ============= Очистка формы с подробной инфой ==============
// ============================================================

void MainWindow::clearMoreInfoForm()
{
    int currentIndex = ui->stackedWidget->currentIndex();
    switch (currentIndex)
    {
        case -1: return;
        case 0:     // Объединение
        {
            ui->alID->clear();           // ID
            ui->alName->clear();         // Название
            ui->alDirect->clear();      // Напавленность
            ui->alDirectID->clear();
            ui->alOtd->clear();          // Отдел
            ui->alDescript->clear();     // Описание

            // groupInAl
            int rowCount = ui->groupInAl->rowCount();
            for(int i = 0; i < rowCount; i++)
                 ui->groupInAl->removeRow(0);

            break;
        }
        case 1:     // Преподаватель
        {
            ui->teachID->clear();        // ID
            ui->teachSurname->clear();   // Фамилия
            ui->teachName->clear();      // Имя
            ui->teachPatr->clear();      // Отчество
            ui->teachNumPass->clear();   // Номер паспорта
            ui->teachOtd->clear();       // Отдел

            // groupInTeach
            int rowCount = ui->groupInTeach->rowCount();
            for(int i = 0; i < rowCount; i++)
                 ui->groupInTeach->removeRow(0);

            break;
        }
        case 2:     // Учащийся
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

            int rowCount = ui->studGroupTable->rowCount();
            for(int i = 0; i < rowCount; i++)
                 ui->studGroupTable->removeRow(0);

            break;
        }

        case 3: // Направленность
        {
            ui->directID->clear();
            ui->directName->clear();

            int rowCount = ui->alInDirect->rowCount();
            for(int i = 0; i < rowCount; i++)
                 ui->alInDirect->removeRow(0);
            break;

        }
        case 4: // Группа
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
            break;
        }
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
    dialog = new SearchDialog(this);
    connect (dialog, SIGNAL(signalQuery(QString)), this, SLOT(querySlot(QString)));
    dialog->exec();
}

// ============================================================
// ============================================================

void MainWindow::querySlot(QString textQuery)
{
    currentTable->clear();
    currentTable->append("GlobalSearch");

    ui->searchToolBar->actions()[SearchToolButton::Start]->setDisabled(true); //Выключаем простой поиск, иначе будет чешуйня

    lastSelect->clear();
    lastSelect->append(textQuery);

    QSqlQuery query;
    query.exec(textQuery);
    drawHeaders(query, ui->tableWidget, false, searchBox);
    drawRows(query, ui->tableWidget, true);
}

// ============================================================
// =============== Выбрана таблица из дерева ==================
// ============================================================

void MainWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    QString str;
    if (item->text(1).isEmpty())
        str = item->text(column);
    else
        str = item->text(1);
    showTable(str);
}

// ============================================================
// ========= Слот для сигнала Выбрана ячейка таблицы ==========
// ============================================================

void MainWindow::on_tableWidget_cellClicked(int row, int column)
{
    ui->tableWidget->setSortingEnabled(false); // Временно запрещаем сортировку
    if (row < ui->tableWidget->rowCount()-1 &&
            *currentTable != "GlobalSearch")
    {
        showMoreInfo(row);
        if (bases->contains(*currentTable))
        {
            ui->mainToolBar->actions()[MainToolButton::Delete]->setEnabled(true);
            ui->actionDeleteStr->setEnabled(true);
        }
    }
    else
    {
        clearMoreInfoForm();
        ui->mainToolBar->actions()[MainToolButton::Delete]->setDisabled(true);
        ui->actionDeleteStr->setDisabled(true);
    }
    ui->tableWidget->setSortingEnabled(true);
}

// ============================================================
// ========= Слот для сигнала Выбрана строка таблицы ==========
// ============================================================

void MainWindow::selectedRowChanged(const QModelIndex current, const QModelIndex previous)
{
    if (current.row() < 0 || current.row() == previous.row())
        return;

    int row = current.row();
    ui->tableWidget->setSortingEnabled(false); // Временно запрещаем сортировку

    if (row < ui->tableWidget->rowCount()-1 &&
            *currentTable != "GlobalSearch")
    {
        showMoreInfo(row);
        if (bases->contains(*currentTable))
        {
            ui->mainToolBar->actions()[MainToolButton::Delete]->setEnabled(true);
            ui->actionDeleteStr->setEnabled(true);
        }
    }
    else
    {
        clearMoreInfoForm();
        ui->mainToolBar->actions()[MainToolButton::Delete]->setDisabled(true);
        ui->actionDeleteStr->setDisabled(true);
    }
    ui->tableWidget->setSortingEnabled(true);
}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------- Экспорт --------------------------------------------
// ----------------------------------------------------------------------------------------------------

// ============================================================
// ==================== Экспорт в Exсel =======================
// ============================================================

void MainWindow::exportInExcel()
{
    // Открываем QFileDialog
    QFileDialog fileDialog;
    QString fileName = fileDialog.getSaveFileName(0, tr("Экспортировать в..."), "", "*.xlsx *.xls");

    if (!fileName.isEmpty())
    {
        ui->lblStatus->setText(tr("Экспорт..."));
        qApp->processEvents();

        //Спасибо drweb86
        //https://forum.qt.io/topic/16547/how-to-export-excel-in-qt/10

        QAxObject* excel = new QAxObject("Excel.Application");

        if (excel->isNull())
        {
            ui->lblStatus->setText(tr("Не могу иницировать взаимодействие с Excel. Возможно, Excel не установлен"));
            return;
        }

        excel->dynamicCall("SetVisible(bool)", false); //Скрываем Excel
        excel->setProperty("DisplayAlerts", 0);        //Выключем предупреждения

        QAxObject *workbooks = nullptr, *workbook = nullptr,
                  *sheets = nullptr, *sheet = nullptr;

        workbooks = excel->querySubObject("Workbooks");

        if (workbooks != nullptr)
            workbook = workbooks->querySubObject("Add");

        if (workbook != nullptr)
            sheets = workbook->querySubObject("Worksheets");

        if (sheets != nullptr)
            sheet = sheets->querySubObject("Add");

        if (sheet == nullptr)
        {
            ui->lblStatus->setText(tr("Неизвестная ошибка. Экспорт не выполнен"));
            return;
        }

        // Вставка значения в отдельную ячейку

       for (int col = 1; col < ui->tableWidget->columnCount(); col++)        // Запись заголовков
       {
            // получение указателя на ячейку [row][col] ((!)нумерация с единицы)
            QAxObject* cell = sheet->querySubObject("Cells(int,int)", 1, col);
            // вставка значения переменной в полученную ячейку
            cell->setProperty("Value", ui->tableWidget->horizontalHeaderItem(col)->text());
            // освобождение памяти
            delete cell;
        }

        for (int row = 1; row < ui->tableWidget->rowCount(); row++)
        {
            for (int col = 1; col < ui->tableWidget->columnCount(); col++)
            {
                // получение указателя на ячейку [row][col] ((!)нумерация с единицы)
                QAxObject* cell = sheet->querySubObject("Cells(int,int)", row + 1, col);
                // вставка значения переменной в полученную ячейку
                cell->setProperty("Value", ui->tableWidget->item(row - 1, col)->text());
                // освобождение памяти
                delete cell;
            }
        }

        if (fileName.right(4) == ".xls")    // В зависимости от расширения файла выбираем, в каком формате сохранять
        {
            QList<QVariant> lstParam;
            lstParam.append(QDir::toNativeSeparators(fileName));
            lstParam.append(-4143);
            lstParam.append("");
            lstParam.append("");
            lstParam.append(false);
            lstParam.append(false);
            lstParam.append(1);
            lstParam.append(2);
            lstParam.append(false);
            lstParam.append(false);
            lstParam.append(false);
            lstParam.append(false);
            workbook->dynamicCall("SaveAs(QVariant, QVariant, QVariant, QVariant, "
                                         "QVariant, QVariant, QVariant, QVariant, "
                                         "QVariant, QVariant, QVariant, QVariant)", lstParam);
        }
        else
        {
            workbook->dynamicCall("SaveAs(const QString&)", QDir::toNativeSeparators(fileName));
        }

        workbook->dynamicCall("Close()");   // Закрываем
        excel->dynamicCall("Quit()");       // Выходим

        delete sheet;
        delete sheets;
        delete workbook;
        delete workbooks;
        delete excel;

        ui->lblStatus->setText(tr("Экспорт завершён"));
    }
}

// ============================================================
// ===================== Экспорт в Html =======================
// ============================================================

void MainWindow::exportInHtml()
{
    QString fileName = QFileDialog::getSaveFileName(0, tr("Экспортировать в..."), "", "*.htm *.html");

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        QString text;
        int columns = ui->tableWidget->columnCount();
        int rows = ui->tableWidget->rowCount() - 1;

        QDate date;

        text.append("<!DOCTYPE html><html><head><meta charset=utf-8></head><body><h3>" + *currentTable + "</h3><br/>" + date.currentDate().toString(Qt::SystemLocaleShortDate) + "<br />");
        text.append("<table border=1px align=justify><tr>");

        if (infoMap.contains(*currentTable) && columns == currentMask.size())
        {
            for (int i = 1; i<columns; i++)
            {
                if (!currentMask[i])
                    text.append("<td><b>" + ui->tableWidget->horizontalHeaderItem(i)->text() + "</b></td>");
            }
            for (int j = 0; j<rows; j++)
            {
                text.append("</tr><tr>");
                for (int i = 1; i<columns; i++)
                {
                    if (!currentMask[i])
                        text.append("<td>" + ui->tableWidget->item(j, i)->text() + "</td>");
                }
            }
            text.append("</tr><tr>");
        }
        else
        {
            for (int i = 1; i<columns; i++)
                text.append("<td><b>" + ui->tableWidget->horizontalHeaderItem(i)->text() + "</b></td>");
            for (int j = 0; j<rows; j++)
            {
                text.append("</tr><tr>");
                for (int i = 1; i<columns; i++)
                {
                    text.append("<td>" + ui->tableWidget->item(j, i)->text() + "</td>");
                }
            }
            text.append("</tr><tr>");
        }

        text.append("</table></body>");
        stream << text;
        file.close();
        ui->lblStatus->setText(tr("Экспорт завершён"));
    }
    else
    {
        ui->lblStatus->setText(tr("Ошибка: экспорт в Html не выполнен"));
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
    query.exec("SELECT `ID`, `Фамилия`, `Имя`, `Отчество`, `Тип документа`, `Номер документа`, `Пол`, `Дата рождения` FROM Учащиеся;");
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
                        qDebug() << strQuery;
                        query.exec(strQuery);
                        strQuery.clear();

                        strQuery.append("SELECT группа.`ID` FROM Нагрузка, Группа WHERE Группа.`ID` = нагрузка.`ID группы` AND Нагрузка.`ID учащегося` = "+ wgt->item(row, 0)->text() + " AND Группа.`Номер` = 'Без группы' AND Группа.`ID объединения` = (SELECT `ID объединения` FROM группа WHERE `ID`  = " + ui->groupID->text() + ")");
                        qDebug() << strQuery;
                        query.exec(strQuery);
                        if (query.next())
                        {
                            int id = query.value(0).toInt();
                            strQuery.clear();
                            strQuery.append("DELETE FROM Нагрузка WHERE `ID учащегося` = " + wgt->item(row, 0)->text() + " AND `ID группы` <> " + ui->groupID->text() + " AND `ID группы` = " + QString::number(id));
                            qDebug() << strQuery;
                            query.exec(strQuery);
                        }
                        strQuery.clear();
                    }
            }
            strQuery.append("SELECT `ID учащегося`, `Фамилия`, `Имя`, `Отчество`, `Телефон`, `e-mail` FROM Состав_групп WHERE `ID группы` = " + ui->groupID->text() + ";");
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

// ============================================================
// ======= Связь между объединением и направленностью =========
// ============================================================

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

// ============================================================
// ============= Показать таблицу с самозаписью ===============
// ============================================================

void MainWindow::showTempTable()
{
    dbDialog->setType("tempDB");
    QTableWidget *wgt = dbDialog->letTable();
    QSqlQuery query(tempDB);
    query.exec("SELECT Запись.`Объединение` `Объединение`, Учащийся.`Фамилия` `Фамилия`, Учащийся.`Имя` `Имя`, Учащийся.`Отчество` `Отчество`, Учащийся.`Тип документа` `Тип документа`, Учащийся.`Номер документа` `Номер документа`, Учащийся.`Пол` `Пол`, Учащийся.`Дата рождения` `Дата рождения`, Учащийся.`Район школы` `Район школы`, Учащийся.`Школа` `Школа`, Учащийся.`Класс` `Класс`, Учащийся.`Родители` `Родители`, Учащийся.`Домашний адрес` `Домашний адрес`, Учащийся.`Телефон` `Телефон`, Учащийся.`e-mail` `e-mail`, Учащийся.`Дата заявления` `Дата заявления` FROM Учащийся, Запись WHERE Учащийся.`Тип документа` = Запись.`Тип документа` AND Учащийся.`Номер документа` = Запись.`Номер документа`;");
    drawHeaders(query, wgt, true, dbDialog->letSearchBox());
    drawRows(query, wgt, false);
    dbDialog->show();
}

// ============================================================
// ==================== Выполнить запрос ======================
// ============================================================

void MainWindow::querySlot(QTableWidget* tableWidget, QString strQuery, bool mainDB)
{
    QSqlQuery query;
    if (!mainDB)
        query = QSqlQuery(tempDB);

    if (!query.exec(strQuery))
    {
        // Иначе выводим сообщение обошибке
        QMessageBox messageBox(QMessageBox::Warning,
                               tr("Ошибка выполнения запроса"),
                               tr("В ходе выполнения запроса:\n") + strQuery + tr("\n\nВозникла ошибка:\n") + query.lastError().text(),
                               QMessageBox::Yes,
                               this);
        messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
        messageBox.exec();
        return;
    }

    drawRows(query, tableWidget, false); // Рисуем строки
}

// ============================================================
// =================== Выполнить запросы ======================
// ============================================================

void MainWindow::queriesSlot(QStringList qsl, bool mainDB)
{
    QSqlQuery query;
    if (!mainDB)
        query = QSqlQuery(tempDB);

    for (QString & strQuery : qsl)
    {
        if (!query.exec(strQuery))
        {
            qDebug() << strQuery;
            // Если запрос не выполнился, выводим сообщение об ошибке
            QMessageBox messageBox(QMessageBox::Warning,
                                   tr("Ошибка выполнения запроса"),
                                   tr("В ходе выполнения запроса:\n") + strQuery + tr("\n\nВозникла ошибка:\n") + query.lastError().text(),
                                   QMessageBox::Yes,
                                   this);
            messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
            messageBox.exec();

            // Посольку выполняется серия запросов, на всякий случай обновляем таблицу
            if(mainDB)
                repeatLastSelect();
            return; // И выходим.
        }
    }

    if(mainDB)
        repeatLastSelect();
}

// ============================================================
// ============ Удаление учащегося из группы ==================
// ============================================================

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
                    if (!query.exec(strQuery))
                    {
                        // Если запрос не выполнился, выводим сообщение об ошибке
                        QMessageBox messageBox(QMessageBox::Warning,
                                               tr("Ошибка выполнения запроса"),
                                               tr("В ходе выполнения запроса возникла ошибка: ") + query.lastError().text(),
                                               QMessageBox::Yes,
                                               this);
                        messageBox.setButtonText(QMessageBox::Yes, tr("ОК"));
                        messageBox.exec();
                        return; // И выходим.
                    }
                    strQuery.clear();
                }
        }
        strQuery.append("SELECT `ID учащегося`, `Фамилия`, `Имя`, `Отчество`, `Телефон`, `e-mail` FROM Состав_групп WHERE `ID группы` = " + ui->groupID->text() + ";");
        query.exec(strQuery);
        drawRows(query, ui->studsInGroupe, false);
    }
}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

void MainWindow::showQtInfo()
{
    QMessageBox messageBox;
    messageBox.aboutQt(this, tr("О библиотеке Qt"));
}

// ============================================================
// ============================================================

void MainWindow::showLicense()
{
    QDialog *license = new QDialog(this);
    license->setWindowTitle(tr("Лицензия"));
    license->setWindowFlags(Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
    QTextEdit* textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    textEdit->setHtml(tr("The MIT License (MIT)"
                         "<br /><br />Copyright © 2016 Domerk"
                         "<br /><br />Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the &quot;Software&quot;), to deal in the Software without restriction, including without limitation the rights"
                         "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:"
                         "<br /><br />The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software."
                         "<br /><br />THE SOFTWARE IS PROVIDED &quot;AS IS&quot;, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS "
                         "OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,"
                         "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE."
                         "<br /><br />Translations into Russian:"
                         "<br /><br />Copyright © 2016 Domerk"
                         "<br /><br />Данная лицензия разрешает лицам, получившим копию данного программного обеспечения и сопутствующей документации (в дальнейшем именуемыми «Программное Обеспечение»), безвозмездно использовать Программное Обеспечение без ограничений, включая "
                         "неограниченное право на использование, копирование, изменение, добавление, публикацию, распространение, сублицензирование и/или продажу копий Программного Обеспечения, а также лицам, которым предоставляется данное Программное Обеспечение, при соблюдении следующих условий:"
                         "<br /><br />Указанное выше уведомление об авторском праве и данные условия должны быть включены во все копии или значимые части данного Программного Обеспечения."
                         "<br /><br />ДАННОЕ ПРОГРАММНОЕ ОБЕСПЕЧЕНИЕ ПРЕДОСТАВЛЯЕТСЯ «КАК ЕСТЬ», БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ, ЯВНО ВЫРАЖЕННЫХ ИЛИ ПОДРАЗУМЕВАЕМЫХ, ВКЛЮЧАЯ ГАРАНТИИ ТОВАРНОЙ ПРИГОДНОСТИ, СООТВЕТСТВИЯ ПО ЕГО КОНКРЕТНОМУ НАЗНАЧЕНИЮ И ОТСУТСТВИЯ НАРУШЕНИЙ, НО НЕ  ОГРАНИЧИВАЯСЬ ИМИ. "
                         "НИ В КАКОМ СЛУЧАЕ АВТОРЫ ИЛИ ПРАВООБЛАДАТЕЛИ НЕ НЕСУТ  ОТВЕТСТВЕННОСТИ ПО КАКИМ-ЛИБО ИСКАМ, ЗА УЩЕРБ ИЛИ ПО ИНЫМ ТРЕБОВАНИЯМ, В ТОМ ЧИСЛЕ, ПРИ ДЕЙСТВИИ КОНТРАКТА, ДЕЛИКТЕ ИЛИ ИНОЙ СИТУАЦИИ, ВОЗНИКШИМ ИЗ-ЗА ИСПОЛЬЗОВАНИЯ ПРОГРАММНОГО ОБЕСПЕЧЕНИЯ ИЛИ ИНЫХ ДЕЙСТВИЙ С ПРОГРАММНЫМ ОБЕСПЕЧЕНИЕМ."
                         "<br /><br /><hr /><br />Иконки панели инструментов предоставлены <a href=http://www.flaticon.com title=Flaticon>www.flaticon.com</a> "
                         "по лицензии <a href=http://creativecommons.org/licenses/by/3.0/ title=Creative Commons BY 3.0>CC BY 3.0</a>.<br /><br />"));
    QVBoxLayout* layout = new QVBoxLayout;
    QDialogButtonBox* button = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect (button, SIGNAL(accepted()), license, SLOT(close()));
    layout->addWidget(textEdit);
    layout->addWidget(button);
    license->setLayout(layout);
    license->exec();
}

// ============================================================
// ============================================================

void MainWindow::showProgramInfo()
{
    QMessageBox *aboutBox;
    aboutBox = new QMessageBox(this);
    aboutBox->setWindowTitle(tr("О программе"));
    aboutBox->setIconPixmap(QPixmap(":/icons/Icons/udod"));
    aboutBox->setText("<strong>UdoDB v1.0.2</strong>");
    QString str;
    str.append("Данная сборка предназначения для работы с MySql-5.7.14, Windows x86.<br /><br />");
    str.append("Программа представляет собой клиент для работы с базой данных учреждения дополнительного образования. Она позволяет просматривать, добавлять, удалять и изменять данные об учащихся, преподавателях, объединениях, учебных группах и направленностях.<br /><br />");
    str.append("Если у Вас остались вопросы или Вы хотите помочь развитию данного проекта, посетите его <a href=https://github.com/Domerk/UdoDB/wiki>домашнюю страницу</a> или обратитесь к администрации образовательного учреждения.<br /><br />");
    str.append("Разработано по заказу Центра детского (юношеского) технического творчества Московского района Санкт-Петербурга.<br /><br /> Программа основана на Qt ");
    str.append(QT_VERSION_STR);
    str.append(" (MSVC 2010, 32 бита).<br /><br />");
    str.append("<strong>Разработчик:</strong><br /><a href=https://github.com/Domerk>Domerk</a><br /><br /><strong>Благодарности:</strong><br /><a href=https://github.com/aksenoff>Alex Aksenoff</a>, <a href=https://github.com/aizenbit>Alex Aizenbit</a>");
    aboutBox->setInformativeText(str);
    aboutBox->exec();
}
