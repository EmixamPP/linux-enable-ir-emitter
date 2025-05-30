use crate::utils::window;

use std::fmt;
use std::path::{Path, PathBuf};
use std::time::{Duration, Instant};

use anyhow::{anyhow, Context, Error, Result};
use derive_more::{AddAssign, Mul};
use v4l::FourCC;
use v4l::buffer::Type;
use v4l::io::mmap::Stream;
use v4l::io::traits::CaptureStream;
use v4l::Device as V4lDevice;
use v4l::video::Capture;

type Frame = Vec<u8>;
type Frames = Vec<Frame>;
type PixelIntensity = u16;
type PixelIntensities = Vec<PixelIntensity>;
type FrameIntensities = Vec<PixelIntensities>;
type FrameIntensityDiff = i32;
type FrameIntensityDiffs = Vec<FrameIntensityDiff>;
type FrameIntensityVar = i64;

/// Represents the sum of the intensity variation of a frame.
/// See [`Stream::get_intensity_var_sum()`] for more details.
#[derive(Debug, Clone, Copy, PartialEq, Mul, AddAssign, PartialOrd)]
#[mul(forward)]
pub struct FrameIntensityVarSum(u64);

/// Default duration of the video capture
const CAPTURE_TIME: Duration = Duration::from_secs(1);
/// Timeout when waiting for a frame to be captured (should be a bit more than `CAPTURE_TIME`)
const CAPTURE_TIME_OUT: Duration = Duration::from_secs(3);

/// Coefficient applied on [`Stream::ref_intensity_var_sum`] that determines the significance
const MAGIC_REF_INTENSITY_VAR_COEF: FrameIntensityVarSum = FrameIntensityVarSum(50); // TODO: make this configurable?

/// Infrared stream device that can be used captures video frames.
pub struct IrStream {
    /// Path to the stream device
    device: PathBuf,
    /// Width of the captured stream image
    width: u32,
    /// Height of the captured stream image
    height: u32,
    /// FourCC pixel format of the stream image
    fourcc: String,
    /// Opened V4L video stream
    stream: Stream<'static>,
}

impl IrStream {
    pub fn open<P: AsRef<Path>>(
        device: P,
        width: Option<u32>,
        height: Option<u32>,
    ) -> Result<Self> {
        // Prepare a V4L device to open a stream for video capturing
        let v4l_device = V4lDevice::with_path(&device)?;
        // Set the capture format for the V4L device
        let mut format = v4l_device.format()?;
        // Try to set the pixel format in grayscale
        format.fourcc = FourCC::new(b"GREY");
        format = v4l_device.set_format(&format)?;
        // Try to set the resolution if provided
        if let Some(width) = width {
            format.width = width;
            format = v4l_device.set_format(&format)?;
        }
        if let Some(height) = height {
            format.height = height;
            v4l_device.set_format(&format)?;
        }

        // Create the video stream
        let mut stream = Self {
            device: device.as_ref().to_path_buf(),
            width: format.width,
            height: format.height,
            fourcc: format.fourcc.to_string(),
            stream: Stream::new(&v4l_device, Type::VideoCapture)?,
        };
        stream.stream.set_timeout(CAPTURE_TIME_OUT); // prevent blocking indefinitely
        stream.capture()?; // warm up the stream

        Ok(stream)
    }

    /// Returns the device path of the stream.
    pub fn dev_path(&self) -> &Path {
        &self.device
    }

    /// Returns the width and height of the stream image.
    pub fn resolution(&self) -> (u32, u32) {
        (self.width, self.height)
    }

    /// Returns the fourcc pixel format of the stream image.
    ///
    /// For an infrared stream, the expected value is `GREY`.
    pub fn fourcc(&self) -> &str {
        &self.fourcc
    }

    /// Returns the stream of the stream.
    fn capture(&mut self) -> Result<&[u8]> {
        let (buf, _metadata) = self.stream.next().map_err(Error::from).context("failed to fetch a frame")?;
        Ok(buf)
    }

    /// Captures frames from the stream for a given duration.
    fn capture_during(&mut self, duration: Duration) -> Result<Frames> {
        let mut frames = Frames::new();
        let start_time = Instant::now();
        while start_time.elapsed() < duration {
            frames.push(self.capture()?.to_vec());
        }
        Ok(frames)
    }

    /// Obtains the intensity variation sum of a set of frames.
    /// This can be used to determine if the IR emitter is working,
    /// by comparing two intensity variation sums.
    /// The first one is the reference one, obtained when the IR is not working.
    /// The second one is obtained when the IR is working.
    /// See [`Stream::is_ir_working()`].
    ///
    /// # Errors
    /// Returns an error if no frame can be captured from the stream.
    pub fn get_intensity_var_sum(&mut self) -> Result<FrameIntensityVarSum> {
        // capture frames
        let frames = self.capture_during(CAPTURE_TIME)?;
        // compute lighting intensity for each pixel of each frame
        let intensities = compute_intensities(frames);
        // compute difference between each consecutive frame intensity
        let diffs = compute_intensities_diff(intensities);
        // compute difference between each consecutive intensity difference
        // this is the variation in the lighting intensity of the fames
        // and sum them all
        Ok(compute_sum_intensities_variation(diffs))
    }

