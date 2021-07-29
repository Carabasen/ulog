#include "ulog.h"

#include <chrono>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <stdarg.h>
#include <condition_variable>
#include <functional>
#include <thread>
#include <atomic>
#ifdef _WIN32
	#include <windows.h>
#endif

#ifndef _WIN32
	#define localtime_s(t, sse) localtime_r(sse, t)
	#define vsprintf_s vsprintf
#endif

namespace unm
{
	//--------------------------------------------------------------------- settings block
	//_CRT_DISABLE_PERFCRIT_LOCKS
	constexpr bool log2con = true;
	constexpr bool log2file = true;
	constexpr bool timestamps_in_console = false;          // add timestamps to console, if not, timestaps are written only to a log file
	constexpr int flush_interval_ms = 0;                   // interval betwen log file flush, 0 - flush every write, -1 - do not flush at all
	constexpr int max_files = 15;                          // max log files before rotating
	constexpr bool add_thread_names = true;                // add current thread name to each log line
	ustring ULog::file_path = utf8_to_native(u8"");        // path to store log files, if empty, then the current working directory is used UTF8
	ustring ULog::file_prefix = utf8_to_native(u8"ulog_"); // ulog file prefix UTF8
	ustring ULog::file_ext = utf8_to_native(u8".log");     // ulog file extension UTF8
	//---------------------------------------------------------------------
	ustring ULog::file_name;

	FILE *ULog::log_file = nullptr;
	thread_local string ULog::fmt_thread_name;
}

unm::ULog &ulog = unm::ULog::get_instance();

namespace unm
{
	namespace ch = std::chrono;
	namespace fs = std::filesystem;

