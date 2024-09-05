#include"rendThreadPool.hpp"

//---------------------thread-pool--------------------------

RendThreadPool::RendThreadPool(size_t num_threads) :
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

bool RendThreadPool::new_batch(int jobs)
{
    if (is_busy())
        return false;
    m_jobs.store(jobs);
    return true;
}

void RendThreadPool::enqueue(IRenderingSection* section)
{
    {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_tasks.push(section);
    }
    m_cv.notify_one();
}

RendThreadPool::~RendThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_stop = true;
    }

    m_jobs.store(m_jobs.load() + m_threads.size());
    m_cv.notify_all();

    for (auto& thread : m_threads)
    {
        thread.join();
    }
}

//---------------------helpers--------------------------

IRenderingSectionFactory::IRenderingSectionFactory(int taskNumber, int workers) :
    m_taskNumber(taskNumber),
    m_workersNumber(workers),
    m_sectionSize(m_taskNumber / m_workersNumber),
    m_remainder(m_taskNumber % m_workersNumber)
{}

void IRenderingSectionFactory::set_task_number(int taskNumber)
{
    m_taskNumber = taskNumber;
    m_sectionSize = m_taskNumber / m_workersNumber;
    m_remainder = m_taskNumber % m_workersNumber;
}

int IRenderingSectionFactory::get_section(int index) const
{
    if (index >= 0 && index < m_workersNumber)
        if (index == m_workersNumber - 1)
            return m_sectionSize + m_remainder;
        else
            return m_sectionSize;
    else
        throw std::invalid_argument("Rendering section selected is out of range.");
}