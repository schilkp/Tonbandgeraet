export function download_trace(trace: Uint8Array, file_name: string) {
    var a = document.createElement("a");
    document.body.appendChild(a);
    // a.style = "display: none";
    a.setAttribute("style", "display: none");

    const blob = new Blob([trace]);
    var url = window.URL.createObjectURL(blob);
    a.href = url;
    a.download = file_name;
    a.click();
    window.URL.revokeObjectURL(url);
};
