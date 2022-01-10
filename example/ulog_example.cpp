#include "../ulog.h"

//--------------------------------------------------------------------- upoint3f
struct upoint3f
{
	friend umsg &operator+(umsg &msg, const upoint3f &p)
	{
		return msg(p.x, ", ", p.y, ", ", p.z);
	}
	float x, y, z;
};
//--------------------------------------------------------------------- uuser
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
//---------------------------------------------------------------------
int main()
{
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

	upoint3f pos{ 122.3f, 17.4f, 442.0f };
	uuser user{ "Vaniya", 120, pos };

	ulog.val("User:", user);
	//"[27.09.2019 18:48:35:964] User: name(Vaniya), HP(120), pos(122.300003, 17.400000, 442.000000)"

	return 0;
}

