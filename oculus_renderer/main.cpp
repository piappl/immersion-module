#include <QApplication>
#include <QMessageBox>

#include "launch_dialog.h"
#include "main_window.h"

#include "exception.h"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    // Load style sheet
    QFile file("Style/darkorange.stylesheet");
    file.open(QFile::ReadOnly);
    QString style_sheet = QLatin1String{file.readAll()};
    application.setStyleSheet(style_sheet);

    int result{0};

    try
    {
        LaunchDialog conf_dialog;
        if (conf_dialog.exec())
        {
            auto video_width = conf_dialog.getVideoWidth();
            auto video_height = conf_dialog.getVideoHeight();
            auto video_index = conf_dialog.getVideoSource();
            auto udp_port = conf_dialog.getUDPPort();

            MainWindow main_window(video_width, video_height, video_index, udp_port);
            main_window.show();

            result = application.exec();
        }
    }
    catch (const Exception &ex)
    {
        QMessageBox msg;
        msg.setIcon(QMessageBox::Critical);
        msg.setText(ex.what().c_str());
        msg.exec();
    }

    return result;
}