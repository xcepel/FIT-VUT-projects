/*********************************************************************
 * @file  generation.cpp
 * 
 * @brief Simulatation of deforestation in Brazil.
 *
 * This program generates tree loss (deforestation) through the years.
 * 
 * @author Kateřina Čepelková <xcepel03>, Tomáš Ebert <xebert00>
 * @date Last modified on 2023-12-10
 *********************************************************************/

#include "generation.h"
#include "all_matrix.h"

int deforest_count = 2;
int year = 2015;
float old_value = FIRST_FOREST_VALUE;


// Test matrix print
void matrix_print(vector<std::vector<int>>& matrixPrint) {
    for (int row = ROWS-1; row < ROWS; row++) {
		vector<string> values;
		for (int col = 0; col < COLS; col++) {
            values.push_back(to_string(matrixPrint[row][col]));
        }
            // Print the contents of the vector of strings
        for (const std::string& value : values) {
            std::cout << value;
        }
		cout << endl;
    }
}


// Returns true if cell in position(row, col) exists in given matrix
bool is_valid_cell(vector<std::vector<int>>& matrix, int int_row, int int_col) {
    long unsigned int row = static_cast<long unsigned int>(int_row);
    long unsigned int col = static_cast<long unsigned int>(int_col);
	if (row < matrix.size()) {
		if (col < matrix[row].size()) {
			return true;
        }
    }
    return false;
}

// If cell exists in matrix => return its value, otherwise return -1
int number_from_cell(vector<std::vector<int>>& matrix, int row, int col) {
	if (is_valid_cell(matrix, row, col)) {
		return matrix[row][col];
    }
    return -1;
}

// Returns value according to the nearby instance of given type
float type_moore_neighborhood(int row, int col, typeOfLand type) {
	float count = 0;
    vector<std::vector<int>> matrix;
	switch(type) {
			case WATER:
                matrix = matrixWater;
                break;
			case CITY:
                matrix = matrixCities;
                break;
			case ROAD:
                matrix = matrixRoads;
                break;
			case MINE:
                matrix = matrixMines;
                break;
    }

    // Primary neighborhood
	for (int offset : {-1, 1}) {
		if (number_from_cell(matrix, row, col + offset) == 1) { // top + bottom
			count += 1;
        }
		if (number_from_cell(matrix, row + offset, col) == 1) { // left + right
			count += 1;
        }
    }

	if (number_from_cell(matrix, row - 1, col - 1) == 1) { // top left
		count += pow((1/sqrt(2)), 2);
    }
	if (number_from_cell(matrix, row - 1, col + 1) == 1) { // top right
		count += pow((1/sqrt(2)), 2);
    }
	if (number_from_cell(matrix, row + 1, col - 1) == 1) { // bottom left
		count += pow((1/sqrt(2)), 2);
    }
	if (number_from_cell(matrix, row + 1, col + 1) == 1) { // bottom right
		count += pow((1/sqrt(2)), 2);
    }

    // Seconary neighborhood
    if (number_from_cell(matrix, row - 2, col - 2) == 1) { // top left
		count += pow((1/sqrt(8)), 2);
    }
	if (number_from_cell(matrix, row - 2, col + 2) == 1) { // top right
		count += pow((1/sqrt(8)), 2);
    }
	if (number_from_cell(matrix, row + 2, col - 2) == 1) { // bottom left
		count += pow((1/sqrt(8)), 2);
    }
	if (number_from_cell(matrix, row + 2, col + 2) == 1) { // bottom right
		count += pow((1/sqrt(8)), 2);
    }

    if (number_from_cell(matrix, row + 2, col) == 1) {
		count += pow(0.5, 2);
    }
	if (number_from_cell(matrix, row - 2, col) == 1) {
		count += pow(0.5, 2);
    }
	if (number_from_cell(matrix, row, col + 2) == 1) {
		count += pow(0.5, 2);
    }
	if (number_from_cell(matrix, row, col - 2) == 1) {
		count += pow(0.5, 2);
    }

    if (number_from_cell(matrix, row + 2, col - 1) == 1) {
		count += pow((1/sqrt(5)), 2);
    }
	if (number_from_cell(matrix, row + 2, col + 1) == 1) {
		count += pow((1/sqrt(5)), 2);
    }
	if (number_from_cell(matrix, row - 2, col - 1) == 1) {
		count += pow((1/sqrt(5)), 2);
    }
	if (number_from_cell(matrix, row - 2, col + 1) == 1) {
		count += pow((1/sqrt(5)), 2);
    }
    if (number_from_cell(matrix, row - 1, col + 2) == 1) {
		count += pow((1/sqrt(5)), 2);
    }
	if (number_from_cell(matrix, row + 1, col + 2) == 1) {
		count += pow((1/sqrt(5)), 2);
    }
	if (number_from_cell(matrix, row - 1, col - 2) == 1) {
		count += pow((1/sqrt(5)), 2);
    }
	if (number_from_cell(matrix, row + 1, col - 2) == 1) {
		count += pow((1/sqrt(5)), 2);
    }

	return count;
}

