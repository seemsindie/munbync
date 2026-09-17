#include <napi.h>
#include <vector>
#include <cmath>

extern "C" {
#include "munbyn_printer.h"
}

// Helper: throw a JS error from a munbyn error code and return undefined
static Napi::Value ThrowMunbynError(Napi::Env env, munbyn_error_t err) {
    const char* msg;
    switch (err) {
        case MUNBYN_ERROR_INVALID_HANDLE:     msg = "Invalid printer handle"; break;
        case MUNBYN_ERROR_COMMUNICATION:      msg = "Communication error"; break;
        case MUNBYN_ERROR_INVALID_PARAMETER:  msg = "Invalid parameter"; break;
        case MUNBYN_ERROR_BUFFER_OVERFLOW:    msg = "Buffer overflow"; break;
        case MUNBYN_ERROR_TIMEOUT:            msg = "Timeout"; break;
        case MUNBYN_ERROR_NOT_INITIALIZED:    msg = "Printer not initialized"; break;
        default:                              msg = "Unknown error"; break;
    }
    Napi::Error::New(env, msg).ThrowAsJavaScriptException();
    return env.Undefined();
}

// Validate before narrowing JS numbers to fixed-width C values.
static bool CheckInteger(Napi::Env env, Napi::Value value, double min, double max) {
    if (!value.IsNumber()) {
        Napi::TypeError::New(env, "Expected an integer").ThrowAsJavaScriptException();
        return false;
    }
    double n = value.As<Napi::Number>().DoubleValue();
    if (!std::isfinite(n) || std::floor(n) != n || n < min || n > max) {
        Napi::RangeError::New(env, "Integer outside supported range").ThrowAsJavaScriptException();
        return false;
    }
    return true;
}

// Check result macro - throws and returns undefined on error
#define CHECK_RESULT(env, result) \
    if ((result) != MUNBYN_OK) return ThrowMunbynError(env, result)

class MunbynPrinter : public Napi::ObjectWrap<MunbynPrinter> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "MunbynPrinter", {
            // Connection
            InstanceMethod("openUsb", &MunbynPrinter::OpenUsb),
            InstanceMethod("openSerial", &MunbynPrinter::OpenSerial),
            InstanceMethod("openNetwork", &MunbynPrinter::OpenNetwork),
            InstanceMethod("close", &MunbynPrinter::Close),

            // Core
            InstanceMethod("initialize", &MunbynPrinter::Initialize),
            InstanceMethod("writeData", &MunbynPrinter::WriteData),
            InstanceMethod("readData", &MunbynPrinter::ReadData),
            InstanceMethod("getStatus", &MunbynPrinter::GetStatus),

            // Basic operations
            InstanceMethod("cutPaper", &MunbynPrinter::CutPaper),
            InstanceMethod("feedAndCut", &MunbynPrinter::FeedAndCut),
            InstanceMethod("feedLines", &MunbynPrinter::FeedLines),
            InstanceMethod("printAndCut", &MunbynPrinter::PrintAndCut),
            InstanceMethod("openDrawer", &MunbynPrinter::OpenDrawer),
            InstanceMethod("openDrawerDefault", &MunbynPrinter::OpenDrawerDefault),
            InstanceMethod("selfTest", &MunbynPrinter::SelfTest),

            // Text control
            InstanceMethod("lineFeed", &MunbynPrinter::LineFeed),
            InstanceMethod("carriageReturn", &MunbynPrinter::CarriageReturn),
            InstanceMethod("horizontalTab", &MunbynPrinter::HorizontalTab),
            InstanceMethod("setHorizontalTabPositions", &MunbynPrinter::SetHorizontalTabPositions),
            InstanceMethod("clearHorizontalTabPositions", &MunbynPrinter::ClearHorizontalTabPositions),

            // Character set
            InstanceMethod("setInternationalCharset", &MunbynPrinter::SetInternationalCharset),
            InstanceMethod("setCodepage", &MunbynPrinter::SetCodepage),
            InstanceMethod("getCodepageName", &MunbynPrinter::GetCodepageName),

            // Text formatting
            InstanceMethod("setJustification", &MunbynPrinter::SetJustification),
            InstanceMethod("setFont", &MunbynPrinter::SetFont),
            InstanceMethod("setTextMode", &MunbynPrinter::SetTextMode),
            InstanceMethod("setEmphasis", &MunbynPrinter::SetEmphasis),
            InstanceMethod("setDoubleStrike", &MunbynPrinter::SetDoubleStrike),
            InstanceMethod("setUnderline", &MunbynPrinter::SetUnderline),
            InstanceMethod("setUnderlineKanji", &MunbynPrinter::SetUnderlineKanji),
            InstanceMethod("setLineSpacingDefault", &MunbynPrinter::SetLineSpacingDefault),
            InstanceMethod("setLineSpacing", &MunbynPrinter::SetLineSpacing),
            InstanceMethod("setMotionUnits", &MunbynPrinter::SetMotionUnits),
            InstanceMethod("setCharacterSpacing", &MunbynPrinter::SetCharacterSpacing),
            InstanceMethod("setLeftMargin", &MunbynPrinter::SetLeftMargin),
            InstanceMethod("setPrintAreaWidth", &MunbynPrinter::SetPrintAreaWidth),

            // Rotation and inversion
            InstanceMethod("setRotate90", &MunbynPrinter::SetRotate90),
            InstanceMethod("setUpsideDown", &MunbynPrinter::SetUpsideDown),

            // Advanced text
            InstanceMethod("setInvertedText", &MunbynPrinter::SetInvertedText),
            InstanceMethod("setTextScale", &MunbynPrinter::SetTextScale),
            InstanceMethod("cancelAllFormatting", &MunbynPrinter::CancelAllFormatting),

            // Print direction/position
            InstanceMethod("setPrintDirection", &MunbynPrinter::SetPrintDirection),
            InstanceMethod("setRelativeHorizontalPosition", &MunbynPrinter::SetRelativeHorizontalPosition),
            InstanceMethod("setAbsoluteHorizontalPosition", &MunbynPrinter::SetAbsoluteHorizontalPosition),

            // Barcode
            InstanceMethod("setBarcodeHeight", &MunbynPrinter::SetBarcodeHeight),
            InstanceMethod("setBarcodeWidth", &MunbynPrinter::SetBarcodeWidth),
            InstanceMethod("setHriPosition", &MunbynPrinter::SetHriPosition),
            InstanceMethod("setHriFont", &MunbynPrinter::SetHriFont),
            InstanceMethod("printBarcode", &MunbynPrinter::PrintBarcode),

