#include "data/port-types/port-serial.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "module/module-serial.hpp"
#include <format>
#include <unordered_map>
#include <array>

#if defined(ADAM_PLATFORM_WINDOWS)
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <poll.h>
#include <cerrno>
#include <cstring>
#endif

namespace adam::modules::serial
{
    const configuration_parameter_list& port_serial::get_user_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;

            auto up = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
            
            auto path_param = std::make_unique<adam::configuration_parameter_string>("path"_ct);
            path_param->set_description(language_english, "The COM port or device path (e.g. COM1 or /dev/ttyUSB0)."_ct);
            path_param->set_description(language_german, "Der COM-Port oder Gerätepfad (z.B. COM1 oder /dev/ttyUSB0)."_ct);
            up->add(std::move(path_param));

            configuration_parameter_integer::presets_container baud_presets = {};
            
            baud_presets.emplace(300);
            baud_presets.emplace(600);
            baud_presets.emplace(1200);
            baud_presets.emplace(1800);
            baud_presets.emplace(2400);
            baud_presets.emplace(4800);
            baud_presets.emplace(7200);
            baud_presets.emplace(9600);
            baud_presets.emplace(19200);
            baud_presets.emplace(38400);
            baud_presets.emplace(57600);
            baud_presets.emplace(115200);
            baud_presets.emplace(230400);
            baud_presets.emplace(460800);
            baud_presets.emplace(921600);

            auto baud_rate_param = std::make_unique<adam::configuration_parameter_integer>("baud_rate"_ct, 115200, std::move(baud_presets));
            baud_rate_param->set_description(language_english, "The communication speed in baud (bits per second)."_ct);
            baud_rate_param->set_description(language_german, "Die Kommunikationsgeschwindigkeit in Baud (Bits pro Sekunde)."_ct);
            up->add(std::move(baud_rate_param));
            
            configuration_parameter_integer::presets_container data_bits_presets = {};
            
            data_bits_presets.emplace(5);
            data_bits_presets.emplace(6);
            data_bits_presets.emplace(7);
            data_bits_presets.emplace(8);

            auto data_bits_param = std::make_unique<adam::configuration_parameter_integer>("data_bits"_ct, 8, std::move(data_bits_presets));
            data_bits_param->set_description(language_english, "The number of data bits per byte."_ct);
            data_bits_param->set_description(language_german, "Die Anzahl der Datenbits pro Byte."_ct);
            up->add(std::move(data_bits_param));

            configuration_parameter_integer::presets_container stop_bits_presets = {};
            
            stop_bits_presets.emplace(1);
            stop_bits_presets.emplace(2);

            auto stop_bits_param = std::make_unique<adam::configuration_parameter_integer>("stop_bits"_ct, 1, std::move(stop_bits_presets));
            stop_bits_param->set_description(language_english, "The number of stop bits indicating the end of a byte."_ct);
            stop_bits_param->set_description(language_german, "Die Anzahl der Stoppbits, die das Ende eines Bytes markieren."_ct);
            up->add(std::move(stop_bits_param));

            configuration_parameter_string::presets_container parity_presets = {};
            
            parity_presets.emplace("0"_ct, std::make_unique<adam::configuration_parameter_string>("none"_ct, "none"_ct));
            parity_presets.emplace("1"_ct, std::make_unique<adam::configuration_parameter_string>("odd"_ct, "odd"_ct));
            parity_presets.emplace("2"_ct, std::make_unique<adam::configuration_parameter_string>("even"_ct, "even"_ct));
            parity_presets.emplace("3"_ct, std::make_unique<adam::configuration_parameter_string>("mark"_ct, "mark"_ct));
            parity_presets.emplace("4"_ct, std::make_unique<adam::configuration_parameter_string>("space"_ct, "space"_ct));

            auto parity_param = std::make_unique<adam::configuration_parameter_string>("parity"_ct, "none"_ct, std::move(parity_presets));
            parity_param->set_description(language_english, "The parity bit configuration."_ct);
            parity_param->set_description(language_german, "Die Konfiguration des Paritätsbits."_ct);
            up->add(std::move(parity_param));

