#include "configure_dialog.h"
#include "ui_configure_dialog.h"

ConfigureDialog::ConfigureDialog(QWidget *parent) : QDialog(parent), ui_(new Ui::ConfigureDialog)
{
    // Configure window
    ui_->setupUi(this);
    setFixedSize(this->width(), this->height());
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint & ~Qt::WindowCloseButtonHint);

    connect(ui_->spinLVWidth1, SIGNAL(valueChanged(double)), this,
            SLOT(on_videoConfigurationChanged()));
    connect(ui_->spinLVHeight1, SIGNAL(valueChanged(double)), this,
            SLOT(on_videoConfigurationChanged()));
    connect(ui_->spinLVOffsetH1, SIGNAL(valueChanged(double)), this,
            SLOT(on_videoConfigurationChanged()));
    connect(ui_->spinLVOffsetV1, SIGNAL(valueChanged(double)), this,
            SLOT(on_videoConfigurationChanged()));
    connect(ui_->spinRVWidth1, SIGNAL(valueChanged(double)), this,
            SLOT(on_videoConfigurationChanged()));
    connect(ui_->spinRVHeight1, SIGNAL(valueChanged(double)), this,
            SLOT(on_videoConfigurationChanged()));
    connect(ui_->spinRVOffsetH1, SIGNAL(valueChanged(double)), this,
            SLOT(on_videoConfigurationChanged()));
    connect(ui_->spinRVOffsetV1, SIGNAL(valueChanged(double)), this,
            SLOT(on_videoConfigurationChanged()));

    connect(ui_->spinLVWidth2, SIGNAL(valueChanged(double)), this,
            SLOT(on_videoConfigurationChanged()));
    connect(ui_->spinLVHeight2, SIGNAL(valueChanged(double)), this,
            SLOT(on_videoConfigurationChanged()));
    connect(ui_->spinLVOffsetH2, SIGNAL(valueChanged(double)), this,
            SLOT(on_videoConfigurationChanged()));
    connect(ui_->spinLVOffsetV2, SIGNAL(valueChanged(double)), this,
            SLOT(on_videoConfigurationChanged()));
    connect(ui_->spinRVWidth2, SIGNAL(valueChanged(double)), this,
            SLOT(on_videoConfigurationChanged()));
    connect(ui_->spinRVHeight2, SIGNAL(valueChanged(double)), this,
            SLOT(on_videoConfigurationChanged()));
    connect(ui_->spinRVOffsetH2, SIGNAL(valueChanged(double)), this,
            SLOT(on_videoConfigurationChanged()));
    connect(ui_->spinRVOffsetV2, SIGNAL(valueChanged(double)), this,
            SLOT(on_videoConfigurationChanged()));

    // Load configuration from file
    loadConfiguration();
}

ConfigureDialog::~ConfigureDialog()
{
    delete ui_;
}

