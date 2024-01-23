#pragma once

#include <linux/uvcvideo.h>
#include <memory>
#include <string>
#include <vector>
using namespace std;

#include "opencv.hpp"

class CameraInstruction;

class Camera
{
private:
    bool no_gui_ = false;
    int fd_ = -1;
    const shared_ptr<cv::VideoCapture> cap_ = make_shared<cv::VideoCapture>();
    const vector<int> cap_para_;
    int id_;

    void open_fd();

    void close_fd() noexcept;

    void open_cap();

    void close_cap() noexcept;

    int execute_uvc_query(const uvc_xu_control_query &query);

    bool is_emitter_working_ask();

    bool is_emitter_working_ask_no_gui();

public:
    const string device;

    Camera() = delete;

    explicit Camera(const string &device, int width = -1, int height = -1);

    virtual ~Camera();

    Camera &operator=(const Camera &) = delete;

    Camera(const Camera &) = delete;

    Camera &operator=(Camera &&other) = delete;

    Camera(Camera &&other) = delete;

    void disable_gui();

    function<void()> play();

    void play_forever();

    cv::Mat read1();

    vector<cv::Mat> read_during(unsigned capture_time_ms);

    virtual bool is_emitter_working();

    bool is_gray_scale();

    bool apply(const CameraInstruction &instruction);

    int uvc_set_query(uint8_t unit, uint8_t selector, vector<uint8_t> &control);

    int uvc_get_query(uint8_t query_type, uint8_t unit, uint8_t selector, vector<uint8_t> &control);

    uint16_t uvc_len_query(uint8_t unit, uint8_t selector);

    static shared_ptr<Camera> FindGrayscaleCamera(int width = -1, int height = -1);
};

class CameraException : public exception
{
private:
    string message;

public:
    CameraException(const string &device);

    const char *what() const noexcept override;
};
