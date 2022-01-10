#pragma once
#include <type_traits>
#include <string>
/*
Usage:
	Basic usage:

	ulog.val("Anything you want to log", strings, numbers, user_types, "separated by comma");          // ulog.val("Number is", 0); -> "Number is 0"
	ulog("same as ulog.val", "but the parameters are not separated by a space automatically");         // ulog("Number is", 0); -> "Number is0" 
	L(param) macro: (param) unwraps to "param", "=", param                                             // auto t = "text"; auto v == 10; 
	                                                                                                   // ulog.val(L(t), L(v)); -> t = text v = 10
	ERR("Macro wrap for errors", "same as ulog.val plus function name, file name and line number ");   // ERR("Code:", 0); -> "ERR: Code: 0, main [ C:\ulog\ulog_test\ulog_test.cpp 97 ]"
	WARN("Same as ERR, but WARN");
	LOG(...) - macro for log name/value at once                                                        // bool needMoney = true; LOG(needMoney); -> "needMoney = true"
	ulog.pf(...) - good old printf, if someone need to, plus timestamp and file log.

	For detailed description please refer to https://github.com/Carabasen/ulog
*/
#define ERR(...) ulog.val("ERR:", __VA_ARGS__, "[", __FUNCTION__, __FILE__, __LINE__, "]")
#define WARN(...) ulog.val("WARN:", __VA_ARGS__, "[", __FUNCTION__, __FILE__, __LINE__, "]")
#define LOG(...) ulog.val(#__VA_ARGS__, "=", __VA_ARGS__)
#define L(...) #__VA_ARGS__, "=", __VA_ARGS__

#ifdef _WIN32
	#define ustring std::wstring
#else
	#define ustring std::string
#endif

//--------------------------------------------------------------------- umsg
// todo maybe std::to_chars for better performance?
// todo add wstring
class umsg
{
public:
	template<class T> std::enable_if_t<std::is_arithmetic_v<T>, umsg &> operator+(T p) { buf += std::to_string(p); return *this; }
	umsg &operator+(bool p) { buf += p ? "true" : "false"; return *this; }
	umsg &operator+(const char *p) { buf.append(p); return *this; }
	umsg &operator+(const unsigned char *p) { buf.append(reinterpret_cast<const char *>(p)); return *this; }
	umsg &operator+(const std::string &p) { buf.append(p); return *this; }
	umsg &operator+(const std::string_view &p) { buf.append(p); return *this; }
	umsg &operator+(const umsg &p) { buf.append(p.buf); return *this; }

	const std::string &get_buf() const { return buf; }

	template<class... Args> umsg &operator()(const Args &... args)
	{
		(*this + ... + args);
		return *this;
	}

	template<class... Args> umsg &val(const Args &... args)
	{
		(*this - ... - args);
		buf.pop_back();	// remove unnecessary last space symbol
		return *this;
	}

	template<class T> umsg &operator-(const T &v)
	{
		(*this + v).buf.append(" ");
		return *this;
	}

private:
	std::string buf;
};

//---------------------------------------------------------------------
namespace unm
{
	using std::string;
	class uvigilant_caller;
	
#if __cplusplus > 201703L
	ustring utf8_to_native(const std::u8string &istr);
#else
	ustring utf8_to_native(const std::string &istr);
#endif

	//--------------------------------------------------------------------- unm::ulogger
	class ulogger
	{
	public:
		static ulogger &get_instance();
		static void set_this_thread_name(const string &name);
		void flush() { fflush(log_file); }

		template<class... Args> void val(const Args &... args)
		{
			umsg s;
			s.val(args...);
			get_instance().to_log(s.get_buf());
		}
		template<class... Args> void operator()(const Args &... args)
		{
			umsg s;
			s(args...);
			get_instance().to_log(s.get_buf());
		}
		// if someone need to...
		void pf(char const *const format, ...);

	private:
		ulogger();
		~ulogger();
		static void kill_ulog();

		ulogger(const ulogger&) = delete;
		ulogger& operator=(const ulogger&) = delete;
		ulogger(ulogger&&) = delete;
		ulogger& operator=(ulogger&&) = delete;

		bool create_log_file();
		void rotate_log_file();
		void to_log(const string &buf);

		string current_time();
		string current_date();

	private:
		static ustring file_ext;
		static ustring file_prefix;
		static ustring file_path;
		static ustring file_name;
		static std::FILE *log_file;
		static thread_local string fmt_thread_name;

		uvigilant_caller *flusher;
	};
}
extern unm::ulogger &ulog;
