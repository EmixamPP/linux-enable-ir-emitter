/// `XUControlDescription` represents a XU control.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct XuControl {
    /// Unit used on the device to identify the control
    unit: u8,
    /// Selector used on the device to identify the control
    selector: u8,
    /// Current value of the control
    cur: Vec<u8>,
    /// Initial value of the control, before any modification allowed by [`Self::cur_mut()`]
    init: Vec<u8>,
    /// Maximum value of the control
    max: Option<Vec<u8>>,
    /// Minimum value of the control
    min: Option<Vec<u8>>,
    /// Resolution of the control
    res: Option<Vec<u8>>,
    /// Default value of the control
    def: Option<Vec<u8>>,
    /// `true` if the control is writable, `false` otherwise
    writable: bool,
}

impl XuControl {
    /// Creates a new `XuControl`.
    ///
    /// All the field must have the same length if specified.
    #[allow(clippy::too_many_arguments)]
    pub fn new(
        unit: u8,
        selector: u8,
        cur: Vec<u8>,
        max: Option<Vec<u8>>,
        min: Option<Vec<u8>>,
        res: Option<Vec<u8>>,
        def: Option<Vec<u8>>,
        writable: bool,
    ) -> std::io::Result<Self> {
        let len = cur.len();
        if let Some(ref v) = max
            && v.len() != len
        {
            return Err(std::io::Error::new(
                std::io::ErrorKind::InvalidInput,
                format!(
                    "maximum length does not match for control unit {} and selector {}",
                    unit, selector
                ),
            ));
        }
        if let Some(ref v) = min
            && v.len() != len
        {
            return Err(std::io::Error::new(
                std::io::ErrorKind::InvalidInput,
                format!(
                    "minimum length does not match for control unit {} and selector {}",
                    unit, selector
                ),
            ));
        }
        if let Some(ref v) = res
            && v.len() != len
        {
            return Err(std::io::Error::new(
                std::io::ErrorKind::InvalidInput,
                format!(
                    "resolution length does not match for control unit {} and selector {}",
                    unit, selector
                ),
            ));
        }
        if let Some(ref v) = def
            && v.len() != len
        {
            return Err(std::io::Error::new(
                std::io::ErrorKind::InvalidInput,
                format!(
                    "default length does not match for control unit {} and selector {}",
                    unit, selector
                ),
            ));
        }
        Ok(Self {
            unit,
            selector,
            init: cur.clone(),
            cur,
            max,
            min,
            res,
            def,
            writable,
        })
    }

    /// Returns the unit of the control.
    pub fn unit(&self) -> u8 {
        self.unit
    }

    /// Returns the selector of the control.
    pub fn selector(&self) -> u8 {
        self.selector
    }

    /// Returns a reference to the value of the control.
    pub fn cur(&self) -> &[u8] {
        &self.cur
    }

    /// Returns a mutable reference to he value of the control.
    pub fn cur_mut(&mut self) -> &mut [u8] {
        &mut self.cur
    }

    /// Returns a reference to the initial value of the control.
    pub fn init(&self) -> &[u8] {
        &self.init
    }

    /// Resets the current value to the initial value.
    pub fn reset(&mut self) {
        if self.cur.len() == self.init.len() {
            self.cur.copy_from_slice(&self.init);
        }
    }

    /// Returns the maximum value of the control, if any.
    pub fn max(&self) -> Option<&[u8]> {
        self.max.as_deref()
    }

    /// Returns the minimum value of the control, if any.
    pub fn _min(&self) -> Option<&[u8]> {
        self.min.as_deref()
    }

    /// Returns the resolution of the control, if any.
    pub fn _res(&self) -> Option<&[u8]> {
        self.res.as_deref()
    }

    /// Returns the default value of the control, if any.
    pub fn _def(&self) -> Option<&[u8]> {
        self.def.as_deref()
    }

    /// Returns `true` if the control is writable, `false` otherwise.
    pub fn writable(&self) -> bool {
        self.writable
    }

    /// Only clones the [`Self::unit`], [`Self::selector`], [`Self::cur`] and [`Self::writable`] fields, the other are ignored.
    /// Even [`Self::init`] is ignored and will be empty.
    pub fn essential_clone(&self) -> Self {
        Self {
            unit: self.unit,
            selector: self.selector,
            cur: self.cur.clone(),
            init: vec![],
            max: None,
            min: None,
            res: None,
            def: None,
            writable: self.writable,
        }
    }
}

impl std::fmt::Display for XuControl {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "unit={} selector={} cur={:?}",
            self.unit, self.selector, self.cur,
        )?;
        if let Some(max) = &self.max {
            write!(f, " max={:?}", max)?;
        }
        if let Some(min) = &self.min {
            write!(f, " min={:?}", min)?;
        }
        if let Some(res) = &self.res {
            write!(f, " res={:?}", res)?;
        }
        if let Some(def) = &self.def {
            write!(f, " def={:?}", def)?;
        }
        write!(f, " writable={}", self.writable)?;
        Ok(())
    }
}
