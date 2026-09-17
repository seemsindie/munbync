#include "munbyn_encoding_tables.inc"

munbyn_error_t munbyn_print_encoded(munbyn_handle_t handle, const char* utf8,
                                   munbyn_text_encoding_t encoding, uint8_t codepage)
{
    if (!handle || !handle->initialized || !utf8 || encoding < MUNBYN_ENCODING_ASCII ||
        encoding > MUNBYN_ENCODING_WINDOWS1252) return MUNBYN_ERROR_INVALID_PARAMETER;
    size_t length = strlen(utf8);
    if (length > SIZE_MAX - 8) return MUNBYN_ERROR_BUFFER_OVERFLOW;
    uint8_t* command = malloc(length + 8);
    if (!command) return MUNBYN_ERROR_BUFFER_OVERFLOW;
    const uint8_t prefix[] = {FS, 0x2E, ESC, 0x52, 0, ESC, 0x74, codepage};
    memcpy(command, prefix, sizeof(prefix));
    size_t used = sizeof(prefix), i = 0;
    while (i < length) {
        uint32_t scalar = (uint8_t)utf8[i++];
        unsigned int extra = 0;
        uint32_t minimum = 0;
        if (scalar >= 0xC2 && scalar <= 0xDF) { extra = 1; minimum = 0x80; scalar &= 0x1F; }
        else if (scalar >= 0xE0 && scalar <= 0xEF) { extra = 2; minimum = 0x800; scalar &= 0x0F; }
        else if (scalar >= 0xF0 && scalar <= 0xF4) { extra = 3; minimum = 0x10000; scalar &= 7; }
        else if (scalar >= 0x80) goto invalid;
        if (length - i < extra) goto invalid;
        for (unsigned int j = 0; j < extra; ++j) {
            uint8_t byte = (uint8_t)utf8[i++];
            if ((byte & 0xC0) != 0x80) goto invalid;
            scalar = (scalar << 6) | (byte & 0x3F);
        }
        if (scalar < minimum || scalar > 0x10FFFF || (scalar >= 0xD800 && scalar <= 0xDFFF)) goto invalid;
        if (scalar < 128) {
            if ((scalar < 32 && scalar != 9 && scalar != 10 && scalar != 13) || scalar == 127) goto invalid;
            command[used++] = (uint8_t)scalar;
        } else {
            if (encoding == MUNBYN_ENCODING_ASCII) goto invalid;
            unsigned int byte;
            for (byte = 0; byte < 128; ++byte)
                if (munbyn_encoding_tables[encoding - 1][byte] == scalar && scalar != 0xFFFF) break;
            if (byte == 128) goto invalid;
            command[used++] = (uint8_t)(byte + 128);
        }
    }
    munbyn_error_t result = munbyn_write_data(handle, command, used);
    free(command);
    if (result == MUNBYN_OK) {
        handle->current_codepage = (munbyn_codepage_t)codepage;
        handle->current_charset = MUNBYN_INTL_USA;
    }
    return result;
invalid:
    free(command);
    return MUNBYN_ERROR_INVALID_PARAMETER;
}
