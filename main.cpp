
#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace
{
	struct point_t
	{
		int x, y;
	};

	struct grid_t
	{
		enum cell_t
		{
			empty,
			wall,
		};

		int width, height;

		std::vector<cell_t> cells;

		point_t start;
		point_t target;

		int index_of(point_t const & point) const { return point.x + point.y * width; }
	};

	std::vector<std::string> read_lines(std::istream & is)
	{
		std::vector<std::string> lines;

		do
		{
			lines.emplace_back();
		}
		while (std::getline(is, lines.back()));

		lines.pop_back();

		return lines;
	}

	bool read_grid(std::istream & is, grid_t & grid)
	{
		std::vector<std::string> lines = read_lines(is);

		grid.height = lines.size();
		grid.width = std::accumulate(lines.begin(), lines.end(), 0, [](std::size_t width, auto const & line){ return std::max(width, line.size()); });

		grid.cells.resize(grid.width * grid.height, grid_t::empty);
		for (int y = 0; y < lines.size(); y++)
		{
			for (int x = 0; x < lines[y].size(); x++)
			{
				int const grid_index = grid.index_of({x, y});
				switch (lines[y][x])
				{
				case '#':
					grid.cells[grid_index] = grid_t::wall;
					break;
				case ' ':
					grid.cells[grid_index] = grid_t::empty;
					break;
				case 's':
					grid.cells[grid_index] = grid_t::empty;
					grid.start = point_t{x, y};
					break;
				case 't':
					grid.cells[grid_index] = grid_t::empty;
					grid.target = point_t{x, y};
					break;
				default:
					std::cerr << "invalid character at (" << x << ", " << y << ")\n";
					return false;
				}
			}
		}
		return true;
	}

	std::vector<point_t> compute_path(grid_t const & grid)
	{
		// naive solution, make a better one!
		std::vector<point_t> path;
		for (int x = grid.start.x; x < grid.width; x++)
		{
			path.push_back({x, grid.start.y});
		}
		return path;
	}

	struct intersection_t
	{
		enum : uint8_t { up = 1, left = 2, right = 4, down = 8, visited = 16 };

		uint8_t value = 0;
	};

	std::vector<intersection_t> compute_intersections(grid_t const & grid, std::vector<point_t> const & path)
	{
		std::vector<intersection_t> intersections(grid.width * grid.height);
		int prev_grid_index = -(grid.width + 1); // guaranteed not to be adjacent
		for (auto const & point : path)
		{
			bool const inside_grid =
				0 <= point.x && point.x < grid.width &&
				0 <= point.y && point.y < grid.height;
			if (!inside_grid)
			{
				prev_grid_index = -(grid.width + 1);
				continue;
			}

			int const grid_index = grid.index_of(point);
			intersections[grid_index].value = intersection_t::visited;

			if (grid_index == prev_grid_index - grid.width)
			{
				intersections[grid_index].value |= intersection_t::down;
				intersections[prev_grid_index].value |= intersection_t::up;
			}
			else if (grid_index == prev_grid_index - 1 && point.x < grid.width - 1)
			{
				intersections[grid_index].value |= intersection_t::right;
				intersections[prev_grid_index].value |= intersection_t::left;
			}
			else if (grid_index == prev_grid_index + 1 && point.x > 0)
			{
				intersections[grid_index].value |= intersection_t::left;
				intersections[prev_grid_index].value |= intersection_t::right;
			}
			else if (grid_index == prev_grid_index + grid.width)
			{
				intersections[grid_index].value |= intersection_t::up;
				intersections[prev_grid_index].value |= intersection_t::down;
			}
			prev_grid_index = grid_index;
		}
		return intersections;
	}

	void print_path(std::ostream & os, grid_t const & grid, std::vector<point_t> const & path)
	{
		std::vector<intersection_t> const intersections = compute_intersections(grid, path);
		for (int y = 0; y < grid.height; y++)
		{
			for (int x = 0; x < grid.width; x++)
			{
				int const grid_index = grid.index_of({x, y});
				switch (grid.cells[grid_index])
				{
				case grid_t::empty:
					if (grid.start.x == x && grid.start.y == y)
					{
						os << 's';
					}
					else if (grid.target.x == x && grid.target.y == y)
					{
						os << 't';
					}
					else
					{
						char const intersection_graphics[] = {
							' ', '?', '?', '?',
							'?', '?', '?', '?',
							'?', '?', '?', '?',
							'?', '?', '?', '?',

							'o', '|', '-', '/',
							'-', '\\', '-', '?',
							'|', '|', '\\', '?',
							'/', '?', '?', '?',
						};
						os << intersection_graphics[intersections[grid_index].value];
					}
					break;
				case grid_t::wall:
					os << (intersections[grid_index].value == 0 ? '#' : 'X');
					break;
				}
			}
			os << '\n';
		}
	}
}

int main(int argc, char const * const * const argv)
{
	grid_t grid;

	if (argc < 2)
	{
		if (!read_grid(std::cin, grid))
			return -1;
	}
	else
	{
		std::ifstream is(argv[1]);
		if (!is)
		{
			std::cerr << "input file not valid\n";
			return -1;
		}

		if (!read_grid(is, grid))
			return -1;
	}

	std::vector<point_t> path = compute_path(grid);

	print_path(std::cout, grid, path);

	return 0;
}
