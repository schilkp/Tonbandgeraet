<script setup lang="ts">
document.addEventListener("frtrace_log", (e: any) => {
  const table = <HTMLTableElement>document.getElementById("logs");
  const row = table.insertRow(-1);

  const cell_lvl = row.insertCell(-1);
  cell_lvl.setAttribute("width", "30px");
  cell_lvl.style.paddingTop = "1px";
  cell_lvl.style.paddingBottom = "1px";
  cell_lvl.innerHTML = log_marker(e.detail.lvl);

  const cell_msg = row.insertCell(-1);
  cell_msg.style.paddingTop = "1px";
  cell_msg.style.paddingBottom = "1px";
  cell_msg.innerHTML = String(e.detail.msg);

  const div = <HTMLDivElement>document.getElementById("logs-panel");
  div.scrollTop = table.scrollHeight;
});

function log_marker(lvl: string): string {
  var lbl_class = "";
  switch (lvl) {
    case "Trace":
    case "Debug":
    case "Info":
      lbl_class = "";
      break;
    case "Success":
      lbl_class = "uk-label-success";
      break;
    case "Warn":
      lbl_class = "uk-label-warning";
      break;
    case "Error":
    default:
      lbl_class = "uk-label-danger";
      break;
  }

  return (
    '<span class ="uk-label uk-width-1-1 uk-text-center ' +
    lbl_class +
    '">' +
    lvl +
    "</span>"
  );
}
</script>

<template>
  <div
    class="uk-panel uk-panel-scrollable"
    style="height:250px"
    id="logs-panel"
  >
    <table class="uk-table uk-table-small" id="logs">
      <tbody></tbody>
    </table>
  </div>
</template>
