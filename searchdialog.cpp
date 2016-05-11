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

    columns = new QString();
    from = new QString();
    where = new QString();
    query = new QString();

    fromStud = new QStringList();
    fromTeach = new QStringList();
    fromAss = new QStringList();
    fromGroup = new QStringList();

    whereStud = new QStringList();
    whereTeach = new QStringList();
    whereAss = new QStringList();
    whereGroup = new QStringList();
}

SearchDialog::~SearchDialog()
{
    delete ui;

    delete columns;
    delete from;
    delete where;
    delete query;

    delete fromStud;
    delete fromTeach;
    delete fromAss;
    delete fromGroup;

    delete whereStud;
    delete whereTeach;
    delete whereAss;
    delete whereGroup;
}

void SearchDialog::on_buttonBox_accepted()
{

    // Флаги, отмечающие, какие таблицы используются
    bool useTeach = false;       // Преподаватели
    bool useStud = false;       // Учащиеся
    bool useAss = false;        // Объединения              Да-да, это любимое сокращение Dive (dive155)
    bool useGroup = false;      // Группы

    // ---------------------------------------------------------------
    // ------------------------ Преподаватель ------------------------
    // ---------------------------------------------------------------

    if (ui->teachSurname_ch->isChecked())
        fromTeach->append("Фамилия");           // Человек, решивший передалать эту прогу и переименовать поля в базе будет страдать.

    if (ui->teachName_ch->isChecked())      // Если чекнут чекбокс
        fromTeach->append("Имя");           // Заносим в стринглист имя поля, которому соотвествует этот чекбокс

    if (ui->teachPatrname_ch->isChecked())
        fromTeach->append("Отчество");

    if (ui->teachPass_ch->isChecked())
        fromTeach->append("Паспорт");

    if (ui->teachOtd_ch->isChecked())
        fromTeach->append("Отдел");

    // ---------------------------------------------------------------

    if (ui->teachSurname->isModified()) // Если textEdit был изменён
        getDataFromLineEdit(whereTeach, ui->teachSurname, "`Фамилия` LIKE '%%1%'");

    if (ui->teachName->isModified())
        getDataFromLineEdit(whereTeach, ui->teachName, "`Имя` LIKE '%%1%'");

    if (ui->teachPatrname->isModified())
        getDataFromLineEdit(whereTeach, ui->teachPatrname, "`Отчество` LIKE '%%1%'");

    if (ui->teachPass->isModified())
        getDataFromLineEdit(whereTeach, ui->teachPass, "`Паспорт` LIKE '%%1%'");

    if (ui->teachOtd->isModified())
        getDataFromLineEdit(whereTeach, ui->teachOtd, "`Отдел` LIKE '%%1%'");


    // ---------------------------------------------------------------
    // -------------------------- Группа -----------------------------
    // ---------------------------------------------------------------

    if (ui->grNum_ch->isChecked())
        fromGroup->append("Номер");

    if (ui->grYear_ch->isChecked())
        fromGroup->append("Год обучения");

    // ---------------------------------------------------------------

    if (ui->grNum->isModified())
        getDataFromLineEdit(whereGroup, ui->grNum, "`Номер` LIKE '%%1%'");

    if (ui->grYear->value() != 0)
        whereGroup->append("`Год обучения` = " + ui->grYear->text());

    // ---------------------------------------------------------------
    // ---------------------- Объединение ----------------------------
    // ---------------------------------------------------------------

    if(ui->assName_ch->isChecked())
        fromAss->append("Название");

    if(ui->assOtd_ch->isChecked())
        fromAss->append("Отдел");

    if(ui->assDirect_ch->isChecked())
        fromAss->append("Направленность");

    // ---------------------------------------------------------------

    if (ui->assName->isModified())
        getDataFromLineEdit(whereAss, ui->assName, "`Название` LIKE '%%1%'");

    if (ui->assOtd->isModified())
        getDataFromLineEdit(whereAss, ui->assOtd, "`Отдел` LIKE '%%1%'");

    if (ui->assDirect->isModified())
        getDataFromLineEdit(whereAss, ui->assDirect, "`Направленность` LIKE '%%1%'");

    // ---------------------------------------------------------------
    // ------------------------- Учащийся ----------------------------
    // ---------------------------------------------------------------

    if (ui->studSurname_ch->isChecked())
        fromStud->append("Фамилия");

    if (ui->studSurname->isModified())
        getDataFromLineEdit(whereStud, ui->studSurname, "`Фамилия` LIKE '%%1%'");


    if (ui->studName_ch->isChecked())
        fromStud->append("Имя");

    if (ui->studName->isModified())
        getDataFromLineEdit(whereStud, ui->studName, "`Имя` LIKE '%%1%'");


    if (ui->studPatr_ch->isChecked())
        fromStud->append("Отчество");

    if (ui->studPatr->isModified())
        getDataFromLineEdit(whereStud, ui->studPatr, "`Отчество` LIKE '%%1%'");


    if (ui->studAdr_ch->isChecked())
        fromStud->append("Адрес");

    if (ui->studAdr->isModified())
        getDataFromLineEdit(whereStud, ui->studAdr, "`Адрес` LIKE '%%1%'");


    if (ui->studBYear_ch->isChecked())
        fromStud->append("Дата рождения");

    if (ui->studBYear_min->value() != ui->studBYear_min->minimum())
    {
        whereStud->append("`Дата рождения` >= 01.01." + QString::number(ui->studBYear_min->value()));
    }

    if (ui->studBYear_max->value() != ui->studBYear_max->minimum())
    {
        whereStud->append("`Дата рождения` <= 31.12." + QString::number(ui->studBYear_max->value()));
    }


    if (ui->studClass_ch->isChecked())
        fromStud->append("Класс");

    if (ui->studClass->isModified())
        getDataFromLineEdit(whereStud, ui->studClass, "`Класс` LIKE '%%1%'");


    if (ui->studDataIn_ch->isChecked())
        fromStud->append("Дата заявления");

   /* if (ui->studDataIn->isModified())
    {
        QString str = ui->studDataIn->text().simplified().replace(QRegularExpression("-{2,}"), "-");
        if (!str.isEmpty())
        {
            whereStud->append("Дата заявления");
            whereStud->append(str);
        }
    } */

    if (ui->studDataOut_ch->isChecked())
        fromStud->append("Когда выбыл");

   /* if (ui->studDataOut->isModified())
    {
        QString str = ui->studDataOut->text().simplified().replace(QRegularExpression("-{2,}"), "-");
        if (!str.isEmpty())
        {
            whereStud->append("Когда выбыл");
            whereStud->append(str);
        }
    } */


    if (ui->studDocType_ch->isChecked())
        fromStud->append("Тип документа");

    if (ui->studDocType->currentIndex() != 0)
    {
        QString str = ui->studDocType->currentText();
        if (!str.isEmpty())
        {
            whereStud->append("`Тип документа` LIKE '%" + str + "%'");
        }
    }

    if (ui->studDocNum_ch->isChecked())
        fromStud->append("Номер документа");

    if (ui->studDocNum->isModified())
        getDataFromLineEdit(whereStud, ui->studDocNum, "`Номер документа` LIKE '%%1%'");


    if (ui->studEduForm_ch->isChecked())
        fromStud->append("Форма обучения");

    if (ui->studEduForm->currentIndex() != 0)
    {
        QString str = ui->studEduForm->currentText();
        whereStud->append("`Форма обучения` LIKE '%" + str + "%'");
    }


    if (ui->studGender_ch->isChecked())
        fromStud->append("Пол");

    if (ui->studGender->currentIndex() != 0)
    {
        QString str = ui->studGender->currentText();
        whereStud->append("`Пол` LIKE '%" + str + "%'");
    }

    if (ui->studMail_ch->isChecked())
        fromStud->append("e-mail");

    if (ui->studMail->isModified())
        getDataFromLineEdit(whereStud, ui->studMail, "`e-mail` LIKE '%%1%'");


    if (ui->studParents_ch->isChecked())
        fromStud->append("Родители");

    if (ui->studParents->isModified())
        getDataFromLineEdit(whereStud, ui->studParents, "`Родители` LIKE '%%1%'");

    if (ui->studPhone_ch->isChecked())
        fromStud->append("Телефон");

    if (ui->studPhone->isModified())
        getDataFromLineEdit(whereStud, ui->studPhone, "`Телефон` LIKE '%%1%'");


    if (ui->studSchoolReg_ch->isChecked())
        fromStud->append("Район школы");

    if (ui->studSchoolReg->isModified())
        getDataFromLineEdit(whereStud, ui->studSchoolReg, "`Район школы` LIKE '%%1%'");

    if (ui->studSchool_ch->isChecked())
        fromStud->append("Школа");

    if (ui->studSchool->isModified())
        getDataFromLineEdit(whereStud, ui->studSchool, "`Школа` LIKE '%%1%'");

    if (ui->studIncom_ch->isChecked())
        fromStud->append("Неполная семья");

    if (ui->studIncom->currentIndex() != 0)
        getDataFromBoolComboBox(whereStud, ui->studIncom, "`Неполная семья`");

    if (ui->studInv_ch->isChecked())
        fromStud->append("Инвалид");

    if (ui->studInv->currentIndex() != 0)
        getDataFromBoolComboBox(whereStud, ui->studInv, "`Инвалид`");

    if (ui->studLarge_ch->isChecked())
        fromStud->append("Многодетная семья");

    if (ui->studLarge->currentIndex() != 0)
        getDataFromBoolComboBox(whereStud, ui->studLarge, "`Многодетная семья`");

    if (ui->studMigrants_ch->isChecked())
        fromStud->append("Мигранты");

    if (ui->studMigrants->currentIndex() != 0)
        getDataFromBoolComboBox(whereStud, ui->studMigrants, "`Мигранты`");

    if (ui->studNeedy_ch->isChecked())
        fromStud->append("Малообеспеченная семь");

    if (ui->studNeedy->currentIndex() != 0)
        getDataFromBoolComboBox(whereStud, ui->studNeedy, "`Малообеспеченная семья`");

    if (ui->studOrph_ch->isChecked())
        fromStud->append("Сирота");

    if (ui->studOrph->currentIndex() != 0)
        getDataFromBoolComboBox(whereStud, ui->studOrph, "`Сирота`");

    if (ui->studWhealth_ch->isChecked())
        fromStud->append("С ослабленным здоровьем");

    if (ui->studWhealth->currentIndex() != 0)
        getDataFromBoolComboBox(whereStud, ui->studWhealth, "`С ослабленным здоровьем`");

    if (ui->studPolice_ch->isChecked())
        fromStud->append("На учёте в полиции");

    if (ui->studPolice->currentIndex() != 0)
        getDataFromBoolComboBox(whereStud, ui->studPolice, "`На учёте в полиции`");

    // ---------------------------------------------------------------
    // ---------------------------------------------------------------
    // ---------------------------------------------------------------

    useStud = !fromStud->isEmpty() || !whereStud->isEmpty();
    useTeach = !fromTeach->isEmpty() || !whereTeach->isEmpty();
    useAss = !fromAss->isEmpty() || !whereAss->isEmpty();
    useGroup = !fromGroup->isEmpty() || !whereGroup->isEmpty();

    if (useStud)
    {
        from->append(" Учащиеся,");

        for (QString & str : *fromStud)
        {
            columns->append(" Учащиеся.`" + str + "` `Учащийся: " + str + "`,");
        }

        if (!whereStud->isEmpty())
        {
            if (!where->isEmpty())
                where->append(" AND");
            where->append(" Учащиеся." + whereStud->at(0));

            for (int i = 1; i<whereStud->size(); i++)
            {
                where->append(" AND ");
                where->append(" Учащиеся." + whereStud->at(i));
            }
        }

    }

    // ---------------------------------------------------------------

    if (useAss)
    {
        from->append(" Объединения, ");

        for (QString & str : *fromAss)
        {
            columns->append(" Объединения.`" + str + "` `Объединениe: " + str + "`,");
        }

        if (useStud)
        {
            from->append(" Группа, Нагрузка,");
            if (!where->isEmpty())
                where->append(" AND");
            where->append(" Объединения.`ID` = Группа.`ID объединения` AND Учащиеся.`ID` = Нагрузка.`ID учащегося` AND Группа.`ID` = Нагрузка.`ID группы`");
        }

        if (!whereAss->isEmpty())
        {
            if (!where->isEmpty())
                where->append(" AND");
            where->append(" Объединения." + whereAss->at(0));

            for (int i = 1; i<whereAss->size(); i++)
            {
                where->append(" AND ");
                where->append(" Объединения." + whereAss->at(i));
            }
        }
    }

    // ---------------------------------------------------------------

    if (useGroup)
    {
        from->append(" Группа,");
        if (useTeach)
        {
            if (!where->isEmpty())
                where->append(" AND");
            where->append(" Преподаватели.`ID` = Группа.`ID преподавателя`");
        }

        if (useStud)
        {
            from->append(" Нагрузка,");
            if (!where->isEmpty())
                where->append(" AND");
            where->append(" Учащиеся.`ID` = Нагрузка.`ID учащегося` AND Группа.`ID` = Нагрузка.`ID группы`");
        }

        if (useAss)
        {
            if (!where->isEmpty())
                where->append(" AND");
            where->append(" Объединения.`ID` = Группа.`ID объединения`");
        }

        for (QString & str : *fromGroup)
        {
            columns->append(" Группа.`" + str + "` `Группа: " + str + "`,");
        }

        if (!whereGroup->isEmpty())
        {
            if (!where->isEmpty())
                where->append(" AND");
            where->append(" Группа." + whereGroup->at(0));

            for (int i = 1; i<whereGroup->size(); i++)
            {
                where->append(" AND ");
                where->append(" Группа." + whereGroup->at(i));
            }
        }

    }

    // ---------------------------------------------------------------

    if (useTeach)
    {
        from->append(" Преподаватели,");

        for (QString & str : *fromTeach)
        {
            columns->append(" Преподаватели.`" + str + "` `Преподаватель: " + str + "`,");
        }

        if (useStud)
        {
            from->append(" Группа, Нагрузка,");
            if (!where->isEmpty())
                where->append(" AND");
            where->append(" Преподаватели.`ID` = Группа.`ID преподавателя` AND Учащиеся.`ID` = Нагрузка.`ID учащегося` AND Группа.`ID` = Нагрузка.`ID группы`");
        }

        if (!whereTeach->isEmpty())
        {
            if (!where->isEmpty())
                where->append(" AND");
            where->append(" Преподаватели." + whereTeach->at(0));

            for (int i = 1; i<whereTeach->size(); i++)
            {
                where->append(" AND ");
                where->append(" Преподаватели." + whereTeach->at(i));
            }
        }
    }

    // ---------------------------------------------------------------

    columns->resize(columns->size()-1);     // Убираем запятую в конце
    from->resize(from->size()-1);           // Убираем запятую в конце FROM
    query->append("SELECT" + *columns + " FROM" + *from);

    if (!where->isEmpty())
        query->append(" WHERE " + *where);

    query->append(";");
    signalQuery(*query);
}

void SearchDialog::getDataFromLineEdit(QStringList *slist, QLineEdit *ledit, QString text)
{
    QString str = ledit->text().simplified().replace(QRegularExpression("-{2,}"), "-");
    if (!str.isEmpty())
    {
        slist->append(text.arg(str));
    }
}

void SearchDialog::getDataFromBoolComboBox(QStringList *slist, QComboBox *box, QString text)
{
    if (box->currentText() == "Да")
        slist->append(text + " = 'true'");
    else
        slist->append(text + " = 'false'");
}
