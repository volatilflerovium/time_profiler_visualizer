/*********************************************************************
* TimeProfiler is a class for profiling elapsed time.                *
*                                                                    *
* The class will generate a .js file containing the samples of elapsed
* time measured which can be loaded into the companion appImage      *
* (https://github.com/volatilflerovium/time_profiler_visualizer/blob/main/app/wxElapsedTimeVisualizer-x86_64.AppImage)
* which will plot those sample in a line chart.                      *
*                                                                    *
* Version: 1.0                                                       *
* Date:    18-10-2025                                                *
* Author:  Dan Machado                                               *
**********************************************************************/
#ifndef TIME_PROFILER_H
#define TIME_PROFILER_H

#include <fstream>
#include <cstring>
#include <ctime>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>

#ifndef ENABLE_STOPWATCH
	#ifdef DEBUG
		#define ENABLE_STOPWATCH
	#endif
#endif

//====================================================================

namespace profiler
{
	inline namespace internal
	{
		template<typename T>
		struct TimeType
		{
			enum {value=false};
		};

		template<>
		struct TimeType<std::chrono::seconds>
		{
			enum {value=true};
			static constexpr const char* timeUnit="secs";
		};

		template<>
		struct TimeType<std::chrono::milliseconds>
		{
			enum {value=true};
			static constexpr const char* timeUnit="ms";
		};

		template<>
		struct TimeType<std::chrono::microseconds>
		{
			enum {value=true};
			static constexpr const char* timeUnit="μs";
		};

		template<>
		struct TimeType<std::chrono::nanoseconds>
		{
			enum {value=true};
			static constexpr const char* timeUnit="ns";
		};

		inline std::string setFileName(const char* outputDir, const char* name, const char* prefix)
		{
			std::srand(static_cast<unsigned int>(time(0)));
			std::string filePath=outputDir;
			if(filePath.length()>0){
				filePath.append("/");
			}
			filePath.append(prefix);
			filePath.append(name);
			filePath.append(std::to_string(10+(rand()%90)));
			char timeString[32];
			std::time_t time = std::time({});
			std::memset(timeString, 0, 32);
			std::strftime(timeString, 31, "_%y%m%d%H%M%S", std::gmtime(&time));
			filePath.append(timeString);
			filePath.append(".js");
			return filePath;
		}

		template<typename TM, bool=profiler::TimeType<TM>::value>
		class TimeProfiler
		{};
	}

//====================================================================

/*
 * Example:
 * 
 * profiler::TimeProfiler<std::chrono::microseconds> timeProfiler("someName", "#colour").
 * 
 * example 1)
 * 
 * do_loop{
 * 	do something
 * 
 *    timeProfiler.start()
 *    do something else
 *    timeProfiler.takeSample(true)    // here we capture individual samples
 *  
 *   do more stuff
 * 
 * }
 * 
 * example 2)
 * 
 * do_loop{
 * 	do something
 * 
 *    timeProfiler.start()
 *    do something else
 *    timeProfiler.pause()    // here we pause the clock
 *  
 *   do more stuff
 * 
 * }
 * 
 * timeProfiler.takeSample(true); // here we capture the average of the elapsed time
 *                        // of the do_something_else task
 * 
 * */

//====================================================================

template<typename TM>
class TimeProfiler<TM, true>
{
	public:
		/*
		 * Constructor
		 * 
		 * @param name a string to identify the dataset
		 * @param colour the colour for the dataset as it is ploted the 
		 *        elapsed time visualizer app
		 * @param outputDir path to the directory where the js with the dataset
		 *        file will be created. Default will be the directory where the executable 
		 *        is being called.
		 * */
		TimeProfiler([[maybe_unused]] const char* name, [[maybe_unused]] const char* colour, [[maybe_unused]] const char* outputDir="")
		{
			#ifdef ENABLE_STOPWATCH
			m_buffer.reserve(64);
			m_outputFile.open(setFileName(outputDir, name, "line_dataset_"));
			if(m_outputFile.is_open()){
				m_outputFile<<"{\"dataSet\" : [\n";
				m_outputFile<<"{\"name\": "<<"\""<<name<<"\", \"color\": \""<<colour;
				m_outputFile<<"\", \"data\":[";
			}
			#endif
		}
		
		~TimeProfiler()
		{
			flush();
		}

