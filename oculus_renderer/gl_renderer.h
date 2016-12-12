#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL/SOIL.h>

#include <QDebug>
#include <QGLWidget>
#include <QTimer>

#include <fstream>
#include <memory>
#include <unordered_map>

#include "configure_dialog.h"
#include "exception.h"
#include "gst_streamer.h"
#include "robot_state.h"

struct Mesh
{
    GLuint ebo_{0};
    GLuint vao_{0};
    GLuint vbo_vertices_{0};
    GLuint vbo_tex_coords_{0};
    int indices_count_{0};
};

struct Shader
{
    GLuint handle_{0};
    GLuint vertex_shader_{0};
    GLuint fragment_shader_{0};
    std::unordered_map<std::string, GLint> uniforms_;
    std::string name_{"unnamed_shader"};
};

class GLRenderer : public QGLWidget
{
    Q_OBJECT

public:
    explicit GLRenderer(QWidget *parent = 0)
    {
        Q_UNUSED(parent);

        render_timer_ = new QTimer(this);
        connect(render_timer_, SIGNAL(timeout()), this, SLOT(update()));

        gui_timer_ = new QTimer(this);
        connect(gui_timer_, SIGNAL(timeout()), this, SLOT(hideGui()));
    }

    virtual ~GLRenderer()
    {
        if (mask_texture_)
            glDeleteTextures(1, &mask_texture_);

        for (auto &shader : shaders_)
        {
            glDeleteShader(shader->vertex_shader_);
            glDeleteShader(shader->fragment_shader_);
            glDeleteProgram(shader->handle_);
        }

        if (gui_textures_.emergency_texture_[0])
            glDeleteTextures(2, gui_textures_.emergency_texture_);

        if (gui_textures_.turtle_texture_[0])
            glDeleteTextures(2, gui_textures_.turtle_texture_);

        if (gui_textures_.reverse_texture_[0])
            glDeleteTextures(2, gui_textures_.reverse_texture_);

        if (gui_textures_.battery_texture_[0])
            glDeleteTextures(3, gui_textures_.battery_texture_);

        delete gui_timer_;
        delete render_timer_;
    }

    GstStreamer& gstStreamer() const
    {
        return GstStreamer::instance();
    }

    RobotState getRobotState() const
    {
        return robot_state_;
    }

    void setGUIScale(float scale)
    {
        gui_scale_ = scale;
    }

    void setRobotState(RobotState state)
    {
        robot_state_ = state;
        robot_state_received_ = true;
    }

    void setViewportColor(const glm::vec3 &color)
    {
        viewport_color_ = color;
    }

protected:
    int checkShaderCompileStatus(GLuint shader_handle);
    int loadShaderCode(std::string file_name, std::string &shader_code);
    int loadShaderProgram(std::string vs_path, std::string fs_path, std::shared_ptr<Shader> shader);
    int loadShaderPrograms();
    std::shared_ptr<Shader> getShaderProgram(std::string name) const;
    std::string getShaderCompileMessage(GLuint shader_handle);
    void activateShaderProgram(std::string name);
    void calculateModelMatrices(int window_width, int window_height);

    void clearViewport()
    {
        glViewport(0, 0, viewport_width_, viewport_height_);

        glClearColor(viewport_color_.r, viewport_color_.g, viewport_color_.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void createBuffers();
    void initializeGL();
    void loadGuiTextures();
    void loadTexture(std::string path, GLuint &texture);
    void paintGL();
    void renderRobotState();
    void resizeGL(int width, int height);

protected:
    bool mask_texture_changed_{false};
    bool parameters_updated_{false};
    bool recalculate_gui_matrices_{false};
    bool render_robot_state_{false};
    bool robot_state_received_{false};
    ConfigurationParams video_params_{};
    float gui_scale_{0.5f};
    glm::mat4 model_matrix_video_0_;
    glm::mat4 model_matrix_video_1_;
    glm::vec3 viewport_color_{0.0f, 0.0f, 1.0f};
    GLuint mask_texture_{0};
    int viewport_height_{240};
    int viewport_width_{320};
    Mesh square_{};
    QTimer *gui_timer_{nullptr};
    QTimer *render_timer_{nullptr};
    RobotState robot_state_{};
    std::shared_ptr<Shader> activated_shader_;
    std::vector<std::shared_ptr<Shader>> shaders_;

    struct GUITextures
    {
        GLuint battery_texture_[3]{};
        GLuint emergency_texture_[2]{};
        GLuint reverse_texture_[2]{};
        GLuint turtle_texture_[2]{};
    } gui_textures_;

    HDC device_context_{0};
    HGLRC gl_context_{0};

public slots:
    void hideGui();
    void parametersReceived(ConfigurationParams params);
};

#endif // GL_RENDERER_H