void ConfigureDialog::loadConfiguration()
{
    QString configuration_file(QApplication::applicationDirPath() + "/Settings.ini");
    QFileInfo file(configuration_file);
    if (!file.exists())
    {
        QMessageBox msg;
        msg.setIcon(QMessageBox::Warning);
        msg.setText("Configuration file not found.");
        msg.exec();
        return;
    }

    QSettings settings(configuration_file, QSettings::IniFormat);

    settings.beginGroup("VideoSettings");
    if (settings.value("configuration_index").toInt() == 1)
    {
        ui_->radioConfig1->setChecked(true);
        ui_->radioConfig2->setChecked(false);
    }
    else if (settings.value("configuration_index").toInt() == 2)
    {
        ui_->radioConfig1->setChecked(false);
        ui_->radioConfig2->setChecked(true);
    }

    ui_->spinLVWidth1->setValue(settings.value("left_width_1").toInt());
    ui_->spinLVHeight1->setValue(settings.value("left_height_1").toInt());
    ui_->spinLVOffsetV1->setValue(settings.value("left_offset_v_1").toInt());
    ui_->spinLVOffsetH1->setValue(settings.value("left_offset_h_1").toInt());
    ui_->spinRVWidth1->setValue(settings.value("right_width_1").toInt());
    ui_->spinRVHeight1->setValue(settings.value("right_height_1").toInt());
    ui_->spinRVOffsetV1->setValue(settings.value("right_offset_v_1").toInt());
    ui_->spinRVOffsetH1->setValue(settings.value("right_offset_h_1").toInt());

    ui_->spinLVWidth2->setValue(settings.value("left_width_2").toInt());
    ui_->spinLVHeight2->setValue(settings.value("left_height_2").toInt());
    ui_->spinLVOffsetV2->setValue(settings.value("left_offset_v_2").toInt());
    ui_->spinLVOffsetH2->setValue(settings.value("left_offset_h_2").toInt());
    ui_->spinRVWidth2->setValue(settings.value("right_width_2").toInt());
    ui_->spinRVHeight2->setValue(settings.value("right_height_2").toInt());
    ui_->spinRVOffsetV2->setValue(settings.value("right_offset_v_2").toInt());
    ui_->spinRVOffsetH2->setValue(settings.value("right_offset_h_2").toInt());
    settings.endGroup();

    settings.beginGroup("chromatic_aberration");
    ui_->checkBoxAberrationEnabled->setChecked(settings.value("enabled").toBool());
    ui_->spinRedScale->setValue(settings.value("red_scale").toDouble());
    ui_->spinGreenScale->setValue(settings.value("green_scale").toDouble());
    ui_->spinRedOffX->setValue(settings.value("red_off_x").toDouble());
    ui_->spinRedOffY->setValue(settings.value("red_off_y").toDouble());
    ui_->spinGreenOffX->setValue(settings.value("green_off_x").toDouble());
    ui_->spinGreenOffY->setValue(settings.value("green_off_y").toDouble());
    ui_->labelFilePath->setText(settings.value("mask_path").toString());
    settings.endGroup();

    settings.beginGroup("zoom");
    ui_->sliderZoom->setValue(settings.value("value").toInt());
    settings.endGroup();

    settings.beginGroup("gui");
    ui_->checkBoxHideGui->setChecked(settings.value("auto_hide").toBool());
    settings.endGroup();

    // Fill parameters struct
    configuration_.red_scale_ = ui_->spinRedScale->value();
    configuration_.green_scale_ = ui_->spinGreenScale->value();
    configuration_.red_offset_x_ = ui_->spinRedOffX->value();
    configuration_.red_offset_y_ = ui_->spinRedOffY->value();
    configuration_.green_offset_x_ = ui_->spinGreenOffX->value();
    configuration_.green_offset_y_ = ui_->spinGreenOffY->value();
    configuration_.zoom_ = ui_->sliderZoom->value();
    configuration_.mask_texture_path_ = ui_->labelFilePath->text();
    configuration_.chromatic_aberration_enabled_ = ui_->checkBoxAberrationEnabled->isChecked();
    configuration_.hide_gui_ = ui_->checkBoxHideGui->isChecked();

    if (ui_->radioConfig1->isChecked())
    {
        configuration_.lv_width_ = ui_->spinLVWidth1->value();
        configuration_.lv_height_ = ui_->spinLVHeight1->value();
        configuration_.lv_offset_x_ = ui_->spinLVOffsetH1->value();
        configuration_.lv_offset_y_ = ui_->spinLVOffsetV1->value();
        configuration_.rv_width_ = ui_->spinRVWidth1->value();
        configuration_.rv_height_ = ui_->spinRVHeight1->value();
        configuration_.rv_offset_x_ = ui_->spinRVOffsetH1->value();
        configuration_.rv_offset_y_ = ui_->spinRVOffsetV1->value();
    }
    else
    {
        configuration_.lv_width_ = ui_->spinLVWidth2->value();
        configuration_.lv_height_ = ui_->spinLVHeight2->value();
        configuration_.lv_offset_x_ = ui_->spinLVOffsetH2->value();
        configuration_.lv_offset_y_ = ui_->spinLVOffsetV2->value();
        configuration_.rv_width_ = ui_->spinRVWidth2->value();
        configuration_.rv_height_ = ui_->spinRVHeight2->value();
        configuration_.rv_offset_x_ = ui_->spinRVOffsetH2->value();
        configuration_.rv_offset_y_ = ui_->spinRVOffsetV2->value();
    }

    loadMaskTexture(configuration_.mask_texture_path_);
}

void ConfigureDialog::loadMaskTexture(QString path)
{
    QPixmap image(path);
    ui_->labelMaskImage->setPixmap(image);

    QFileInfo file(path);
    ui_->labelFilePath->setText(file.absoluteFilePath());
}

void ConfigureDialog::on_pushButtonOpenFile_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this, "Open mask texture", "", "PNG (*.png)");
    if (file_name.isEmpty())
        return;

    loadMaskTexture(file_name);

    configuration_.mask_texture_path_ = file_name;

    emit parametersUpdated(configuration_);
}

void ConfigureDialog::on_pushButtonApplyChannels_clicked()
{
    configuration_.red_scale_ = ui_->spinRedScale->value();
    configuration_.green_scale_ = ui_->spinGreenScale->value();
    configuration_.red_offset_x_ = ui_->spinRedOffX->value();
    configuration_.red_offset_y_ = ui_->spinRedOffY->value();
    configuration_.green_offset_x_ = ui_->spinGreenOffX->value();
    configuration_.green_offset_y_ = ui_->spinGreenOffY->value();

    emit parametersUpdated(configuration_);
}

void ConfigureDialog::on_sliderZoom_valueChanged(int value)
{
    configuration_.zoom_ = value;

    emit parametersUpdated(configuration_);
}

void ConfigureDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    emit parametersUpdated(configuration_);
}

void ConfigureDialog::on_checkBoxHideGui_clicked()
{
    configuration_.hide_gui_ = ui_->checkBoxHideGui->isChecked();
    emit parametersUpdated(configuration_);
}

void ConfigureDialog::on_pushButtonSaveSettings_clicked()
{
    QSettings settings(QApplication::applicationDirPath() + "/Settings.ini", QSettings::IniFormat);

    settings.beginGroup("VideoSettings");
    settings.setValue("configuration_index", ui_->radioConfig1->isChecked() ? 1 : 2);
    settings.setValue("left_width_1", ui_->spinLVWidth1->value());
    settings.setValue("left_height_1", ui_->spinLVHeight1->value());
    settings.setValue("left_offset_v_1", ui_->spinLVOffsetV1->value());
    settings.setValue("left_offset_h_1", ui_->spinLVOffsetH1->value());
    settings.setValue("right_width_1", ui_->spinRVWidth1->value());
    settings.setValue("right_height_1", ui_->spinRVHeight1->value());
    settings.setValue("right_offset_v_1", ui_->spinRVOffsetV1->value());
    settings.setValue("right_offset_h_1", ui_->spinRVOffsetH1->value());
    settings.setValue("left_width_2", ui_->spinLVWidth2->value());
    settings.setValue("left_height_2", ui_->spinLVHeight2->value());
    settings.setValue("left_offset_v_2", ui_->spinLVOffsetV2->value());
    settings.setValue("left_offset_h_2", ui_->spinLVOffsetH2->value());
    settings.setValue("right_width_2", ui_->spinRVWidth2->value());
    settings.setValue("right_height_2", ui_->spinRVHeight2->value());
    settings.setValue("right_offset_v_2", ui_->spinRVOffsetV2->value());
    settings.setValue("right_offset_h_2", ui_->spinRVOffsetH2->value());
    settings.endGroup();

    settings.beginGroup("chromatic_aberration");
    settings.setValue("enabled", ui_->checkBoxAberrationEnabled->isChecked() ? 1 : 0);
    settings.setValue("mask_path", ui_->labelFilePath->text());
    settings.setValue("red_scale", ui_->spinRedScale->value());
    settings.setValue("green_scale", ui_->spinGreenScale->value());
    settings.setValue("red_off_x", ui_->spinRedOffX->value());
    settings.setValue("red_off_y", ui_->spinRedOffY->value());
    settings.setValue("green_off_x", ui_->spinGreenOffX->value());
    settings.setValue("green_off_y", ui_->spinGreenOffY->value());
    settings.endGroup();

    settings.beginGroup("zoom");
    settings.setValue("value", ui_->sliderZoom->value());
    settings.endGroup();

    settings.beginGroup("gui");
    settings.setValue("auto_hide", ui_->checkBoxHideGui->isChecked() ? 1 : 0);
    settings.endGroup();
}

void ConfigureDialog::on_checkBoxAberrationEnabled_clicked()
{
    configuration_.chromatic_aberration_enabled_ = ui_->checkBoxAberrationEnabled->isChecked();

    emit parametersUpdated(configuration_);
}

void ConfigureDialog::on_radioConfig1_clicked()
{
    on_videoConfigurationChanged();
}

void ConfigureDialog::on_radioConfig2_clicked()
{
    on_videoConfigurationChanged();
}

void ConfigureDialog::on_videoConfigurationChanged()
{
    if (ui_->radioConfig1->isChecked())
    {
        configuration_.lv_width_ = ui_->spinLVWidth1->value();
        configuration_.lv_height_ = ui_->spinLVHeight1->value();
        configuration_.lv_offset_x_ = ui_->spinLVOffsetH1->value();
        configuration_.lv_offset_y_ = ui_->spinLVOffsetV1->value();
        configuration_.rv_width_ = ui_->spinRVWidth1->value();
        configuration_.rv_height_ = ui_->spinRVHeight1->value();
        configuration_.rv_offset_x_ = ui_->spinRVOffsetH1->value();
        configuration_.rv_offset_y_ = ui_->spinRVOffsetV1->value();
    }
    else
    {
        configuration_.lv_width_ = ui_->spinLVWidth2->value();
        configuration_.lv_height_ = ui_->spinLVHeight2->value();
        configuration_.lv_offset_x_ = ui_->spinLVOffsetH2->value();
        configuration_.lv_offset_y_ = ui_->spinLVOffsetV2->value();
        configuration_.rv_width_ = ui_->spinRVWidth2->value();
        configuration_.rv_height_ = ui_->spinRVHeight2->value();
        configuration_.rv_offset_x_ = ui_->spinRVOffsetH2->value();
        configuration_.rv_offset_y_ = ui_->spinRVOffsetV2->value();
    }

    emit parametersUpdated(configuration_);
}