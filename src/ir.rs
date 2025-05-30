mod camera;
pub use camera::IrCamera;

mod control;
pub use control::{Error, IrControl, Result};

mod stream;
pub use stream::{FrameIntensityVarSum, IrStream};
