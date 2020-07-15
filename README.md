# ulog
Fast, lightweight and compact C++17 logging library. [![Build status](https://ci.appveyor.com/api/projects/status/0rc9kvnal4n4g4a1?svg=true)](https://ci.appveyor.com/project/Carabasen/ulog)

## License
The Unlicense

## Platform
* Windows
* Linux

## Features
* Very fast, lightweight and compact
* Cross-platform
* Thread safe
* No third-party "includes" and monstrous iostream
* Multiple log targets: console and/or file
* Rotating log files
* Millisecond precision timestamps
* Flexible log file flush interval setting
* Easy to use, just call 
  ``` c++
	ulog("Anything you want to log", strings, numbers, user_types, "separated by comma");
  ```
* User defined types logging via friend operator
  ``` c++
	friend UMsg &operator+(UMsg &msg, const UUserType &p)
	{
		return msg("Anything..", p.strings, p.numbers, p.user_types, "separated by comma");
	}
	...
	UUserType ut;
	ulog("User:", ut);
  ```

## Usage
* Basic
  ``` c++
	#include "ulog.h"
	...
	std::string fname = "useful_file.txt";
	bool isOpened = true;

	// ulog.val(...)
	// accepts a variable number of parameters
	// each call - one line of the log with timestamp
	ulog.val("Any number of parameters separated by comma", fname, "opened =", isOpened);
	// "[27.09.2019 18:48:35:963] Any number of parameters separated by comma useful_file.txt opened = true"

	// ulog(...) - same as ulog.val(...) but the parameters are not separated by a space automatically
	ulog(fname, " is opened = ", isOpened);
	// "[27.09.2019 18:48:35:964] useful_file.txt is opened = true"
	ulog("Processing...");
	// "[27.09.2019 18:48:35:964] Processing..."

	// ulog.pf(...) - good old printf, if someone need to, plus timestamp and file log.
	char txt[] = "horse";
	ulog.pf("Good old %s, from %ds", txt, 2000);
	// same as
	ulog("Good old ", txt, ", from ", 2000, "s");
	//"[27.09.2019 18:48:35:964] Good old horse, from 2000s"

	// ERR - Macros for errors, same as ulog.val(...) plus function name, file name and line number
	ERR("File not found", fname);
	//"[27.09.2019 18:48:35:964] ERR:  File not found useful_file.txt [ main C:\ulog\ulog_test\ulog_test.cpp 97 ]"
	
	// WARN - same as ERR, but WARN
	WARN("File not found", fname);
	//"[27.09.2019 18:48:35:964] WARN:  File not found useful_file.txt [ main C:\ulog\ulog_test\ulog_test.cpp 99 ]"
	
	// LOG(...) - macro for log name/value at once
	LOG(isOpened);
	//"[27.09.2019 18:48:35:964] isOpened = true"
  ```
* User types
  ``` c++
	#include "ulog.h"
	...
	struct UPoint3f
	{
		friend UMsg &operator+(UMsg &msg, const UPoint3f &p)
		{
			return msg(p.x, ", ", p.y, ", ", p.z);
		}
		float x, y, z;
	};

	struct UUser
	{
		friend UMsg &operator+(UMsg &msg, const UUser &p)
		{
			return msg("name(", p.name, "), HP(", p.health, "), pos(", p.pos, ")");
		}

		std::string name;
		int health;
		UPoint3f pos;
	};
	...

	{
		UPoint3f pos{ 122.3f, 17.4f, 442.0f };
		UUser user{ "Vaniya", 120, pos };
		ulog.val("User:", user);
		//"[27.09.2019 18:48:35:965] User: name(Vaniya), HP(120), pos(122.300003, 17.400000, 442.000000)"
	}

  ```
* Settings
  ``` c++
	//--------------------------------------------------------------------- settings
	constexpr bool log2con = true;
	constexpr bool log2file = true;
	constexpr bool timestamps_in_console = false;    // add timestamps to console, if not, timestaps are written only to a log file
	constexpr int flush_interval_ms = 100;           // interval betwen log file flush, 0 - flush every write, -1 - do not flush at all
	constexpr int max_files = 15;                    // max log files before rotating
	constexpr std::string_view ulog_ext(".log");     // ulog file extension
	constexpr std::string_view ulog_prefix("ulog_"); // ulog file prefix
	std::string ULog::log_file_path;                 // path to store log files, if empty, then the current working directory is used
  ```
