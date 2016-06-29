#include "help.h"
#include "ui_help.h"

Help::Help(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Help)
{
    ui->setupUi(this);
    ui->splitter->setSizes(QList <int> () << 120 << 400);
    ui->treeWidget->setColumnHidden(1, true);
    this->setWindowTitle(tr("Справка"));
    this->setWindowFlags(Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
}

Help::~Help()
{
    delete ui;
}

void Help::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    QString index = item->text(1);
}
