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
        baseQuery = "SELECT `ID`, `Фамилия`, `Имя`, `Отчество`, `Тип документа`, `Номер документа`, `Пол`, `Год рождения` FROM Учащиеся;";
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
        ui->comments->setText(tr("Выберите одного или нескольких учащихся:"));
        ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Ок"));
        mainDB = false;
        baseQuery = "SELECT Запись.`Объединение` `Объединение`, Учащийся.`Фамилия` `Фамилия`, Учащийся.`Имя` `Имя`, Учащийся.`Отчество` `Отчество`, Учащийся.`Тип документа` `Тип документа`, Учащийся.`Номер документа` `Номер документа`, Учащийся.`Пол` `Пол`, Учащийся.`Год рождения` `Год рождения`, Учащийся.`Район школы` `Район школы`, Учащийся.`Школа` `Школа`, `Класс` `Класс`, Учащийся.`Родители` `Родители`, Учащийся.`Домашний адрес` `Домашний адрес`, Учащийся.`Телефон` `Телефон`, Учащийся.`e-mail` `e-mail` FROM Учащийся, Запись WHERE Учащийся.`Тип документа` = Запись.`Тип документа` AND Учащийся.`Номер документа` = Запись.`Номер документа`;";
        forSearch = "AND";
    }
}
