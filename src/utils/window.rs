mod frame;
mod winit_app;
pub use frame::ARGBFrame;

use std::num::NonZeroU32;

use softbuffer::{Context, Surface};
use winit::event::{Event, WindowEvent};
use winit::event_loop::{ControlFlow, EventLoop};

use std::time::{Duration, Instant};

/// Target frame rate
pub const FPS: f64 = 30.0;

/// Create a window of size `width`x`height` and show it. Will block the current thread until the window is closed.
///
/// The `frame_provider` function is called every time the window needs to be redrawn; at approx [`FPS`],
/// passing a mutable reference to the 32-bit ARGB frame buffer.
/// If the `frame_provider` function returns `false`, the event loop will exit and the window will close releasing the thread.
///
/// The `exit_button` parameter controls whether the window should close when the exit button in the decorations is clicked.
///
/// # Panics
/// Panics if the window cannot be created or a frame fails to be presented.
pub fn show<F>(exit_button: bool, width: u32, height: u32, mut frame_provider: F)
where
    F: FnMut(&mut ARGBFrame) -> bool,
{
    let mut last_frame_time = Instant::now();
    let frame_duration = Duration::from_secs_f64(1.0 / FPS);

    let event_loop = EventLoop::new().unwrap();

    let mut app = winit_app::WinitAppBuilder::with_init(
        |elwt| {
            let window = winit_app::make_window(elwt, |w| w.with_title(env!("CARGO_PKG_NAME")));
            let context = Context::new(window.clone()).unwrap();
            (window, context)
        },
        |_elwt, (window, context)| Surface::new(context, window.clone()).unwrap(),
    )
    .with_event_handler(|(window, _context), surface, event, elwt| {
        elwt.set_control_flow(ControlFlow::Poll);

        match event {
            Event::WindowEvent {
                window_id,
                event: WindowEvent::RedrawRequested,
            } if window_id == window.id() => {
                let Some(surface) = surface else {
                    panic!("Surface is not available.");
                };

                surface
                    .resize(
                        NonZeroU32::new(width).unwrap(),
                        NonZeroU32::new(height).unwrap(),
                    )
                    .unwrap();
                let mut surface_buffer = surface.buffer_mut().unwrap();
                if !frame_provider(&mut ARGBFrame::from(surface_buffer.as_mut())) {
                    elwt.exit(); // exit requested by the `frame_provider`
                }

                surface_buffer.present().unwrap();
            }
            Event::AboutToWait => {
                let now = Instant::now();
                if now < last_frame_time + frame_duration {
                    std::thread::sleep(last_frame_time + frame_duration - now);
                }
                last_frame_time = Instant::now();

                window.request_redraw();
            }
            Event::WindowEvent {
                window_id,
                event: WindowEvent::CloseRequested,
            } if exit_button && window_id == window.id() => {
                elwt.exit();
            }
            _ => {}
        }
    });

    //event_loop.run_app(&mut app).unwrap();
    winit_app::run_app(event_loop, &mut app);
}
