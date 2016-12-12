#include "main_window.h"
#include "ui_main_window.h"

MainWindow::MainWindow(int video_width, int video_height, int video_source, int udp_port) :
    QMainWindow(0), ui_(new Ui::MainWindow), conf_dialog_(this)
{
    ui_->setupUi(this);

    // Configure video rendering
    ui_->openGLWidget->setViewportColor(glm::vec3{0.0f, 0.0f, 0.0f});
    ui_->openGLWidget->gstStreamer().setVideoSize(video_width, video_height);
    ui_->openGLWidget->setGUIScale(0.05f);

    switch (video_source)
    {
    case 0:
        ui_->openGLWidget->gstStreamer().setVideoSource(GstStreamer::StreamInput::CAMERAS);
        break;
    case 1:
        ui_->openGLWidget->gstStreamer().setVideoSource(GstStreamer::StreamInput::TEST_FILE);
        break;
    }

    // Configure UDP receiver
    udp_rec_.initUDPClient(udp_port);
    connect(&udp_rec_, &UDPReceiver::robotStateReceived, this, &MainWindow::robotStateReceived);

    // Video parameters signal
    connect(&conf_dialog_, &ConfigureDialog::parametersUpdated, ui_->openGLWidget,
            &GLRenderer::parametersReceived);

    conf_dialog_.show();
}

MainWindow::~MainWindow()
{
    delete ui_;
}

void MainWindow::robotStateReceived(RobotState robot_state)
{
    ui_->openGLWidget->setRobotState(robot_state);
}