use std::fmt;
use std::path::{Path, PathBuf};
use std::time::Duration;

use anyhow::{Context, Result, bail};
use glob::glob;
pub use image::{DynamicImage as Image, GrayImage};
use v4l::buffer::Type;
use v4l::io::traits::Stream as _;
use v4l::io::{mmap::Stream as V4lStream, traits::CaptureStream};
use v4l::video::Capture;
use v4l::{Device as V4lDevice, FourCC, Fraction};

/// Timeout when waiting for a frame to be captured
const CAPTURE_TIME_OUT: Duration = Duration::from_secs(3);

/// Stream device that can be used captures grey video images.
pub struct Stream {
    /// Path to the stream device
    device: PathBuf,
    /// Width of the captured stream image
    width: u32,
    /// Height of the captured stream image
    height: u32,
    /// FourCC pixel format of the stream image
    fourcc: String,
    // FPS
    fps: u32,
    /// Opened V4L video stream
    stream: V4lStream<'static>,
}

impl Stream {
    /// Tries to open a video stream from the specified device.
    /// If no width, height, and fps or specified, the device's default values are used.
    pub fn open<P: AsRef<Path>>(
        device: P,
        width: Option<u32>,
        height: Option<u32>,
        fps: Option<u32>,
    ) -> Result<Self> {
        // Prepare a V4L device to open a stream for video capturing
        let v4l_device = V4lDevice::with_path(&device)?;

        // Set the capture format for the V4L device
        let mut format = v4l_device.format()?;

        // Try to set the grey pixel format
        format.fourcc = FourCC::new(b"GREY");
        format = v4l_device.set_format(&format)?;

        if format.fourcc != FourCC::new(b"GREY") {
            // allow YUYV format in debug mode for testing with any webcams
            #[cfg(not(debug_assertions))]
            bail!("device does not support GREY scale pixel format");
        }

        // Try to set the resolution if provided
        if let Some(width) = width {
            format.width = width;
            format = v4l_device.set_format(&format)?;
            if format.width != width {
                bail!("device does not support the requested width: {width}");
            }
        }
        if let Some(height) = height {
            format.height = height;
            format = v4l_device.set_format(&format)?;
            if format.height != height {
                bail!("device does not support the requested height: {height}");
            }
        }

        // Set the capture parameters for the V4L device
        let mut params = v4l_device.params()?;

        // Try to set the frames per second if provided
        if let Some(fps) = fps {
            params.interval = Fraction::new(1, fps);
            params = v4l_device.set_params(&params)?;
            if params.interval.denominator != fps {
                bail!("device does not support the requested fps: {fps}");
            }
        }

        // Create the video stream
        let mut stream = Self {
            device: device.as_ref().to_path_buf(),
            width: format.width,
            height: format.height,
            fourcc: format.fourcc.to_string(),
            fps: params.interval.denominator,
            stream: V4lStream::new(&v4l_device, Type::VideoCapture)?,
        };

        // prevent blocking indefinitely
        stream.stream.set_timeout(CAPTURE_TIME_OUT);

        Ok(stream)
    }

    /// Tries to obtain an image of the device stream.
    pub fn capture(&mut self) -> Result<Image> {
        let (buf, _metadata) = self.stream.next().context("failed to fetch an image")?;
        // Convert the raw buffer to an image based on pixel format
        let grey_img = match self.fourcc.as_str() {
            "GREY" => GrayImage::from_raw(self.width, self.height, buf.to_vec())
                .context("failed to create greyscale image")?,
            #[cfg(debug_assertions)]
            "YUYV" => Self::yuyv_to_grey(buf, self.width, self.height),
            _ => {
                // should not happen due to the check in open()
                bail!("unsupported pixel format: {}", self.fourcc);
            }
        };

        Ok(grey_img.into())
    }

    /// Convert YUYV image buffer to Gray image.
    #[cfg(debug_assertions)]
    #[inline(always)]
    fn yuyv_to_grey(yuyv: &[u8], width: u32, height: u32) -> GrayImage {
        let mut img = GrayImage::new(width, height);
        // Each pixel uses 2 bytes (because YUYV is 4 bytes per 2 pixels)
        for (i, pixel) in img.pixels_mut().enumerate() {
            let y_index = i * 2; // every 2 bytes contains Y
            let y = yuyv[y_index]; // this is the greyscale luminance
            *pixel = image::Luma([y]);
        }
        img
    }
}

impl Drop for Stream {
    fn drop(&mut self) {
        if let Err(e) = self.stream.stop() {
            log::error!("error stopping video stream: {e:?}");
        }
    }
}

impl fmt::Display for Stream {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "device: {} with v4L capture format: width={} height={} fourcc={} fps={}",
            self.device.display(),
            self.width,
            self.height,
            self.fourcc,
            self.fps
        )
    }
}

pub fn grey_devices() -> Vec<PathBuf> {
    let mut devices = Vec::new();

    let entries = match glob("/dev/video*") {
        Ok(entries) => entries,
        Err(err) => {
            log::warn!("failed to read /dev/video* entries: {err:?}");
            return devices;
        }
    };

    for path in entries.flatten() {
        // Try to open the device and check if it supports GREY format
        if let Ok(dev) = V4lDevice::with_path(&path)
            && let Ok(true) = dev.enum_formats().map(|formats| {
                formats.iter().any(|f| {
                    #[cfg(not(debug_assertions))]
                    {
                        f.fourcc == FourCC::new(b"GREY")
                    }
                    #[cfg(debug_assertions)]
                    {
                        let _ = f; // avoid unused variable warning
                        true
                    }
                })
            })
        {
            devices.push(path);
        }
    }

    devices
}
