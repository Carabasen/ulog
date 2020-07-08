#include "../ulog.h"

//--------------------------------------------------------------------- UPoint3f
struct UPoint3f
{
	friend UMsg &operator+(UMsg &msg, const UPoint3f &p)
	{
		return msg(p.x, ", ", p.y, ", ", p.z);
	}
	float x, y, z;
};
//--------------------------------------------------------------------- UUser
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
//---------------------------------------------------------------------
int main()
{
	std::string fname = "useful_file.txt";
	bool isOpened = true;

	// ulog.val(...)
	// accepts a variable number of parameters
	// each call - one line of the log with timestamp
	ulog.val("Any number of parameters separated by comma", fname, "opened =", isOpened);
	// "[27.09.2019 18:48:35:963] Any number of parameters separated by comma useful_file.txt opened = true"

	// ulog(...) - same as ulog.val(...) but the parameters are not automatically separated by a space
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

	UPoint3f pos{ 122.3f, 17.4f, 442.0f };
	UUser user{ "Vaniya", 120, pos };

	ulog.val("User:", user);
	//"[27.09.2019 18:48:35:964] User: name(Vaniya), HP(120), pos(122.300003, 17.400000, 442.000000)"

	return 0;
}

