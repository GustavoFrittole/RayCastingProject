#ifndef RENDTHREADPOOL_HPP
#define RENDTHREADPOOL_HPP

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <atomic>

class IRenderingSection
{
public:
	IRenderingSection() = default;
	IRenderingSection(int start, int end) :
		m_start(start),
		m_end(end) {}
	virtual void operator()() const = 0;
protected:
	int m_start = 0, m_end = 0;
};

class IRenderingSectionFactory
{
    public:
		IRenderingSectionFactory(int taskNumber, int workers);
		void set_task_number(int taskNumber);
		int get_size() { return (m_workersNumber); }
    protected:
        int m_taskNumber = 0, m_workersNumber = 0, m_sectionSize = 0, m_remainder = 0;
		int get_section(int index) const;
};

class RendThreadPool
{
public:
	RendThreadPool(size_t num_threads = std::thread::hardware_concurrency()) :
		m_jobs(num_threads)
	{
		for (size_t i = 0; i < num_threads; ++i)
		{
			m_threads.emplace_back([this]
								  { 
                while (true) { 
                    IRenderingSection* task; 
                    { 
                        std::unique_lock<std::mutex> lock( 
                            m_queue_mutex); 

						m_jobs--;

                        m_cv.wait(lock, [this] { 
                            return (!m_tasks.empty() && m_jobs > 0) || m_stop; 
                        });  
  
                        if (m_stop && m_tasks.empty()) { 
                            return; 
                        }
  
                        task = m_tasks.front(); 
                        m_tasks.pop(); 
                    } 
  
					(*(task))();
                } 
			});
		}
	}

	bool is_busy()
	{
		return (m_jobs > 0);
	}

	bool new_batch(int jobs)
	{
		if (is_busy())
			return false;
		m_jobs.store(jobs);
		return true;
	}

	~RendThreadPool()
	{
		{
			std::unique_lock<std::mutex> lock(m_queue_mutex);
			m_stop = true;
		}

		m_jobs.store(m_jobs.load() + m_threads.size());
		m_cv.notify_all();

		for (auto &thread : m_threads)
		{
			thread.join();
		}
	}

	void enqueue(IRenderingSection* section)
	{
		{
			std::unique_lock<std::mutex> lock(m_queue_mutex);
			m_tasks.push(section);
		}
		m_cv.notify_one();
	}

	int get_size() const { return m_threads.size(); }

private:
	std::vector<std::thread> m_threads;
	std::queue<IRenderingSection*> m_tasks;
	std::mutex m_queue_mutex;
	std::condition_variable m_cv;
	std::atomic<int> m_jobs = 0;
	bool m_stop = false;
};

#endif