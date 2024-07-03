# Instrumenting Your Code

After Tonbandgerät has been installed, configured, and ported, you can start to collect
trace events by instrumenting your code.

These tracing event are split into two main groups. Firstly, there are base tracing events that
can be used in any firmware project. These consist of:
- [Event Markers](./evtmarkers.md)
- [Value Markers](./valmarkers.md)
- [ISR Tracing](./interrupts.md)

Then there are [FreeRTOS-specific tracing events](./freertos.md):
- [Task Tracing](./freertos_tasks.md)
- [Resource Tracing](./freertos_resources.md)
- [Task-local markers](./freertos_task_local_markers.md)
