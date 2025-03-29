---
name: Configuration has failed
about: Describe your situation in order to get help
title: ''
labels: ''
assignees: ''

---

**Before opening the issue**
- [ ] I looked at the [docs](https://github.com/EmixamPP/linux-enable-ir-emitter/blob/master/docs/README.md)
- [ ] I installed the [release tarball provided here on Github](https://github.com/EmixamPP/linux-enable-ir-emitter?tab=readme-ov-file#installation)
<!---Most of the issues comes from OpenCV distro packaging -->

**Ouput of `cat /var/local/log/linux-enable-ir-emitter/linux-enable-ir-emitter.log`**
```
paste here
```

**Output of `sudo linux-enable-ir-emitter tweak`**
```
paste here
```

**Output of `v4l2-ctl --list-devices`**
```
paste here
```

**Ouput of `for dev in /dev/video*; do echo $dev && v4l2-ctl -d $dev --list-formats-ext; done`**
```
paste here
```

**Output of `ls -l /dev/v4l/by-path`**
```
paste here
```

**Give more information if you have**
<!-- describe here if you have -->

**Additional info**
 - Computer (or camera) model:
 - Linux distribution:
 - Version of linux-enable-ir-emitter: <!--- linux-enable-ir-emitter -V -->
