#include "gl_renderer.h"

void GLRenderer::initializeGL()
{
    gl_context_ = wglGetCurrentContext();
    device_context_ = wglGetCurrentDC();

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        qDebug() << "GLRenderer::initializeGL(): GLEW initialization error.";
        throw Exception("GLRenderer::initializeGL()", "GLEW initialization error.");
    }

    loadShaderPrograms();
    createBuffers();

    // Prepare GUI textures
    loadGuiTextures();

    // Create mask texture
    glGenTextures(1, &mask_texture_);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    GstStreamer::instance().initialize(gl_context_, device_context_);
    GstStreamer::instance().createPipelines();

    // Start rendering
    render_timer_->start(25);
}

int GLRenderer::loadShaderPrograms()
{
    // Shader for rendering video
    std::shared_ptr<Shader> video_shader(new Shader{});
    video_shader->name_ = "video_shader";
    if (loadShaderProgram("Shaders/video_vs.glsl", "Shaders/video_fs.glsl", video_shader))
        return -1;

    video_shader->uniforms_["model_matrix"] = glGetUniformLocation(video_shader->handle_,
                                                                  "model_matrix");
    video_shader->uniforms_["mask_texture"] = glGetUniformLocation(video_shader->handle_,
                                                                  "mask_texture");
    video_shader->uniforms_["frame_texture"] = glGetUniformLocation(video_shader->handle_,
                                                                  "frame_texture");
    video_shader->uniforms_["red_scale"] = glGetUniformLocation(video_shader->handle_,
                                                               "red_scale");
    video_shader->uniforms_["green_scale"] = glGetUniformLocation(video_shader->handle_,
                                                                 "green_scale");
    video_shader->uniforms_["red_offset_x"] = glGetUniformLocation(video_shader->handle_,
                                                                 "red_offset_x");
    video_shader->uniforms_["red_offset_y"] = glGetUniformLocation(video_shader->handle_,
                                                                 "red_offset_y");
    video_shader->uniforms_["green_offset_x"] = glGetUniformLocation(video_shader->handle_,
                                                                 "green_offset_x");
    video_shader->uniforms_["green_offset_y"] = glGetUniformLocation(video_shader->handle_,
                                                                 "green_offset_y");
    video_shader->uniforms_["chromatic_aber_enabled"] = glGetUniformLocation(video_shader->handle_,
                                                                         "chromatic_aber_enabled");
    video_shader->uniforms_["zoom"] = glGetUniformLocation(video_shader->handle_, "zoom");

    // Shader for rendering gui elements
    std::shared_ptr<Shader> gui_shader(new Shader{});
    gui_shader->name_ = "gui_shader";
    if (loadShaderProgram("Shaders/gui_vs.glsl", "Shaders/gui_fs.glsl", gui_shader))
        return -1;

    gui_shader->uniforms_["model_matrix"] = glGetUniformLocation(gui_shader->handle_,
                                                                 "model_matrix");
    gui_shader->uniforms_["gui_texture"] = glGetUniformLocation(gui_shader->handle_, "gui_texture");

    // Add shaders to shader container
    shaders_.push_back(video_shader);
    shaders_.push_back(gui_shader);

    return 0;
}

void GLRenderer::createBuffers()
{
    GLfloat vertices[] = {
        -1.0f,  1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
    };

    GLfloat texture_coords[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
    };

    GLuint indices[] = {0, 1, 2, 2, 1, 3};
    square_.indices_count_ = 6;

    glGenBuffers(1, &square_.ebo_);
    glGenBuffers(1, &square_.vbo_vertices_);
    glGenBuffers(1, &square_.vbo_tex_coords_);
    glGenVertexArrays(1, &square_.vao_);

    glBindVertexArray(square_.vao_);

    glBindBuffer(GL_ARRAY_BUFFER, square_.vbo_vertices_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, square_.vbo_tex_coords_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coords), texture_coords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, square_.ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLRenderer::activateShaderProgram(std::string name)
{
    auto shader = getShaderProgram(name);
    if (shader)
    {
        activated_shader_ = shader;
        glUseProgram(activated_shader_->handle_);
    }
}

std::shared_ptr<Shader> GLRenderer::getShaderProgram(std::string name) const
{
    for (const auto &shader : shaders_)
    {
        if (name == shader->name_)
            return shader;
    }

    qDebug() << "GLRenderer::getShaderProgram(): Shader " << name.c_str() << " not found.";
    return std::shared_ptr<Shader>{nullptr};
}

int GLRenderer::checkShaderCompileStatus(GLuint shader_handle)
{
    int shader_status{-1};
    glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &shader_status);
    if (shader_status != GL_TRUE)
        return -1;

    return 0;
}