		/*
		 * Start the internal stopwatch.
		 * 
		 * */
		void start()
		{
			#ifdef ENABLE_STOPWATCH
			m_isInitialized=true;
			m_startPoint=std::chrono::high_resolution_clock::now();
			#endif
		}

		/*
		 * Stop the clock at the point of calling 
		 * 
		 * @param print if true, it will print the elapsed time to standard output.
		 * 
		 * */
		void takeSample([[maybe_unused]] bool print=false)
		{
			#ifdef ENABLE_STOPWATCH
			if(!m_isInitialized && m_count==0){
				std::cout<<"Timer did not start."<<'\n';
				return;
			}// */

			if(m_count==0){
				m_partial=elapsedTime();
			}
			
			if(print){
				std::cout<<"Elapsed time:"<<m_partial<<" "<<TimeType<TM>::timeUnit<<"\n";
			}
			m_buffer.push_back(static_cast<double>(m_partial));
			m_total=m_total+m_partial;
			m_partial=0;
			m_count=0;			
			m_isInitialized=false;
			#endif
		}
		 
		 /*
		 * Use in tandem with pause. Will take the current elapsed time 
		 * and average on the times pause was called. This reset 
		 * current elapsed time and counters.
		 * 
		 * @param print if true, it will print the elapsed time to standard output. 
		 * 
		 * */		
		void takeAverageSample([[maybe_unused]] bool print=false)
		{
			#ifdef ENABLE_STOPWATCH
			if(m_count==0){
				std::cout<<"use pause() to capture elapsed times\n";
				return;
			}

			double averageTime=static_cast<double>(m_partial)/static_cast<double>(m_count);
			m_buffer.push_back(averageTime);
			
			m_count=0;

			if(print){
				std::ios_base::fmtflags f(std::cout.flags());
				std::cout<<"Average elapsed time: ";
				std::cout<<std::fixed<<std::setprecision(3)<<averageTime<<TimeType<TM>::timeUnit<<std::endl;
				std::cout.flags(f);
			}

			m_total=m_total+m_partial;
			m_partial=0;
			m_isInitialized=false;
			#endif
		}

		/*
		 * Stop the clock at the time of calling and adding it to the current
		 * elapsed time.
		 * 
		 * */
		void pause()
		{
			#ifdef ENABLE_STOPWATCH
			if(m_isInitialized){
				m_partial=m_partial+elapsedTime();
				m_count++;
			}
			else{
				std::cout<<"Timer did not start."<<'\n';
			}
			m_isInitialized=false;
			#endif
		}

		void totalTime() const
		{
			#ifdef ENABLE_STOPWATCH
			std::cout<<m_total<<TimeType<TM>::timeUnit<<std::endl;
			#endif
		}

		/*
		 * Reset the current elapsed time, counters.
		 *  
		 * */
		void reset()
		{
			#ifdef ENABLE_STOPWATCH
			m_isInitialized=false;
			m_total=0;
			m_partial=0;
			m_count=0;
			m_buffer.clear();
			#endif
		}	

	private:
		mutable std::vector<double> m_buffer{};
		std::ofstream m_outputFile{};
		
		std::chrono::system_clock::time_point m_startPoint{};
		long long m_total{0};
		long long m_partial{0};
		long long m_count{0};
		bool m_isInitialized{false};

		long long elapsedTime() __attribute__((always_inline))
		{
			auto elapsed = std::chrono::high_resolution_clock::now() - m_startPoint;
			return std::chrono::duration_cast<TM>(elapsed).count();
		}
		
		/*
		 * Force to dump the dataset. This method is called by the destructor.
		 *
		 * */
		void flush();		
};

//--------------------------------------------------------------------

template<typename TM>
void TimeProfiler<TM, true>::flush()
{
	#ifdef ENABLE_STOPWATCH
	if(m_outputFile.is_open()){
		bool a=false;
		for(double data : m_buffer){
			if(a){
				m_outputFile<<", ";
			}
			m_outputFile<<data;
			a=true;
		}
		m_outputFile<<"]}\n";

		m_outputFile<<"], \"timeUnits\": \""<<TimeType<TM>::timeUnit<<"\"}\n";
		m_outputFile.flush();
		m_outputFile.close();
	}

	reset();
	#endif
}

//====================================================================

}

#endif
