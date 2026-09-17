// Wire syntax validation matching src/munbyn_barcode.inc. GS1 AI business rules
// (dates, check digits, application-specific field lengths) remain caller-owned.
export function validateBarcode(type: number, data: Uint8Array): void {
  const fail = (): never => { throw new RangeError('Invalid barcode type, characters, length or code-set syntax'); };
  if (![0,1,2,3,4,5,6,65,66,67,68,69,70,71,72,73,74,75,76,77,78].includes(type) ||
      data.length < 1 || data.length > 255) fail();
  const digits = (bytes: Uint8Array) => bytes.every(b => b >= 48 && b <= 57);
  const base = type >= 65 && type <= 71 ? type - 65 : type;
  if (base <= 3) {
    const minimum = base <= 1 ? 11 : base === 2 ? 12 : 7;
    if (![minimum, minimum + 1].includes(data.length) || !digits(data)) fail();
  } else if (base === 4) {
    const first = data[0] === 42 ? 1 : 0;
    const last = data.length > first && data[data.length - 1] === 42 ? data.length - 1 : data.length;
    if (first === last || !data.slice(first, last).every(b => '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%+-./'.includes(String.fromCharCode(b)))) fail();
  } else if (base === 5) {
    if (data.length % 2 || !digits(data)) fail();
  } else if (base === 6) {
    const ends = [data[0], data[data.length - 1]];
    if (data.length < 2 || ends.some(b => b < 65 || b > 68) ||
        !data.slice(1, -1).every(b => '0123456789$+-./:'.includes(String.fromCharCode(b)))) fail();
  } else if (type === 72) {
    if (data.some(b => b > 127)) fail();
  } else if (type === 73) {
    if (data.length < 3 || data[0] !== 123 || data[1] < 65 || data[1] > 67) fail();
    let set = data[1], shift = false, hasData = false;
    for (let i = 2; i < data.length; i++) {
      const value = data[i];
      if (value === 123) {
        if (++i === data.length) fail();
        const escape = data[i];
        if (escape >= 65 && escape <= 67) {
          if (shift) fail();
          set = escape;
          continue;
        }
        if (escape === 83) {
          if (set === 67 || shift) fail();
          shift = true;
          continue;
        }
        if (escape >= 49 && escape <= 52) {
          if (shift || (set === 67 && escape !== 49)) fail();
          hasData = true;
          continue;
        }
        if (escape !== 123) fail();
      }
      const active = shift ? (set === 65 ? 66 : 65) : set;
      if ((active === 65 && value > 95) || (active === 66 && (value < 32 || value > 127)) ||
          (active === 67 && value > 99)) fail();
      shift = false;
      hasData = true;
    }
    if (!hasData || shift) fail();
  } else if (type >= 75 && type <= 77) {
    if (data.length !== 13 || !digits(data) || (type === 77 && data[0] > 49)) fail();
  } else {
    const first = data[0] === 40 ? 1 : 0;
    if (data.length < first + 2 || !digits(data.slice(first, first + 2))) fail();
    let parentheses = 0;
    for (let i = 0; i < data.length; i++) {
      const ch = data[i];
      if (type === 74 && ch > 127) fail();
      if (type === 78 && (ch < 32 || ch > 123 || '#&@[\\]^`'.includes(String.fromCharCode(ch)))) fail();
      if (ch === 40 && ++parentheses > 1) fail();
      if (ch === 41 && --parentheses < 0) fail();
      if (ch === 123 && (++i === data.length || !'13()*{'.includes(String.fromCharCode(data[i])))) fail();
    }
    if (parentheses !== 0) fail();
  }
}
