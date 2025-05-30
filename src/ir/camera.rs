use super::{FrameIntensityVarSum, IrControl, IrStream};
use crate::uvc::Device;

use std::fmt;
use std::path::Path;

use anyhow::{Error, Result};

/// Represents an infrared camera.
pub struct IrCamera {
    /// The video stream
    stream: IrStream,
    /// The UVC device
    uvc_device: Device,
    /// The reference intensity variation sum when the camera is opened
    ref_intensity_var_sum: FrameIntensityVarSum,
}

impl IrCamera {
    /// Open a potential infrared camera
    ///
    /// # Errors
    /// Returns an error if the camera cannot be opened,
    /// if the video stream cannot be set up,
    /// or if the camera is not grayscale.
    pub fn open<P: AsRef<Path>>(
        dev_path: P,
        width: Option<u32>,
        height: Option<u32>,
    ) -> Result<Self> {
        let uvc_device = Device::open(dev_path.as_ref())?;
        let mut stream = IrStream::open(uvc_device.dev_path(), width, height)?;
        // TODO uncomment
        // if !stream.is_gray_scale()? {
        //     return Err(Error::new(
        //         ErrorKind::InvalidInput,
        //         "not a grayscale camera",
        //     ));
        // }

        Ok(Self {
            ref_intensity_var_sum: stream.get_intensity_var_sum()?,
            stream,
            uvc_device,
        })
    }

    /// Get all the controls of the camera
    ///
    /// An empty list will be returned if the camera has no controls.
    ///
    /// # Errors
    /// A detailed [`Error`] is returned if the controls cannot be read.
    pub fn controls(&self) -> Result<Vec<IrControl>> {
        Ok(self
            .uvc_device
            .controls()?
            .into_iter()
            .filter_map(|ctrl_desc| IrControl::new(ctrl_desc).ok())
            .collect())
    }

    /// Apply the `ctrl` to the camera.
    ///
    /// Because of system call limitation, `ctrl` must be mutable even if it will not be modified.
    ///
    /// # Errors
    /// A detailed [`Error`] is returned if the control cannot be applied
    pub fn apply(&mut self, ctrl: &mut IrControl) -> Result<()> {
        self.uvc_device.apply_control(ctrl).map_err(Error::from)
    }

    /// Apply the `ctrl` to the camera and check if the emitter is blinking.
    ///
    /// Because of system call limitation, `ctrl` must be mutable even if it will not be modified.
    ///
    /// # Errors
    /// A detailed [`Error`] is returned if the control cannot be applied
    pub fn apply_and_check_ir(&mut self, ctrl: &mut IrControl) -> Result<bool> {
        self.apply(ctrl)?;
        self.stream.is_ir_working(self.ref_intensity_var_sum)
    }

    /// Play a GUI camera stream in the current thread until the user exists the window.
    ///
    /// # Errors
    /// A detailed [`Error`] is returned if the stream cannot be played
    pub fn play(&mut self) -> Result<()> {
        self.stream.play()
    }

    pub fn dev_path(&self) -> &Path {
        self.uvc_device.dev_path()
    }
}

impl fmt::Display for IrCamera {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "UVC device: {}, IR stream: {}",
            self.uvc_device, self.stream
        )
    }
}