            configuration_parameter_string::presets_container flow_ctrl_presets = {};
            
            flow_ctrl_presets.emplace("0"_ct, std::make_unique<adam::configuration_parameter_string>("none"_ct, "none"_ct));
            flow_ctrl_presets.emplace("1"_ct, std::make_unique<adam::configuration_parameter_string>("hardware"_ct, "hardware"_ct));
            flow_ctrl_presets.emplace("2"_ct, std::make_unique<adam::configuration_parameter_string>("xon_xoff"_ct, "xon_xoff"_ct));

            auto flow_ctrl_param = std::make_unique<adam::configuration_parameter_string>("flow_ctrl"_ct, "none"_ct, std::move(flow_ctrl_presets));
            flow_ctrl_param->set_description(language_english, "The hardware or software flow control."_ct);
            flow_ctrl_param->set_description(language_german, "Die Hardware- oder Software-Flusssteuerung."_ct);
            up->add(std::move(flow_ctrl_param));

            auto rit_param = std::make_unique<adam::configuration_parameter_integer>("read_interval_timeout"_ct, 1);
            rit_param->set_description(language_english, "Maximum time between bytes before returning (-1 = immediate)."_ct);
            rit_param->set_description(language_german, "Maximale Zeit zwischen Bytes vor der Rückgabe (-1 = sofort)."_ct);
            up->add(std::move(rit_param));

            auto rttc_param = std::make_unique<adam::configuration_parameter_integer>("read_total_timeout_constant"_ct, 50);
            rttc_param->set_description(language_english, "Constant timeout to wait for data (ms)."_ct);
            rttc_param->set_description(language_german, "Konstantes Timeout zum Warten auf Daten (ms)."_ct);
            up->add(std::move(rttc_param));

            auto rttm_param = std::make_unique<adam::configuration_parameter_integer>("read_total_timeout_multiplier"_ct, 10);
            rttm_param->set_description(language_english, "Multiplier timeout per expected byte (ms)."_ct);
            rttm_param->set_description(language_german, "Multiplikator-Timeout pro erwartetem Byte (ms)."_ct);
            up->add(std::move(rttm_param));

            auto wttc_param = std::make_unique<adam::configuration_parameter_integer>("write_total_timeout_constant"_ct, 50);
            wttc_param->set_description(language_english, "Constant timeout to wait for writes (ms) (Not used on POSIX)."_ct);
            wttc_param->set_description(language_german, "Konstantes Timeout zum Warten auf Schreibvorgänge (ms) (Unter POSIX nicht verwendet)."_ct);
            up->add(std::move(wttc_param));

            auto wttm_param = std::make_unique<adam::configuration_parameter_integer>("write_total_timeout_multiplier"_ct, 10);
            wttm_param->set_description(language_english, "Multiplier timeout per written byte (ms) (Not used on POSIX)."_ct);
            wttm_param->set_description(language_german, "Multiplikator-Timeout pro geschriebenem Byte (ms) (Unter POSIX nicht verwendet)."_ct);
            up->add(std::move(wttm_param));

            p.add(std::move(up));
            
