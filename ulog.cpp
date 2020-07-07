#include "ulog.h"
#include <chrono>
#include <filesystem>
#include <vector>
#include <algorithm>

namespace ch = std::chrono;
namespace fs = std::filesystem;

#ifndef _WIN32
	#define localtime_s(t, sse) localtime_r(sse, t)
#endif

//---------------------------------------------------------------------
//_CRT_DISABLE_PERFCRIT_LOCKS
constexpr bool ulog_log2con = true;
constexpr bool ulog_log2file = true;
constexpr bool ulog_force_flush = true; // fflush every write
constexpr bool ulog_time_to_console = false; // add timestamps to console, if not, timestaps are written only to a log file
constexpr int ulog_max_files = 15; // max log files before rotating
#define ULOG_EXT ".log"
#define ULOG_PREFIX "ulog_"
std::string ULog::log_file_path;
//---------------------------------------------------------------------
std::string ULog::log_file_name;

FILE *ULog::log_file = nullptr;
ULog &ulog = ULog::get_instance();

//--------------------------------------------------------------------- ULog
bool ULog::create_log_file()
{
	if (nullptr != log_file) fclose(log_file);
	if (log_file_path.empty()) log_file_path = fs::current_path().string(); else fs::create_directories(log_file_path);
	log_file_name = ULOG_PREFIX + current_date() + ULOG_EXT;
#ifdef _WIN32
	log_file = _fsopen((log_file_path + "/" + log_file_name).c_str(), "a+", _SH_DENYWR);
#else
	log_file = fopen((log_file_path + "/" + log_file_name).c_str(), "a+");
#endif
	rotate_log_file();
	return nullptr == log_file;
}
//---------------------------------------------------------------------
std::string ULog::current_time()
{
	auto msec_se = ch::duration_cast<ch::milliseconds>(ch::system_clock::now().time_since_epoch()).count();
	time_t sec_se = time_t(msec_se / 1000);
	tm t;
	localtime_s(&t, &sec_se);

	char time_buf[64];
	snprintf(time_buf, sizeof(time_buf), "[%02d.%02d.%04d %02d:%02d:%02d:%03d]", 
		1 + t.tm_mon, t.tm_mday, 1900 + t.tm_year, t.tm_hour, t.tm_min, t.tm_sec, static_cast<int>(msec_se % 1000));
	return std::string(time_buf);
}
//---------------------------------------------------------------------
std::string ULog::current_date()
{
	time_t sec_se = time_t(ch::duration_cast<ch::seconds>(ch::system_clock::now().time_since_epoch()).count());
	tm t;
	localtime_s(&t, &sec_se);

	char date_buf[32];
	snprintf(date_buf, sizeof(date_buf), "%04d-%02d-%02d", 1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday);
	return std::string(date_buf);
}
//---------------------------------------------------------------------
void ULog::rotate_log_file()
{
	std::vector<std::string> logs;

	for (const auto &entry : fs::directory_iterator(log_file_path))
	{
		auto name = entry.path().filename().string();
		if (ULOG_EXT == entry.path().extension().string() && log_file_name.size() == name.size())
		{
			logs.emplace_back(std::move(name));
		}
	}
	if (logs.size() > ulog_max_files)
	{
		std::sort(std::begin(logs), std::end(logs));
		for (int i = 0; i < static_cast<int>(logs.size()) - ulog_max_files; ++i)
		{
			fs::remove(log_file_path + "/" + logs[i]);
		}
	}
}
//---------------------------------------------------------------------
ULog::ULog()
{
	if constexpr (ulog_log2file) create_log_file();
}
//---------------------------------------------------------------------
ULog::~ULog()
{
	if constexpr (ulog_log2file) if (nullptr != log_file) fclose(log_file);
}
//---------------------------------------------------------------------
ULog &ULog::get_instance()
{
	static ULog instance; // it's mt safe since c++11
	return instance;
}
//---------------------------------------------------------------------
void ULog::to_log(const std::string &buf)
{
	// printf and fprintf is mt safe by POSIX
	if constexpr (ulog_log2con)
	{
		if constexpr (ulog_time_to_console)
		{
			printf("%s %s\n", current_time().c_str(), buf.c_str());
		}
		else
		{
			printf("%s\n", buf.c_str());
		}
	}
	
	if constexpr (ulog_log2file)
	{
		if (nullptr != log_file)
		{
			fprintf(log_file, "%s %s\n", current_time().c_str(), buf.c_str());
			if constexpr (ulog_force_flush) fflush(log_file);
		}
	}
}
