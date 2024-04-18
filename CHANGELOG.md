# 2024 - 6.0.0-beta
- Usage of YAML to store camera configuration
- Save of all the camera instructions
- Tweak manually any instructions of a camera
- Specify camera resolution
- Catch ctrl-c (one time) to avoid breaking camera
- Zsh completion
- Fix video feedback freeze
- Better logging
- Fix test command
- Fix multiple v4l path for same device
- OpenCV imshow exception
- Remove boot command
- Migrate all Python code to C++
- Reduction in installation size
# Wed Nov 1 2023 - 5.2.4
- Fix config generated but not found
- Fix for OpenCV exception thrown
- Reduction in installation size
# Fri Oct 20 2023 - 5.2.1
- Fix unable to execute commands
# Thu Oct 19 2023 - 5.2.0
- New test command
- Improve efficiency
- Significant reduction in installation size
# Mon Oct 9 2023 - 5.0.4
- New advice and hint messages
# Wed Aug 23 2023 - 5.0.2
- Minor search fix
- Longer search by default
# Tue Aug 8 2023 - 5.0.0
- Automatic ir camera detection
- Automatic ir emitter configuration
- Exhaustive search
- Video feedback
# Fri Jun 23 2023 - 4.8.2
- Fix systemd hanging in some cases
- More appropriate installation paths
# Wed Jun 21 2023 - 4.8.0
- Native SELinux compatibility
# Tue Jun 20 2023 - 4.7.0
- Add openrc support
- Python >= 3.10
# Son Feb 26 2023 - 4.5.0
- Improvement of config generation 
# Fri Feb 24 2023 - 4.4.2
- Fix command not found
- Smaller size
# Fri Feb 17 2023 - 4.4.0
- Total rework of the implementation
- Support multiple emitters camera
- Memorize broken instructions to skip them 
- Usage of /dev/v4l/by-path for persistence
# Tue Sep 13 2022 - 4.1.5
- Fix boot service for custom device 
# Thu Aug 11 2022 - 4.1.4
- Force V4l2 backend in opencv
- Improvement of config generation
# Mon Jul 4 2022 - 4.1.2
- Asynchronous camera triggering
- Fix camera triggering issue
- Fix device symlink boot service side effect
# Sun Jun 19 2022 - 4.0.0
- Rework, optimization and improvement of config generation 
- Remove manual configuration commands
- Remove option for integration into Howdy
# Thu Dec 9 2021 - 3.2.5
- Tweak for integration into Howdy(https://github.com/boltgolt/howdy)  
- Bash auto completion
- Better systemd support
# Thu Nov 4 2021 - 3.2.2
- Support any device path format
- Improve systemd service
# Sat Oct 23 2021 - 3.2.0
- Multiple device support
# Thu Sep 23 2021 - 3.1.1
- Limit in negative answers for a same pattern
# Wed Sep 22 2021 - 3.1.0
- New configuration system
- Exit codes
- Change configuration file location
# Sun Aug 29 2021 - 2.1.0
- New fix command, to resolve well know problems
- Systemd service file modified to prevent /dev/video file descriptor error
# Thu Aug 12 2021 - 2.0.1
- Initial release
