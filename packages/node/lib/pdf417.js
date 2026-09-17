const bwipjs = require('bwip-js');

/** Encode locally: no native PDF417 commands, canvas, or network service. */
function encodePdf417(data, options = {}) {
  const { columns = 0, ecLevel = 2, moduleSize = 2, maxWidth = 512 } = options;
  for (const [value, min, max] of [[columns, 0, 30], [ecLevel, 0, 8], [moduleSize, 1, 8], [maxWidth, 1, 65535]]) {
    if (!Number.isInteger(value) || value < min || value > max) throw new RangeError('Invalid PDF417 options');
  }
  if (typeof data !== 'string' && !(data instanceof Uint8Array)) throw new TypeError('Expected text or Uint8Array');
  const payload = typeof data === 'string' ? new TextEncoder().encode(data) : data;
  if (!payload.length || payload.length > 2710) throw new RangeError('PDF417 input must contain 1..2710 bytes; actual capacity depends on content');
  // Explicit UTF-8 ECI for strings. Escape every byte so user carets cannot be
  // interpreted as BWIPP directives; byte inputs carry no implicit encoding.
  const text = (typeof data === 'string' ? '^ECI000026' : '') +
    Array.from(payload, b => '^' + b.toString().padStart(3, '0')).join('');
  const maxColumns = Math.min(30, Math.floor((Math.floor(Math.floor(maxWidth / 8) * 8 / moduleSize) - 4 - 69) / 17));
  if (maxColumns < 1 || columns > maxColumns) throw new RangeError('PDF417 cannot fit the active print width');
  let lastError;
  // Prefer a compact symbol; grow columns only when capacity requires it.
  for (let count = columns || Math.min(4, maxColumns); count <= (columns || maxColumns); count++) {
    try {
      const encoderOptions = { columns: count, eclevel: ecLevel, binarytext: true, parse: true, parsefnc: true };
      const symbol = bwipjs.raw('pdf417', text, encoderOptions)[0];
      if (!symbol || !('pixs' in symbol)) throw new Error('PDF417 encoder did not return a bitmap');
      const rows = symbol.pixs.length / symbol.pixx;
      // BWIPP 2026 reports pixy including its 3x row height while pixs contains
      // one logical row per codeword row. Older versions reported logical pixy.
      const rowHeight = Math.round((symbol.height / rows) / (symbol.width / symbol.pixx));
      if (!Number.isInteger(rows) || rows < 3 || rowHeight < 3) throw new Error('Invalid PDF417 row geometry');
      const width = (symbol.pixx + 4) * moduleSize;
      const height = (rows * rowHeight + 4) * moduleSize;
      if (Math.ceil(width / 8) * 8 > maxWidth) throw new RangeError('PDF417 padded raster exceeds the active print width');
      const stride = Math.ceil(width / 8);
      const bitmap = new Uint8Array(stride * height);
      const border = 2 * moduleSize;
      for (let y = 0; y < rows; y++) for (let x = 0; x < symbol.pixx; x++) {
        if (!symbol.pixs[y * symbol.pixx + x]) continue;
        for (let dy = 0; dy < rowHeight * moduleSize; dy++) for (let dx = 0; dx < moduleSize; dx++) {
          const px = border + x * moduleSize + dx;
          const py = border + y * rowHeight * moduleSize + dy;
          bitmap[py * stride + (px >> 3)] |= 0x80 >> (px & 7);
        }
      }
      return { bitmap, width, height };
    } catch (error) { lastError = error; }
  }
  throw new RangeError(`PDF417 does not fit the selected capacity/width: ${String(lastError)}`);
}

module.exports = { encodePdf417 };