int GLRenderer::loadShaderCode(std::string file_name, std::string &shader_code)
{
    std::ifstream shader_file(file_name, std::ios::in);
    if (shader_file.is_open())
    {
        std::string line;
        while (std::getline(shader_file, line))
            shader_code += line + "\n";

        shader_file.close();
    }
    else
        return -1;

    return 0;
}

int GLRenderer::loadShaderProgram(std::string vs_path, std::string fs_path,
                                  std::shared_ptr<Shader> shader)
{
    shader->handle_ = glCreateProgram();

    shader->vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
    shader->fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);

    for (const auto &type : {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER})
    {
        GLuint *sh{nullptr};
        std::string path;
        if (type == GL_VERTEX_SHADER)
        {
            sh = &shader->vertex_shader_;
            path = vs_path;
        }
        else if (type == GL_FRAGMENT_SHADER)
        {
            sh = &shader->fragment_shader_;
            path = fs_path;
        }

        std::string shader_code;
        const char *shader_code_ptr{nullptr};

        if (loadShaderCode(path, shader_code))
        {
            qDebug() << "GLRenderer::loadShaderProgram(): Shader " << path.c_str() << " not found.";
            return -1;
        }

        shader_code_ptr = shader_code.c_str();
        glShaderSource(*sh, 1, &shader_code_ptr, nullptr);
        glCompileShader(*sh);
        if (checkShaderCompileStatus(*sh))
        {
            qDebug() << "GLRenderer::loadShaderProgram(): " << getShaderCompileMessage(*sh).c_str();
            return -1;
        }
    }

    glAttachShader(shader->handle_, shader->vertex_shader_);
    glAttachShader(shader->handle_, shader->fragment_shader_);
    glLinkProgram(shader->handle_);

    return 0;
}

std::string GLRenderer::getShaderCompileMessage(GLuint shader_handle)
{
    GLint log_size{256};
    GLchar *log_text = new GLchar[log_size];
    glGetShaderInfoLog(shader_handle, log_size, nullptr, log_text);
    std::string compile_msg = std::string(log_text);
    if (compile_msg.empty())
        compile_msg = "[NO WARNINGS AND ERRORS]";

    delete log_text;
    return compile_msg;
}

void GLRenderer::parametersReceived(ConfigurationParams params)
{
    if (video_params_.mask_texture_path_ != params.mask_texture_path_)
        mask_texture_changed_ = true;

    video_params_ = params;
    parameters_updated_ = true;

    if (video_params_.hide_gui_)
    {
        if (!gui_timer_->isActive())
            gui_timer_->start(1000);
    }
    else
    {
        gui_timer_->stop();
        render_robot_state_ = true;
    }
}

void GLRenderer::resizeGL(int width, int height)
{
    viewport_width_ = width;
    viewport_height_ = height;

    calculateModelMatrices(viewport_width_, viewport_height_);

    recalculate_gui_matrices_ = true;
}

void GLRenderer::calculateModelMatrices(int window_width, int window_height)
{
    // Video size in screen coords
    // Left video
    float lv_height{static_cast<float>(video_params_.lv_height_) /
                static_cast<float>(window_height)};
    float lv_width{static_cast<float>(video_params_.lv_width_) /
                static_cast<float>(window_width)};
    // Right video
    float rv_height{static_cast<float>(video_params_.rv_height_) /
                static_cast<float>(window_height)};
    float rv_width{static_cast<float>(video_params_.rv_width_) /
                static_cast<float>(window_width)};

    // Calculate video offset in screen coords
    // Left video
    float lv_offset_v{static_cast<float>(video_params_.lv_offset_y_) /
                static_cast<float>(window_height)};
    float lv_offset_h{static_cast<float>(video_params_.lv_offset_x_) /
                static_cast<float>(window_width)};
    // Right video
    float rv_offset_v{static_cast<float>(video_params_.rv_offset_y_) /
                static_cast<float>(window_height)};
    float rv_offset_h{static_cast<float>(video_params_.rv_offset_x_) /
                static_cast<float>(window_width)};

    // Calculate translations vectors
    // Left video
    float lv_x{-1.0f + lv_width + lv_offset_h * 2.0f};
    float lv_y{1.0f - lv_height - lv_offset_v * 2.0f};

    model_matrix_video_0_ = glm::translate(glm::mat4{1.0f}, glm::vec3{lv_x, lv_y, 0.0f});
    model_matrix_video_0_ = glm::scale(model_matrix_video_0_, glm::vec3{lv_width, lv_height, 0.0f});

    // Right video
    float rv_x{0.0f + rv_width + rv_offset_h * 2.0f};
    float rv_y{1.0f - rv_height - rv_offset_v * 2.0f};

    model_matrix_video_1_ = glm::translate(glm::mat4{1.0f}, glm::vec3{rv_x, rv_y, 0.0f});
    model_matrix_video_1_ = glm::scale(model_matrix_video_1_, glm::vec3{rv_width, rv_height, 0.0f});
}

