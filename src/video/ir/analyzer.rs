use crate::video::stream::Image;
use std::cell::OnceCell;

use anyhow::{Context, Result, bail};
use image::GenericImageView as _;
use tokio::sync::mpsc::{Receiver, Sender};

type PixelIntensity = u16;
type ImageIntensity = Vec<PixelIntensity>;
type IntensityDiff = i32;
type IntensityVar = i64;
type IntensityVarSum = u64;

/// Tool to analyze a stream of images to determine if the IR emitter is working.
#[derive(Debug, Default)]
pub struct StreamAnalyzer {
    image_intensity: Option<ImageIntensity>,
    intensity_diff: Option<IntensityDiff>,
    intensity_var_sum: IntensityVarSum,
    nbr_images: u64,
    ref_intensity_var_mean: OnceCell<IntensityVarSum>,
    ref_intensity_var_coef: IntensityVarSum,
    size: OnceCell<u32>,
}

impl StreamAnalyzer {
    /// Creates a new StreamAnalyzer.
    pub fn new(ref_intensity_var_coef: u64) -> Self {
        Self {
            ref_intensity_var_coef,
            size: OnceCell::new(),
            ..Self::default()
        }
    }

    /// Computes lighting intensity for each pixel of an image.
    fn compute_intensity(image: Image) -> ImageIntensity {
        let mut intensities =
            ImageIntensity::with_capacity((image.width() * image.height()) as usize);
        intensities.extend(image.pixels().map(|(_, _, p)| {
            p[0] as PixelIntensity + p[1] as PixelIntensity + p[2] as PixelIntensity
        }));
        intensities
    }

    /// Computes the difference for two image lighting intensity.
    fn compute_intensities_diff(
        intensity0: &ImageIntensity,
        intensity1: &ImageIntensity,
    ) -> IntensityDiff {
        intensity0
            .iter()
            .zip(intensity1.iter())
            .map(|(&a, &b)| a as IntensityDiff - b as IntensityDiff)
            .sum()
    }

    /// Compute the sum of the absolute differences between two images intensities difference.
    fn compute_intensities_variation(
        diff0: IntensityDiff,
        diff1: IntensityDiff,
    ) -> IntensityVarSum {
        (diff1 as IntensityVar - diff0 as IntensityVar).unsigned_abs()
    }

    /// Feed an image to the analyzer.
    ///
    /// The first image to be fed sets the expected image size.
    /// Subsequent images must match this size.
    pub fn feed(&mut self, image: Image) -> Result<()> {
        let size = image.width() * image.height();
        if &size != self.size.get_or_init(|| size) {
            bail!("image size does not match stream analyzer config");
        }
        self.nbr_images += 1;

        // compute lighting intensity for the current image
        let intensity1: Vec<u16> = Self::compute_intensity(image);
        // compute difference with the previous image intensity if any
        if let Some(ref intensity0) = self.image_intensity {
            // compute intensity difference
            let diff1 = Self::compute_intensities_diff(intensity0, &intensity1);
            // compute variation sum if previous difference is available
            if let Some(diff0) = self.intensity_diff {
                let var = Self::compute_intensities_variation(diff0, diff1);
                // accumulate the variation for the two previous image, i.e. summing over the stream
                self.intensity_var_sum += var;
            }
            // store the current intensity difference for the next call
            self.intensity_diff = Some(diff1);
        }
        // store the current image intensity for the next call
        self.image_intensity = Some(intensity1);

        Ok(())
    }

    pub fn nbr_images(&self) -> u64 {
        self.nbr_images
    }

    /// If this is the first call, save the current analyzed images as base reference for future comparisons
    /// with [`StreamAnalyzer::is_ir_working`].
    ///
    /// For subsequent calls, this clears the current analyzed images.
    pub fn start_analyzing(&mut self) -> Result<()> {
        // compute reference intensity variation mean if not already done
        let ref_intensity_var_mean = self.ref_intensity_var_mean.get();
        if ref_intensity_var_mean.is_none() {
            if self.nbr_images == 0 {
                bail!("feed() never called");
            }
            let mean = (self.intensity_var_sum * self.ref_intensity_var_coef) / self.nbr_images;
            self.ref_intensity_var_mean.set(mean).unwrap();
        }

        // clear current analyzed images
        self.image_intensity = None;
        self.intensity_diff = None;
        self.intensity_var_sum = 0;
        self.nbr_images = 0;
        Ok(())
    }

