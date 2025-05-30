use crate::uvc::{Control, XUControlDescription};

use derive_more::Error as DeriveError;

/// Specialized error type for the `IrControl` module
#[derive(Debug, DeriveError)]
pub enum Error {
    /// The control is not writable
    NotWritable,
    /// The control will become out of range
    OutOfRange,
    /// The control is not writable
    NoMaximum,
}

/// Specialized error type for the `IrControl` module
pub type Result<T> = std::result::Result<T, Error>;

#[derive(Debug)]
pub struct IrControl {
    /// Unit
    unit: u8,
    /// Selector
    selector: u8,
    /// Control value
    value: Vec<u8>,
    /// The current control byte that is being incremented
    inc_cur_byte: usize,
    /// Copy of the current value of the control
    init_value: Vec<u8>,
    /// The maximum value of the control
    max_value: Option<Vec<u8>>,
}

impl IrControl {
    /// Create a new infrared control from the XUControlDescription
    ///
    /// Sets the initial value to the minimum value if it is provided
    /// or to the current value if not.
    ///
    /// # Errors
    /// Returns an error if the control is not writable
    pub fn new(control: XUControlDescription) -> Result<Self> {
        if !control.writable {
            return Err(Error::NotWritable);
        }

        Ok(Self {
            unit: control.unit,
            selector: control.selector,
            init_value: control.cur.clone(),
            value: match control.min {
                Some(min) => min,
                None => control.cur,
            },
            inc_cur_byte: 0,
            max_value: control.max,
        })
    }

    /// Increment the control value by 1 in an big endian representation
    /// When the first byte of the control is at its maximum if defined or 255,
    /// it is set to its initial value and the next byte is incremented
    ///
    /// # Errors
    /// Returns an error if impossible to increment the value;
    /// if the value is already at its maximum
    /// or if the resolution value is at its maximum
    pub fn increment(&mut self) -> Result<()> {
        if self.inc_cur_byte >= self.value.len() {
            return Err(Error::OutOfRange);
        }

        let max = self
            .max_value
            .as_ref()
            .and_then(|v| v.get(self.inc_cur_byte).copied())
            .unwrap_or(255);

        // Check if the current byte is at its maximum
        if self.value[self.inc_cur_byte] == max {
            // Set the current byte to its initial byte
            self.value[self.inc_cur_byte] = 0;
            // Increment the next byte
            self.inc_cur_byte += 1;
            return self.increment();
        } else {
            // Increment the current byte
            self.value[self.inc_cur_byte] += 1;
        }

        Ok(())
    }

    /// Try to set the control value to its maximum value
    ///
    /// # Errors
    /// Returns an error if the maximum value is not defined
    pub fn try_set_max(&mut self) -> Result<()> {
        if let Some(max) = &self.max_value {
            self.value = max.clone();
            return Ok(());
        }
        Err(Error::NoMaximum)
    }

    /// Reset the control value to its initial value
    pub fn reset(&mut self) {
        self.value = self.init_value.clone();
    }
}

impl Control for IrControl {
    fn unit(&self) -> u8 {
        self.unit
    }

    fn selector(&self) -> u8 {
        self.selector
    }

    fn value_ref(&self) -> &[u8] {
        &self.value
    }

    fn value_mut(&mut self) -> &mut [u8] {
        &mut self.value
    }

    fn value(self) -> Vec<u8> {
        self.value
    }
}

impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Error::NotWritable => write!(f, "the control is not writable"),
            Error::OutOfRange => write!(f, "the control is out of range"),
            Error::NoMaximum => write!(f, "the maximum value is not defined"),
        }
    }
}
