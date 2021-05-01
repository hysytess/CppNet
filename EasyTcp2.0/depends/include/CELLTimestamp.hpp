#ifndef _CELLTimestamp_hpp_
#define _CELLTimestamp_hpp_

#include <chrono> // c++11 std.
using namespace std::chrono;

class CELLTime
{
public:
	// ms 获取当前时间
	static time_t getNowInMillisec()
	{
		return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
	}
};

class CELLTimestamp
{
public:
	CELLTimestamp()
	{
		update();
	}
	~CELLTimestamp() {}

	void update()
	{
		_begin = high_resolution_clock::now();
	}

	// s
	double getElapsedSecond()
	{
		return getElapsedTimeInMicrosec() * 0.000001;
	}

	// ms
	double getElapsedTimeInMillisec()
	{
		return this->getElapsedTimeInMicrosec() * 0.001;
	}

	// us
	long long getElapsedTimeInMicrosec()
	{
		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
	}

protected:
	time_point<high_resolution_clock>_begin;
};

#endif