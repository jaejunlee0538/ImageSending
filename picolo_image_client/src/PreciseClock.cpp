/*
made by jaejunlee, 2015.10.23
*/
#include <picolo_image_client/PreciseClock.h>

PreciseClock::PreciseClock()
{
#if defined(WIN32) | defined(WIN64)
	LARGE_INTEGER freq;
	if ( !QueryPerformanceFrequency(&freq) )
		throw "QueryPerformanceFrequency() failed!";
	this->freq_coeff = 1.0 / freq.QuadPart;
#endif
	this->reset();
}

void PreciseClock::reset(void)
{
	this->t_start = this->getCPUClock_sec();
	this->t_current = this->t_start;
}

double PreciseClock::getElapsedTime_sec(void)
{
	this->t_current = this->getCPUClock_sec();
	return (this->t_current - this->t_start);
}

double PreciseClock::getCPUClock_sec() const 
{
#if defined(WIN32) | defined(WIN64)
	LARGE_INTEGER current_time;
	QueryPerformanceCounter(&current_time);
	return current_time.QuadPart * freq_coeff;
#endif

#if defined(__linux__)
	timeval current_time;
	gettimeofday(&current_time, 0);
	return current_time.tv_sec + 0.000001*current_time.tv_usec;
#endif
}

ClockTimeout::ClockTimeout(const double& timeout_sec)
{
	this->duration_sec = timeout_sec;
	this->resetTimeout();
}

ClockTimeout::~ClockTimeout()
{

}

void ClockTimeout::resetTimeout(bool precision_offset)
{
	if(precision_offset && this->is_timeout){
		double offset = this->getElapsedFromTimeout_sec();
		this->reset();
		this->t_start += offset;
	}
	else{
		this->reset();	
	}

	this->is_timeout = false;
}


void ClockTimeout::setTimeoutDuration(const double& timeout_sec)
{
	this->duration_sec = timeout_sec;
}

bool ClockTimeout::isTimeout(void)
{
	if(!this->is_timeout && this->getElapsedTime_sec() > duration_sec)
		this->is_timeout = true;
	return this->is_timeout;
}

double ClockTimeout::getElapsedFromStart_sec(void)
{
	return this->getElapsedTime_sec();
}

double ClockTimeout::getElapsedFromTimeout_sec(void)
{
	return this->getElapsedTime_sec() - this->duration_sec;
}


FrequencyEstimator::FrequencyEstimator(const double& sampling_period)
	:sampler(sampling_period)
{
	this->event_count = 0;
	this->frequency = 0.0;
}

FrequencyEstimator::~FrequencyEstimator(){}

double FrequencyEstimator::update(const unsigned int& count)
{
	event_count += count;
	if(sampler.isTimeout()){
		frequency = event_count / sampler.getElapsedFromStart_sec();
		sampler.resetTimeout(false); //Do not use precision-offset of timeout.
		event_count = 0;
	}
	return frequency;
}