    /// Determines if the IR emitter is working based intensity variations.
    /// Using the image given via [`StreamAnalyzer::feed`].
    ///
    /// It requires a reference intensity to be computed when the IR emitter is off,
    /// made by calling [`StreamAnalyzer::start_analyzing`].
    /// If this is not done, returns an error.
    pub fn is_ir_working(&self) -> Result<bool> {
        let intensity_var_mean = self
            .intensity_var_sum
            .checked_div(self.nbr_images)
            .context("feed() never called")?;

        Ok(intensity_var_mean
            > *self
                .ref_intensity_var_mean
                .get()
                .context("start_analyzing() never called")?)
    }
}

/// Requests sent to the [`StreamAnalyzer`].
#[derive(Debug)]
pub enum Message {
    /// Sends an image to be analyzed. No response expected.
    Image(Image),
    /// Asks if the IR emitter is working. Expects a response of type [`IsIrWorking`].
    IsIrWorking,
}

/// Responses to a [`Message::IsIrWorking`] from the [`StreamAnalyzer`].
#[derive(Debug, PartialEq, Eq)]
pub enum IsIrWorking {
    Yes,
    No,
    Maybe,
}

/// Handle [`Message`] request for a [`StreamAnalyzer`] instance asynchronously.
/// Until the `request_rx` channel is closed.
pub async fn analyze(
    mut analyzer: StreamAnalyzer,
    response_tx: Sender<IsIrWorking>,
    mut request_rx: Receiver<Message>,
    images_before_answer: u64,
) -> Result<()> {
    let mut waiting_for_res = false;
    while let Some(message) = request_rx.recv().await {
        match message {
            Message::Image(image) => {
                // the image is discarded if not waiting for a response
                if waiting_for_res {
                    analyzer.feed(image)?;

                    if analyzer.nbr_images() >= images_before_answer {
                        let res = match analyzer.is_ir_working() {
                            Ok(true) => IsIrWorking::Yes,
                            Ok(false) => IsIrWorking::No,
                            Err(_) => IsIrWorking::Maybe,
                        };
                        log::debug!("Stream analyzer response sent: {:?}", res);
                        response_tx.send(res).await?;
                        waiting_for_res = false;
                        analyzer.start_analyzing()?;
                    }
                }
            }
            Message::IsIrWorking => {
                log::debug!("Stream analyzer starting to analyze future images");
                waiting_for_res = true;
            }
        }
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use tokio::sync::mpsc;

    fn create_image(width: u32, height: u32, greyscale: u8) -> Image {
        Image::ImageLuma8(image::GrayImage::from_fn(width, height, |_, _| {
            image::Luma([greyscale])
        }))
    }

    #[test]
    fn test_new() {
        let analyzer = StreamAnalyzer::new(10);
        assert_eq!(analyzer.ref_intensity_var_coef, 10);
        assert!(analyzer.image_intensity.is_none());
        assert!(analyzer.intensity_diff.is_none());
        assert_eq!(analyzer.intensity_var_sum, 0);
        assert!(analyzer.ref_intensity_var_mean.get().is_none());
    }

    #[test]
    fn test_compute_intensity() {
        let image = create_image(2, 1, 50); // 2 pixels, each with intensity 50
        let intensity = StreamAnalyzer::compute_intensity(image);
        assert_eq!(intensity, vec![150, 150]);
    }

    #[test]
    fn test_compute_intensities_diff() {
        let intensity0 = vec![100, 200, 50];
        let intensity1 = vec![110, 190, 55];
        let diff = StreamAnalyzer::compute_intensities_diff(&intensity0, &intensity1);
        // (100 - 110) + (200 - 190) + (50 - 55) = -10 + 10 - 5 = -5
        assert_eq!(diff, -5);
    }

    #[test]
    fn test_compute_intensities_variation() {
        let var1 = StreamAnalyzer::compute_intensities_variation(100, 150);
        assert_eq!(var1, 50);

        let var2 = StreamAnalyzer::compute_intensities_variation(150, 100);
        assert_eq!(var2, 50);

        let var3 = StreamAnalyzer::compute_intensities_variation(-10, 10);
        assert_eq!(var3, 20);
    }

    #[test]
    fn test_feed_wrong_size() {
        let mut analyzer = StreamAnalyzer::new(1);
        analyzer.feed(create_image(5, 5, 0)).unwrap();
        let result = analyzer.feed(create_image(4, 4, 0));
        assert!(result.is_err());
    }

    #[test]
    fn test_feed_sequence() {
        let mut analyzer = StreamAnalyzer::new(1);
        // Frame 0
        analyzer.feed(create_image(2, 1, 10)).unwrap();
        assert_eq!(analyzer.image_intensity, Some(vec![30, 30]));
        assert_eq!(analyzer.intensity_diff, None);
        assert_eq!(analyzer.intensity_var_sum, 0);

        // Frame 1
        analyzer.feed(create_image(2, 1, 20)).unwrap();
        let diff1 = (30 - 60) + (30 - 60); // -60
        assert_eq!(analyzer.image_intensity, Some(vec![60, 60]));
        assert_eq!(analyzer.intensity_diff, Some(diff1));
        assert_eq!(analyzer.intensity_var_sum, 0); // No variation yet

        // Frame 2
        analyzer.feed(create_image(2, 1, 15)).unwrap();
        let diff2 = (60 - 45) + (60 - 45); // 30
        let var = (diff2 as i64 - diff1 as i64).unsigned_abs(); // |30 - (-60)| = 90
        assert_eq!(analyzer.image_intensity, Some(vec![45, 45]));
        assert_eq!(analyzer.intensity_diff, Some(diff2));
        assert_eq!(analyzer.intensity_var_sum, var);
    }

    #[test]
    fn test_start_analyzing() {
        let mut analyzer = StreamAnalyzer::new(1);
        analyzer.feed(create_image(2, 1, 10)).unwrap();
        analyzer.feed(create_image(2, 1, 20)).unwrap();
        analyzer.feed(create_image(2, 1, 30)).unwrap();
        analyzer.ref_intensity_var_mean.set(100).unwrap(); // Set a dummy value to ensure it's not cleared
        analyzer.start_analyzing().unwrap();

        assert!(analyzer.image_intensity.is_none());
        assert!(analyzer.intensity_diff.is_none());
        assert_eq!(analyzer.intensity_var_sum, 0);
        assert_eq!(*analyzer.ref_intensity_var_mean.get().unwrap(), 100); // Should not be cleared
    }

    #[test]
    fn test_start_analyzing_no_image() {
        let mut analyzer = StreamAnalyzer::new(1);
        assert!(analyzer.start_analyzing().is_err());
    }

    #[test]
    fn test_save_intensity_var_sum() {
        let mut analyzer = StreamAnalyzer::new(5);
        analyzer.feed(create_image(2, 1, 10)).unwrap();
        analyzer.feed(create_image(2, 1, 20)).unwrap();
        analyzer.feed(create_image(2, 1, 10)).unwrap();
        // diff1 = (30-60)*2 = -60
        // diff2 = (60-30)*2 = 60
        // var = |60 - (-60)| = 120
        assert_eq!(analyzer.intensity_var_sum, 120);

        analyzer.start_analyzing().unwrap();

        assert_eq!(*analyzer.ref_intensity_var_mean.get().unwrap(), 200); // 120 * 5 / 3 = 200
        // Check that clear() was called
        assert!(analyzer.image_intensity.is_none());
        assert!(analyzer.intensity_diff.is_none());
        assert_eq!(analyzer.intensity_var_sum, 0);
    }

    #[test]
    fn test_is_ir_working_no_baseline() {
        let analyzer = StreamAnalyzer::new(2);
        let result = analyzer.is_ir_working();
        assert!(result.is_err());
    }

    #[test]
    fn test_is_ir_working_no_image() {
        let mut analyzer = StreamAnalyzer::new(2);
        analyzer.feed(create_image(1, 1, 10)).unwrap();
        analyzer.start_analyzing().unwrap();
        let result = analyzer.is_ir_working();
        assert!(result.is_err());
    }

    #[test]
    fn test_is_ir_working() {
        let mut analyzer = StreamAnalyzer::new(2);

        // 1. Establish a baseline (IR off)
        analyzer.feed(create_image(1, 1, 10)).unwrap();
        analyzer.feed(create_image(1, 1, 12)).unwrap();
        analyzer.feed(create_image(1, 1, 10)).unwrap();
        // diff1 = 30-36 = -6
        // diff2 = 36-30 = 6
        // var = |6 - (-6)| = 12
        assert_eq!(analyzer.intensity_var_sum, 12);

        assert!(analyzer.is_ir_working().is_err()); // start_analyzing not called yet

        analyzer.start_analyzing().unwrap();
        assert_eq!(*analyzer.ref_intensity_var_mean.get().unwrap(), 8); // 12 * 2 / 3 = 8
        assert!(analyzer.is_ir_working().is_err()); // feed never called

        // 2. Simulate IR turning on (large intensity change)
        analyzer.feed(create_image(1, 1, 10)).unwrap();
        analyzer.feed(create_image(1, 1, 80)).unwrap();
        analyzer.feed(create_image(1, 1, 10)).unwrap();
        // diff1 = 30-240 = -210
        // diff2 = 240-30 = 210
        // var = |210 - (-210)| = 420
        assert_eq!(analyzer.intensity_var_sum, 420);

        // The new variation (420) is greater than the reference (24)
        assert!(analyzer.is_ir_working().unwrap());

        // 3. Simulate another "off" period, should not be working
        analyzer.start_analyzing().unwrap();
        analyzer.feed(create_image(1, 1, 10)).unwrap();
        analyzer.feed(create_image(1, 1, 11)).unwrap();
        analyzer.feed(create_image(1, 1, 10)).unwrap();
        // diff1 = 30-33 = -3
        // diff2 = 33-30 = 3
        // var = |3 - (-3)| = 6
        assert_eq!(analyzer.intensity_var_sum, 6);
        assert!(!analyzer.is_ir_working().unwrap()); // 6 is not > 24
    }

    #[tokio::test]
    async fn test_analyze_images_flow() {
        let analyzer = StreamAnalyzer::new(2);
        let (response_tx, mut response_rx) = mpsc::channel(1);
        let (request_tx, request_rx) = mpsc::channel(1);
        let images_before_answer = 2;

        let analyze_handle = tokio::spawn(analyze(
            analyzer,
            response_tx,
            request_rx,
            images_before_answer,
        ));

        // Establish baseline
        request_tx.send(Message::IsIrWorking).await.unwrap();
        request_tx
            .send(Message::Image(create_image(1, 1, 10)))
            .await
            .unwrap();
        request_tx
            .send(Message::Image(create_image(1, 1, 12)))
            .await
            .unwrap();

        // Ask for result
        request_tx.send(Message::IsIrWorking).await.unwrap();
        assert_eq!(response_rx.recv().await, Some(IsIrWorking::Maybe)); // first result is Maybe

        request_tx
            .send(Message::Image(create_image(1, 1, 11)))
            .await
            .unwrap();

        // Should not have a response yet
        assert!(response_rx.try_recv().is_err());

        // Send the final image to meet `images_before_answer`
        request_tx
            .send(Message::Image(create_image(1, 1, 11)))
            .await
            .unwrap();

        // Now we should get the response
        assert!(response_rx.recv().await.is_some());

        drop(analyze_handle);
    }
}
