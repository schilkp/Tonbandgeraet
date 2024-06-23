use std::{
    sync::{Arc, Condvar, Mutex},
    time::{Duration, SystemTime},
};

use log::{debug, info, trace, warn};
use rouille::Response;

const ORIGIN: &str = "https://ui.perfetto.dev";

fn start_trace_server(trace: &[u8], temporary: bool) -> anyhow::Result<()> {
    if temporary {
        info!("Starting temporary trace-provider server..");
    } else {
        info!("Starting trace-provider server..");
    }

    let trace = trace.to_vec();

    let notif_trace_served = Arc::new(Condvar::new());
    let wait_trace_served = notif_trace_served.clone();

    let now = SystemTime::now()
        .duration_since(SystemTime::UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let etag: String = ((now as u64) ^ 0xd3f4_0305_c9f8_e911_u64).to_string();

    let server = rouille::Server::new("127.0.0.1:9001", move |request| {
        trace!("SERVER: Received request '{:?}'", request);

        let resp = match request.method() {
            "GET" => {
                if request.url() == "/trace.proto" {
                    info!("Server: Serving trace.");
                    notif_trace_served.notify_all();
                    Response::from_data("application/octet-stream", trace.clone())
                        .with_etag(request, etag.clone())
                        .with_additional_header("Access-Control-Allow-Origin", ORIGIN.to_string())
                } else {
                    warn!("Server: Unknown GET request.");
                    Response::html("404 error.").with_status_code(404)
                }
            }
            "POST" => {
                debug!("Server: Acknowledging POST request.");
                Response::html("").with_status_code(200)
            }
            "OPTIONS" => {
                debug!("Server: Acknowledging OPTIONS request.");
                Response::html("").with_status_code(200)
            }
            _ => {
                warn!("Server: Unknown request method.");
                Response::html("404 error.").with_status_code(404)
            }
        };

        trace!("SERVER: Response: '{:?}'", resp);
        resp
    })
    .unwrap();

    let (handle, stop) = server.stoppable();

    if temporary {
        let m: Mutex<()> = Mutex::new(());
        let l = m.lock().unwrap();
        let _lock = wait_trace_served.wait(l).unwrap();
        std::thread::sleep(Duration::from_millis(250));
        info!("Stopping server..");
        stop.send(()).unwrap();
    }

    let _ = handle.join();

    info!("Server stopped.");

    Ok(())
}

pub fn serve_trace(trace: &[u8]) -> anyhow::Result<()> {
    let link = format!("{ORIGIN}/#!/?url=http://127.0.0.1:9001/trace.proto");
    info!("Serving trace.\n\n  Link: {link}\n");
    start_trace_server(trace, false)?;
    Ok(())
}

pub fn open_trace(trace: &[u8]) -> anyhow::Result<()> {
    let link = format!("{ORIGIN}/#!/?url=http://127.0.0.1:9001/trace.proto");
    info!("Opening trace in perfetto..");
    webbrowser::open(&link)?;
    start_trace_server(trace, true)?;
    Ok(())
}
