#include"mapGenerator.hpp"
#include<stack>


MapGenerator::MapGenerator(int startX, int startY, int width, int height, std::string& tiles) :
	m_startX(startX), m_startY(startY), m_width(width), m_height(height), m_tiles(tiles), m_finish(m_startX, m_startY)
{
	m_tiles.assign(width * height, 'w');
	std::random_device rd;
	m_rand_gen.seed(rd());
	m_active = true;

	for (int i = 0; i < m_height; ++i)
	{
		if (i == 0 || i == m_height - 1)
			for (int c = 0; c < m_width; ++c)
				m_tiles.at(i * m_width + c) = 'b';
		else
		{
			m_tiles.at(i * m_width + 0) = 'b';
			m_tiles.at(i * m_width + m_width - 1) = 'b';
		}
	}

	m_tiles.at(m_startX + m_width * m_startY) = ' ';
	
};

bool MapGenerator::generate_map()
{
	if (!m_active || m_done)
		return false;
	while(generate_map_step()){}
	return true;
}

bool MapGenerator::generate_map_step()
{
	if (!m_active || m_done || (m_width < 2 && m_height < 2))
		return false;
	
	auto not_OOB = [this](int index) -> bool
	{
		return (index < m_tiles.size() && index >= 0);
	};
	auto drawpath = [this](int indexS, int indexF)
	{
		m_tiles.at(indexF) = ' ';
		m_tiles.at(indexS + (indexF - indexS) / 2) = ' ';
	};
	auto checkWalkable = [this](int indexS, int indexF) -> bool
	{
		return (m_tiles.at(indexF) == 'w' &&
			m_tiles.at(indexS + (indexF - indexS) / 2) == 'w');
	};

	int startI = m_startX + m_width * m_startY;
	int dirs[4] = { startI - m_width * 2, startI - 2, startI + m_width * 2, startI + 2 };
	bool move = false;
	for (int i = 0; i < 4; ++i)
	{
		if (not_OOB(dirs[i]) && checkWalkable(startI, dirs[i]))
			move = true;
	}
	if (move)
	{
		move = false;
		do
		{
			int i = m_distr_0_3(m_rand_gen);
			if (not_OOB(dirs[i]) && checkWalkable(startI, dirs[i]))
			{
				m_stack.push({ m_startX, m_startY });
				drawpath(startI, dirs[i]);
				m_startX = dirs[i] % m_width;
				m_startY = dirs[i] / m_width;
				m_currentDist += 1;
				if (m_currentDist > m_finishDist)
				{
					m_finish.first = m_startX;
					m_finish.second = m_startY;
					m_finishDist = m_currentDist;
				}
				move = true;
			}

		} while (move == false);
	}
	else
	{
		m_currentDist--;
		std::pair<int,int> previus = m_stack.top();
		m_stack.pop();
		m_startX = previus.first;
		m_startY = previus.second;
	}

	if (m_stack.empty())
	{
		m_tiles.at(m_finish.first + m_width * m_finish.second) = 'g';
		m_active = false;
		m_done = true;
		//std::cout << std::endl;
		//for (int i = 0; i < m_height; ++i)
		//{
		//	for (int c = 0; c < m_width; ++c) {
		//		std::cout << m_tiles.at(i*m_width + c) << " ";
		//	}
		//	std::cout << std::endl;
		//}
	}
	return true;
}