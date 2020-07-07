 #pragma once
#include <string>

#define ERR(...) ulog.val("ERR: ", __VA_ARGS__, __FUNCTION__, __FILE__, __LINE__)
#define WARN(...) ulog.val("WARN: ", __VA_ARGS__, __FUNCTION__, __FILE__, __LINE__)
#define LOG(...) ulog.val(#__VA_ARGS__, "=", __VA_ARGS__)

//---------------------------------------------------------------------
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

//---------------------------------------------------------------------
class ULog
{
public:
	static ULog &get_instance();

	void line() { str_internal("----------"); }
	template<class... Args> void val(const Args &... args) { val_internal(args...); }
	template<class... Args> void operator()(const Args &... args) { str_internal(args...); }

protected:
	template<class... Args> static void str_internal(const Args &... args)
	{
		UMsg s;
		s(args...);
		get_instance().to_log(s.get_buf());
	}

	template<class... Args> static void val_internal(const Args &... args)
	{
		UMsg s;
		s.val(args...);
		get_instance().to_log(s.get_buf());
	}

private:
	ULog();
	~ULog();

	bool create_log_file();
	void rotate_log_file();
	void to_log(const std::string &buf);

	inline std::string current_time();
	inline std::string current_date();

private:
	static std::string log_file_name;
	static std::string log_file_path;
	static std::FILE *log_file;
};

extern ULog &ulog;
