#include"mapGenerator.hpp"
#include<random>
#include<stack>
#include <iostream>

struct pos
{
	int x = 0, y = 0;
};

void MapGenerator::generate_map(int startX, int startY, int width, int height, std::string& tiles)
{
	tiles.assign(width * height, 'w');
	std::random_device rd;
	std::mt19937 gen(2);
	std::uniform_int_distribution<> distr(0, 3);
	std::stack<pos> stack;
	pos finish{ startX,startY };
	int finishDist = 0;
	int currentDist = 0;

	for (int i = 0; i < height; ++i)
	{
		if (i == 0 || i == height - 1)
			for (int c = 0; c < height; ++c)
				tiles.at(i * width + c) = 'e';
		else
		{
			tiles.at(i * width + 0) = 'e';
			tiles.at(i * width + width - 1) = 'e';
		}
		std::cout << std::endl;
	}

	tiles.at(startX + width * startY) = ' ';

	auto not_OOB = [&tiles](int index) -> bool
		{
			return (index < tiles.size() && index >= 0);
		};
	auto drawpath = [&tiles](int indexS, int indexF)
		{
			tiles.at(indexF) = ' ';
			tiles.at(indexS + (indexF - indexS) / 2) = ' ';
		};
	auto checkWalkable = [&tiles](int indexS, int indexF) -> bool
		{
			return (tiles.at(indexF) == 'w' &&
				tiles.at(indexS + (indexF - indexS) / 2) == 'w');
		};
	do
	{
		int startI = startX + width * startY;
		int dirs[4] = { startI - width * 2, startI - 2, startI + width * 2, startI + 2 };
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
				int i = distr(gen);
				if (not_OOB(dirs[i]) && checkWalkable(startI, dirs[i]))
				{
					stack.push({ startX, startY });
					drawpath(startI, dirs[i]);
					startX = dirs[i] % width;
					startY = dirs[i] / width;
					currentDist += 1;
					if (currentDist > finishDist)
					{
						finish.x = startX;
						finish.y = startY;
						finishDist = currentDist;
					}
					move = true;
				}

			} while (move == false);
		}
		else
		{
			currentDist--;
			pos previus = stack.top();
			stack.pop();
			startX = previus.x;
			startY = previus.y;
		}

	} while (!stack.empty());

	tiles.at(finish.x + width * finish.y) = 'g';

	// for (int i = 0; i < height; ++i)
	// {
	// 	for (int c = 0; c < height; ++c)
	// 	{
	// 		std::cout << tiles.at(i * width + c) << " ";
	// 	}
	// 	std::cout << std::endl;
	// }
}