            return p;
        }();
        return params;
    }

    port_serial::port_serial(const string_hashed& item_name) 
     :  port_in_out(item_name)
        #if defined(ADAM_PLATFORM_WINDOWS)
        , m_handle(INVALID_HANDLE_VALUE)
        #else
        , m_fd(-1)
        #endif
    {
        get_parameter<adam::configuration_parameter_string>("type"_ct)->set_value(type_name());
        get_parameter<adam::configuration_parameter_string>("type_origin_module"_ct)->set_value(get_adam_module()->get_name());

        add_parameters(port_serial::get_user_parameters());

        auto user_params = get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
        m_rttc = user_params->get<adam::configuration_parameter_integer>("read_total_timeout_constant"_ct);
        m_rttm = user_params->get<adam::configuration_parameter_integer>("read_total_timeout_multiplier"_ct);
        m_rit = user_params->get<adam::configuration_parameter_integer>("read_interval_timeout"_ct);
    }

    port_serial::~port_serial() 
    {
        stop();
    }

    bool port_serial::start()
    {
        set_state(state_starting);
        auto user_params = get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct);

        auto path = user_params->get<adam::configuration_parameter_string>("path"_ct)->get_value();
        
        if (path.empty())
        {
            log_event_msg(log::error, log_event::path_empty);
            return false;
        }

        int baud_rate = static_cast<int>(user_params->get<adam::configuration_parameter_integer>("baud_rate"_ct)->get_value());
        int data_bits = static_cast<int>(user_params->get<adam::configuration_parameter_integer>("data_bits"_ct)->get_value());
        int stop_bits = static_cast<int>(user_params->get<adam::configuration_parameter_integer>("stop_bits"_ct)->get_value());
        const auto& parity = user_params->get<adam::configuration_parameter_string>("parity"_ct)->get_value();
        const auto& flow_ctrl = user_params->get<adam::configuration_parameter_string>("flow_ctrl"_ct)->get_value();

        #if defined(ADAM_PLATFORM_WINDOWS)
        int64_t rit = user_params->get<adam::configuration_parameter_integer>("read_interval_timeout"_ct)->get_value();
        int64_t rttc = user_params->get<adam::configuration_parameter_integer>("read_total_timeout_constant"_ct)->get_value();
        int64_t rttm = user_params->get<adam::configuration_parameter_integer>("read_total_timeout_multiplier"_ct)->get_value();
        int64_t wttc = user_params->get<adam::configuration_parameter_integer>("write_total_timeout_constant"_ct)->get_value();
        int64_t wttm = user_params->get<adam::configuration_parameter_integer>("write_total_timeout_multiplier"_ct)->get_value();
        m_handle = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (m_handle == INVALID_HANDLE_VALUE)
        {
            DWORD err = ::GetLastError();
            language lang = get_controller()->get_language();
            std::string_view err_txt = get_error_log_text(resolve_serial_error(err), lang);
            std::string err_desc = std::format("{} ({})", err_txt, err);
            log_event_msg(log::error, log_event::open_failed, path, err_desc);
            return false;
        }

        DCB dcbSerialParams = {0};
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
        if (GetCommState(m_handle, &dcbSerialParams)) 
        {
            dcbSerialParams.BaudRate = baud_rate;
            dcbSerialParams.ByteSize = static_cast<BYTE>(data_bits);
            dcbSerialParams.StopBits = stop_bits == 2 ? TWOSTOPBITS : ONESTOPBIT;
            
            if (parity == "odd"_ct) dcbSerialParams.Parity = ODDPARITY;
            else if (parity == "even"_ct) dcbSerialParams.Parity = EVENPARITY;
            else if (parity == "mark"_ct) dcbSerialParams.Parity = MARKPARITY;
            else if (parity == "space"_ct) dcbSerialParams.Parity = SPACEPARITY;
            else dcbSerialParams.Parity = NOPARITY;

            if (flow_ctrl == "hardware"_ct)
            {
                dcbSerialParams.fOutxCtsFlow = TRUE;
                dcbSerialParams.fRtsControl = RTS_CONTROL_HANDSHAKE;
                dcbSerialParams.fInX = FALSE;
                dcbSerialParams.fOutX = FALSE;
            }
            else if (flow_ctrl == "xon_xoff"_ct)
            {
                dcbSerialParams.fOutxCtsFlow = FALSE;
                dcbSerialParams.fRtsControl = RTS_CONTROL_ENABLE;
                dcbSerialParams.fInX = TRUE;
                dcbSerialParams.fOutX = TRUE;
            }
            else // none
            {
                dcbSerialParams.fOutxCtsFlow = FALSE;
                dcbSerialParams.fRtsControl = RTS_CONTROL_ENABLE;
                dcbSerialParams.fInX = FALSE;
                dcbSerialParams.fOutX = FALSE;
            }

            SetCommState(m_handle, &dcbSerialParams);
        }

        COMMTIMEOUTS timeouts = {0};
        // We treat -1 as MAXDWORD to allow users an intuitive way to define infinite or immediate timeouts
        timeouts.ReadIntervalTimeout = (rit < 0) ? MAXDWORD : static_cast<DWORD>(rit);
        timeouts.ReadTotalTimeoutConstant = (rttc < 0) ? MAXDWORD : static_cast<DWORD>(rttc);
        timeouts.ReadTotalTimeoutMultiplier = (rttm < 0) ? MAXDWORD : static_cast<DWORD>(rttm);
        timeouts.WriteTotalTimeoutConstant = (wttc < 0) ? MAXDWORD : static_cast<DWORD>(wttc);
        timeouts.WriteTotalTimeoutMultiplier = (wttm < 0) ? MAXDWORD : static_cast<DWORD>(wttm);
        SetCommTimeouts(m_handle, &timeouts);

        // Flush any old data in the kernel buffer
        PurgeComm(m_handle, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
        
        log_event_msg(log::info, log_event::open_success, path, std::to_string(baud_rate));
        #else
        m_fd = ::open(path.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
        if (m_fd < 0)
        {
            int err = errno;
            language lang = get_controller()->get_language();
            std::string_view err_txt = get_error_log_text(resolve_serial_error(err), lang);
            std::string err_desc = std::format("{} ({})", err_txt, err);
            log_event_msg(log::error, log_event::open_failed, path, err_desc);
            return false;
        }

        struct termios tty;
        if (tcgetattr(m_fd, &tty) == 0) 
        {
            speed_t speed = B115200;
            switch (baud_rate) 
            {
                case 300:    speed = B300;    break;
                case 600:    speed = B600;    break;
                case 1200:   speed = B1200;   break;
                case 1800:   speed = B1800;   break;
                case 2400:   speed = B2400;   break;
                case 4800:   speed = B4800;   break;
                case 7200:
                #ifdef B7200
                             speed = B7200;   break;
                #else
                             speed = B9600;   break;
                #endif
                case 9600:   speed = B9600;   break;
                case 19200:  speed = B19200;  break;
                case 38400:  speed = B38400;  break;
                case 57600:  speed = B57600;  break;
                case 115200: speed = B115200; break;
                case 230400:
                #ifdef B230400
                             speed = B230400; break;
                #else
                             speed = B115200; break;
                #endif
                case 460800:
                #ifdef B460800
                             speed = B460800; break;
                #else
                             speed = B115200; break;
                #endif
                case 921600:
                #ifdef B921600
                             speed = B921600; break;
                #else
                             speed = B115200; break;
                #endif
                default:     speed = B115200; break;
            }
            cfsetospeed(&tty, speed);
            cfsetispeed(&tty, speed);

            tty.c_cflag &= ~CSIZE;
            switch (data_bits)
            {
                case 5: tty.c_cflag |= CS5; break;
                case 6: tty.c_cflag |= CS6; break;
                case 7: tty.c_cflag |= CS7; break;
                case 8: tty.c_cflag |= CS8; break;
                default: tty.c_cflag |= CS8; break;
            }

            if (parity == "odd"_ct)
            {
                tty.c_cflag |= PARENB;
                tty.c_cflag |= PARODD;
            }
            else if (parity == "even"_ct)
            {
                tty.c_cflag |= PARENB;
                tty.c_cflag &= ~PARODD;
            }
            else if (parity == "mark"_ct)
            {
                tty.c_cflag |= PARENB;
                tty.c_cflag |= PARODD;
                #ifdef CMSPAR
                tty.c_cflag |= CMSPAR;
                #endif
            }
            else if (parity == "space"_ct)
            {
                tty.c_cflag |= PARENB;
                tty.c_cflag &= ~PARODD;
                #ifdef CMSPAR
                tty.c_cflag |= CMSPAR;
                #endif
            }
            else
            {
                tty.c_cflag &= ~PARENB;
                tty.c_cflag &= ~PARODD;
                #ifdef CMSPAR
                tty.c_cflag &= ~CMSPAR;
                #endif
            }

            if (flow_ctrl == "hardware"_ct)
            {
                tty.c_cflag |= CRTSCTS;
                tty.c_iflag &= ~(IXON | IXOFF | IXANY);
            }
            else if (flow_ctrl == "xon_xoff"_ct)
            {
                tty.c_cflag &= ~CRTSCTS;
                tty.c_iflag |= (IXON | IXOFF | IXANY);
            }
            else
            {
                tty.c_cflag &= ~CRTSCTS;
                tty.c_iflag &= ~(IXON | IXOFF | IXANY);
            }

            tty.c_iflag &= ~IGNBRK;
            tty.c_lflag = 0;
            tty.c_oflag = 0;
            // Make read() fully non-blocking. We will use select() to wait for data.
            tty.c_cc[VMIN]  = 0;
            tty.c_cc[VTIME] = 0;

            tty.c_cflag |= (CLOCAL | CREAD);

            if (stop_bits == 2)
                tty.c_cflag |= CSTOPB;
            else
                tty.c_cflag &= ~CSTOPB;

            tcsetattr(m_fd, TCSANOW, &tty);
            
            // Flush any old data in the kernel buffer
            tcflush(m_fd, TCIOFLUSH);
        }
        log_event_msg(log::info, log_event::open_success, path, std::to_string(baud_rate));
        #endif
        return port::start();
    }

    bool port_serial::stop()
    {
        set_state(state_stopping);
        close_handle();
        return port::stop();
    }

    void port_serial::close_handle()
    {
        #if defined(ADAM_PLATFORM_WINDOWS)
        if (m_handle != INVALID_HANDLE_VALUE) 
        {
            ::PurgeComm(m_handle, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
            ::CancelIoEx(m_handle, NULL);

            CloseHandle(m_handle);
            m_handle = INVALID_HANDLE_VALUE;
            log_event_msg(log::info, log_event::close_success);
        }
        #else
        if (m_fd >= 0) 
        {
            ::close(m_fd);
            m_fd = -1;
            log_event_msg(log::info, log_event::close_success);
        }
        #endif
    }

    bool port_serial::read(buffer*& buff)
    {
        uint8_t temp_buf[4096];
        int bytes_read = 0;

        #if defined(ADAM_PLATFORM_LINUX)
        struct pollfd pfd;
        pfd.fd = m_fd;
        pfd.events = POLLIN;
        #endif

        // Loop until data is read or the port is stopped.
        // This makes the call blocking from the caller's perspective,
        // but correctly aborts if the port is shut down.
        while (is_running())
        {
            #if defined(ADAM_PLATFORM_WINDOWS)
            DWORD dwRead = 0;
            if (ReadFile(m_handle, temp_buf, sizeof(temp_buf), &dwRead, NULL)) 
            {
                if (dwRead > 0)
                {
                    bytes_read = static_cast<int>(dwRead);
                    break;
                }
                else
                {
                    // Read timed out (0 bytes). Yield to prevent 100% CPU spinning.
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
            else
            {
                DWORD err = ::GetLastError();
                if (is_running())
                {
                    language lang = get_controller()->get_language();
                    std::string_view err_txt = get_error_log_text(resolve_serial_error(err), lang);
                    log_event_msg(log::error, log_event::read_failed, err_txt, std::to_string(err));
                    set_state(state_error);
                    close_handle();
                }
                return false;
            }
            #else
            int rv = poll(&pfd, 1, 100); // 100ms interval for responsive shutdown
            if (rv < 0) 
            {
                if (errno == EINTR) continue;
                int err = errno;
                if (is_running())
                {
                    language lang = get_controller()->get_language();
                    std::string_view err_txt = get_error_log_text(resolve_serial_error(err), lang);
                    log_event_msg(log::error, log_event::read_failed, err_txt, std::to_string(err));
                    set_state(state_error);
                    close_handle();
                }
                return false;
            }
            else if (rv > 0)
            {
                int chunk = ::read(m_fd, temp_buf + bytes_read, sizeof(temp_buf) - bytes_read);
                if (chunk < 0 && (errno == EINTR || errno == EAGAIN))
                {
                    continue;
                }
                if (chunk <= 0)
                {
                    int err = errno;
                    if (is_running())
                    {
                        language lang = get_controller()->get_language();
                        std::string_view err_txt = get_error_log_text(resolve_serial_error(err), lang);
                        log_event_msg(log::error, log_event::read_failed, err_txt, std::to_string(err));
                        set_state(state_error);
                        close_handle();
                    }
                    return false;
                }
                
                bytes_read += chunk;

                int interval_timeout = (m_rit->get_value() < 0) ? 0 : static_cast<int>(m_rit->get_value());
                if (interval_timeout > 0)
                {
                    while (bytes_read < static_cast<int>(sizeof(temp_buf)))
                    {
                        int burst_rv = poll(&pfd, 1, interval_timeout);
                        if (burst_rv > 0)
                        {
                            chunk = ::read(m_fd, temp_buf + bytes_read, sizeof(temp_buf) - bytes_read);
                            if (chunk > 0) bytes_read += chunk;
                            else break;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                break;
            }
            #endif
        }

        if (bytes_read > 0)
        {
            buff = buffer_manager::get().request_buffer(bytes_read);
            if (!buff) return false;

            std::memcpy(buff->data_as<uint8_t>(), temp_buf, bytes_read);
            buff->set_size(bytes_read);
            buff->set_timestamp();
            return true;
        }

        return false;
    }

    bool port_serial::write(buffer* buff)
    {
        if (!buff || buff->get_size() == 0) return false;

        const uint8_t* data = buff->data_as<const uint8_t>();
        size_t size = buff->get_size();
        int bytes_written = 0;

        #if defined(ADAM_PLATFORM_WINDOWS)
        DWORD dwWritten = 0;
        if (WriteFile(m_handle, data, static_cast<DWORD>(size), &dwWritten, NULL)) 
        {
            bytes_written = static_cast<int>(dwWritten);
        }
        else
        {
            DWORD err = ::GetLastError();
            log_event_msg(log::error, log_event::write_failed, std::to_string(err));
        }
        #else
        bytes_written = ::write(m_fd, data, size);
        if (bytes_written < 0)
        {
            int err = errno;
            log_event_msg(log::error, log_event::write_failed, std::to_string(err));
        }
        #endif

        return bytes_written == static_cast<int>(size);
    }

    void port_serial::log_event_msg(log::level lvl, log_event ev, std::string_view arg1, std::string_view arg2)
    {
        auto* ctrl = get_controller();
        language lang = ctrl->get_language();
        std::string_view fmt = get_log_event_text(ev, lang);

        std::string msg;
        if (!arg2.empty())
        {
            msg = std::vformat(fmt, std::make_format_args(arg1, arg2));
        }
        else if (!arg1.empty())
        {
            msg = std::vformat(fmt, std::make_format_args(arg1));
        }
        else
        {
            msg = std::string(fmt);
        }

        ctrl->log(adam::log(lvl, std::format("Port ({}) [serial]: {}", get_name().c_str(), msg)));
    }

    std::string_view port_serial::get_log_event_text(log_event ev, language lang)
    {
        using arr = std::array<std::string_view, adam::languages_count>;

        static const std::unordered_map<log_event, arr> table =
        {
            { log_event::path_empty,
              { "Device path is empty.",
                "Gerätepfad ist leer." }},
            { log_event::open_failed,
              { "Open device \"{}\" failed: {}",
                "Öffnen des Geräts \"{}\" fehlgeschlagen: {}" }},
            { log_event::open_success,
              { "Device \"{}\" opened successfully (Baud: {}).",
                "Gerät \"{}\" erfolgreich geöffnet (Baud: {})." }},
            { log_event::close_success,
              { "Device closed.",
                "Gerät geschlossen." }},
            { log_event::read_failed,
              { "Read failed: {} ({})",
                "Lesefehler: {} ({})" }},
            { log_event::write_failed,
              { "Write failed ({})",
                "Schreibfehler ({})" }},
            { log_event::device_removed,
              { "Device was disconnected or removed. ({} ({}))",
                "Gerät wurde getrennt oder entfernt. ({} ({}))" }}
        };

        auto it = table.find(ev);
        if (it != table.end())
            return it->second[static_cast<int>(lang)];

        return lang == language_german ? "Unbekanntes Ereignis." : "Unknown event.";
    }

    port_serial::serial_error_t port_serial::resolve_serial_error(uint32_t os_error)
    {
        #if defined(ADAM_PLATFORM_WINDOWS)
        switch (os_error)
        {
            case 0:                          return serial_error_t::success;
            case ERROR_FILE_NOT_FOUND:       return serial_error_t::error_file_not_found;
            case ERROR_ACCESS_DENIED:        return serial_error_t::error_access_denied;
            case ERROR_INVALID_HANDLE:       return serial_error_t::error_invalid_handle;
            case ERROR_BAD_COMMAND:          return serial_error_t::error_bad_command;
            case ERROR_GEN_FAILURE:          return serial_error_t::error_gen_failure;
            case ERROR_OPERATION_ABORTED:    return serial_error_t::error_operation_aborted;
            case ERROR_DEVICE_REMOVED:       return serial_error_t::error_device_removed;
            case ERROR_DEVICE_NOT_CONNECTED: return serial_error_t::error_device_not_connected;
            default:                         return serial_error_t::error_unknown;
        }
        #else
        switch (os_error)
        {
            case 0:      return serial_error_t::success;
            case ENODEV: return serial_error_t::error_no_device;
            case EIO:    return serial_error_t::error_io;
            case EBADF:  return serial_error_t::error_invalid_handle;
            case ENOTTY: return serial_error_t::error_not_a_tty;
            case ENOENT: return serial_error_t::error_file_not_found;
            case EACCES: return serial_error_t::error_access_denied;
            default:     return serial_error_t::error_unknown;
        }
        #endif
    }

    std::string_view port_serial::get_error_log_text(serial_error_t err, language lang)
    {
        using arr = std::array<std::string_view, adam::languages_count>;

        static const std::unordered_map<serial_error_t, arr> table =
        {
            { serial_error_t::success,
              { "Success.",
                "Erfolgreich." }},
            { serial_error_t::error_file_not_found,
              { "Not found.",
                "Nicht gefunden." }},
            { serial_error_t::error_access_denied,
              { "Access denied.",
                "Zugriff verweigert." }},
            { serial_error_t::error_invalid_handle,
              { "Invalid handle.",
                "Ungültiges Handle." }},
            { serial_error_t::error_bad_command,
              { "Device does not recognize the command.",
                "Gerät erkennt den Befehl nicht." }},
            { serial_error_t::error_gen_failure,
              { "Device is not functioning.",
                "Gerät funktioniert nicht." }},
            { serial_error_t::error_operation_aborted,
              { "Operation aborted.",
                "Vorgang abgebrochen." }},
            { serial_error_t::error_device_removed,
              { "Device was removed.",
                "Gerät wurde entfernt." }},
            { serial_error_t::error_device_not_connected,
              { "Device not connected.",
                "Gerät nicht verbunden." }},
            { serial_error_t::error_io,
              { "I/O error.",
                "E/A-Fehler." }},
            { serial_error_t::error_no_device,
              { "No such device.",
                "Gerät existiert nicht." }},
            { serial_error_t::error_not_a_tty,
              { "Not a serial terminal.",
                "Kein serielles Terminal." }},
            { serial_error_t::error_unknown,
              { "Unknown serial error.",
                "Unbekannter serieller Fehler." }}
        };

        auto it = table.find(err);
        if (it != table.end())
            return it->second[static_cast<int>(lang)];

        return lang == language_german ? "Unbekannter Fehler." : "Unknown error.";
    }

}