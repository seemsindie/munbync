import type { Transport, WebSerialOptions } from '../types.js';
import { readTimeout, readWithTimeout } from './timeout.js';

export class WebSerialTransport implements Transport {
  private port: SerialPort | null = null;
  private writer: WritableStreamDefaultWriter<Uint8Array> | null = null;
  private reader: ReadableStreamDefaultReader<Uint8Array> | null = null;
  private buffered = new Uint8Array(0);
  private reading = false;
  private timeoutMs: number;

  constructor(private options: WebSerialOptions = {}) {
    this.timeoutMs = readTimeout(options.readTimeoutMs);
  }

  get connected(): boolean {
    return this.port !== null && this.writer !== null;
  }

  async connect(): Promise<void> {
    if (typeof navigator === 'undefined' || !('serial' in navigator)) {
      throw new Error('WebSerial API is not supported in this browser');
    }
    await this.connectToPort(await navigator.serial.requestPort({
      filters: this.options.filters ?? [],
    }));
  }

  async connectToPort(port: SerialPort): Promise<void> {
    if (this.port) throw new Error('Serial port is already connected');
    await port.open({
      baudRate: this.options.baudRate ?? 9600,
      dataBits: 8, stopBits: 1, parity: 'none', flowControl: 'none',
    });
    this.port = port;
    try {
      if (!port.writable || !port.readable) throw new Error('Serial streams unavailable');
      this.writer = port.writable.getWriter();
      this.reader = port.readable.getReader();
    } catch (error) {
      await this.disconnect().catch(() => {});
      throw error;
    }
  }

  async disconnect(): Promise<void> {
    const port = this.port;
    const reader = this.reader;
    const writer = this.writer;
    this.port = null;
    this.reader = null;
    this.writer = null;
    this.buffered = new Uint8Array(0);
    try {
      if (reader) {
        try { await reader.cancel(); } finally { reader.releaseLock(); }
      }
    } finally {
      writer?.releaseLock();
      if (port) await port.close();
    }
  }

  async write(data: Uint8Array): Promise<void> {
    if (!this.writer) throw new Error('Serial port is not open for writing');
    await this.writer.write(data);
  }

  async read(length: number): Promise<Uint8Array> {
    if (!this.reader) throw new Error('Serial port is not open for reading');
    if (!Number.isInteger(length) || length < 1) throw new RangeError('Invalid read length');
    if (this.reading) throw new Error('A serial read is already in progress');
    this.reading = true;
    try {
      if (this.buffered.length === 0) {
        const { value, done } = await readWithTimeout(
          this.reader.read(), this.timeoutMs, () => this.disconnect()
        );
        if (done || !value) throw new Error('Serial stream closed');
        this.buffered = new Uint8Array(value);
      }
      const result = this.buffered.slice(0, length);
      this.buffered = this.buffered.slice(result.length);
      return result;
    } finally {
      this.reading = false;
    }
  }
}
