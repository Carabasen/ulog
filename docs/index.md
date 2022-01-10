# ulog
Fast, lightweight, cross-platform and compact C++ logging library.

## License
The Unlicense

## Platform
* Windows
* Linux

## Features
* Very fast, lightweight and compact.
* Cross-platform.
* Thread safe.
* Custom thread names: `ulog.set_this_thread_name("UI thread");`
* No third-party "includes" and monstrous iostream.
* Multiple log targets: console and/or file.
* Rotating log files.
* Millisecond precision timestamps.
* Flexible log file flush interval setting.
* Easy to use, just call:
  ``` c++
	ulog("Anything you want to log", strings, numbers, user_types, "separated by comma");
  ```
* User defined types logging via friend operator:
  ``` c++
	friend umsg &operator+(umsg &msg, const uuser_type &p)
	{
		return msg("Anything..", p.strings, p.numbers, p.user_types, "separated by comma");
	}
	...
	uuser_type ut;
	ulog("User:", ut);
  ```

## Usage
* Basic
  ``` c++
	#include "ulog.h"
	...
	std::string fname = "useful_file.txt";
	bool is_opened = true;

	// ulog.val(...)
	// accepts a variable number of parameters
	// each call - one line of the log with timestamp
	ulog.val("Any number of parameters separated by comma", fname, "opened =", is_opened);
	// "[27.09.2019 18:48:35:963] [main] Any number of parameters separated by comma useful_file.txt opened = true"

	// ulog(...) - same as ulog.val(...) but the parameters are not separated by a space automatically
	ulog(fname, " is opened = ", is_opened);
	// "[27.09.2019 18:48:35:964] [main] useful_file.txt is opened = true"
	ulog.val(L(fname), L(is_opened));
	// "[27.09.2019 18:48:35:964] [main] fname = useful_file.txt is_opened = true"
	ulog("Processing...");
	// "[27.09.2019 18:48:35:964] [main] Processing..."

	// ulog.pf(...) - good old printf, if someone need to, plus timestamp and file log.
	char txt[] = "horse";
	ulog.pf("Good old %s, from %ds", txt, 2000);
	// same as
	ulog("Good old ", txt, ", from ", 2000, "s");
	//"[27.09.2019 18:48:35:964] [main] Good old horse, from 2000s"

	// ERR - Macros for errors, same as ulog.val(...) plus function name, file name and line number
	ERR("File not found", fname);
	//"[27.09.2019 18:48:35:964] [main] ERR: File not found useful_file.txt [ main C:\ulog\ulog_test\ulog_test.cpp 97 ]"
	
	// WARN - same as ERR, but WARN
	WARN("File not found", fname);
	//"[27.09.2019 18:48:35:964] [main] WARN: File not found useful_file.txt [ main C:\ulog\ulog_test\ulog_test.cpp 99 ]"
	
	// LOG(...) - macro for log name/value at once
	LOG(is_opened);
	//"[27.09.2019 18:48:35:964] [main] is_opened = true"
  ```
* User types
  ``` c++
	#include "ulog.h"
	...
	struct upoint3f
	{
		friend umsg &operator+(umsg &msg, const upoint3f &p)
		{
			return msg(p.x, ", ", p.y, ", ", p.z);
		}
		float x, y, z;
	};

	struct uuser
	{
		friend umsg &operator+(umsg &msg, const uuser &p)
		{
			return msg("name(", p.name, "), HP(", p.health, "), pos(", p.pos, ")");
		}

		std::string name;
		int health;
		upoint3f pos;
	};
	...

	{
		upoint3f pos{ 122.3f, 17.4f, 442.0f };
		uuser user{ "Vaniya", 120, pos };
		ulog.val("User:", user);
		//"[27.09.2019 18:48:35:965] [main] User: name(Vaniya), HP(120), pos(122.300003, 17.400000, 442.000000)"
	}

  ```
* Settings
  ``` c++
	//--------------------------------------------------------------------- settings
	constexpr bool log2con = true;
	constexpr bool log2file = true;
	constexpr bool timestamps_in_console = false;          // add timestamps to console, if not, timestaps are written only to a log file
	constexpr int flush_interval_ms = 0;                   // interval betwen log file flush, 0 - flush every write, -1 - do not flush at all
	constexpr int max_files = 15;                          // max log files before rotating
	constexpr bool add_thread_names = true;                // add current thread name to each log line
	ustring ulogger::file_path = utf8_to_native(u8"\\Logs");  // path to store log files, if empty, then the current working directory is used // UTF8
	ustring ulogger::file_prefix = utf8_to_native(u8"ulog_"); // ulog file prefix UTF8
	ustring ulogger::file_ext = utf8_to_native(u8".log");     // ulog file extension UTF8
  ```
