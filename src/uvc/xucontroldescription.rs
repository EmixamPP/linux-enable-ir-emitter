use super::Control;

/// `XUControlDescription` represents a XU control.
#[derive(Debug)]
pub struct XUControlDescription {
    /// Unit used on the device to identify the control
    pub unit: u8,
    /// Selector used on the device to identify the control
    pub selector: u8,
    /// Current value of the control
    pub cur: Vec<u8>,
    /// Maximum value of the control
    pub max: Option<Vec<u8>>,
    /// Minimum value of the control
    pub min: Option<Vec<u8>>,
    /// Resolution of the control
    pub res: Option<Vec<u8>>,
    /// Default value of the control
    pub def: Option<Vec<u8>>,
    /// `true` if the control is writable, `false` otherwise
    pub writable: bool,
}

impl std::fmt::Display for XUControlDescription {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        writeln!(f, "unit: {} selector {}:", self.unit, self.selector)?;
        writeln!(f, "  writable: {}", self.writable)?;
        writeln!(f, "  current: {:?}", self.cur)?;
        if let Some(ref max) = self.max {
            writeln!(f, "  maximum: {:?}", max)?;
        }
        if let Some(ref min) = self.min {
            writeln!(f, "  minimum: {:?}", min)?;
        }
        if let Some(ref res) = self.res {
            writeln!(f, "  resolution: {:?}", res)?;
        }
        if let Some(ref def) = self.def {
            writeln!(f, "  default: {:?}", def)?;
        }
        Ok(())
    }
}

impl From<XUControlDescription> for Box<dyn Control> {
    fn from(desc: XUControlDescription) -> Self {
        Box::new(desc)
    }
}

// Ensure `XUControlDescription` implements the `Control` trait
impl Control for XUControlDescription {
    fn unit(&self) -> u8 {
        self.unit
    }

    fn selector(&self) -> u8 {
        self.selector
    }

    fn value_ref(&self) -> &[u8] {
        &self.cur
    }

    fn value_mut(&mut self) -> &mut [u8] {
        &mut self.cur
    }

    fn value(self) -> Vec<u8> {
        self.cur
    }
}
