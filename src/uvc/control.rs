use derive_more::Error as DeriveError;

/// Specialized error type for the UVC control module
#[derive(Debug, DeriveError)]
pub enum Error {
    /// The control is out of range
    OutOfRange,
    /// The size of the control is invalid
    InvalidSize,
}

impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Error::OutOfRange => write!(f, "the control is out of range"),
            Error::InvalidSize => write!(f, "the size of the control is invalid"),
        }
    }
}

/// Specialized result type for the UVC control module.
pub type Result<T> = std::result::Result<T, Error>;

pub trait Control {
    /// Returns the unit of the control.
    fn unit(&self) -> u8;

    /// Returns the selector of the control.
    fn selector(&self) -> u8;

    /// Returns a reference to the value of the control.
    fn value_ref(&self) -> &[u8];

    /// Returns the value of the control, giving up ownership.
    fn value(self) -> Vec<u8>;

    /// Returns a mutable reference to the value of the control.
    fn value_mut(&mut self) -> &mut [u8];

    /// Tries to safely increment the control using a resolution value.
    ///
    /// And optional maximum value can be provided to limit the increment.
    ///
    /// # Errors
    /// Returns [`Error::InvalidSize`] if the size of the resolution and maximum values do not match with the control.
    ///
    /// Returns [`Error::OutOfRange`] if the increment would exceed the maximum value if that later is defined, or 255 if not to avoid overflow.
    fn increment_with_res(&mut self, res: &[u8], max: &Option<&[u8]>) -> Result<()> {
        // Verify that the size of the values provided matches with the control
        if self.value_ref().len() != res.len()
            || self.value_ref().len() != max.as_ref().map_or(0, |v| v.len())
        {
            return Err(Error::InvalidSize);
        }

        // Verify of the control does not exceed the maximum value
        for (i, res_i) in res.iter().enumerate() {
            let max_i = max
                .as_ref()
                .and_then(|max| max.get(i).copied())
                .unwrap_or(255);
            if self.value_ref()[i] as u16 + *res_i as u16 > max_i as u16 {
                return Err(Error::OutOfRange);
            }
        }

        // proceed to the increment
        for (i, res_i) in res.iter().enumerate() {
            self.value_mut()[i] += res_i;
        }
        Ok(())
    }
}

impl std::fmt::Display for dyn Control {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "unit: {}, selector: {}, value: {:?}",
            self.unit(),
            self.selector(),
            self.value_ref()
        )
    }
}
