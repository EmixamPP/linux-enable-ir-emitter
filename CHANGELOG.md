# Fri Feb 17 2023 Maxime Dirksen - 4.4.0
- Total rework of the implementation
- Support multiple emitters camera
- Memorize broken instructions to skip them 
- Usage of /dev/v4l/by-path for persistence
- Drop distribution repositories support
# Tue Sep 13 2022 Maxime Dirksen - 4.1.5
- Fix boot service for custom device 
# Thu Aug 11 2022 Maxime Dirksen - 4.1.4
- Force V4l2 backend in opencv
- Improvement of driver generation
# Mon Jul 4 2022 Maxime Dirksen - 4.1.2
- Asynchronous camera triggering
- Fix camera triggering issue
- Fix device symlink boot service side effect
# Sun Jun 19 2022 Maxime Dirksen - 4.0.0
- Rework, optimization and improvement of driver generation 
- Remove manual configuration commands
- Remove option for integration into Howdy
# Thu Dec 9 2021 Maxime Dirksen - 3.2.5
- Tweak for integration into Howdy(https://github.com/boltgolt/howdy)  
- Bash auto completion
- Better systemd support
# Thu Nov 4 2021 Maxime Dirksen - 3.2.2
- Support any device path format
- Improve systemd service
# Sat Oct 23 2021 Maxime Dirksen - 3.2.0
- Multiple device support
# Thu Sep 23 2021 Maxime Dirksen - 3.1.1
- Limit in negative answers for a same pattern
# Wed Sep 22 2021 Maxime Dirksen - 3.1.0
- New configuration system
- Exit codes
- Change configuration file location
# Sun Aug 29 2021 Maxime Dirksen - 2.1.0
- New fix command, to resolve well know problems
- Systemd service file modified to prevent /dev/video file descriptor error
# Thu Aug 12 2021 Maxime Dirksen - 2.0.1
- Initial package