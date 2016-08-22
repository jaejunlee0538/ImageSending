#ifndef PRECISE_CLOCK_H_
#define PRECISE_CLOCK_H_
/*
made by jaejunlee, 2015.10.23
*/
#include <iostream>
#include <iomanip>
#include <sstream>

#if defined(WIN32) | defined(WIN64)
#include <windows.h> 
#pragma comment (lib, "winmm.lib") 

#elif defined(__linux__)
#include <unistd.h>
#include <sys/time.h>
#include <iosfwd>

#endif

class PreciseClock
{
public:
    PreciseClock();
    virtual ~PreciseClock() {};
public: 
    virtual void reset(void);
    double getElapsedTime_sec(void);
	inline double getCPUClock_sec(void) const;

protected:	
	double t_start, t_current; //To increase accuracy, think changing this to long long.

#if defined(WIN32) | defined(WIN64)
private: 	
	double freq_coeff;
#elif defined(__linux__)

#endif
};

class ElapsedTimePrinter: protected PreciseClock
{
public:
	ElapsedTimePrinter(const std::string& message="")
			:msg(message){
		if(msg.empty())
			msg="Elapsed Time";
	}

	void reset(void){
		PreciseClock::reset();
	}

	std::string getElapsedTimeMessage(){
		std::ostringstream oss;
		oss<<msg<<" : "<<std::setprecision(6)<<getElapsedTime_sec()<<std::endl;
		return oss.str();
	}

protected:
	std::string msg;
};

class ElapsedTimeAutoPrinter: protected PreciseClock
{
public:
	ElapsedTimeAutoPrinter(const std::string& message="", std::ostream& _os=std::cout)
			:msg(message), os(_os){
		if(msg.empty())
			msg="Elapsed Time";
	}

	/*
	 * When this is destructed, it outputs elapsed time from it's construction.
	 */
	~ElapsedTimeAutoPrinter(){
		os<<msg<<" : "<<std::setprecision(6)<<getElapsedTime_sec()<<std::endl;
	}

protected:
	std::ostream& os;
	std::string msg;
};

class ClockTimeout : protected PreciseClock
{
public:

	ClockTimeout(const double& timeout_sec=0.0);

	virtual ~ClockTimeout();

	void resetTimeout(bool precision_offset = true);

	inline double getTimeoutDuration(void) const { return this->duration_sec; }

	void setTimeoutDuration(const double& timeout_sec);

	bool isTimeout(void);

	double getElapsedFromStart_sec(void);

	double getElapsedFromTimeout_sec(void);

protected:
	double duration_sec;
	bool is_timeout;
};

class FrequencyEstimator
{
public:
	FrequencyEstimator(const double& sampling_period = 0.5);

	virtual ~FrequencyEstimator();

	double update(const unsigned int& count=1);

	inline void setSamplingPeriod(const double& sampling_period) { sampler.setTimeoutDuration(sampling_period); }

	inline double getSamplingPeriod(void) const { return sampler.getTimeoutDuration(); }

	inline double getFrequency() const { return this->frequency; }

protected:
	ClockTimeout sampler;
	int event_count;
	double frequency;
};

#endif