	ustring utf8_to_native(const std::string &istr)
	{
	#ifdef _WIN32
		if (istr.empty()) return std::wstring();
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, &istr[0], (int)istr.size(), NULL, 0);
		std::wstring wstr(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &istr[0], (int)istr.size(), &wstr[0], size_needed);
		return wstr;
	#else
		return istr;
	#endif
	}

	// we are doing our best to minimize includes in the header, so... some more clutter here
	static std::atomic<int> default_thread_id(1);
	//--------------------------------------------------------------------- UVigilantCaller
	class UVigilantCaller
	{
	public:
		UVigilantCaller(int interval_ms, const std::function<void()> &func);
		~UVigilantCaller();

	public:
		std::condition_variable cv;
		std::mutex mut;
		std::thread caller;
		bool enabled;
	};
	//---------------------------------------------------------------------
	UVigilantCaller::UVigilantCaller(int interval_ms, const std::function<void()> &func)
	{
		enabled = interval_ms > 0;
		if (!enabled) return;
		auto duration_ms = std::chrono::milliseconds(interval_ms);

		caller = std::thread([=]()
			{
				while (true)
				{
					std::unique_lock<std::mutex> ulock(this->mut);
					if (this->cv.wait_for(ulock, duration_ms, [this] { return !this->enabled; })) return;
					func();
				}
			});
	}
	//---------------------------------------------------------------------
	UVigilantCaller::~UVigilantCaller()
	{
		if (caller.joinable())
		{
			{
				std::lock_guard<std::mutex> lg(mut);
				enabled = false;
			}
			cv.notify_one();
			caller.join();
		}
	}

	//--------------------------------------------------------------------- ULog
	ULog &ULog::get_instance()
	{
		// todo proper phoenix multithreaded singleton
		// a "leaking" singleton it is ok, but dirty
		static ULog *single = new ULog;
		return *single;

//		static std::mutex ulog_guard;
//		if (!single)
//		{
//			std::unique_lock<std::mutex> lock(ulog_guard);
//			if (!single)
//			{
//				single = new ULog;
//				//todo infinite-leaking
//				//std::atexit(kill_ulog);
//			}
//		}
//		return *single;

		// no more meyers
		//		static ULog instance; // it's mt safe since c++11
		//		return instance;
	}
	//---------------------------------------------------------------------
	void ULog::set_this_thread_name(const string &name)
	{
		if (name.empty())
		{
			ERR("[ULOG.thread_name] cannot be set empty");
			return;
		}

		string old_name = fmt_thread_name;
		fmt_thread_name = "[" + name + "] ";

		if (!old_name.empty() && fmt_thread_name != old_name)
		{
			ulog("[ULOG.thread_name] was ", old_name, "now ", fmt_thread_name);
		}
	}
	//---------------------------------------------------------------------
	// pf() is officially not recommended, use ulog() and ulog.val() instead
	//---------------------------------------------------------------------
	void ULog::pf(char const *const format, ...)
	{
		char buf[1024];
		va_list args;
		va_start(args, format);
		vsprintf_s(buf, format, args);
		va_end(args);
		ulog(buf);
	}
	//---------------------------------------------------------------------
	ULog::ULog()
	{
		if constexpr (log2file) create_log_file();
		flusher = new UVigilantCaller(flush_interval_ms, [this]() {this->flush(); });
		set_this_thread_name("main");
	}
	//---------------------------------------------------------------------
	ULog::~ULog()
	{
		delete flusher;
		if constexpr (log2file) if (nullptr != log_file) fclose(log_file);
	}
	//---------------------------------------------------------------------
	void ULog::kill_ulog()
	{
//		if (single) single->~ULog();
//		single = nullptr;
	}
	//---------------------------------------------------------------------
	bool ULog::create_log_file()
	{
		if (nullptr != log_file) fclose(log_file);

		if (!fs::path(file_path).is_absolute()) file_path = fs::current_path().native() + file_path;
		fs::create_directories(file_path);

		auto date = current_date();
		file_name = file_prefix + ustring(date.begin(), date.end()) + file_ext;
#ifdef _WIN32
		log_file = _wfsopen((file_path + L"/" + file_name).c_str(), L"a+", _SH_DENYWR);
#else
		log_file = fopen((file_path + "/" + file_name).c_str(), "a+");
#endif
		rotate_log_file();
		return nullptr == log_file;
	}
	//---------------------------------------------------------------------
	void ULog::rotate_log_file()
	{
		std::vector<ustring> logs;

		for (const auto &entry : fs::directory_iterator(file_path))
		{
			ustring name = entry.path().filename().native();
			if (file_ext == entry.path().extension().native() && file_name.size() == name.size())
			{
				logs.emplace_back(std::move(name));
			}
		}
		if (logs.size() > max_files)
		{
			std::sort(std::begin(logs), std::end(logs));
			for (int i = 0; i < static_cast<int>(logs.size()) - max_files; ++i)
			{
				fs::remove(file_path + utf8_to_native("/") + logs[i]);
			}
		}
	}
	//---------------------------------------------------------------------
	void ULog::to_log(const string &buf)
	{
		string prefix;
		const string cur_time = current_time();

		// set simply default thread name
		if (fmt_thread_name.empty())
		{
			//set_this_thread_name(std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id())));
			set_this_thread_name("thread " + std::to_string(default_thread_id++));
		}
		
		// printf and fprintf is mt safe by POSIX
		if constexpr (log2con)
		{
			if constexpr (timestamps_in_console) prefix = cur_time;
			if constexpr (add_thread_names) prefix += fmt_thread_name;
			printf("%s%s\n", prefix.c_str(), buf.c_str());
		}

		if (log2file && nullptr != log_file)
		{
			if constexpr (!timestamps_in_console)
			{
				prefix = cur_time;
				if constexpr (add_thread_names) prefix += fmt_thread_name;
			}
			fprintf(log_file, "%s%s\n", prefix.c_str(), buf.c_str());
			if constexpr (0 == flush_interval_ms) flush();
		}
	}
	//---------------------------------------------------------------------
	string ULog::current_time()
	{
		auto msec_se = ch::duration_cast<ch::milliseconds>(ch::system_clock::now().time_since_epoch()).count();
		time_t sec_se = time_t(msec_se / 1000);
		tm t;
		localtime_s(&t, &sec_se);

		char time_buf[64];
		snprintf(time_buf, sizeof(time_buf), "[%02d.%02d.%04d %02d:%02d:%02d:%03d] ",
			1 + t.tm_mon, t.tm_mday, 1900 + t.tm_year, t.tm_hour, t.tm_min, t.tm_sec, static_cast<int>(msec_se % 1000));
		return string(time_buf);
	}
	//---------------------------------------------------------------------
	string ULog::current_date()
	{
		time_t sec_se = time_t(ch::duration_cast<ch::seconds>(ch::system_clock::now().time_since_epoch()).count());
		tm t;
		localtime_s(&t, &sec_se);

		char date_buf[32];
		snprintf(date_buf, sizeof(date_buf), "%04d-%02d-%02d", 1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday);
		return string(date_buf);
	}
}