            // 2D barcodes
            InstanceMethod("printQr", &MunbynPrinter::PrintQr),
            InstanceMethod("printPdf417", &MunbynPrinter::PrintPdf417),

            // Image
            InstanceMethod("printRasterImage", &MunbynPrinter::PrintRasterImage),
            InstanceMethod("printBitImage", &MunbynPrinter::PrintBitImage),
            InstanceMethod("defineDownloadedBitImage", &MunbynPrinter::DefineDownloadedBitImage),
            InstanceMethod("printDownloadedBitImage", &MunbynPrinter::PrintDownloadedBitImage),
            InstanceMethod("printNvBitImage", &MunbynPrinter::PrintNvBitImage),
            InstanceMethod("defineNvBitImage", &MunbynPrinter::DefineNvBitImage),

            // Page mode
            InstanceMethod("selectPageMode", &MunbynPrinter::SelectPageMode),
            InstanceMethod("selectStandardMode", &MunbynPrinter::SelectStandardMode),
            InstanceMethod("printPageMode", &MunbynPrinter::PrintPageMode),
            InstanceMethod("formFeed", &MunbynPrinter::FormFeed),
            InstanceMethod("cancelPageData", &MunbynPrinter::CancelPageData),
            InstanceMethod("setPageArea", &MunbynPrinter::SetPageArea),
            InstanceMethod("setAbsoluteVerticalPosition", &MunbynPrinter::SetAbsoluteVerticalPosition),
            InstanceMethod("setRelativeVerticalPosition", &MunbynPrinter::SetRelativeVerticalPosition),

            // Misc text / user-defined characters
            InstanceMethod("printAndFeedUnits", &MunbynPrinter::PrintAndFeedUnits),
            InstanceMethod("setPeripheralDevice", &MunbynPrinter::SetPeripheralDevice),
            InstanceMethod("selectUserDefinedCharset", &MunbynPrinter::SelectUserDefinedCharset),
            InstanceMethod("defineUserDefinedChars", &MunbynPrinter::DefineUserDefinedChars),
            InstanceMethod("cancelUserDefinedChar", &MunbynPrinter::CancelUserDefinedChar),

            // Status & real-time
            InstanceMethod("realtimeRequest", &MunbynPrinter::RealtimeRequest),
            InstanceMethod("realtimeDrawerPulse", &MunbynPrinter::RealtimeDrawerPulse),
            InstanceMethod("transmitStatus", &MunbynPrinter::TransmitStatus),
            InstanceMethod("setAsb", &MunbynPrinter::SetAsb),
            InstanceMethod("setPaperEndSensors", &MunbynPrinter::SetPaperEndSensors),
            InstanceMethod("setStopPrintSensors", &MunbynPrinter::SetStopPrintSensors),
            InstanceMethod("executeTestPrint", &MunbynPrinter::ExecuteTestPrint),

            // Mechanism / sound / macros
            InstanceMethod("setPanelButtons", &MunbynPrinter::SetPanelButtons),
            InstanceMethod("buzzer", &MunbynPrinter::Buzzer),
            InstanceMethod("buzzerAlarm", &MunbynPrinter::BuzzerAlarm),
            InstanceMethod("macroDefineToggle", &MunbynPrinter::MacroDefineToggle),
            InstanceMethod("executeMacro", &MunbynPrinter::ExecuteMacro),

            // Kanji
            InstanceMethod("setKanjiMode", &MunbynPrinter::SetKanjiMode),
            InstanceMethod("selectKanji", &MunbynPrinter::SelectKanji),
            InstanceMethod("cancelKanji", &MunbynPrinter::CancelKanji),
            InstanceMethod("setKanjiSpacing", &MunbynPrinter::SetKanjiSpacing),
            InstanceMethod("setKanjiQuadSize", &MunbynPrinter::SetKanjiQuadSize),

            // Network / WiFi (vendor)
            InstanceMethod("setWifi", &MunbynPrinter::SetWifi),
            InstanceMethod("setWifiStatic", &MunbynPrinter::SetWifiStatic),
            InstanceMethod("setDhcp", &MunbynPrinter::SetDhcp),

            // State
            InstanceAccessor("isOpen", &MunbynPrinter::IsOpen, nullptr),
        });

        Napi::FunctionReference* constructor = new Napi::FunctionReference();
        *constructor = Napi::Persistent(func);
        env.SetInstanceData(constructor);

        exports.Set("MunbynPrinter", func);
        return exports;
    }

    MunbynPrinter(const Napi::CallbackInfo& info)
        : Napi::ObjectWrap<MunbynPrinter>(info), handle_(nullptr) {}

    ~MunbynPrinter() {
        if (handle_) {
            munbyn_close(handle_);
            handle_ = nullptr;
        }
    }

