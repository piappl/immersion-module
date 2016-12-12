#include "launch_dialog.h"

LaunchDialog::LaunchDialog(QWidget *parent) : QDialog(parent), ui_(new Ui::LaunchDialog)
{
    ui_->setupUi(this);
    setFixedSize(this->width(), this->height());
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

LaunchDialog::~LaunchDialog()
{
    delete ui_;
}

void LaunchDialog::on_pushButtonClose_clicked()
{
    reject();
}

void LaunchDialog::on_pushButtonOK_clicked()
{
    if (ui_->lineEditVideoHeight->text().isEmpty() || ui_->lineEditVideoWidth->text().isEmpty())
    {
        QMessageBox msg;
        msg.setIcon(QMessageBox::Information);
        msg.setText("You must specify video size.");
        msg.setWindowIcon(QIcon(":/Icons/Data/Icon.ico"));
        msg.setWindowTitle("VR Renderer");
        msg.exec();
        return;
    }

    accept();
}