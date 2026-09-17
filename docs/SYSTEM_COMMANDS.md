# System and diagnostic commands

## Proprietary self-test

`munbyn_self_test(handle)` sends `1F 1B 1F 67`. This command is retained from the
existing implementation but is absent from the bundled ITPP047 manual. Firmware
support and printed content must be verified on the target device. A successful
return means the bytes were sent, not that the printer completed a diagnostic.

```c
munbyn_status_t status;
munbyn_error_t result = munbyn_get_status(printer, &status);
if (result == MUNBYN_OK && status.online && status.paper_present &&
    status.cover_closed && !status.error_occurred) {
    result = munbyn_self_test(printer);
}
```

Some one-way interfaces can print without returning status. Applications must
handle that transport limitation explicitly instead of treating missing status
as a healthy printer. `examples/self_test_example.c` demonstrates USB/serial
connection, status reporting, and sending the self-test.

## Manual hex-dump command

`munbyn_execute_test_print(handle, n, m)` sends `GS ( A`. Despite the historical
API name, §2.48 on page 37 documents **hex-dump mode**, with `n=0/48` and `m=1/49`.
It is distinct from the proprietary self-test. Node/browser wrappers default to
`n=0, m=1`. Subsequent output may be hexadecimal diagnostics until the device is
reset; this command is not part of normal receipt verification.

## Status

`munbyn_get_status()` sends `DLE EOT 1`, `2`, `3`, and `4` in order and reads one
byte per request. All replies must be valid before the result is populated.
Timeouts, failed reads, and malformed bytes return errors. C clears the supplied
status structure on query failure; Node/browser callers receive an exception.
Keep ASB disabled during these synchronous queries and reconnect after a timeout.

See [command coverage](COMMAND_COVERAGE.md) for PDF references, exact parameter
limits, and firmware-dependent extensions.