    /// Determines if the IR emitter is working using a reference intensity variation sum.
    ///
    /// The reference can be obtained by calling [`Stream::get_intensity_var_sum()`]
    /// when the IR emitter was not working for sure.
    pub fn is_ir_working(&mut self, reference: FrameIntensityVarSum) -> Result<bool> {
        // compare with the one of reference times a magic factor for the significance
        let ref_intensity_var_sum = reference * MAGIC_REF_INTENSITY_VAR_COEF;
        Ok(self.get_intensity_var_sum()? > ref_intensity_var_sum)
    }

    /// Determines if the stream is in grayscale mode by capturing frames.
    ///
    /// A second method that relies on the driver is to check the fourcc pixel format of the stream
    /// by calling [`Stream::fourcc()`], the expected value is `GREY`.
    pub fn is_gray_scale(&mut self) -> Result<bool> {
        let frame = self.capture()?;
        if frame.len() % 3 != 0 {
            return Ok(false);
        }
        for chunk in frame.chunks_exact(3) {
            if chunk[0] != chunk[1] || chunk[0] != chunk[2] {
                return Ok(false);
            }
        }
        Ok(true)
    }

    /// Start the stream and display the video stream in a window until it is closed by the user.
    ///
    /// # Errors
    /// Returns an error if the video stream cannot be created.
    ///
    /// Or if the fourcc pixel format is not supported.
    ///
    /// Or if no frame can be grabbed from the stream after `[window::FPS]` times in a row.
    ///
    /// # Panics
    /// Panics if the window cannot be created or a frame fails to be presented.
    pub fn play(&mut self) -> Result<()> {
        self.open_window(true)
    }

    /// Open a window to display the video stream from the stream, blocking the current thread
    /// until the window is closed.
    ///
    /// If `exit_button` is true, can be closed using the exit button.
    fn open_window(&mut self, exit_button: bool) -> Result<()> {
        // Update the frame_provider to check the running flag
        let mut err_counter: f64 = 0.0;
        let mut err: Option<String> = None;
        window::show(exit_button, self.width, self.height, |frame| {
            match self.stream.next() {
                Ok((buf, _)) => {
                    if let Err(e) = frame.copy_from_slice_fourcc(buf, &self.fourcc) {
                        err = Some(e.to_string());
                        return false;
                    }
                    err_counter = 0.0;
                }
                Err(_) => {
                    err_counter += 1.0;
                    if err_counter > window::FPS {
                        err = Some("Failed to grab frames from the stream.".to_string());
                        return false;
                    }
                }
            }
            true
        });

        // forward the errors coming from the frame capture above
        if let Some(err_msg) = err {
            return Err(anyhow!(err_msg));
        }
        Ok(())
    }
}

/// Computes lighting intensity for each pixel of each frame.
fn compute_intensities(frames: Frames) -> FrameIntensities {
    let mut intensities = FrameIntensities::new();
    for frame in frames {
        let mut intensity: Vec<u16> = PixelIntensities::new();
        for chunk in frame.chunks_exact(3) {
            let pixel_intensity = chunk[0] as PixelIntensity
                + chunk[1] as PixelIntensity
                + chunk[2] as PixelIntensity;
            intensity.push(pixel_intensity);
        }
        intensities.push(intensity);
    }
    intensities
}

/// Computes the difference for each two consecutive frame lighting intensity.
fn compute_intensities_diff(intensities: FrameIntensities) -> FrameIntensityDiffs {
    let mut diffs = FrameIntensityDiffs::new();
    for i in 1..intensities.len() {
        let intensity1 = &intensities[i - 1];
        let intensity2 = &intensities[i];
        let mut diff = 0;
        for j in 0..intensity1.len() {
            diff += intensity1[j] as FrameIntensityDiff - intensity2[j] as FrameIntensityDiff;
        }
        diffs.push(diff);
    }
    diffs
}

/// Compute the sum of the absolute differences between each two consecutive frames intensities difference.
fn compute_sum_intensities_variation(diffs: FrameIntensityDiffs) -> FrameIntensityVarSum {
    let mut sum = FrameIntensityVarSum(0);
    for i in 1..diffs.len() {
        sum += FrameIntensityVarSum(
            (diffs[i - 1] as FrameIntensityVar - diffs[i] as FrameIntensityVar).unsigned_abs(),
        );
    }
    sum
}

impl fmt::Display for IrStream {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        writeln!(
            f,
            "IR device: {:?} & V4L capture format: width: {}, height: {}, fourcc: {}",
            self.dev_path(),
            self.width,
            self.height,
            self.fourcc
        )
    }
}

// pub fn is_ir_working(&self) -> bool {
//     let running = Arc::new(AtomicBool::new(true));
//     let running_clone = Arc::clone(&running);

//     // Spawn a thread to wait for user input
//     let handle = thread::spawn(move || {
//         print!("Is the ir emitter or the video flashing (not just turned on)? Yes/No? ");
//         loop {
//         std::io::stdout().flush().unwrap();

//         let mut input = String::new();
//         if let Ok(_) = std::io::stdin().read_line(&mut input) {
//             let input = input.trim().to_lowercase();
//             match input.as_str() {
//             "y" | "yes" => {
//                 running_clone.store(false, Ordering::SeqCst);
//                 return true;
//             }
//             "n" | "no" => {
//                 running_clone.store(false, Ordering::SeqCst);
//                 return false;
//             }
//             _ => {
//                 print!("Yes/No? ");
//             }
//             }
//         }
//         }
//     });

//     open_window(false);

//     // Wait for the thread to finish and return the result
//     handle.join().unwrap()

//     //self.open_window(false);
// }
