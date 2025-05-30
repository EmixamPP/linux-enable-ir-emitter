/// Represents a frame of ARGB data.
pub struct ARGBFrame<'a>(&'a mut [u32]);

impl<'a> ARGBFrame<'a> {
    /// Creates a new `ARGBFrame` from a mutable slice of `u32`.
    pub fn from(frame: &'a mut [u32]) -> Self {
        Self(frame)
    }

    /// Copies data from a slice of `u8` to the `ARGBFrame`.
    /// The format is determined by the `fourcc` string.
    /// Supported formats are "YUYV" and "GREY".
    ///
    /// # Errors
    /// Returns an error if the input slice length does not match the expected size for the supported format.
    pub fn copy_from_slice_fourcc(&mut self, src: &[u8], format: &str) -> Result<(), String> {
        match format {
            "YUYV" => self.copy_from_slice_yuyv(src),
            "GREY" => self.copy_from_slice_grey(src),
            _ => Err(format!("Unsupported format: {}", format)),
        }
    }

    /// Copies data from a slice of `u32` to the `ARGBFrame`.
    ///
    /// # Errors
    /// Returns an error if the input slice `src` does not have the same length.
    pub fn copy_from_slice(&mut self, src: &[u32]) -> Result<(), String> {
        if src.len() != self.0.len() {
            return Err(format!(
                "Input ARGB must be the same size as output ARGB, but got {} and {}",
                src.len(),
                self.0.len()
            ));
        }
        self.0.copy_from_slice(src);
        Ok(())
    }

    /// Copies data from a slice of `u8` in YUYV format to the `ARGBFrame`.
    ///
    /// # Errors
    /// Returns an error if the input slice length is not twice the size of the output ARGB frame.
    pub fn copy_from_slice_yuyv(&mut self, src: &[u8]) -> Result<(), String> {
        yuyv_to_argb(src, self.0)
    }

    pub fn copy_from_slice_grey(&mut self, src: &[u8]) -> Result<(), String> {
        grey_to_argb(src, self.0)
    }
}

fn yuyv_to_argb(input: &[u8], output: &mut [u32]) -> Result<(), String> {
    if input.len() / 2 != output.len() {
        return Err(format!(
            "Input YUYV must be 2x the size of output ARGB, but got {} and {}",
            input.len(),
            output.len()
        ));
    }

    for (i, chunk) in input.chunks_exact(4).enumerate() {
        let y0 = chunk[0] as f32;
        let u = chunk[1] as f32 - 128.0;
        let y1 = chunk[2] as f32;
        let v = chunk[3] as f32 - 128.0;

        output[2 * i] = yuv_to_argb(y0, u, v);
        output[2 * i + 1] = yuv_to_argb(y1, u, v);
    }

    Ok(())
}

#[inline(always)]
fn yuv_to_argb(y: f32, u: f32, v: f32) -> u32 {
    let r = (y + 1.402 * v).clamp(0.0, 255.0) as u8;
    let g = (y - 0.344 * u - 0.714 * v).clamp(0.0, 255.0) as u8;
    let b = (y + 1.772 * u).clamp(0.0, 255.0) as u8;
    0xFF_00_00_00 | ((r as u32) << 16) | ((g as u32) << 8) | b as u32
}

fn grey_to_argb(input: &[u8], output: &mut [u32]) -> Result<(), String> {
    if input.len() != output.len() {
        return Err(format!(
            "Input GREY must be the same size as output ARGB, but got {} and {}",
            input.len(),
            output.len()
        ));
    }

    for (i, &y) in input.iter().enumerate() {
        let y = y as u32;
        output[i] = 0xFF_00_00_00 | (y << 16) | (y << 8) | y;
    }

    Ok(())
}
