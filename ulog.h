#pragma once
#include <string>
/*
Usage:
	Basic usage:

	ulog.val("Anything you want to log", strings, numbers, user_types, "separated by comma");          // ulog.val("Number is", 0); -> "Number is 0"
	ulog("same as ulog.val", "but the parameters are not separated by a space automatically");         // ulog("Number is", 0); -> "Number is0" 
	ERR("Macro wrap for errors", "same as ulog.val plus function name, file name and line number ");   // ERR("Code", 0); -> "ERR:  0, main [ C:\ulog\ulog_test\ulog_test.cpp 97 ]"
	WARN("Same as ERR, but WARN");
	LOG(...) - macro for log name/value at once                                                        // bool needMoney = true; LOG(needMoney); -> "needMoney = true"
	ulog.pf(...) - good old printf, if someone need to, plus timestamp and file log.

	For detailed description please refer to https://github.com/Carabasen/ulog
*/
#define ERR(...) ulog.val("ERR: ", __VA_ARGS__, "[", __FUNCTION__, __FILE__, __LINE__, "]")
#define WARN(...) ulog.val("WARN: ", __VA_ARGS__, "[", __FUNCTION__, __FILE__, __LINE__, "]")
#define LOG(...) ulog.val(#__VA_ARGS__, "=", __VA_ARGS__)

//--------------------------------------------------------------------- UMsg
// todo maybe std::to_chars for better perfomance?
// todo add wstring
class UMsg
{
public:
	template<class T> UMsg &operator+(T p) { buf += std::to_string(p); return *this; }
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

namespace unm
{
	using std::string;
	class UVigilantCaller;

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

		bool create_log_file();
		void rotate_log_file();
		void to_log(const string &buf);

		string current_time();
		string current_date();

	private:
		static string log_file_name;
		static string log_file_path;
		static std::FILE *log_file;
		static thread_local string fmt_thread_name;

		UVigilantCaller *flusher;
	};
}
extern unm::ULog &ulog;
