/*********************************************************************
 * @file  generation.h
 * 
 * @brief A header file.
 * 
 * @author Kateřina Čepelková <xcepel03>, Tomáš Ebert <xebert00>
 * @date Last modified on 2023-12-10
 *********************************************************************/

#ifndef GENERATION_H
#define GENERATION_H
#include <iostream>
#include <cstdio> 
#include <cmath>
#include <iomanip> 

#define ROWS 100
#define COLS 100
#define FIRST_FOREST_VALUE 5030189.000

using namespace std;

extern int deforest_count;
extern int year;
extern float old_value;

enum typeOfLand {
    WATER,
    CITY,
    ROAD,
    MINE
};
#endif // GENERATION_H