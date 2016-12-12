#ifndef CONFIGURE_DIALOG_H
#define CONFIGURE_DIALOG_H

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QString>

namespace Ui
{
    class ConfigureDialog;
} // namespace Ui

struct ConfigurationParams
{
    bool chromatic_aberration_enabled_;
    QString mask_texture_path_;
    float red_scale_;
    float red_offset_x_;
    float red_offset_y_;
    float green_scale_;
    float green_offset_x_;
    float green_offset_y_;
    int zoom_;
    bool hide_gui_;
    int lv_width_;
    int lv_height_;
    int lv_offset_x_;
    int lv_offset_y_;
    int rv_width_;
    int rv_height_;
    int rv_offset_x_;
    int rv_offset_y_;
};

class ConfigureDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigureDialog(QWidget *parent = 0);
    virtual ~ConfigureDialog();

protected:
    void loadConfiguration();
    void loadMaskTexture(QString path);
    void showEvent(QShowEvent *event) override;

private:
    ConfigurationParams configuration_;
    Ui::ConfigureDialog *ui_;

private slots:
    void on_checkBoxAberrationEnabled_clicked();
    void on_checkBoxHideGui_clicked();
    void on_pushButtonApplyChannels_clicked();
    void on_pushButtonOpenFile_clicked();
    void on_pushButtonSaveSettings_clicked();
    void on_radioConfig1_clicked();
    void on_radioConfig2_clicked();
    void on_sliderZoom_valueChanged(int value);
    void on_videoConfigurationChanged();

signals:
    void parametersUpdated(ConfigurationParams configuration);
};

#endif // CONFIGURE_DIALOG_H