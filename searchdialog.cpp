#include "searchdialog.h"
#include "ui_searchdialog.h"

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("Расширенный поиск"));
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Поиск"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Отмена"));
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

void SearchDialog::on_buttonBox_accepted()
{
    // Флаги, отмечающие, какие таблицы используются
    bool useTech = false;       // Преподаватели
    bool useStud = false;       // Учащиеся
    bool useAss = false;        // Объединения              Да-да, это любимое сокращение Dive (dive155)
    bool useGroup = false;      // Группы

    QString *columns = new QString();
    QString *from = new QString();
    QString *where = new QString();

    QStringList *fromStud = new QStringList();
    QStringList *fromTeach = new QStringList();
    QStringList *fromAss = new QStringList();
    QStringList *fromGroup = new QStringList();

    QStringList *whereStud = new QStringList();
    QStringList *whereTeach = new QStringList();
    QStringList *whereAss = new QStringList();
    QStringList *whereGroup = new QStringList();

    // ---------------------------------------------------------------
    // ------------------------ Преподаватель ------------------------
    // ---------------------------------------------------------------

    if (ui->teachName_ch->isChecked())      // Если чекнут чекбокс
    {
        fromTeach->append("Имя");           // Заносим в стринглист имя поля, которому соотвествует этот чекбокс
        useTech = true;                     // И поднимаем флаг "Используем эту таблицу!"
    }

    if (ui->teachSurname_ch->isChecked())
    {
        fromTeach->append("Фамилия");           // Человек, решивший передалать эту прогу и переименовать поля в базе будет страдать.
        useTech = true;                         // Очень сильно страдать.
    }

    if (ui->teachPatrname_ch->isChecked())
    {
        fromTeach->append("Отчество");
        useTech = true;
    }

    if (ui->teachPass_ch->isChecked())
    {
        fromTeach->append("Паспорт");
        useTech = true;
    }

    if (ui->teachOtd_ch->isChecked())
    {
        fromTeach->append("Отдел");
        useTech = true;
    }

    // ---------------------------------------------------------------

    if (ui->teachName->isModified())        // Если textEdit был изменён
    {
        QString str = ui->teachName->text().replace("--", "-"). simplified();           // Берём из него инфу, отрезав вс лишнее
        if (!str.isEmpty())         // Если после отрезания от инфы что-нибудь осталось
        {
            whereTeach->append("Имя");      // То заносим в стринглист имя поля и его значение
            whereTeach->append(str);
            useTech = true;
        }
    }

    if (ui->teachSurname->isModified())
    {
        QString str = ui->teachSurname->text().replace("--", "-"). simplified();
        if (!str.isEmpty())
        {
        whereTeach->append("Фамилия");
        whereTeach->append(str);
        useTech = true;
        }
    }

    if (ui->teachPatrname->isModified())
    {
        QString str = ui->teachPatrname->text().replace("--", "-"). simplified();
        if (!str.isEmpty())
        {
        whereTeach->append("Отчество");
        whereTeach->append(str);
        useTech = true;
        }
    }

    if (ui->teachPass->isModified())
    {
        QString str = ui->teachPass->text().replace("--", "-"). simplified();
        if (!str.isEmpty())
        {
        whereTeach->append("Паспорт");
        whereTeach->append(str);
        useTech = true;
        }
    }

    if (ui->teachOtd->isModified())
    {
        QString str = ui->teachOtd->text().replace("--", "-"). simplified();
        if (!str.isEmpty())
        {
        whereTeach->append("Отдел");
        whereTeach->append(str);
        useTech = true;
        }
    }

    // ---------------------------------------------------------------
    // -------------------------- Группа -----------------------------
    // ---------------------------------------------------------------

    if (ui->grNum_ch->isChecked())
    {
        fromGroup->append("Номер");
        useGroup = true;
    }

    if (ui->grYear_ch->isChecked())
    {
        fromGroup->append("Год обучения");
        useGroup = true;
    }

    // ---------------------------------------------------------------

    if (ui->grNum->isModified())
    {
        QString str = ui->grNum->text().replace("--", "-"). simplified();
        if (!str.isEmpty())
        {
            whereGroup->append("Номер");
            whereGroup->append(str);
            useTech = true;
        }
    }

    if (ui->grYear->value() != 0)
    {
        whereGroup->append("Год обучения");
        whereGroup->append(ui->grYear->text());
        useTech = true;
    }

    // ---------------------------------------------------------------
    // ---------------------- Объединение ----------------------------
    // ---------------------------------------------------------------

    if(ui->assName_ch->isChecked())
    {
        fromAss->append("Название");
        useAss = true;
    }

    if(ui->assOtd_ch->isChecked())
    {
        fromAss->append("Отдел");
        useAss = true;
    }

    if(ui->assDirect_ch->isChecked())
    {
        fromAss->append("Направленность");
        useAss = true;
    }

    // ---------------------------------------------------------------

    if (ui->assName->isModified())
    {
        QString str = ui->assName->text().replace("--", "-"). simplified();
        if (!str.isEmpty())
        {
            whereAss->append("Название");
            whereAss->append(str);
            useTech = true;
        }
    }

    if (ui->assOtd->isModified())
    {
        QString str = ui->assOtd->text().replace("--", "-"). simplified();
        if (!str.isEmpty())
        {
            whereAss->append("Отдел");
            whereAss->append(str);
            useTech = true;
        }
    }

    if (ui->assDirect->isModified())
    {
        QString str = ui->assDirect->text().replace("--", "-"). simplified();
        if (!str.isEmpty())
        {
            whereAss->append("Направленность");
            whereAss->append(str);
            useTech = true;
        }
    }

    // ---------------------------------------------------------------
    // ------------------------- Учащийся ----------------------------
    // ---------------------------------------------------------------

}

