#include"rendThreadPool.hpp"

IRenderingSectionFactory::IRenderingSectionFactory(int taskNumber, int workers) :
    m_taskNumber(taskNumber),
    m_workersNumber(workers),
    m_sectionSize(m_taskNumber / m_workersNumber),
    m_remainder(m_taskNumber % m_workersNumber)
{}

int IRenderingSectionFactory::get_section_size(int index) const
{
    if (index >= 0 && index < m_workersNumber)
        if (index == m_workersNumber - 1)
            return m_sectionSize + m_remainder;
        else
            return m_sectionSize;
    else
        throw std::invalid_argument("Rendering section selected is out of range.");
}