// Adiabatic Von Neumann neighborhood
// Return added values for given cell from cells up, down, left, right - if they dont exist adds value from given cell
int von_neumann_neighborhood(vector<std::vector<int>>& matrix, int row, int col) {
	int count = 0;
	for (int offset : {-1, 1}) {
		if (number_from_cell(matrix, row, col + offset) == -1) { // top + bottom
			count += number_from_cell(matrix, row, col);
        } else {
			count += number_from_cell(matrix, row, col + offset);
        }

		if (number_from_cell(matrix, row + offset, col) == -1) { // left + right
			count += number_from_cell(matrix, row, col);
        } else {
			count += number_from_cell(matrix, row + offset, col);
        }
    }
	return count;
}

// Adiabatic Moore neighborhood without Von Neumann neighborhood
// Return added values for given cell from corner neighbors - if they dont exist adds value from given cell
int moore_reduced_neighborhood(vector<std::vector<int>>& matrix, int row, int col) {
	int count = 0;
	if (number_from_cell(matrix, row - 1, col - 1) == -1) { // top left
		count += number_from_cell(matrix, row, col);
    } else {
        count += number_from_cell(matrix, row - 1, col  - 1);
    }
		
	if (number_from_cell(matrix, row - 1, col + 1) == -1) { // top right
		count += number_from_cell(matrix, row, col);
    } else {
		count += number_from_cell(matrix, row - 1, col  + 1);
    }

	if (number_from_cell(matrix, row + 1, col - 1) == -1){ // bottom left
		count += number_from_cell(matrix, row, col);
	} else {
		count += number_from_cell(matrix, row + 1, col  - 1);
    }

	if (number_from_cell(matrix, row + 1, col + 1) == -1) { // bottom right
		count += number_from_cell(matrix, row, col);
    } else {
		count += number_from_cell(matrix, row + 1, col  + 1);
    }

	return count;
}