private:
    munbyn_handle_t handle_;

    void EnsureOpen(Napi::Env env) {
        if (!handle_) {
            Napi::Error::New(env, "Printer not open").ThrowAsJavaScriptException();
        }
    }

    // --- Connection ---

    Napi::Value OpenUsb(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (handle_) {
            Napi::Error::New(env, "Printer already open").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        if (info.Length() < 1 || !info[0].IsString()) {
            Napi::TypeError::New(env, "Expected device path string").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        std::string path = info[0].As<Napi::String>().Utf8Value();
        if (path.find('\0') != std::string::npos) {
            Napi::TypeError::New(env, "String must not contain NUL").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        munbyn_error_t result = munbyn_open_usb(path.c_str(), &handle_);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value OpenSerial(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (handle_) {
            Napi::Error::New(env, "Printer already open").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        if (info.Length() < 2 || !info[0].IsString() || !info[1].IsNumber()) {
            Napi::TypeError::New(env, "Expected (portName: string, baudRate: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        std::string port = info[0].As<Napi::String>().Utf8Value();
        if (port.find('\0') != std::string::npos) {
            Napi::TypeError::New(env, "String must not contain NUL").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int baud = info[1].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_open_serial(port.c_str(), baud, &handle_);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value OpenNetwork(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (handle_) {
            Napi::Error::New(env, "Printer already open").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        if (info.Length() < 2 || !info[0].IsString() || !info[1].IsNumber()) {
            Napi::TypeError::New(env, "Expected (ip: string, port: number, timeoutMs?: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        std::string ip = info[0].As<Napi::String>().Utf8Value();
        if (ip.find('\0') != std::string::npos) {
            Napi::TypeError::New(env, "String must not contain NUL").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int port = info[1].As<Napi::Number>().Int32Value();
        int timeout = (info.Length() > 2 && info[2].IsNumber()) ? info[2].As<Napi::Number>().Int32Value() : 5000;
        munbyn_error_t result = munbyn_open_network(ip.c_str(), port, timeout, &handle_);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value Close(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (handle_) {
            munbyn_error_t result = munbyn_close(handle_);
            handle_ = nullptr;
            CHECK_RESULT(env, result);
        }
        return env.Undefined();
    }

    Napi::Value IsOpen(const Napi::CallbackInfo& info) {
        return Napi::Boolean::New(info.Env(), handle_ != nullptr);
    }

    // --- Core ---

    Napi::Value Initialize(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        munbyn_error_t result = munbyn_initialize(handle_);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value WriteData(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();

        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Expected Buffer or string").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        munbyn_error_t result;
        if (info[0].IsBuffer()) {
            Napi::Buffer<uint8_t> buf = info[0].As<Napi::Buffer<uint8_t>>();
            result = munbyn_write_data(handle_, buf.Data(), buf.Length());
        } else if (info[0].IsString()) {
            std::string str = info[0].As<Napi::String>().Utf8Value();
            result = munbyn_write_data(handle_, reinterpret_cast<const uint8_t*>(str.c_str()), str.length());
        } else {
            Napi::TypeError::New(env, "Expected Buffer or string").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value ReadData(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 0 && !CheckInteger(env, info[0], 1, 16777216)) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (length: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int length = info[0].As<Napi::Number>().Int32Value();
        if (length <= 0) {
            Napi::RangeError::New(env, "length must be > 0").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        std::vector<uint8_t> buf(static_cast<size_t>(length));
        size_t bytesRead = 0;
        munbyn_error_t result = munbyn_read_data(handle_, buf.data(), buf.size(), &bytesRead);
        CHECK_RESULT(env, result);
        return Napi::Buffer<uint8_t>::Copy(env, buf.data(), bytesRead);
    }

    Napi::Value GetStatus(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();

        munbyn_status_t status;
        munbyn_error_t result = munbyn_get_status(handle_, &status);
        CHECK_RESULT(env, result);

        Napi::Object obj = Napi::Object::New(env);
        obj.Set("paperPresent", Napi::Boolean::New(env, status.paper_present));
        obj.Set("coverClosed", Napi::Boolean::New(env, status.cover_closed));
        obj.Set("online", Napi::Boolean::New(env, status.online));
        obj.Set("errorOccurred", Napi::Boolean::New(env, status.error_occurred));
        obj.Set("cutError", Napi::Boolean::New(env, status.cut_error));
        obj.Set("recoverableError", Napi::Boolean::New(env, status.recoverable_error));
        obj.Set("unrecoverableError", Napi::Boolean::New(env, status.unrecoverable_error));
        return obj;
    }

    // --- Basic operations ---

    Napi::Value CutPaper(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        int mode = (info.Length() > 0 && info[0].IsNumber()) ? info[0].As<Napi::Number>().Int32Value() : MUNBYN_CUT_PARTIAL;
        munbyn_error_t result = munbyn_cut_paper(handle_, static_cast<munbyn_cut_mode_t>(mode));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value FeedAndCut(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        int amount = (info.Length() > 0 && info[0].IsNumber()) ? info[0].As<Napi::Number>().Int32Value() : 7;
        munbyn_error_t result = munbyn_feed_and_cut(handle_, static_cast<uint8_t>(amount));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value FeedLines(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (lines: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int lines = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_feed_lines(handle_, static_cast<uint8_t>(lines));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value PrintAndCut(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsString()) {
            Napi::TypeError::New(env, "Expected (text: string, cutMode?: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        std::string text = info[0].As<Napi::String>().Utf8Value();
        if (text.find('\0') != std::string::npos) {
            Napi::TypeError::New(env, "String must not contain NUL").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int mode = (info.Length() > 1 && info[1].IsNumber()) ? info[1].As<Napi::Number>().Int32Value() : MUNBYN_CUT_PARTIAL;
        munbyn_error_t result = munbyn_print_and_cut(handle_, text.c_str(), static_cast<munbyn_cut_mode_t>(mode));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value OpenDrawer(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 3 || !info[0].IsNumber() || !info[1].IsNumber() || !info[2].IsNumber()) {
            Napi::TypeError::New(env, "Expected (pin: number, onTime: number, offTime: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int pin = info[0].As<Napi::Number>().Int32Value();
        int on = info[1].As<Napi::Number>().Int32Value();
        int off = info[2].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_open_drawer(handle_, static_cast<munbyn_drawer_pin_t>(pin), static_cast<uint8_t>(on), static_cast<uint8_t>(off));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value OpenDrawerDefault(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        int pin = (info.Length() > 0 && info[0].IsNumber()) ? info[0].As<Napi::Number>().Int32Value() : MUNBYN_DRAWER_PIN_2;
        munbyn_error_t result = munbyn_open_drawer_default(handle_, static_cast<munbyn_drawer_pin_t>(pin));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SelfTest(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        munbyn_error_t result = munbyn_self_test(handle_);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    // --- Text control ---

    Napi::Value LineFeed(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        munbyn_error_t result = munbyn_line_feed(handle_);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value CarriageReturn(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        munbyn_error_t result = munbyn_carriage_return(handle_);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value HorizontalTab(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        munbyn_error_t result = munbyn_horizontal_tab(handle_);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetHorizontalTabPositions(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsArray()) {
            Napi::TypeError::New(env, "Expected (positions: number[])").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        Napi::Array arr = info[0].As<Napi::Array>();
        uint32_t len = arr.Length();
        if (len == 0 || len > 32) {
            Napi::RangeError::New(env, "Positions array must have 1-32 elements").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        uint8_t positions[32];
        for (uint32_t i = 0; i < len; i++) {
            positions[i] = static_cast<uint8_t>(arr.Get(i).As<Napi::Number>().Int32Value());
        }
        munbyn_error_t result = munbyn_set_horizontal_tab_positions(handle_, positions, len);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value ClearHorizontalTabPositions(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        munbyn_error_t result = munbyn_clear_horizontal_tab_positions(handle_);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    // --- Character set ---

    Napi::Value SetInternationalCharset(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (charset: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int charset = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_international_charset(handle_, static_cast<munbyn_international_charset_t>(charset));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetCodepage(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (codepage: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int codepage = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_codepage(handle_, static_cast<munbyn_codepage_t>(codepage));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value GetCodepageName(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (codepage: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int cp = info[0].As<Napi::Number>().Int32Value();
        const char* name = munbyn_get_codepage_name(static_cast<munbyn_codepage_t>(cp));
        return Napi::String::New(env, name ? name : "");
    }

    // --- Text formatting ---

    Napi::Value SetJustification(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (justify: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int justify = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_justification(handle_, static_cast<munbyn_justify_t>(justify));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetFont(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (font: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int font = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_font(handle_, static_cast<munbyn_font_t>(font));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetTextMode(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (modes: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int modes = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_text_mode(handle_, static_cast<uint8_t>(modes));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetEmphasis(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsBoolean()) {
            Napi::TypeError::New(env, "Expected (enabled: boolean)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        bool enabled = info[0].As<Napi::Boolean>().Value();
        munbyn_error_t result = munbyn_set_emphasis(handle_, enabled);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetDoubleStrike(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsBoolean()) {
            Napi::TypeError::New(env, "Expected (enabled: boolean)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        bool enabled = info[0].As<Napi::Boolean>().Value();
        munbyn_error_t result = munbyn_set_double_strike(handle_, enabled);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetUnderline(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (mode: number) — 0=off, 1=thin, 2=thick").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int mode = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_underline(handle_, static_cast<uint8_t>(mode));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetUnderlineKanji(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (mode: number) — 0=off, 1=thin, 2=thick").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int mode = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_underline_kanji(handle_, static_cast<uint8_t>(mode));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetLineSpacingDefault(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        munbyn_error_t result = munbyn_set_line_spacing_default(handle_);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetLineSpacing(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (spacing: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int spacing = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_line_spacing(handle_, static_cast<uint8_t>(spacing));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetMotionUnits(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
            Napi::TypeError::New(env, "Expected (horizontal: number, vertical: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int h = info[0].As<Napi::Number>().Int32Value();
        int v = info[1].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_motion_units(handle_, static_cast<uint8_t>(h), static_cast<uint8_t>(v));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetCharacterSpacing(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (spacing: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int spacing = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_character_spacing(handle_, static_cast<uint8_t>(spacing));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetLeftMargin(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (margin: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int margin = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_left_margin(handle_, static_cast<uint16_t>(margin));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetPrintAreaWidth(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (width: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int width = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_print_area_width(handle_, static_cast<uint16_t>(width));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    // --- Rotation / inversion ---

    Napi::Value SetRotate90(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsBoolean()) {
            Napi::TypeError::New(env, "Expected (enabled: boolean)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        munbyn_error_t result = munbyn_set_rotate_90(handle_, info[0].As<Napi::Boolean>().Value());
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetUpsideDown(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsBoolean()) {
            Napi::TypeError::New(env, "Expected (enabled: boolean)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        munbyn_error_t result = munbyn_set_upside_down(handle_, info[0].As<Napi::Boolean>().Value());
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    // --- Advanced text ---

    Napi::Value SetInvertedText(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsBoolean()) {
            Napi::TypeError::New(env, "Expected (enabled: boolean)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        munbyn_error_t result = munbyn_set_inverted_text(handle_, info[0].As<Napi::Boolean>().Value());
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetTextScale(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
            Napi::TypeError::New(env, "Expected (widthScale: number, heightScale: number) — 1-8").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int w = info[0].As<Napi::Number>().Int32Value();
        int h = info[1].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_text_scale(handle_, static_cast<uint8_t>(w), static_cast<uint8_t>(h));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value CancelAllFormatting(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        munbyn_error_t result = munbyn_cancel_all_formatting(handle_);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    // --- Print direction / position ---

    Napi::Value SetPrintDirection(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (direction: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int dir = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_print_direction(handle_, static_cast<uint8_t>(dir));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetRelativeHorizontalPosition(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (position: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int pos = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_relative_horizontal_position(handle_, static_cast<int16_t>(pos));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetAbsoluteHorizontalPosition(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (position: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int pos = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_absolute_horizontal_position(handle_, static_cast<uint16_t>(pos));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    // --- Barcode ---

    Napi::Value SetBarcodeHeight(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (height: number) — 1-255").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int height = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_barcode_height(handle_, static_cast<uint8_t>(height));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetBarcodeWidth(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (width: number) — 2-6").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int width = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_barcode_width(handle_, static_cast<uint8_t>(width));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetHriPosition(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (position: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int pos = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_hri_position(handle_, static_cast<munbyn_hri_position_t>(pos));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetHriFont(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (font: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int font = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_hri_font(handle_, static_cast<munbyn_hri_font_t>(font));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value PrintBarcode(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsString()) {
            Napi::TypeError::New(env, "Expected (type: number, data: string)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int type = info[0].As<Napi::Number>().Int32Value();
        std::string data = info[1].As<Napi::String>().Utf8Value();
        if (data.find('\0') != std::string::npos) {
            Napi::TypeError::New(env, "String must not contain NUL").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        munbyn_error_t result = munbyn_print_barcode(handle_, static_cast<munbyn_barcode_t>(type), data.c_str());
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    // --- 2D barcodes ---

    Napi::Value PrintQr(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 1 && !CheckInteger(env, info[1], 1, 16)) return env.Undefined();
        if (info.Length() > 2 && !CheckInteger(env, info[2], 48, 51)) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsString()) {
            Napi::TypeError::New(env, "Expected (data: string, moduleSize?: number, ecLevel?: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        std::string data = info[0].As<Napi::String>().Utf8Value();
        if (data.find('\0') != std::string::npos) {
            Napi::TypeError::New(env, "String must not contain NUL").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int moduleSize = (info.Length() > 1 && info[1].IsNumber()) ? info[1].As<Napi::Number>().Int32Value() : 6;
        int ecLevel = (info.Length() > 2 && info[2].IsNumber()) ? info[2].As<Napi::Number>().Int32Value() : MUNBYN_QR_EC_M;
        munbyn_error_t result = munbyn_print_qr(handle_, data.c_str(),
                                                static_cast<uint8_t>(moduleSize),
                                                static_cast<munbyn_qr_ec_t>(ecLevel));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value PrintPdf417(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 1 && !CheckInteger(env, info[1], 0, 30)) return env.Undefined();
        if (info.Length() > 2 && !CheckInteger(env, info[2], 0, 8)) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsString()) {
            Napi::TypeError::New(env, "Expected (data: string, columns?: number, ecLevel?: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        std::string data = info[0].As<Napi::String>().Utf8Value();
        if (data.find('\0') != std::string::npos) {
            Napi::TypeError::New(env, "String must not contain NUL").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int columns = (info.Length() > 1 && info[1].IsNumber()) ? info[1].As<Napi::Number>().Int32Value() : 0;
        int ecLevel = (info.Length() > 2 && info[2].IsNumber()) ? info[2].As<Napi::Number>().Int32Value() : 1;
        munbyn_error_t result = munbyn_print_pdf417(handle_, data.c_str(),
                                                    static_cast<uint8_t>(columns),
                                                    static_cast<uint8_t>(ecLevel));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    // --- Image ---

    Napi::Value PrintRasterImage(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 0 && !CheckInteger(env, info[0], 0, 51)) return env.Undefined();
        if (info.Length() > 2 && !CheckInteger(env, info[2], 1, 65535)) return env.Undefined();
        if (info.Length() > 3 && !CheckInteger(env, info[3], 1, 65535)) return env.Undefined();
        if (info.Length() < 4 || !info[0].IsNumber() || !info[1].IsBuffer() ||
            !info[2].IsNumber() || !info[3].IsNumber()) {
            Napi::TypeError::New(env, "Expected (mode: number, bitmap: Buffer, width: number, height: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int mode = info[0].As<Napi::Number>().Int32Value();
        Napi::Buffer<uint8_t> bitmap = info[1].As<Napi::Buffer<uint8_t>>();
        int width = info[2].As<Napi::Number>().Int32Value();
        int height = info[3].As<Napi::Number>().Int32Value();
        const size_t required = ((static_cast<size_t>(width) + 7) / 8) * static_cast<size_t>(height);
        if (bitmap.Length() != required) {
            Napi::RangeError::New(env, "Bitmap length does not match raster dimensions").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        munbyn_error_t result = munbyn_print_raster_image(
            handle_,
            static_cast<munbyn_image_mode_t>(mode),
            bitmap.Data(),
            static_cast<uint16_t>(width),
            static_cast<uint16_t>(height)
        );
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value PrintBitImage(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 0 && !CheckInteger(env, info[0], 0, 33)) return env.Undefined();
        if (info.Length() > 1 && !CheckInteger(env, info[1], 1, 1023)) return env.Undefined();
        if (info.Length() < 3 || !info[0].IsNumber() || !info[1].IsNumber() || !info[2].IsBuffer()) {
            Napi::TypeError::New(env, "Expected (mode: number, widthDots: number, data: Buffer)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int mode = info[0].As<Napi::Number>().Int32Value();
        int widthDots = info[1].As<Napi::Number>().Int32Value();
        Napi::Buffer<uint8_t> data = info[2].As<Napi::Buffer<uint8_t>>();
        munbyn_error_t result = munbyn_print_bit_image(handle_, static_cast<uint8_t>(mode),
                                                       static_cast<uint16_t>(widthDots),
                                                       data.Data(), data.Length());
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value DefineDownloadedBitImage(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 0 && !CheckInteger(env, info[0], 1, 255)) return env.Undefined();
        if (info.Length() > 1 && !CheckInteger(env, info[1], 1, 48)) return env.Undefined();
        if (info.Length() < 3 || !info[0].IsNumber() || !info[1].IsNumber() || !info[2].IsBuffer()) {
            Napi::TypeError::New(env, "Expected (x: number, y: number, data: Buffer)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int x = info[0].As<Napi::Number>().Int32Value();
        int y = info[1].As<Napi::Number>().Int32Value();
        Napi::Buffer<uint8_t> data = info[2].As<Napi::Buffer<uint8_t>>();
        munbyn_error_t result = munbyn_define_downloaded_bit_image(handle_, static_cast<uint8_t>(x),
                                                                   static_cast<uint8_t>(y),
                                                                   data.Data(), data.Length());
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value PrintDownloadedBitImage(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 0 && !CheckInteger(env, info[0], 0, 51)) return env.Undefined();
        int mode = (info.Length() > 0 && info[0].IsNumber()) ? info[0].As<Napi::Number>().Int32Value() : 0;
        munbyn_error_t result = munbyn_print_downloaded_bit_image(handle_, static_cast<uint8_t>(mode));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value PrintNvBitImage(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 0 && !CheckInteger(env, info[0], 1, 255)) return env.Undefined();
        if (info.Length() > 1 && !CheckInteger(env, info[1], 0, 51)) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (n: number, mode?: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int n = info[0].As<Napi::Number>().Int32Value();
        int mode = (info.Length() > 1 && info[1].IsNumber()) ? info[1].As<Napi::Number>().Int32Value() : 0;
        munbyn_error_t result = munbyn_print_nv_bit_image(handle_, static_cast<uint8_t>(n), static_cast<uint8_t>(mode));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value DefineNvBitImage(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 0 && !CheckInteger(env, info[0], 1, 255)) return env.Undefined();
        if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsBuffer()) {
            Napi::TypeError::New(env, "Expected (numImages: number, imageData: Buffer)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int numImages = info[0].As<Napi::Number>().Int32Value();
        Napi::Buffer<uint8_t> data = info[1].As<Napi::Buffer<uint8_t>>();
        munbyn_error_t result = munbyn_define_nv_bit_image(handle_, static_cast<uint8_t>(numImages),
                                                           data.Data(), data.Length());
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    // --- Page mode ---

    Napi::Value SelectPageMode(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        munbyn_error_t result = munbyn_select_page_mode(handle_);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SelectStandardMode(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        munbyn_error_t result = munbyn_select_standard_mode(handle_);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value PrintPageMode(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        munbyn_error_t result = munbyn_print_page_mode(handle_);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value FormFeed(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        munbyn_error_t result = munbyn_form_feed(handle_);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value CancelPageData(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        munbyn_error_t result = munbyn_cancel_page_data(handle_);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetPageArea(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 0 && !CheckInteger(env, info[0], 0, 65535)) return env.Undefined();
        if (info.Length() > 1 && !CheckInteger(env, info[1], 0, 65535)) return env.Undefined();
        if (info.Length() > 2 && !CheckInteger(env, info[2], 0, 65535)) return env.Undefined();
        if (info.Length() > 3 && !CheckInteger(env, info[3], 0, 65535)) return env.Undefined();
        if (info.Length() < 4 || !info[0].IsNumber() || !info[1].IsNumber() ||
            !info[2].IsNumber() || !info[3].IsNumber()) {
            Napi::TypeError::New(env, "Expected (x: number, y: number, dx: number, dy: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int x = info[0].As<Napi::Number>().Int32Value();
        int y = info[1].As<Napi::Number>().Int32Value();
        int dx = info[2].As<Napi::Number>().Int32Value();
        int dy = info[3].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_page_area(handle_, static_cast<uint16_t>(x), static_cast<uint16_t>(y),
                                                     static_cast<uint16_t>(dx), static_cast<uint16_t>(dy));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetAbsoluteVerticalPosition(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 0 && !CheckInteger(env, info[0], 0, 65535)) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (position: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int pos = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_absolute_vertical_position(handle_, static_cast<uint16_t>(pos));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetRelativeVerticalPosition(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 0 && !CheckInteger(env, info[0], -32768, 32767)) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (position: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int pos = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_relative_vertical_position(handle_, static_cast<int16_t>(pos));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    // --- Misc text / user-defined characters ---

    Napi::Value PrintAndFeedUnits(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (units: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int units = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_print_and_feed_units(handle_, static_cast<uint8_t>(units));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetPeripheralDevice(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (n: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int n = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_peripheral_device(handle_, static_cast<uint8_t>(n));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SelectUserDefinedCharset(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsBoolean()) {
            Napi::TypeError::New(env, "Expected (enabled: boolean)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        bool enabled = info[0].As<Napi::Boolean>().Value();
        munbyn_error_t result = munbyn_select_user_defined_charset(handle_, enabled);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value DefineUserDefinedChars(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 0 && !CheckInteger(env, info[0], 3, 3)) return env.Undefined();
        if (info.Length() > 1 && !CheckInteger(env, info[1], 32, 126)) return env.Undefined();
        if (info.Length() > 2 && !CheckInteger(env, info[2], 32, 126)) return env.Undefined();
        if (info.Length() < 4 || !info[0].IsNumber() || !info[1].IsNumber() ||
            !info[2].IsNumber() || !info[3].IsBuffer()) {
            Napi::TypeError::New(env, "Expected (y: number, c1: number, c2: number, data: Buffer)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int y = info[0].As<Napi::Number>().Int32Value();
        int c1 = info[1].As<Napi::Number>().Int32Value();
        int c2 = info[2].As<Napi::Number>().Int32Value();
        Napi::Buffer<uint8_t> data = info[3].As<Napi::Buffer<uint8_t>>();
        munbyn_error_t result = munbyn_define_user_defined_chars(handle_, static_cast<uint8_t>(y),
                                                                 static_cast<uint8_t>(c1), static_cast<uint8_t>(c2),
                                                                 data.Data(), data.Length());
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value CancelUserDefinedChar(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 0 && !CheckInteger(env, info[0], 32, 126)) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (code: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int code = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_cancel_user_defined_char(handle_, static_cast<uint8_t>(code));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    // --- Status & real-time ---

    Napi::Value RealtimeRequest(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 0 && !CheckInteger(env, info[0], 1, 2)) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (n: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int n = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_realtime_request(handle_, static_cast<uint8_t>(n));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value RealtimeDrawerPulse(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 0 && !CheckInteger(env, info[0], 0, 1)) return env.Undefined();
        if (info.Length() > 1 && !CheckInteger(env, info[1], 1, 8)) return env.Undefined();
        int pin = (info.Length() > 0 && info[0].IsNumber()) ? info[0].As<Napi::Number>().Int32Value() : 0;
        int onTime = (info.Length() > 1 && info[1].IsNumber()) ? info[1].As<Napi::Number>().Int32Value() : 1;
        munbyn_error_t result = munbyn_realtime_drawer_pulse(handle_, static_cast<uint8_t>(pin), static_cast<uint8_t>(onTime));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value TransmitStatus(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 0 && !CheckInteger(env, info[0], 1, 50)) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (n: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int n = info[0].As<Napi::Number>().Int32Value();
        uint8_t out = 0;
        munbyn_error_t result = munbyn_transmit_status(handle_, static_cast<uint8_t>(n), &out);
        CHECK_RESULT(env, result);
        return Napi::Number::New(env, out);
    }

    Napi::Value SetAsb(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (n: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int n = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_asb(handle_, static_cast<uint8_t>(n));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetPaperEndSensors(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (n: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int n = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_paper_end_sensors(handle_, static_cast<uint8_t>(n));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetStopPrintSensors(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (n: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int n = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_stop_print_sensors(handle_, static_cast<uint8_t>(n));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value ExecuteTestPrint(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 0 && !CheckInteger(env, info[0], 0, 48)) return env.Undefined();
        if (info.Length() > 1 && !CheckInteger(env, info[1], 1, 49)) return env.Undefined();
        int n = (info.Length() > 0 && info[0].IsNumber()) ? info[0].As<Napi::Number>().Int32Value() : 0;
        int m = (info.Length() > 1 && info[1].IsNumber()) ? info[1].As<Napi::Number>().Int32Value() : 1;
        munbyn_error_t result = munbyn_execute_test_print(handle_, static_cast<uint8_t>(n), static_cast<uint8_t>(m));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    // --- Mechanism / sound / macros ---

    Napi::Value SetPanelButtons(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsBoolean()) {
            Napi::TypeError::New(env, "Expected (enabled: boolean)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        bool enabled = info[0].As<Napi::Boolean>().Value();
        munbyn_error_t result = munbyn_set_panel_buttons(handle_, enabled);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value Buzzer(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 0 && !CheckInteger(env, info[0], 1, 9)) return env.Undefined();
        if (info.Length() > 1 && !CheckInteger(env, info[1], 1, 9)) return env.Undefined();
        int count = (info.Length() > 0 && info[0].IsNumber()) ? info[0].As<Napi::Number>().Int32Value() : 1;
        int duration = (info.Length() > 1 && info[1].IsNumber()) ? info[1].As<Napi::Number>().Int32Value() : 2;
        munbyn_error_t result = munbyn_buzzer(handle_, static_cast<uint8_t>(count), static_cast<uint8_t>(duration));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value BuzzerAlarm(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 0 && !CheckInteger(env, info[0], 1, 20)) return env.Undefined();
        if (info.Length() > 1 && !CheckInteger(env, info[1], 1, 20)) return env.Undefined();
        if (info.Length() > 2 && !CheckInteger(env, info[2], 0, 3)) return env.Undefined();
        int count = (info.Length() > 0 && info[0].IsNumber()) ? info[0].As<Napi::Number>().Int32Value() : 1;
        int interval = (info.Length() > 1 && info[1].IsNumber()) ? info[1].As<Napi::Number>().Int32Value() : 2;
        int mode = (info.Length() > 2 && info[2].IsNumber()) ? info[2].As<Napi::Number>().Int32Value() : 3;
        munbyn_error_t result = munbyn_buzzer_alarm(handle_, static_cast<uint8_t>(count),
                                                    static_cast<uint8_t>(interval), static_cast<uint8_t>(mode));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value MacroDefineToggle(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        munbyn_error_t result = munbyn_macro_define_toggle(handle_);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value ExecuteMacro(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 0 && !CheckInteger(env, info[0], 0, 255)) return env.Undefined();
        if (info.Length() > 1 && !CheckInteger(env, info[1], 0, 255)) return env.Undefined();
        if (info.Length() > 2 && !CheckInteger(env, info[2], 0, 1)) return env.Undefined();
        int times = (info.Length() > 0 && info[0].IsNumber()) ? info[0].As<Napi::Number>().Int32Value() : 1;
        int wait = (info.Length() > 1 && info[1].IsNumber()) ? info[1].As<Napi::Number>().Int32Value() : 0;
        int mode = (info.Length() > 2 && info[2].IsNumber()) ? info[2].As<Napi::Number>().Int32Value() : 0;
        munbyn_error_t result = munbyn_execute_macro(handle_, static_cast<uint8_t>(times),
                                                     static_cast<uint8_t>(wait), static_cast<uint8_t>(mode));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    // --- Kanji ---

    Napi::Value SetKanjiMode(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected (modes: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int modes = info[0].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_kanji_mode(handle_, static_cast<uint8_t>(modes));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SelectKanji(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        munbyn_error_t result = munbyn_select_kanji(handle_);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value CancelKanji(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        munbyn_error_t result = munbyn_cancel_kanji(handle_);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetKanjiSpacing(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
            Napi::TypeError::New(env, "Expected (left: number, right: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int left = info[0].As<Napi::Number>().Int32Value();
        int right = info[1].As<Napi::Number>().Int32Value();
        munbyn_error_t result = munbyn_set_kanji_spacing(handle_, static_cast<uint8_t>(left), static_cast<uint8_t>(right));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetKanjiQuadSize(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsBoolean()) {
            Napi::TypeError::New(env, "Expected (enabled: boolean)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        bool enabled = info[0].As<Napi::Boolean>().Value();
        munbyn_error_t result = munbyn_set_kanji_quad_size(handle_, enabled);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    // --- Network / WiFi (vendor) ---

    Napi::Value SetWifi(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 2 && !CheckInteger(env, info[2], 0, 8)) return env.Undefined();
        if (info.Length() < 2 || !info[0].IsString() || !info[1].IsString()) {
            Napi::TypeError::New(env, "Expected (ssid: string, password: string, keyType?: number)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        std::string ssid = info[0].As<Napi::String>().Utf8Value();
        if (ssid.find('\0') != std::string::npos) {
            Napi::TypeError::New(env, "String must not contain NUL").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        std::string password = info[1].As<Napi::String>().Utf8Value();
        if (password.find('\0') != std::string::npos) {
            Napi::TypeError::New(env, "String must not contain NUL").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int keyType = (info.Length() > 2 && info[2].IsNumber()) ? info[2].As<Napi::Number>().Int32Value() : MUNBYN_WIFI_WPA_WPA2_MIXED;
        munbyn_error_t result = munbyn_set_wifi(handle_, ssid.c_str(), password.c_str(),
                                                static_cast<munbyn_wifi_keytype_t>(keyType));
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetWifiStatic(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() > 2 && !CheckInteger(env, info[2], 0, 8)) return env.Undefined();
        if (info.Length() < 6 || !info[0].IsString() || !info[1].IsString() || !info[2].IsNumber() ||
            !info[3].IsArray() || !info[4].IsArray() || !info[5].IsArray()) {
            Napi::TypeError::New(env, "Expected (ssid, password, keyType, ip[4], mask[4], gateway[4])").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        std::string ssid = info[0].As<Napi::String>().Utf8Value();
        if (ssid.find('\0') != std::string::npos) {
            Napi::TypeError::New(env, "String must not contain NUL").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        std::string password = info[1].As<Napi::String>().Utf8Value();
        if (password.find('\0') != std::string::npos) {
            Napi::TypeError::New(env, "String must not contain NUL").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        int keyType = info[2].As<Napi::Number>().Int32Value();
        uint8_t ip[4], mask[4], gateway[4];
        Napi::Array arrs[3] = {info[3].As<Napi::Array>(), info[4].As<Napi::Array>(), info[5].As<Napi::Array>()};
        uint8_t* outs[3] = {ip, mask, gateway};
        for (int a = 0; a < 3; a++) {
            if (arrs[a].Length() != 4) {
                Napi::RangeError::New(env, "ip/mask/gateway must each have 4 octets").ThrowAsJavaScriptException();
                return env.Undefined();
            }
            for (uint32_t j = 0; j < 4; j++) {
                if (!CheckInteger(env, arrs[a].Get(j), 0, 255)) return env.Undefined();
                outs[a][j] = static_cast<uint8_t>(arrs[a].Get(j).As<Napi::Number>().Int32Value());
            }
        }
        munbyn_error_t result = munbyn_set_wifi_static(handle_, ssid.c_str(), password.c_str(),
                                                       static_cast<munbyn_wifi_keytype_t>(keyType),
                                                       ip, mask, gateway);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }

    Napi::Value SetDhcp(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        EnsureOpen(env);
        if (env.IsExceptionPending()) return env.Undefined();
        if (info.Length() < 1 || !info[0].IsBoolean()) {
            Napi::TypeError::New(env, "Expected (enabled: boolean)").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        bool enabled = info[0].As<Napi::Boolean>().Value();
        munbyn_error_t result = munbyn_set_dhcp(handle_, enabled);
        CHECK_RESULT(env, result);
        return env.Undefined();
    }
};

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    MunbynPrinter::Init(env, exports);

    // Export enum constants for convenience
    Napi::Object constants = Napi::Object::New(env);

    // Justification
    constants.Set("JUSTIFY_LEFT", Napi::Number::New(env, MUNBYN_JUSTIFY_LEFT));
    constants.Set("JUSTIFY_CENTER", Napi::Number::New(env, MUNBYN_JUSTIFY_CENTER));
    constants.Set("JUSTIFY_RIGHT", Napi::Number::New(env, MUNBYN_JUSTIFY_RIGHT));

    // Fonts
    constants.Set("FONT_A", Napi::Number::New(env, MUNBYN_FONT_A));
    constants.Set("FONT_B", Napi::Number::New(env, MUNBYN_FONT_B));

    // Print modes
    constants.Set("MODE_NORMAL", Napi::Number::New(env, MUNBYN_MODE_NORMAL));
    constants.Set("MODE_EMPHASIZED", Napi::Number::New(env, MUNBYN_MODE_EMPHASIZED));
    constants.Set("MODE_DOUBLE_HEIGHT", Napi::Number::New(env, MUNBYN_MODE_DOUBLE_HEIGHT));
    constants.Set("MODE_DOUBLE_WIDTH", Napi::Number::New(env, MUNBYN_MODE_DOUBLE_WIDTH));
    constants.Set("MODE_UNDERLINE", Napi::Number::New(env, MUNBYN_MODE_UNDERLINE));

    // Cut modes
    constants.Set("CUT_ONE_POINT_UNCUT", Napi::Number::New(env, MUNBYN_CUT_ONE_POINT_UNCUT));
    constants.Set("CUT_PARTIAL", Napi::Number::New(env, MUNBYN_CUT_PARTIAL));

    // Drawer pins
    constants.Set("DRAWER_PIN_2", Napi::Number::New(env, MUNBYN_DRAWER_PIN_2));
    constants.Set("DRAWER_PIN_5", Napi::Number::New(env, MUNBYN_DRAWER_PIN_5));

    // Barcode types
    constants.Set("BARCODE_UPC_A", Napi::Number::New(env, MUNBYN_BARCODE_UPC_A));
    constants.Set("BARCODE_UPC_E", Napi::Number::New(env, MUNBYN_BARCODE_UPC_E));
    constants.Set("BARCODE_JAN13", Napi::Number::New(env, MUNBYN_BARCODE_JAN13));
    constants.Set("BARCODE_JAN8", Napi::Number::New(env, MUNBYN_BARCODE_JAN8));
    constants.Set("BARCODE_CODE39", Napi::Number::New(env, MUNBYN_BARCODE_CODE39));
    constants.Set("BARCODE_ITF", Napi::Number::New(env, MUNBYN_BARCODE_ITF));
    constants.Set("BARCODE_CODEBAR", Napi::Number::New(env, MUNBYN_BARCODE_CODEBAR));
    constants.Set("BARCODE_CODE93", Napi::Number::New(env, MUNBYN_BARCODE_CODE93));
    constants.Set("BARCODE_CODE128", Napi::Number::New(env, MUNBYN_BARCODE_CODE128));
    constants.Set("BARCODE_GS1_128", Napi::Number::New(env, MUNBYN_BARCODE_GS1_128));
    constants.Set("BARCODE_GS1_DATABAR_OMNI", Napi::Number::New(env, MUNBYN_BARCODE_GS1_DATABAR_OMNI));
    constants.Set("BARCODE_GS1_DATABAR_TRUNCATED", Napi::Number::New(env, MUNBYN_BARCODE_GS1_DATABAR_TRUNCATED));
    constants.Set("BARCODE_GS1_DATABAR_LIMITED", Napi::Number::New(env, MUNBYN_BARCODE_GS1_DATABAR_LIMITED));
    constants.Set("BARCODE_GS1_DATABAR_EXPANDED", Napi::Number::New(env, MUNBYN_BARCODE_GS1_DATABAR_EXPANDED));

    // HRI position
    constants.Set("HRI_NONE", Napi::Number::New(env, MUNBYN_HRI_NONE));
    constants.Set("HRI_ABOVE", Napi::Number::New(env, MUNBYN_HRI_ABOVE));
    constants.Set("HRI_BELOW", Napi::Number::New(env, MUNBYN_HRI_BELOW));
    constants.Set("HRI_BOTH", Napi::Number::New(env, MUNBYN_HRI_BOTH));

    // HRI font
    constants.Set("HRI_FONT_STANDARD", Napi::Number::New(env, MUNBYN_HRI_FONT_STANDARD));
    constants.Set("HRI_FONT_COMPRESSED", Napi::Number::New(env, MUNBYN_HRI_FONT_COMPRESSED));

    // WiFi key types
    constants.Set("WIFI_WEP64", Napi::Number::New(env, MUNBYN_WIFI_WEP64));
    constants.Set("WIFI_WEP128", Napi::Number::New(env, MUNBYN_WIFI_WEP128));
    constants.Set("WIFI_WPA_AES_PSK", Napi::Number::New(env, MUNBYN_WIFI_WPA_AES_PSK));
    constants.Set("WIFI_WPA_TKIP_PSK", Napi::Number::New(env, MUNBYN_WIFI_WPA_TKIP_PSK));
    constants.Set("WIFI_WPA_TKIP_AES_PSK", Napi::Number::New(env, MUNBYN_WIFI_WPA_TKIP_AES_PSK));
    constants.Set("WIFI_WPA2_AES_PSK", Napi::Number::New(env, MUNBYN_WIFI_WPA2_AES_PSK));
    constants.Set("WIFI_WPA2_TKIP", Napi::Number::New(env, MUNBYN_WIFI_WPA2_TKIP));
    constants.Set("WIFI_WPA2_TKIP_AES_PSK", Napi::Number::New(env, MUNBYN_WIFI_WPA2_TKIP_AES_PSK));
    constants.Set("WIFI_WPA_WPA2_MIXED", Napi::Number::New(env, MUNBYN_WIFI_WPA_WPA2_MIXED));

    // QR error-correction levels
    constants.Set("QR_EC_L", Napi::Number::New(env, MUNBYN_QR_EC_L));
    constants.Set("QR_EC_M", Napi::Number::New(env, MUNBYN_QR_EC_M));
    constants.Set("QR_EC_Q", Napi::Number::New(env, MUNBYN_QR_EC_Q));
    constants.Set("QR_EC_H", Napi::Number::New(env, MUNBYN_QR_EC_H));

    // Image modes
    constants.Set("IMAGE_NORMAL", Napi::Number::New(env, MUNBYN_IMAGE_NORMAL));
    constants.Set("IMAGE_DOUBLE_WIDTH", Napi::Number::New(env, MUNBYN_IMAGE_DOUBLE_WIDTH));
    constants.Set("IMAGE_DOUBLE_HEIGHT", Napi::Number::New(env, MUNBYN_IMAGE_DOUBLE_HEIGHT));
    constants.Set("IMAGE_QUADRUPLE", Napi::Number::New(env, MUNBYN_IMAGE_QUADRUPLE));

    exports.Set("constants", constants);

    return exports;
}

NODE_API_MODULE(munbync, InitAll)
