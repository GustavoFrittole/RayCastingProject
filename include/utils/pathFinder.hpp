#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <string>
#include <vector>
#include <set>

class PathFinder
{
public:
	PathFinder() = delete;
	PathFinder(int,int, const std::string& tiles, std::vector<std::pair<int,int>>&);
	bool find_path(int,int);
private:
	int m_width;
	int m_height;
	char m_walkable = ' ';
	char m_goal= 'g';
	const std::string& m_tiles;
	std::vector<std::pair<int, int>>& m_solVec;

	bool recursive_dfs(std::set<std::pair<int, int>>&);
	bool check_is_oob(int, int);
};

#endif