// Returns new value of the cell according to the cellular automata
int deforestation(vector<std::vector<int>>& matrix, int row, int col) {
	int direct_neighbors = von_neumann_neighborhood(matrix, row, col);
	int corner_neighbors = moore_reduced_neighborhood(matrix, row, col);
	int cell_value = number_from_cell(matrix, row, col);
	float neighborhood = direct_neighbors + corner_neighbors/sqrt(2);    
	
	float near_city = type_moore_neighborhood(row, col, CITY);
	float near_water = type_moore_neighborhood(row, col, WATER);
	float near_road = type_moore_neighborhood(row, col, ROAD);
	float near_mine = type_moore_neighborhood(row, col, MINE);

	float mining_weight = 0.4*2.0/3.0;
	float roads_weight = 1.5*2.0/3.0;
	float water_weight = 1.1*2.0/3.0;
	float cities_weight = 1.3*2.0/3.0;

	float mining_contribution = mining_weight * near_mine;
	float roads_contribution = roads_weight * near_road;
	float water_contribution = water_weight * near_water;
	float cities_contribution = cities_weight * near_city;

	// Calculate the total result based on the contributions
	float result = neighborhood - (mining_contribution + roads_contribution + water_contribution + cities_contribution);

    // Before president's regulations
    if (year < 2023) {
        float offset = 0.05;        
        if (cell_value == 5) {
            if (result >= 28 - (20 - deforest_count) * offset) {
                return 5;
            } else {
                return 4;
            }
        
        } else if (cell_value == 4) {
            if (result >= 32 - (20 - deforest_count) * offset) {
                return 5;
            } else if (result >= 21 - (20 - deforest_count) * offset) {
                return 4;
            } else {
                return 3;
            }

        } else if (cell_value == 3) {
            if (result >= 24 - (20 - deforest_count) * offset) {
                return 4;
            } else if (result >= 15 - (20 - deforest_count) * offset) {
                return 3;
            } else {
                return 2;
            }
        
        } else if (cell_value == 2) {
            if (result >= 20 - (20 - deforest_count) * offset) {
                return 3;
            } else if (result >= 8 - (20 - deforest_count) * offset) {
                return 2;
            } else {
                return 1;
            }

        } else if (cell_value == 1) {
            if (result >= 10.5 - (20 - deforest_count) * offset) {
                return 2;
            } else {
                return 1;
            }
        }
    }

    // After 2022 => after the president's regulations
    result = neighborhood - 0.5*(mining_contribution + roads_contribution + water_contribution + cities_contribution);
    if (cell_value == 5) {
        if (result >= 29) {
            return 5;
        } else {
            return 4;
        }
    
    } else if (cell_value == 4) {
        if (result >= 31.5) {
            return 5;
        } else if (result >= 22) {
            return 4;
        } else {
            return 3;
        }

    } else if (cell_value == 3) {
        if (result >= 23.5) {
            return 4;
        } else if (result >= 16) {
            return 3;
        } else {
            return 2;
        }
    
    } else if (cell_value == 2) {
        if (result >= 19.5) {
            return 3;
        } else if (result >= 8) {
            return 2;
        } else {
            return 1;
        }

    } else if (cell_value == 1) {
        if (result >= 15.5) {
            return 2;
        } else {
            return 1;
        }
    }

    return -1;
}

// update the deforestation
vector<std::vector<int>> update(vector<std::vector<int>>& old_matrix) {
	vector<std::vector<int>> new_matrix(ROWS, vector<int>(COLS));
	for (int row = 0; row < ROWS; row++) {
		for (int col = 0; col < COLS; col++) {
            new_matrix[row][col] = deforestation(old_matrix, row, col);
        }
    }
    
    year++;
    deforest_count++;
	return new_matrix;
}


// prints info about updated forest
void forest_info() {
	int green = 0;
	int neon = 0;
	int lime = 0;
	int yellow = 0;
	int white = 0;
	for (int row = 0; row < ROWS; row++) {
		for (int col = 0; col < COLS; col++) {
			if (matrixForest[row][col] == 5) {
				green++;
            } else if (matrixForest[row][col] == 4) {
				neon++;
            } else if (matrixForest[row][col] == 3) {
				lime++;
			} else if (matrixForest[row][col] == 2) {
				yellow++;
            } else {
				white++;
            }
        }
    }
    float new_value = ((green*1 + neon*0.8 + lime*0.6 + yellow*0.4 + white*0.2) * (29*29));
    cout << "Year: " << year << endl;
    cout << "All: " << fixed << setprecision(3) << new_value << endl;
    cout << "Dissapeared: " << old_value - new_value << endl;
    cout << "--------------------------------------------" << endl;
    old_value = new_value;
}


int main() {
    char input;
    forest_info();
    //matrix_print(matrixForest); // for visualization of forest matrix
    while ((input = cin.get())) {
        vector<std::vector<int>> old_matrix(matrixForest);
        matrixForest = update(old_matrix);
        forest_info();
        //matrix_print(matrixForest); // for visualization of forest matrix
    }
    return 0;
}