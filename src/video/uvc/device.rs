use crate::video::uvc::XuControl;

use std::io::{Error, ErrorKind, Result};
use std::os::fd::AsFd;
use std::path::{Path, PathBuf};

use rustix::{fd, fs, ioctl};

#[repr(u8)]
enum UvcQuery {
    SetCur = 0x01,
    GetCur = 0x81,
    GetMin = 0x82,
    GetMax = 0x83,
    GetRes = 0x84,
    GetLen = 0x85,
    GetDef = 0x87,
}

#[repr(C)]
/// Represents the `uvc_xu_control_mapping` struct from the Linux header `linux/uvcvideo.h`.
struct UvcXuControlQuery {
    unit: u8,
    selector: u8,
    query: u8,
    size: u16,
    data: *mut u8,
}

impl UvcXuControlQuery {
    fn new(unit: u8, selector: u8, query: UvcQuery, data: &mut [u8]) -> Self {
        Self {
            unit,
            selector,
            query: query as u8,
            size: data.len() as u16,
            data: data.as_mut_ptr(),
        }
    }
}

const UVCIOC_CTRL_QUERY: ioctl::Opcode = ioctl::opcode::read_write::<UvcXuControlQuery>(b'u', 0x21);

#[derive(Debug)]
enum Fd {
    Owned(fd::OwnedFd),
    Raw(i32),
}

/// Represents an UVC device.
#[derive(Debug)]
pub struct Device {
    /// Device file descriptor
    fd: Fd,
    /// Device path for display purposes
    path: Option<PathBuf>,
}

impl Device {
    /// Tries to open a supposed UVC device.
    ///
    /// The controls of the can be accessed obtained by calling [`Device::controls`].
    pub fn open<P: AsRef<Path>>(dev_path: P) -> Result<Self> {
        Ok(Self {
            fd: Fd::Owned(fs::open(
                dev_path.as_ref(),
                fs::OFlags::RDWR,
                fs::Mode::empty(),
            )?),
            path: Some(dev_path.as_ref().to_path_buf()),
        })
    }

    pub fn from_fd(fd: i32) -> Result<Self> {
        let device = Self {
            fd: Fd::Raw(fd),
            path: None,
        };
        Ok(device)
    }

    /// Tries to get all the control descriptions of the device.
    ///
    /// An empty list will be returned if the device has no controls.
    pub fn controls(&self) -> Result<Vec<XuControl>> {
        let mut controls = Vec::new();
        // Search for all possible combination of unit and selector
        for unit in 0..255 {
            for selector in 0..255 {
                match self.find_control(unit, selector) {
                    Ok(control) => controls.push(control),
                    Err(err) => match err.kind() {
                        ErrorKind::NotFound | ErrorKind::PermissionDenied => {}
                        _ => return Err(err),
                    },
                }
            }
        }
        Ok(controls)
    }

    /// Tries to apply the control to the device.
    ///
    /// Because of system call limitation, `ctrl` must be mutable even if it will not be modified.
    pub fn apply_control(&self, ctrl: &mut XuControl) -> Result<()> {
        log::info!("Applying control: {}.", ctrl);
        self.uvc_set_query(
            ctrl.unit(),
            ctrl.selector(),
            ctrl.cur_mut(),
            UvcQuery::SetCur,
        )
    }

    /// Tries to find a control with the given `unit` and `selector`.
    fn find_control(&self, unit: u8, selector: u8) -> Result<XuControl> {
        // Try to get the len of the control
        let len = self.uvc_len_query(unit, selector)?;
        // Try to get the current value of the control
        let mut cur = self.uvc_get_cur_query(unit, selector, len)?;

        // Determine if the control is writable
        let writable = self.uvc_set_cur_query(unit, selector, &mut cur).is_ok();

        // Find optional extra information about the control values
        // Try to get the max value of the control
        let max = self.uvc_get_max_query(unit, selector, len).ok();
        // Try to get the min value of the control
        let min = self.uvc_get_min_query(unit, selector, len).ok();
        // Try to get the res value of the control
        let res = self.uvc_get_res_query(unit, selector, len).ok();
        // Try to get the default value of the control
        let def = self.uvc_get_def_query(unit, selector, len).ok();

        // Create the control description
        XuControl::new(unit, selector, cur, max, min, res, def, writable)
    }

    fn uvc_get_cur_query(&self, unit: u8, selector: u8, len: u16) -> Result<Vec<u8>> {
        self.uvc_get_query(unit, selector, len, UvcQuery::GetCur)
    }

    fn uvc_set_cur_query(&self, unit: u8, selector: u8, data: &mut [u8]) -> Result<()> {
        self.uvc_set_query(unit, selector, data, UvcQuery::SetCur)
    }

    fn uvc_get_def_query(&self, unit: u8, selector: u8, len: u16) -> Result<Vec<u8>> {
        self.uvc_get_query(unit, selector, len, UvcQuery::GetDef)
    }

    fn uvc_get_max_query(&self, unit: u8, selector: u8, len: u16) -> Result<Vec<u8>> {
        self.uvc_get_query(unit, selector, len, UvcQuery::GetMax)
    }

    fn uvc_get_min_query(&self, unit: u8, selector: u8, len: u16) -> Result<Vec<u8>> {
        self.uvc_get_query(unit, selector, len, UvcQuery::GetMin)
    }

    fn uvc_get_res_query(&self, unit: u8, selector: u8, len: u16) -> Result<Vec<u8>> {
        self.uvc_get_query(unit, selector, len, UvcQuery::GetRes)
    }

    fn uvc_get_query(&self, unit: u8, selector: u8, len: u16, query: UvcQuery) -> Result<Vec<u8>> {
        let mut data = vec![0u8; len as usize];
        let mut query = UvcXuControlQuery::new(unit, selector, query, &mut data);
        self.uvc_query(&mut query)?;
        Ok(data)
    }

    fn uvc_set_query(
        &self,
        unit: u8,
        selector: u8,
        data: &mut [u8],
        query: UvcQuery,
    ) -> Result<()> {
        let mut query = UvcXuControlQuery::new(unit, selector, query, data);
        self.uvc_query(&mut query)
    }

    fn uvc_len_query(&self, unit: u8, selector: u8) -> Result<u16> {
        let mut data = vec![0u8; 2];
        let mut query = UvcXuControlQuery::new(unit, selector, UvcQuery::GetLen, &mut data);
        self.uvc_query(&mut query)?;
        Ok(u16::from_le_bytes(data.as_slice().try_into().unwrap()))
    }

    fn uvc_query(&self, query: &mut UvcXuControlQuery) -> Result<()> {
        let updater = unsafe { ioctl::Updater::<UVCIOC_CTRL_QUERY, UvcXuControlQuery>::new(query) };
        let fd = match &self.fd {
            Fd::Owned(owned_fd) => owned_fd.as_fd(),
            Fd::Raw(raw_fd) => unsafe { fd::BorrowedFd::borrow_raw(*raw_fd) },
        };
        unsafe { ioctl::ioctl(fd, updater) }.map_err(Error::from)
    }
}

impl std::fmt::Display for Device {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        writeln!(
            f,
            "{}",
            self.path
                .as_ref()
                .map_or("unknown".into(), |p| p.display().to_string())
        )?;
        match self.controls() {
            Err(err) => writeln!(f, "   Failed to get the controls: {err:?}")?,
            Ok(controls) => {
                for control in controls {
                    writeln!(f, "   {}", control)?;
                }
            }
        }
        Ok(())
    }
}
