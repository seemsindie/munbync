import type { Transport, WebUSBOptions } from '../types.js';
import { readTimeout, readWithTimeout } from './timeout.js';

export class WebUSBTransport implements Transport {
  private device: USBDevice | null = null;
  private endpointIn = 0;
  private endpointOut = 0;
  private reading = false;
  private timeoutMs: number;

  constructor(private options: WebUSBOptions = {}) {
    this.timeoutMs = readTimeout(options.readTimeoutMs);
  }

  get connected(): boolean {
    return (this.device?.opened ?? false) && this.endpointOut !== 0;
  }

  async connect(): Promise<void> {
    if (typeof navigator === 'undefined' || !('usb' in navigator)) {
      throw new Error('WebUSB API is not supported in this browser');
    }
    await this.connectToDevice(await navigator.usb.requestDevice({
      filters: this.options.filters ?? [
        { vendorId: 0x0416 }, { vendorId: 0x0483 },
        { vendorId: 0x04b8 }, { vendorId: 0x0fe6 },
      ],
    }));
  }

  async connectToDevice(device: USBDevice): Promise<void> {
    if (this.device) throw new Error('USB device is already connected');
    await device.open();
    this.device = device;
    try {
      if (device.configuration === null) await device.selectConfiguration(1);
      for (const iface of device.configuration!.interfaces) {
        const alternate = iface.alternates.find((alt) =>
          (alt.interfaceClass === 7 || alt.interfaceClass === 0xff) &&
          alt.endpoints.some((ep) => ep.type === 'bulk' && ep.direction === 'out')
        );
        if (!alternate) continue;
        await device.claimInterface(iface.interfaceNumber);
        await device.selectAlternateInterface(iface.interfaceNumber, alternate.alternateSetting);
        for (const ep of alternate.endpoints) {
          if (ep.type !== 'bulk') continue;
          if (ep.direction === 'in') this.endpointIn = ep.endpointNumber;
          else this.endpointOut = ep.endpointNumber;
        }
        return;
      }
      throw new Error('No suitable USB interface with a bulk OUT endpoint');
    } catch (error) {
      await this.disconnect().catch(() => {});
      throw error;
    }
  }

  async disconnect(): Promise<void> {
    const device = this.device;
    this.device = null;
    this.endpointIn = 0;
    this.endpointOut = 0;
    // Closing releases claimed interfaces and aborts pending transfers.
    if (device?.opened) await device.close();
  }

  async write(data: Uint8Array): Promise<void> {
    if (!this.connected || !this.device) throw new Error('USB device is not open');
    const result = await this.device.transferOut(this.endpointOut, data as BufferSource);
    if (result.status !== 'ok' || result.bytesWritten !== data.length) {
      throw new Error('USB write failed or was incomplete');
    }
  }

  async read(length: number): Promise<Uint8Array> {
    if (!this.connected || !this.device || this.endpointIn === 0) {
      throw new Error('USB device is not open or has no IN endpoint');
    }
    if (!Number.isInteger(length) || length < 1) throw new RangeError('Invalid read length');
    if (this.reading) throw new Error('A USB read is already in progress');
    this.reading = true;
    try {
      const result = await readWithTimeout(
        this.device.transferIn(this.endpointIn, length), this.timeoutMs, () => this.disconnect()
      );
      if (result.status !== 'ok' || !result.data?.byteLength) {
        throw new Error('USB read failed or returned no data');
      }
      return new Uint8Array(result.data.buffer, result.data.byteOffset, result.data.byteLength);
    } finally {
      this.reading = false;
    }
  }
}
