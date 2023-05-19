#include <HalonMTA.h>
#include <map>
#include <list>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <chrono>

bool sstop = false;
std::mutex smutex;
std::condition_variable scv;
std::thread sthread;
std::map<std::chrono::duration<double>, std::list<HalonHSLContext*>> sindex;

HALON_EXPORT
int Halon_version()
{
	return HALONMTA_PLUGIN_VERSION;
}

HALON_EXPORT
void sleep_background(HalonHSLContext* hhc, HalonHSLArguments* args, HalonHSLValue* ret)
{
	HalonHSLValue* x;
	double seconds;

	x = HalonMTA_hsl_argument_get(args, 0);
	if (x && HalonMTA_hsl_value_type(x) == HALONMTA_HSL_TYPE_NUMBER)
		HalonMTA_hsl_value_get(x, HALONMTA_HSL_TYPE_NUMBER, &seconds, nullptr);
	else
		return;

	auto ntime = std::chrono::steady_clock::now().time_since_epoch();
	smutex.lock();
	sindex[ntime + std::chrono::duration<double>(seconds)].push_back(hhc);
	smutex.unlock();
	scv.notify_one();
	HalonMTA_hsl_suspend(hhc);
	
	double slept = std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch() - ntime).count();
	HalonMTA_hsl_value_set(ret, HALONMTA_HSL_TYPE_NUMBER, &slept, 0);
}

HALON_EXPORT
bool Halon_hsl_register(HalonHSLRegisterContext* ptr)
{
	HalonMTA_hsl_module_register_function(ptr, "sleep_background", &sleep_background);
	return true;
}

HALON_EXPORT
bool Halon_init(HalonInitContext* hic)
{
	sthread = std::thread([]() {
    	std::unique_lock<std::mutex> lk(smutex);
		while (!sstop)
		{
			if (sindex.empty())
				scv.wait(lk);
			else
			{
				auto ctime = std::chrono::steady_clock::now().time_since_epoch();
				while (!sindex.empty() && sindex.begin()->first <= ctime)
				{
					for (const auto & hhc : sindex.begin()->second)
						HalonMTA_hsl_schedule(hhc);
					sindex.erase(sindex.begin());
				}
				if (!sindex.empty())
					scv.wait_for(lk, sindex.begin()->first - ctime); 
			}
		}
	});
	return true;
}

HALON_EXPORT
void Halon_cleanup()
{
	smutex.lock();
	sstop = true;
	smutex.unlock();
	scv.notify_one();
	sthread.join();
}