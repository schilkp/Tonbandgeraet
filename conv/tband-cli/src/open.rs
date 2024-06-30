use std::{
    sync::{Arc, Condvar, Mutex},
    thread,
    time::{Duration, SystemTime},
};

use axum::{
    http::{header, Response, StatusCode},
    routing::{get, post},
    Router,
};
use log::{debug, info};
use tokio::sync::mpsc;

const ORIGIN: &str = "https://ui.perfetto.dev";

async fn server(trace: Vec<u8>, notif_trace_served: Arc<Condvar>) {
    let now = SystemTime::now()
        .duration_since(SystemTime::UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let etag: String = ((now as u64) ^ 0xd3f4_0305_c9f8_e911_u64).to_string();

    let app = Router::new()
        .route(
            "/trace.proto",
            get(|| async move {
                let resp = Response::builder()
                    .status(200)
                    .header(header::CONTENT_TYPE, "application/octet-stream")
                    .header(header::ETAG, etag.clone())
                    .header("Access-Control-Allow-Origin", ORIGIN.to_string())
                    .body(trace.clone())
                    .unwrap();
                notif_trace_served.notify_all();
                info!("SERVER: Serving trace for /trace.proto GET request.");
                resp.into_parts()
            }),
        )
        .route(
            "/status",
            post(|| async move {
                debug!("SERVER: Serving OK for status GET request.");
                StatusCode::OK
            }),
        );

    let listener = tokio::net::TcpListener::bind("127.0.0.1:9001").await.unwrap();
    axum::serve(listener, app).await.unwrap();
}

fn start_trace_server(trace: &[u8], temporary: bool) -> anyhow::Result<()> {
    if temporary {
        info!("Starting temporary trace-provider server..");
    } else {
        info!("Starting trace-provider server..");
    }

    let trace = trace.to_vec();

    let notif_trace_served = Arc::new(Condvar::new());
    let wait_trace_served = notif_trace_served.clone();

    let (send_stop, mut stop) = mpsc::channel::<()>(1);

    let server_thread = thread::spawn(move || {
        tokio::runtime::Builder::new_current_thread()
            .enable_all()
            .build()
            .unwrap()
            .block_on(async {
                tokio::select! {
                    _ = server(trace, notif_trace_served) => {
                    }
                    _ = stop.recv() => {
                    }
                }
            })
    });

    if temporary {
        let m: Mutex<()> = Mutex::new(());
        let l = m.lock().unwrap();
        let _lock = wait_trace_served.wait(l).unwrap();
        std::thread::sleep(Duration::from_millis(250));
        info!("Stopping server..");
        send_stop.blocking_send(()).unwrap();
    }

    let _ = server_thread.join();
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