void GLRenderer::paintGL()
{
    GstStreamer::instance().iterate();
    clearViewport();

    // Render video
    // Activate video shader for video
    activateShaderProgram("video_shader");

    // Update shader uniforms
    if (parameters_updated_)
    {
        glUniform1iv(activated_shader_->uniforms_["frame_texture"], 1, &static_cast<const GLint&>(0));
        glUniform1iv(activated_shader_->uniforms_["mask_texture"], 1, &static_cast<const GLint&>(1));

        glUniform1i(activated_shader_->uniforms_["chromatic_aber_enabled"],
                    static_cast<int>(video_params_.chromatic_aberration_enabled_));

        glUniform1f(activated_shader_->uniforms_["red_scale"], video_params_.red_scale_);
        glUniform1f(activated_shader_->uniforms_["green_scale"], video_params_.green_scale_);

        glUniform1f(activated_shader_->uniforms_["red_offset_x"], video_params_.red_offset_x_);
        glUniform1f(activated_shader_->uniforms_["red_offset_y"], video_params_.red_offset_y_);

        glUniform1f(activated_shader_->uniforms_["green_offset_x"], video_params_.green_offset_x_);
        glUniform1f(activated_shader_->uniforms_["green_offset_y"], video_params_.green_offset_y_);

        float zoom{static_cast<float>(video_params_.zoom_)};
        zoom /= 100.0f;
        zoom = 1.0f - zoom;
        glUniform1f(activated_shader_->uniforms_["zoom"], zoom);

        if (mask_texture_changed_)
        {
            loadTexture(video_params_.mask_texture_path_.toStdString(), mask_texture_);
            mask_texture_changed_ = false;
        }

        calculateModelMatrices(viewport_width_, viewport_height_);

        parameters_updated_ = false;
    }

    // Bind mask texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mask_texture_);

    // Bind mesh for rendering video
    glBindVertexArray(square_.vao_);

    // Bind video textures and render them
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, GstStreamer::instance().getTexture(0));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);

    glUniformMatrix4fv(activated_shader_->uniforms_["model_matrix"], 1, GL_FALSE,
                       glm::value_ptr(model_matrix_video_0_));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindTexture(GL_TEXTURE_2D, GstStreamer::instance().getTexture(1));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);

    glUniformMatrix4fv(activated_shader_->uniforms_["model_matrix"], 1, GL_FALSE,
                       glm::value_ptr(model_matrix_video_1_));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Unbind mesh
    glBindVertexArray(0);

    // Render robot state on screen
    if (render_robot_state_)
        renderRobotState();
}

