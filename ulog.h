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
	ERR("Macro wrap for errors", "same as ulog.val plus function name, file name and line number ");   // ERR("Code", 0); -> "ERR:  0, main [ C:\ulog\ulog_test\ulog_test.cpp 97 ]"
	WARN("Same as ERR, but WARN");
	LOG(...) - macro for log name/value at once                                                        // bool needMoney = true; LOG(needMoney); -> "needMoney = true"
	ulog.pf(...) - good old printf, if someone need to, plus timestamp and file log.

	For detailed description please refer to https://github.com/Carabasen/ulog
*/
#define ERR(...) ulog.val("ERR: ", __VA_ARGS__, "[", __FUNCTION__, __FILE__, __LINE__, "]")
#define WARN(...) ulog.val("WARN: ", __VA_ARGS__, "[", __FUNCTION__, __FILE__, __LINE__, "]")
#define LOG(...) ulog.val(#__VA_ARGS__, "=", __VA_ARGS__)
#define L(...) #__VA_ARGS__, "=", __VA_ARGS__

#ifdef _WIN32
	#define ustring std::wstring
#else
	#define ustring std::string
#endif

//--------------------------------------------------------------------- UMsg
// todo maybe std::to_chars for better perfomance?
// todo add wstring
class UMsg
{
public:
	template<class T> std::enable_if_t<std::is_arithmetic_v<T>, UMsg &> operator+(T p) { buf += std::to_string(p); return *this; }
	UMsg &operator+(bool p) { buf += p ? "true" : "false"; return *this; }
	UMsg &operator+(const char *p) { buf.append(p); return *this; }
	UMsg &operator+(const unsigned char *p) { buf.append(reinterpret_cast<const char *>(p)); return *this; }
	UMsg &operator+(const std::string &p) { buf.append(p); return *this; }
	UMsg &operator+(const std::string_view &p) { buf.append(p); return *this; }
	UMsg &operator+(const UMsg &p) { buf.append(p.buf); return *this; }

	void buf_chop() { buf.pop_back(); }
	const std::string &get_buf() const { return buf; }

	template<class... Args> UMsg &operator()(const Args &... args)
	{
		(*this + ... + args);
		return *this;
	}

	template<class... Args> UMsg &val(const Args &... args)
	{
		(*this - ... - args);
		buf_chop();	// remove unnecessary last space symbol
		return *this;
	}

	template<class T> UMsg &operator-(const T &v)
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
	class UVigilantCaller;
	
#if __cplusplus > 201703L
	ustring utf8_to_native(const std::u8string &istr);
#else
	ustring utf8_to_native(const std::string &istr);
#endif

	//--------------------------------------------------------------------- unm::ULog
	class ULog
	{
	public:
		static ULog &get_instance();
		static void set_this_thread_name(const string &name);
		void flush() { fflush(log_file); }

		template<class... Args> void val(const Args &... args)
		{
			UMsg s;
			s.val(args...);
			get_instance().to_log(s.get_buf());
		}
		template<class... Args> void operator()(const Args &... args)
		{
			UMsg s;
			s(args...);
			get_instance().to_log(s.get_buf());
		}
		// if someone need to...
		void pf(char const *const format, ...);

	private:
		ULog();
		~ULog();
		static void kill_ulog();

		ULog(const ULog&) = delete;
		ULog& operator=(const ULog&) = delete;
		ULog(ULog&&) = delete;
		ULog& operator=(ULog&&) = delete;

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

		UVigilantCaller *flusher;
	};
}
extern unm::ULog &ulog;
