#pragma once

#include <format>
#include <optional>
#include <vector>
using namespace std;

#include "camera/camera.hpp"
#include "camera/camerainstruction.hpp"

class Configuration;
using Configurations = vector<Configuration>;

class Configuration {
 public:
  // The camera device to witch find a saved configuration
  CameraPtr camera;

 private:
  // Path to the configuration folder
  static const string SAVE_FOLDER_CONFIG_PATH;

  // Path prefix for the v4l devices
  static const string V4L_PREFIX;

  bool save_on_del_ = false;

  // The path to the configuration file (may not exist)
  string path_;

  // The instructions of the configuration
  CameraInstructions instructions_;

  /**
   * @brief Get the v4l name of a device.
   * @throw `Exception` if unable to get the v4l name of the camera
   * @return the v4l name of the camera
   */
  string v4lname() const;

  /**
   * @brief Load the configuration of the camera
   * @param init if unable to load the configuration file, create a new one
   * @throw `Exception` if unable to load the configuration file (and `init` is false)
   */
  void load(bool init);

  /**
   * @brief Save the configuration of the camera
   * @param init does the configuration is considered as the initial
   * @throw `Exception` if unable to save the configuration file
   */
  void save(bool init) const;

 public:
  /**
   * @brief Create a new configuration for a camera
   * @param camera the camera to load the configuration
   * @param init if unable to load the configuration file of the camera, create a new one
   * @throw `Exception` if unable to get the v4l name of the camera
   */
  explicit Configuration(CameraPtr camera, bool init);

  ~Configuration();

  /**
   * Get the instructions of the configuration
   * @return the instructions of the configuration
   */
  CameraInstructions &instructions() noexcept;

  /**
   * @brief Get all the device configured, only based on the file name
   * @return device path of the configured camera
   */
  static Configurations ConfiguredDevices() noexcept;

  CameraInstructions::iterator begin();
  CameraInstructions::iterator end();
  CameraInstructions::const_iterator begin() const;
  CameraInstructions::const_iterator end() const;

  Configuration &operator=(const Configuration &) = default;
  Configuration(const Configuration &) = default;
  Configuration &operator=(Configuration &&other) = default;
  Configuration(Configuration &&other) = default;

  class Exception : public exception {
    string message;

   public:
    template <typename... Args>
    explicit Exception(const string &format, Args... args)
        : message(std::vformat(format, std::make_format_args(args...))) {}

    const char *what() const noexcept override { return message.c_str(); }
  };
};