void GLRenderer::loadTexture(std::string path, GLuint &texture)
{
    int width{0};
    int height{0};
    int channels{0};

    unsigned char *image = SOIL_load_image(path.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO);

    if (!image)
    {
       qDebug() << "GLRenderer::setMaskTexture(): Error loading texture: " << path.c_str();
       throw Exception("GLRenderer::setMaskTexture()", "Error loading texture " + path + ".");
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    if (channels == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    else if (channels == 3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLRenderer::loadGuiTextures()
{
    glGenTextures(2, gui_textures_.emergency_texture_);
    glGenTextures(2, gui_textures_.turtle_texture_);
    glGenTextures(2, gui_textures_.reverse_texture_);
    glGenTextures(3, gui_textures_.battery_texture_);

    loadTexture("Icons/stop_on.png", gui_textures_.emergency_texture_[0]);
    loadTexture("Icons/stop_off.png", gui_textures_.emergency_texture_[1]);
    loadTexture("Icons/turtle_on.png", gui_textures_.turtle_texture_[0]);
    loadTexture("Icons/turtle_off.png", gui_textures_.turtle_texture_[1]);
    loadTexture("Icons/reverse_on.png", gui_textures_.reverse_texture_[0]);
    loadTexture("Icons/reverse_off.png", gui_textures_.reverse_texture_[1]);
    loadTexture("Icons/battery_low.png", gui_textures_.battery_texture_[0]);
    loadTexture("Icons/battery_med.png", gui_textures_.battery_texture_[1]);
    loadTexture("Icons/battery_hi.png", gui_textures_.battery_texture_[2]);
}

void GLRenderer::renderRobotState()
{
    constexpr int gui_elements_count{4};

    static glm::mat4 gui_model_matrix_left_[gui_elements_count];
    static glm::mat4 gui_model_matrix_right_[gui_elements_count];

    activateShaderProgram("gui_shader");

    if (recalculate_gui_matrices_)
    {
        int gui_element_px = gui_scale_ * viewport_width_;
        // GUI icons must be always on the middle of the video
        // Values in px
        int gui_offset_x_lv = video_params_.lv_offset_x_ + video_params_.lv_width_ / 2 -
                              (gui_elements_count - 1) * gui_element_px;
        int gui_offset_x_rv = video_params_.rv_offset_x_ + video_params_.rv_width_ / 2 -
                              (gui_elements_count - 1) * gui_element_px;
        int gui_offset_y = video_params_.lv_offset_y_ + 150;

        glUniform1iv(activated_shader_->uniforms_["gui_texture"], 1, &static_cast<const GLint&>(0));

        float offset_h_screen_lv = static_cast<float>(gui_offset_x_lv) /
                                   static_cast<float>(viewport_width_);
        float offset_h_screen_rv = static_cast<float>(gui_offset_x_rv) /
                                   static_cast<float>(viewport_width_);
        float offset_v_screen = static_cast<float>(gui_offset_y) /
                                static_cast<float>(viewport_height_);

        // "Zero" position for first gui element
        float trans_gui_x_left = -1.0f + gui_scale_ + offset_h_screen_lv * 2.0f;
        float trans_gui_x_right = 0.0f + gui_scale_ + offset_h_screen_rv * 2.0f;
        float trans_gui_y = 1.0f - gui_scale_ - offset_v_screen * 2.0f;

        // Calculate GUI elements model matrices
        float gui_spacing = 2.0f * gui_scale_ + 0.03f;

        for (int i = 0; i < gui_elements_count; i++)
        {
            gui_model_matrix_left_[i] = glm::translate(glm::mat4{1.0f},
                                                       glm::vec3{trans_gui_x_left + i * gui_spacing,
                                                                 trans_gui_y,
                                                                 0.0f});
            gui_model_matrix_left_[i] = glm::scale(gui_model_matrix_left_[i],
                                                   glm::vec3{gui_scale_, gui_scale_, 0.0f});

            gui_model_matrix_right_[i] = glm::translate(glm::mat4{1.0f},
                                                        glm::vec3{trans_gui_x_right + i * gui_spacing,
                                                                  trans_gui_y,
                                                                  0.0f});
            gui_model_matrix_right_[i] = glm::scale(gui_model_matrix_right_[i],
                                                    glm::vec3{gui_scale_, gui_scale_, 0.0f});
        }

        recalculate_gui_matrices_ = false;
    }

    // Render GUI elements
    glBindVertexArray(square_.vao_);

    std::vector<GLuint> textures_up;

    if (robot_state_.turtle_mode_on_)
        textures_up.push_back(gui_textures_.turtle_texture_[0]);
    else
        textures_up.push_back(gui_textures_.turtle_texture_[1]);

    if (robot_state_.reverse_mode_on_)
        textures_up.push_back(gui_textures_.reverse_texture_[0]);
    else
        textures_up.push_back(gui_textures_.reverse_texture_[1]);

    if (robot_state_.battery_level_ >= 66)
        textures_up.push_back(gui_textures_.battery_texture_[2]);
    else if (robot_state_.battery_level_ >= 33 && robot_state_.battery_level_ < 66)
        textures_up.push_back(gui_textures_.battery_texture_[1]);
    else if (robot_state_.battery_level_ < 33)
        textures_up.push_back(gui_textures_.battery_texture_[0]);

    if (robot_state_.emergency_on_)
        textures_up.push_back(gui_textures_.emergency_texture_[0]);
    else
        textures_up.push_back(gui_textures_.emergency_texture_[1]);

    for (int i = 0; i < gui_elements_count; i++)
    {
        glBindTexture(GL_TEXTURE_2D, textures_up[i]);

        glUniformMatrix4fv(activated_shader_->uniforms_["model_matrix"], 1, GL_FALSE,
                           glm::value_ptr(gui_model_matrix_left_[i]));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glUniformMatrix4fv(activated_shader_->uniforms_["model_matrix"], 1, GL_FALSE,
                           glm::value_ptr(gui_model_matrix_right_[i]));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
}

void GLRenderer::hideGui()
{
    if (robot_state_received_)
    {
        render_robot_state_ = true;
        robot_state_received_ = false;
    }
    else
        render_robot_state_ = false;
}