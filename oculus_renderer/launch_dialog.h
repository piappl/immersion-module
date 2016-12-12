#ifndef LAUNCH_DIALOG_H
#define LAUNCH_DIALOG_H

#include <QApplication>
#include <QDialog>
#include <QLayout>
#include <QMessageBox>
#include <QString>

#include "ui_launch_dialog.h"

namespace Ui
{
    class LaunchDialog;
} // namespace Ui

class LaunchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LaunchDialog(QWidget *parent = 0);
    virtual ~LaunchDialog();

    int getUDPPort() const
    {
        return ui_->spinBoxUdpPort->value();
    }

    int getVideoHeight() const
    {
        return ui_->lineEditVideoHeight->text().toInt();
    }

    int getVideoSource() const
    {
        return ui_->comboBoxVideoSource->currentIndex();
    }

    int getVideoWidth() const
    {
        return ui_->lineEditVideoWidth->text().toInt();
    }

private:
    Ui::LaunchDialog *ui_;

private slots:
    void on_pushButtonClose_clicked();
    void on_pushButtonOK_clicked();
};

#endif // LAUNCH_DIALOG_H