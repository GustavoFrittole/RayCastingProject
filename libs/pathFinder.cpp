#include"pathFinder.hpp"
#include<stack>

PathFinder::PathFinder(int width, int height, const std::string& tiles, std::vector<std::pair<int, int>>& vec) :
	m_width(width), m_height(height), m_tiles(tiles), m_solVec(vec) {}

bool PathFinder::find_path(int startX, int startY)
{
	m_solVec.clear();
	m_solVec.push_back({ startX, startY });
	std::set<std::pair<int, int>> visitedTiles;
	visitedTiles.insert({ startX, startY });
	return recursive_dfs(visitedTiles);
}

bool PathFinder::recursive_dfs(std::set<std::pair<int, int>>& visitedTiles)
{
	
	int x = m_solVec.back().first, y = m_solVec.back().second;
	std::pair<int, int> options[] = { { x + 1, y }, { x, y + 1 }, { x - 1, y }, { x, y - 1 } };
	for (auto p : options)
	{
		if (!check_is_oob(p.first, p.second) && visitedTiles.find(p) == visitedTiles.end())
		{
			switch (m_tiles.at(p.first + p.second * m_width))
			{
			case ' ':
				m_solVec.push_back({ p.first, p.second });
				visitedTiles.insert(p);
				if (recursive_dfs(visitedTiles))
					return true;
				break;
			case 'g':
				m_solVec.push_back({ p.first, p.second });
				return true;
				break;
			default:
				break;
			}
		}
	}
	m_solVec.pop_back();
	return false;
}

bool PathFinder::check_is_oob(int x, int y)
{
	return (x < 0 || y < 0 || x >= m_width || y >= m_height);
}
