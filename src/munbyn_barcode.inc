/* GS k validation and framing. Manual 1.00 section 2.61; Epson GS1 extensions.
 * GS1 checks here cover wire syntax, not business rules for every GS1 AI. */
static bool barcode_digits(const uint8_t* data, size_t length)
{
    for (size_t i = 0; i < length; ++i)
        if (data[i] < '0' || data[i] > '9') return false;
    return true;
}

static bool barcode_code128(const uint8_t* data, size_t length)
{
    if (length < 3 || data[0] != '{' || data[1] < 'A' || data[1] > 'C') return false;
    uint8_t set = data[1];
    bool shift = false, has_data = false;
    for (size_t i = 2; i < length; ++i) {
        unsigned int value = data[i];
        if (value == '{') {
            if (++i == length) return false;
            uint8_t escape = data[i];
            if (escape >= 'A' && escape <= 'C') {
                if (shift) return false;
                set = escape;
                continue;
            }
            if (escape == 'S') {
                if (set == 'C' || shift) return false;
                shift = true;
                continue;
            }
            if (escape >= '1' && escape <= '4') {
                if (shift || (set == 'C' && escape != '1')) return false;
                has_data = true;
                continue;
            }
            if (escape != '{') return false;
        }
        uint8_t active = shift ? (set == 'A' ? 'B' : 'A') : set;
        if ((active == 'A' && value > 95) ||
            (active == 'B' && (value < 32 || value > 127)) ||
            (active == 'C' && value > 99)) return false;
        shift = false;
        has_data = true;
    }
    return has_data && !shift;
}

static bool barcode_valid(int type, const uint8_t* data, size_t length)
{
    if (!data || length == 0 || length > 255) return false;
    int base = type >= 65 && type <= 71 ? type - 65 : type;
    if (base >= 0 && base <= 3) {
        size_t minimum = base <= 1 ? 11 : (base == 2 ? 12 : 7);
        return (length == minimum || length == minimum + 1) && barcode_digits(data, length);
    }
    if (base == 4) {
        size_t first = data[0] == '*' ? 1 : 0;
        size_t last = length > first && data[length - 1] == '*' ? length - 1 : length;
        if (first == last) return false;
        for (size_t i = first; i < last; ++i)
            if (!data[i] || !strchr("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%+-./", data[i])) return false;
        return true;
    }
    if (base == 5) return length % 2 == 0 && barcode_digits(data, length);
    if (base == 6) {
        if (length < 2 || data[0] < 'A' || data[0] > 'D' ||
            data[length - 1] < 'A' || data[length - 1] > 'D') return false;
        for (size_t i = 1; i + 1 < length; ++i)
            if (!data[i] || !strchr("0123456789$+-./:", data[i])) return false;
        return true;
    }
    if (type == 72) {
        for (size_t i = 0; i < length; ++i) if (data[i] > 127) return false;
        return true;
    }
    if (type == 73) return barcode_code128(data, length);
    if (type >= 75 && type <= 77)
        return length == 13 && barcode_digits(data, length) && (type != 77 || data[0] <= '1');
    if (type == 74 || type == 78) {
        size_t first = data[0] == '(' ? 1 : 0;
        if (length < first + 2 || !barcode_digits(data + first, 2)) return false;
        int parentheses = 0;
        for (size_t i = 0; i < length; ++i) {
            uint8_t ch = data[i];
            if (type == 74 && ch > 127) return false;
            if (type == 78 && (ch < 32 || ch > 123 || strchr("#&@[\\]^`", ch))) return false;
            if (ch == '(' && ++parentheses > 1) return false;
            if (ch == ')' && --parentheses < 0) return false;
            if (ch == '{') {
                if (++i == length || !data[i] || (data[i] != '1' && data[i] != '3' &&
                    !strchr("()*{", data[i]))) return false;
            }
        }
        return parentheses == 0;
    }
    return false;
}

munbyn_error_t munbyn_print_barcode_bytes(munbyn_handle_t handle, munbyn_barcode_t type,
                                         const uint8_t* data, size_t length)
{
    if (!handle || !handle->initialized || !barcode_valid(type, data, length))
        return MUNBYN_ERROR_INVALID_PARAMETER;
    bool function_b = type >= 65;
    uint8_t command[259];
    command[0] = GS; command[1] = 0x6B; command[2] = (uint8_t)type;
    if (function_b) command[3] = (uint8_t)length;
    memcpy(command + (function_b ? 4 : 3), data, length);
    if (!function_b) command[3 + length] = 0;
    return munbyn_write_data(handle, command, length + 4);
}

munbyn_error_t munbyn_print_barcode(munbyn_handle_t handle, munbyn_barcode_t type, const char* data)
{
    if (!data) return MUNBYN_ERROR_INVALID_PARAMETER;
    return munbyn_print_barcode_bytes(handle, type, (const uint8_t*)data, strlen(data));
}
