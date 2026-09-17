export async function readWithTimeout<T>(
  operation: Promise<T>,
  timeoutMs: number,
  disconnect: () => Promise<void>
): Promise<T> {
  let timer: ReturnType<typeof setTimeout> | undefined;
  let timedOut = false;
  try {
    return await Promise.race([
      operation,
      new Promise<never>((_, reject) => {
        timer = setTimeout(() => {
          timedOut = true;
          reject(new Error('Printer read timed out; reconnect before retrying'));
        }, timeoutMs);
      }),
    ]);
  } catch (error) {
    // Close the transport to cancel the pending read. Otherwise a late reply
    // could be mistaken for the response to a subsequent status request.
    if (timedOut) await disconnect().catch(() => {});
    throw error;
  } finally {
    if (timer !== undefined) clearTimeout(timer);
  }
}

export function readTimeout(value: number | undefined): number {
  const timeout = value ?? 1000;
  if (!Number.isInteger(timeout) || timeout < 1 || timeout > 2147483647) {
    throw new RangeError('readTimeoutMs must be a positive 32-bit integer');
  }
  return timeout;
}
