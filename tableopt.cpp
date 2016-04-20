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
}

QTableWidget* TableOpt::letTable()
{
    return ui->tableWidget;
}

void TableOpt::setType(QString type)
{
    myType.clear();
    myType = type;

    if (myType == "Ass")
    {
        this->setWindowTitle(tr("Объединения"));
        ui->searchParam->setText(tr("Название:"));
        ui->comments->setText(tr("Выберите объединение:"));
    }

    if (myType == "Stud")
    {
        this->setWindowTitle(tr("Учащиеся"));
        ui->searchParam->setText(tr("Фамилия:"));
        ui->comments->setText(tr("Выберите одного или нескольких учащихся:"));
    }

    if (myType == "Teach")
    {
        this->setWindowTitle(tr("Преподаватель"));
        ui->searchParam->setText(tr("Фамилия:"));
        ui->comments->setText(tr("Выберите преподавателя:"));
    }

    if (myType == "Direct")
    {
        this->setWindowTitle(tr("Направленность"));
        ui->searchParam->hide();
        ui->buttonSearch->hide();
        ui->lineSearch->hide();
        ui->comments->setText(tr("Выберите направленность:"));
    }
}
