#ifndef MAPGENERATOR_HPP
#define MAPGENERATOR_HPP
#include<string>
#include<random>
#include<stack>
class MapGenerator
{
public:
	MapGenerator() = delete;
	MapGenerator(int, int, int, int, std::string&);

	bool generate_map();
	bool generate_map_step();
	bool is_active() const { return m_active; };
	bool is_done() const { return m_done; };
private:
	int m_startX;
	int m_startY;
	int m_width;
	int m_height;
	std::string& m_tiles;
	std::mt19937 m_rand_gen;
	std::stack<std::pair<int, int>> m_stack;
	std::uniform_int_distribution<> m_distr_0_3 = std::uniform_int_distribution<>(0, 3);
	std::pair<int, int> m_finish;
	int m_finishDist = 0;
	int m_currentDist = 0;
	bool m_active = false, m_done = false;
};

#endif