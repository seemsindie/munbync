export interface PrinterStatus {
  paperPresent: boolean;
  coverClosed: boolean;
  online: boolean;
  errorOccurred: boolean;
  cutError: boolean;
  recoverableError: boolean;
  unrecoverableError: boolean;
}

export interface Transport {
  connect(): Promise<void>;
  disconnect(): Promise<void>;
  write(data: Uint8Array): Promise<void>;
  /** Read up to length bytes. Reject on timeout; never wait indefinitely. */
  read(length: number): Promise<Uint8Array>;
  readonly connected: boolean;
}

export interface WebSerialOptions {
  /** Default 1000 ms. A timed-out read closes the connection. */
  readTimeoutMs?: number;
  baudRate?: number;
  filters?: SerialPortFilter[];
}

export interface WebUSBOptions {
  /** Default 1000 ms. A timed-out read closes the connection. */
  readTimeoutMs?: number;
  filters?: USBDeviceFilter[];
}
