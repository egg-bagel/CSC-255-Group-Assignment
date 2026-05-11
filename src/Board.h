#pragma once
#include "Ship.h"
#include <vector>
#include <string>
#include <random>

// Represents one player's 10x10 grid.
// Person A is responsible for implementing Board.cpp.

// Result returned by receiveAttack() so Game can react appropriately.
enum class AttackResult {
    MISS,
    HIT,
    SUNK,
    ALREADY_ATTACKED
};

class Board {
public:
    static const int SIZE = 10; // Grid is SIZE x SIZE

    Board() {
    	for (int row = 0; row < SIZE; row++)
    	{
    		for (int col = 0; col < SIZE; col++)
    		{
    			grid_[row][col] = '~';
    			attacked_[row][col] = false;
    		}
    	}
    }

    // --- Setup ---

    // Attempt to place a ship manually (human player).
    // `startRow`, `startCol`: top-left origin of the ship.
    // `horizontal`: true = place left-to-right, false = place top-to-bottom.
    // Returns false if placement is out of bounds or overlaps another ship.
    bool placeShip(Ship& ship, int startRow, int startCol, bool horizontal) {


    	// Compare startRow and startCol plus the direction to ship.getCoordinates
    	if (horizontal == true)
    	{
    		// Check the ship length. If startCol + ship.getSize() > 9, it is out of bounds.
    		if ((startCol + ship.getSize() - 1) > 9)
    		{
    			return false;  // out of bounds
    		}

    		// Check every cell in the proposed ship footprint.
    		//
    		for (int c = startCol; c < startCol + ship.getSize(); c++)
    		{
    			if (grid_[startRow][c] == '~')
    			{
    				continue;
    			}
    			else
    			{
    				return false;
    			}
    		}
    	}
    	else
    	{
    		if ((startRow + ship.getSize() - 1) > 9)
    		{
    			return false;  // out of bounds
    		}

    		// Check every cell in the proposed ship footprint.
    		//
    		for (int r = startRow; r < startRow + ship.getSize(); r++)
    		{
    			if (grid_[r][startCol] == '~')
    			{
    				continue;
    			}
    			else
    			{
    				return false;
    			}
    		}
    	}

    	// If all the tests above are passed, we need to do ship.place() with a vector of coordinates.
    	std::vector<Coordinate> shipCoords;
    	if (horizontal)
    	{
    		for (int c = startCol; c < (startCol + ship.getSize()); c++)
    		{
    			Coordinate coord;
    			coord.row = startRow;
    			coord.col = c;
    			shipCoords.push_back(coord);
    		}
    	}
    	else
    	{
    		for (int r = startRow; r < (startRow + ship.getSize()); r++)
    		{
    			Coordinate coord;
    			coord.row = r;
    			coord.col = startCol;
    			shipCoords.push_back(coord);
    		}
    	}

    	ship.place(shipCoords);

    	// update grid
    	for (const auto& coord : shipCoords) {
    	    grid_[coord.row][coord.col] = 'S';
    	}

    	ships_.push_back(&ship);  // add ship to vector

    	return true;
    }

    // Place all ships randomly using the provided RNG engine.
    // Uses std::mt19937 passed in from AIPlayer (or for human auto-place).
    // Call once per ship; retries internally until a valid position is found.
    void placeShipRandomly(Ship& ship, std::mt19937& rng) {
    	std::uniform_int_distribution<int> dist(0, 9);
    	std::uniform_int_distribution<int> boolDist(0, 1);

    	int row = dist(rng);
    	int col = dist(rng);
    	bool horizontal = boolDist(rng);

    	while (!placeShip(ship, row, col, horizontal))
    	{
    		row = dist(rng);
    		col = dist(rng);
    		horizontal = boolDist(rng);
    	}
    }

    // --- Gameplay ---

    // Process an incoming attack at (row, col).
    // Updates the display grid and delegates hit-checking to ships.
    // Returns an AttackResult so the caller knows what happened.
    AttackResult receiveAttack(int row, int col) {
    	if (attacked_[row][col] == true)
    	{
    		return AttackResult::ALREADY_ATTACKED;
    	}

    	attacked_[row][col] = true;

    	for (Ship* ship : ships_)
    	{
    		for (const Coordinate& c : ship->getCoordinates())
    		{
    			if ((row == c.row) && (col == c.col))
    			{
    				grid_[row][col] = 'X';
    				ship->checkHit(row, col);
    				if (ship->isSunk())
    				{
    					return AttackResult::SUNK;
    				}
    				else
    				{
    					return AttackResult::HIT;
    				}
    			}
    		}
    	}
    	grid_[row][col] = 'O';
    	return AttackResult::MISS;
    }

    // Returns true when every ship on this board has been sunk.
    bool allShipsSunk() const {
    	for (Ship* ship : ships_)
    	{
    		if (!ship->isSunk())
    		{
    			return false;
    		}
    	}
    	return true;
    }

    // --- Display ---

    // Print the board to stdout.
    // If hideShips is true, ship cells appear as water (used for opponent's board view).
    void display(bool hideShips) const
    {
    	std::cout << "  A B C D E F G H I J\n";
		for (int i = 0; i < SIZE; i++)
		{
			std::cout << i + 1 << " ";
			for (int j = 0; j < SIZE; j++)
			{
				char cell = grid_[i][j];
				if (hideShips && cell == 'S')
				{
					cell = '~';
				}
				std::cout << cell << ' ';
			}
			std::cout << '\n';
		}
    }

    // --- Helpers ---

    // Returns true if (row, col) is within bounds.
    static bool inBounds(int row, int col) {
    	if ((row >= 0) && (row < SIZE))
    	{
    		if ((col >= 0) && (col < SIZE))
    		{
    			return true;
    		}
    	}
    	return false;
    }

    // Convert a column index (0–9) to a letter ('A'–'J') for display.
    static char colToLetter(int col) {
    	return 'A' + col;
    }

    // Convert a letter ('A'–'J', case-insensitive) to a column index (0–9).
    // Returns -1 if the character is invalid.
    static int letterToCol(char letter) {
    	letter = toupper(letter);

    	if ((letter >= 'A') && (letter <= 'J'))
    	{
    		return letter - 'A';
    	}
    	return -1;
    }

private:
    // Grid symbols:
    //   '~' = untouched water
    //   'S' = ship (only shown when hideShips == false)
    //   'X' = hit
    //   'O' = miss
    char grid_[SIZE][SIZE];

    // Tracks which cells have already been attacked (prevents re-attacking).
    bool attacked_[SIZE][SIZE];

    // All ships placed on this board.
    std::vector<Ship*> ships_;
};
