#include "tableopt.h"
#include "ui_tableopt.h"

TableOpt::TableOpt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TableOpt)
{
    ui->setupUi(this);

    myType = "None";

    connect(ui->buttonSearch, SIGNAL(clicked()), this, SLOT(search()));
    connect(ui->lineSearch, SIGNAL(returnPressed()), this, SLOT(search()));

    ui->buttonSearch->setIcon(QIcon(":/icons/Icons/search.png"));
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Добавить"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Отмена"));

    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

}

TableOpt::~TableOpt()
{
    delete ui;
}

void TableOpt::on_lineSearch_textChanged(const QString &arg1)
{
    ui->buttonSearch->setEnabled(true);
}

void TableOpt::search()
{
    QString serchStr = ui->lineSearch->text().simplified().replace(QRegularExpression("-{2,}"), "-");
    QString strQuery = baseQuery.replace(";", " ") + forSearch + " `" + ui->searchParam->currentText() + "` LIKE '%" + serchStr + "%'";
    emit signalQuery(ui->tableWidget, strQuery, mainDB);
}

QTableWidget* TableOpt::letTable()
{
    return ui->tableWidget;
}

QComboBox* TableOpt::letSearchBox()
{
    return ui->searchParam;
}

void TableOpt::setType(QString type)
{
    myType.clear();
    baseQuery.clear();
    forSearch.clear();
    myType = type;
    mainDB = true;

    ui->deleteButton->hide();
    ui->insertButton->hide();

    if (ui->searchParam->isHidden())
    {
        ui->searchParam->show();
        ui->buttonSearch->show();
        ui->lineSearch->show();
    }

    if (myType == "Ass")
    {
        this->setWindowTitle(tr("Объединения"));
        ui->comments->setText(tr("Выберите объединение:"));
        baseQuery = "SELECT `ID`, `Название`, `Отдел`, `Описание` FROM Объединения;";
        forSearch = "WHERE";
    }

    if (myType == "Stud")
    {
        this->setWindowTitle(tr("Учащиеся"));
        ui->comments->setText(tr("Выберите одного или нескольких учащихся:"));
        baseQuery = "SELECT `ID`, `Фамилия`, `Имя`, `Отчество`, `Тип документа`, `Номер документа`, `Пол`, `Дата рождения` FROM Учащиеся;";
        forSearch = "WHERE";
    }

    if (myType == "Teach")
    {
        this->setWindowTitle(tr("Преподаватель"));
        ui->comments->setText(tr("Выберите преподавателя:"));
        baseQuery = "SELECT `ID`, `Фамилия`, `Имя`, `Отчество`, `Отдел` FROM Преподаватели;";
        forSearch = "WHERE";
    }

    if (myType == "Direct")
    {
        this->setWindowTitle(tr("Направленность"));
        ui->searchParam->hide();
        ui->buttonSearch->hide();
        ui->lineSearch->hide();
        ui->comments->setText(tr("Выберите направленность:"));
        baseQuery = "SELECT `ID`, `Название` FROM Направленности;";
        forSearch = "WHERE";
    }

    if (myType == "tempDB")
    {
        this->setWindowTitle(tr("Учащиеся - самозапись"));
        ui->comments->setText(tr("Выберите учащегося и действие:"));
        ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Ок"));
        ui->deleteButton->show();
        ui->insertButton->show();
        mainDB = false;
        baseQuery = "SELECT Запись.`Объединение` `Объединение`, Учащийся.`Фамилия` `Фамилия`, Учащийся.`Имя` `Имя`, Учащийся.`Отчество` `Отчество`, Учащийся.`Тип документа` `Тип документа`, Учащийся.`Номер документа` `Номер документа`, Учащийся.`Пол` `Пол`, Учащийся.`Дата рождения` `Дата рождения`, Учащийся.`Район школы` `Район школы`, Учащийся.`Школа` `Школа`, Учащийся.`Класс` `Класс`, Учащийся.`Родители` `Родители`, Учащийся.`Домашний адрес` `Домашний адрес`, Учащийся.`Телефон` `Телефон`, Учащийся.`e-mail` `e-mail`, Учащийся.`Дата заявления` `Дата заявления` FROM Учащийся, Запись WHERE Учащийся.`Тип документа` = Запись.`Тип документа` AND Учащийся.`Номер документа` = Запись.`Номер документа`;";
        forSearch = "AND";
    }
}

void TableOpt::on_insertButton_clicked()
{
    int row = ui->tableWidget->currentRow();
    if (row > -1) // Если есть выделенные строки
    {
        QStringList qsl;
        QString strQuery;
        for (QTableWidgetSelectionRange selectionRange : ui->tableWidget->selectedRanges())
        {
            if (selectionRange.rowCount() > 0)
            {
                for (int row = selectionRange.topRow(); row <= selectionRange.bottomRow(); row++)
                {
                    // `Фамилия`, `Имя`, `Отчество`,`Тип документа`, `Номер документа`, `Пол`, `Дата рождения`, `Район школы`, `Школа`, `Класс`, `Родители`, `Домашний адрес`, `Телефон`, `e-mail`, `Дата заявления`
                    strQuery.append("INSERT INTO Учащиеся (`Фамилия`, `Имя`, `Отчество`,`Тип документа`, `Номер документа`, `Пол`, `Дата рождения`, `Район школы`, `Школа`, `Класс`, `Родители`, `Домашний адрес`, `Телефон`, `e-mail`, `Дата заявления`) VALUES (");
                    for (int i = 1; i<ui->tableWidget->columnCount(); i++)
                        strQuery.append(" '" + ui->tableWidget->item(row, i)->text() + "',");
                    strQuery.resize(strQuery.size()-1);
                    strQuery.append(");");
                    qsl.append(strQuery);
                    strQuery.clear();
                    strQuery.append("INSERT INTO Нагрузка (`ID учащегося`, `ID группы`) VALUES ((SELECT `ID` FROM Учащиеся WHERE `Тип документа` = '" + ui->tableWidget->item(row, 4)->text() + "' AND `Номер документа` = '" + ui->tableWidget->item(row, 5)->text() + "'), (SELECT Группа.`ID` FROM Группа, Объединение WHERE Группа.`ID объединения` = Объединение.`ID` AND Объединение.`Название` = '" +ui->tableWidget->item(row, 0)->text()  + "' AND Группа.`Номер` = 'Без группы'));");
                    qsl.append(strQuery);
                }
            }
        }
        emit signalQueries(qsl, true);
        on_deleteButton_clicked();
        emit signalQuery(ui->tableWidget, baseQuery, mainDB);      
    }
}

void TableOpt::on_deleteButton_clicked()
{
    int row = ui->tableWidget->currentRow();
    if (row > -1) // Если есть выделенные строки
    {
        QStringList qsl;
        for (QTableWidgetSelectionRange selectionRange : ui->tableWidget->selectedRanges())
        {
            if (selectionRange.rowCount() > 0)
                for (int row = selectionRange.topRow(); row <= selectionRange.bottomRow(); row++)
                {
                    qsl.append("DELETE FROM Учащийся WHERE `Тип документа` = '" + ui->tableWidget->item(row, 4)->text() + "' AND `Номер документа` = '" + ui->tableWidget->item(row, 5)->text() + "'");
                }
        }
        emit signalQueries(qsl, false);
        emit signalQuery(ui->tableWidget, baseQuery, mainDB);
    }
}
