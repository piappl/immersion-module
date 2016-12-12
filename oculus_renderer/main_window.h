#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QDataStream>
#include <QMainWindow>
#include <QString>

#include "configure_dialog.h"
#include "udp_receiver.h"

namespace Ui
{
    class MainWindow;
} // namespace Ui

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(int video_width, int video_height, int video_source, int udp_port);
    virtual ~MainWindow();

private:
    ConfigureDialog conf_dialog_;
    UDPReceiver udp_rec_;
    Ui::MainWindow *ui_;

private slots:
    void robotStateReceived(RobotState robot_state);
};

#endif // MAIN_WINDOW_H