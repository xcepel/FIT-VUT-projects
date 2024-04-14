//======== Copyright (c) 2021, FIT VUT Brno, All rights reserved. ============//
//
// Purpose:     White Box - Tests suite
//
// $NoKeywords: $ivs_project_1 $white_box_code.cpp
// $Author:     Katerina Cepelkova <xcepel03@stud.fit.vutbr.cz>
// $Date:       $2022-03-02
//============================================================================//
/**
 * @file white_box_tests.cpp
 * @author Katerina Cepelkova
 * 
 * @brief Implementace testu prace s maticemi.
 */

#include "gtest/gtest.h"
#include "white_box_code.h"

//============================================================================//
// ** ZDE DOPLNTE TESTY **
//
// Zde doplnte testy operaci nad maticemi. Cilem testovani je:
// 1. Dosahnout maximalniho pokryti kodu (white_box_code.cpp) testy.
// 2. Overit spravne chovani operaci nad maticemi v zavislosti na rozmerech 
//    matic.
//============================================================================//

class Matrix_ : public ::testing::Test
{
    protected:
        Matrix *matrix;
        
        virtual void SetUp()    {
            matrix = new Matrix();
        }
        virtual void TearDown() {
            delete matrix;
        }
};

class Matrix1x1 : public ::testing::Test
{
    protected:
        Matrix *matrix;
        
        virtual void SetUp()    {
            matrix = new Matrix();
        }
        virtual void TearDown() {
            delete matrix;
        }
};


class Matrix2x3 : public ::testing::Test
{
    protected:
        Matrix *matrix;

        virtual void SetUp()    {
            matrix = new Matrix(2,3);
        }
        virtual void TearDown() {
            delete matrix;
        }
};


class Matrix3x2 : public ::testing::Test
{
    protected:
        Matrix *matrix;

        virtual void SetUp()    {
            matrix = new Matrix(3,2);
        }
        virtual void TearDown() {
            delete matrix;
        }
};


class Matrix4x4 : public ::testing::Test
{
    protected:
        Matrix *matrix;

        virtual void SetUp()    {
            matrix = new Matrix(4,4);
        }
        virtual void TearDown() {
            delete matrix;
        }
};


class Matrix5x3 : public ::testing::Test
{
    protected:
        Matrix *matrix;

        virtual void SetUp()    {
            matrix = new Matrix(5,3);
        }
        virtual void TearDown() {
            delete matrix;
        }
};


class Matrix2x6 : public ::testing::Test
{
    protected:
        Matrix *matrix;

        virtual void SetUp()    {
            matrix = new Matrix(2,6);
        }
        virtual void TearDown() {
            delete matrix;
        }
};

//Test konstruktoru matice - vytvoření matic
TEST_F(Matrix_, Constructor) {
    EXPECT_NO_THROW(Matrix());
    EXPECT_NO_THROW(Matrix(1,1));
    EXPECT_NO_THROW(Matrix(2,2));
    EXPECT_NO_THROW(Matrix(4,2));
    EXPECT_NO_THROW(Matrix(6,9));

    EXPECT_ANY_THROW(Matrix(0,1)); 
    EXPECT_ANY_THROW(Matrix(1,0));
    EXPECT_ANY_THROW(Matrix(5,0));
    EXPECT_ANY_THROW(Matrix(0,5));
    EXPECT_ANY_THROW(Matrix(0,0));
}

//Test funkce SET (nastavování 1 hodnoty v matici) 
TEST_F(Matrix1x1, Set_Value) { 
    EXPECT_EQ(matrix->get(0,0),0);

    EXPECT_TRUE(matrix->set(0,0,1));
    EXPECT_EQ(matrix->get(0,0),1);
    EXPECT_TRUE(matrix->set(0,0,4.2));
    EXPECT_EQ(matrix->get(0,0),4.2);
    EXPECT_TRUE(matrix->set(0,0,-5));
    EXPECT_EQ(matrix->get(0,0),-5);

    EXPECT_FALSE(matrix->set(0,1,1));
    EXPECT_FALSE(matrix->set(1,0,1));
    EXPECT_FALSE(matrix->set(20,31,1));
    EXPECT_FALSE(matrix->set(0,85,1));
    EXPECT_FALSE(matrix->set(42,0,1)); 
    EXPECT_EQ(matrix->get(0,0),-5);
}

TEST_F(Matrix2x3, Set_Value) { 
    EXPECT_EQ(matrix->get(0,2),0);
    EXPECT_EQ(matrix->get(1,1),0);
    EXPECT_EQ(matrix->get(0,0),0);

    EXPECT_TRUE(matrix->set(0,2,1));
    EXPECT_EQ(matrix->get(0,2),1);
    EXPECT_TRUE(matrix->set(1,1,4.2));
    EXPECT_EQ(matrix->get(1,1),4.2);
    EXPECT_TRUE(matrix->set(0,0,-5));
    EXPECT_EQ(matrix->get(0,0),-5);

    EXPECT_FALSE(matrix->set(0,3,1));
    EXPECT_FALSE(matrix->set(2,0,1));
    EXPECT_FALSE(matrix->set(20,31,1));
    EXPECT_FALSE(matrix->set(0,85,1));
    EXPECT_FALSE(matrix->set(42,0,1)); 
    EXPECT_EQ(matrix->get(0,0),-5);
}

TEST_F(Matrix5x3, Set_Value) { 
    EXPECT_EQ(matrix->get(4,2),0);
    EXPECT_EQ(matrix->get(1,2),0);
    EXPECT_EQ(matrix->get(0,0),0);

    EXPECT_TRUE(matrix->set(4,2,1));
    EXPECT_EQ(matrix->get(4,2),1);
    EXPECT_TRUE(matrix->set(1,2,4.2));
    EXPECT_EQ(matrix->get(1,2),4.2);
    EXPECT_TRUE(matrix->set(0,0,-5));
    EXPECT_EQ(matrix->get(0,0),-5);

    EXPECT_FALSE(matrix->set(0,3,1));
    EXPECT_FALSE(matrix->set(5,0,1));
    EXPECT_FALSE(matrix->set(20,31,1));
    EXPECT_FALSE(matrix->set(0,85,1));
    EXPECT_FALSE(matrix->set(42,0,1)); 
    EXPECT_EQ(matrix->get(0,0),-5);
}

//Test funkce SET (nastavování hodnot v cele matici) 
TEST_F(Matrix1x1, Set_ValueArray)   {  
    EXPECT_EQ(matrix->get(0,0),0);
    std::vector<std::vector<double>> trueValues{
        {42}
    };
    EXPECT_TRUE(matrix->set(trueValues));
    EXPECT_EQ(matrix->get(0,0),42);
 
    std::vector<std::vector<double>> falseValues1{
        {1, 5}
    };
    EXPECT_FALSE(matrix->set(falseValues1));
    EXPECT_EQ(matrix->get(0,0),42);

    std::vector<std::vector<double>> falseValues2{
        {1},
        {52}
    };
    EXPECT_FALSE(matrix->set(falseValues2));
    EXPECT_EQ(matrix->get(0,0),42);
    
    std::vector<std::vector<double>> falseValues3{
        {}
    };
    EXPECT_FALSE(matrix->set(falseValues3));
    EXPECT_EQ(matrix->get(0,0),42);
}

TEST_F(Matrix2x3, Set_ValueArray)   {  
    for (int r = 0; r < 2; r++)
        for (int c = 0; c < 3; c++)
            EXPECT_EQ(matrix->get(r,c), 0);
    std::vector<std::vector<double>> trueValues{
        {42, 1, -8},
        {85, 4.3, 7}
    };
    EXPECT_TRUE(matrix->set(trueValues)); 
    for (int r = 0; r < 2; r++)
        for (int c = 0; c < 3; c++)
            EXPECT_EQ(matrix->get(r,c), trueValues[r][c]);
 
    std::vector<std::vector<double>> falseValues1{
        {1, 5, 5.7}
    };
    EXPECT_FALSE(matrix->set(falseValues1));
    for (int r = 0; r < 2; r++)
        for (int c = 0; c < 3; c++)
            EXPECT_EQ(matrix->get(r,c), trueValues[r][c]);
    
    std::vector<std::vector<double>> falseValues2{
        {1, 8, -9},
        {52, 57, 33},
        {71, 5.0, 4},
        {84, 3, 12}
    };
    EXPECT_FALSE(matrix->set(falseValues2));
    for (int r = 0; r < 2; r++)
        for (int c = 0; c < 3; c++)
            EXPECT_EQ(matrix->get(r,c), trueValues[r][c]);

    std::vector<std::vector<double>> falseValues3{
        {1, -8},
        {52, 57}
    };
    EXPECT_FALSE(matrix->set(falseValues3));
    for (int r = 0; r < 2; r++)
        for (int c = 0; c < 3; c++)
            EXPECT_EQ(matrix->get(r,c), trueValues[r][c]);
    
    std::vector<std::vector<double>> falseValues4{
        {1, 8, -9, 4},
        {52, 57, 33, 8.5}
    };
    EXPECT_FALSE(matrix->set(falseValues4));
    for (int r = 0; r < 2; r++)
        for (int c = 0; c < 3; c++)
            EXPECT_EQ(matrix->get(r,c), trueValues[r][c]);
    
    std::vector<std::vector<double>> falseValues5{
        {1, 8, -9},
        {52, 57, 33, 8.5}
    };
    EXPECT_FALSE(matrix->set(falseValues5));
    for (int r = 0; r < 2; r++)
        for (int c = 0; c < 3; c++)
            EXPECT_EQ(matrix->get(r,c), trueValues[r][c]);
    
    std::vector<std::vector<double>> falseValues6{
        {}
    };
    EXPECT_FALSE(matrix->set(falseValues6));
    for (int r = 0; r < 2; r++)
        for (int c = 0; c < 3; c++)
            EXPECT_EQ(matrix->get(r,c), trueValues[r][c]);
}

TEST_F(Matrix3x2, Set_ValueArray)   {  
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 2; c++)
            EXPECT_EQ(matrix->get(r,c), 0);
    std::vector<std::vector<double>> trueValues{
        {42, -1},
        {85, 4.3},
        {2, -1}
    };
    EXPECT_TRUE(matrix->set(trueValues)); 
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 2; c++)
            EXPECT_EQ(matrix->get(r,c), trueValues[r][c]);
 
    std::vector<std::vector<double>> falseValues1{
        {1, 5.7}
    };
    EXPECT_FALSE(matrix->set(falseValues1));
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 2; c++)
            EXPECT_EQ(matrix->get(r,c), trueValues[r][c]);
  
    std::vector<std::vector<double>> falseValues2{
        {1, 8},
        {52, 57},
        {84, 3},
        {1, 0}
    };
    EXPECT_FALSE(matrix->set(falseValues2));
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 2; c++)
            EXPECT_EQ(matrix->get(r,c), trueValues[r][c]);
    
    std::vector<std::vector<double>> falseValues3{
        {1, 8, 0},
        {52, 57, 5},
        {7.3, 9, 8}
    };
    EXPECT_FALSE(matrix->set(falseValues3));
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 2; c++)
            EXPECT_EQ(matrix->get(r,c), trueValues[r][c]);
    
    std::vector<std::vector<double>> falseValues4{
        {1},
        {52},
        {84}
    };
    EXPECT_FALSE(matrix->set(falseValues4));
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 2; c++)
            EXPECT_EQ(matrix->get(r,c), trueValues[r][c]);
    
    std::vector<std::vector<double>> falseValues5{
        {}
    };
    EXPECT_FALSE(matrix->set(falseValues5));
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 2; c++)
            EXPECT_EQ(matrix->get(r,c), trueValues[r][c]);
}

TEST_F(Matrix4x4, Set_ValueArray)   {  
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            EXPECT_EQ(matrix->get(r,c), 0);
    std::vector<std::vector<double>> trueValues{
        {42, -1, 41, 4},
        {85, 4.3, -2, 0},
        {2, -1, 77, 61},
        {1, 3, 8, 9}
    };
    EXPECT_TRUE(matrix->set(trueValues)); 
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            EXPECT_EQ(matrix->get(r,c), trueValues[r][c]);
 
    std::vector<std::vector<double>> falseValues1{
        {1, 5, 5.7, 1}
    };
    EXPECT_FALSE(matrix->set(falseValues1));
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            EXPECT_EQ(matrix->get(r,c), trueValues[r][c]);
    
    std::vector<std::vector<double>> falseValues2{
        {1, 5, 5.7, 1},
        {7, -3, 7, 5},
        {8, 44, 3, 0},
        {7, 4, 2, 66},
        {0, 1, 8, 9}
    };
    EXPECT_FALSE(matrix->set(falseValues2));
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            EXPECT_EQ(matrix->get(r,c), trueValues[r][c]);
    
    std::vector<std::vector<double>> falseValues3{
        {1, 5, 5.7, 1, 5},
        {7, -3, 7, 5, 77},
        {8, 44, 3, 0, 84},
        {7, 4, 2, 66, 0},
        {0, 1, 8, 9, 99}
    };
    EXPECT_FALSE(matrix->set(falseValues3));
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            EXPECT_EQ(matrix->get(r,c), trueValues[r][c]);
    
    std::vector<std::vector<double>> falseValues4{
        {1, 5, 5.7},
        {7, -3, 7},
        {8, 44, 3},
        {7, 4, 2},
        {0, 1, 8}
    };
    EXPECT_FALSE(matrix->set(falseValues4));
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            EXPECT_EQ(matrix->get(r,c), trueValues[r][c]);
    
    std::vector<std::vector<double>> falseValues5{
        {}
    };
    EXPECT_FALSE(matrix->set(falseValues5));
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            EXPECT_EQ(matrix->get(r,c), trueValues[r][c]);
}

//Test funkce GET (vraceni hodnoty matice na pozici(r,c))
TEST_F(Matrix1x1, Get)  { 
    EXPECT_EQ(matrix->get(0,0),0);
    matrix->set(0,0,7);
    EXPECT_EQ(matrix->get(0,0),7); 
    
    EXPECT_ANY_THROW(matrix->get(-3,0));
    EXPECT_ANY_THROW(matrix->get(0,85));
    EXPECT_ANY_THROW(matrix->get(1,1));
    
    EXPECT_EQ(matrix->get(0,0),7); 
}

TEST_F(Matrix3x2, Get)  {
    EXPECT_EQ(matrix->get(0,0),0);
    matrix->set(1,1,7.1);
    EXPECT_EQ(matrix->get(1,1),7.1); 
    
    EXPECT_ANY_THROW(matrix->get(-3,0));
    EXPECT_ANY_THROW(matrix->get(3,0));
    EXPECT_ANY_THROW(matrix->get(1,85));
    EXPECT_ANY_THROW(matrix->get(4,3)); 
}

TEST_F(Matrix4x4, Get)  {
    EXPECT_EQ(matrix->get(0,0),0);
    matrix->set(2,3,7.1);
    EXPECT_EQ(matrix->get(2,3),7.1); 
    
    EXPECT_ANY_THROW(matrix->get(-3,0));
    EXPECT_ANY_THROW(matrix->get(4,2));
    EXPECT_ANY_THROW(matrix->get(0,5));
    EXPECT_ANY_THROW(matrix->get(74,6214)); 
}

TEST_F(Matrix2x6, Get)  {
    EXPECT_EQ(matrix->get(0,0),0);
    matrix->set(0,5,7.1);
    EXPECT_EQ(matrix->get(0,5),7.1); 
    
    EXPECT_ANY_THROW(matrix->get(-3,0));
    EXPECT_ANY_THROW(matrix->get(3,3));
    EXPECT_ANY_THROW(matrix->get(0, 10));
    EXPECT_ANY_THROW(matrix->get(49,35)); 
}

//Testy operatoru == (vraci true/false pokud se matice (ne)rovnaji)
TEST_F(Matrix_, Comparison)   {
    Matrix matrix1x1_1 = Matrix();
    Matrix matrix1x1_2 = Matrix();
    Matrix matrix2x3_1 = Matrix(2,3);
    Matrix matrix2x3_2 = Matrix(2,3);
    Matrix matrix3x2 = Matrix(3,2);
    Matrix matrix4x4 = Matrix(4,4);
    Matrix matrix5x2 = Matrix(5,2);
    Matrix matrix3x6 = Matrix(3,6);

    EXPECT_TRUE(matrix1x1_1==matrix1x1_2);
    EXPECT_TRUE(matrix5x2==matrix5x2);
    EXPECT_TRUE(matrix3x6==matrix3x6);

    EXPECT_ANY_THROW(matrix1x1_1==matrix2x3_1);
    EXPECT_ANY_THROW(matrix1x1_2==matrix4x4);
    EXPECT_ANY_THROW(matrix2x3_1==matrix3x2);
    EXPECT_ANY_THROW(matrix3x2==matrix2x3_2);
    EXPECT_ANY_THROW(matrix5x2==matrix3x6);
     
    matrix1x1_1.set(0,0,5);
    EXPECT_FALSE(matrix1x1_1==matrix1x1_2);
    
    matrix1x1_2.set(0,0,5);
    EXPECT_TRUE(matrix1x1_1==matrix1x1_2);

    matrix2x3_1.set(1,2,3);
    EXPECT_FALSE(matrix2x3_1==matrix2x3_2);
    matrix2x3_1.set(1,2,0.111);
    EXPECT_FALSE(matrix2x3_1==matrix2x3_2);
    matrix2x3_2.set(1,2,0.111);
    EXPECT_TRUE(matrix2x3_1==matrix2x3_2);
    matrix2x3_1.set(1,2,-0.224);
    EXPECT_FALSE(matrix2x3_1==matrix2x3_2);
    matrix2x3_2.set(1,2,-0.224);
    EXPECT_TRUE(matrix2x3_1==matrix2x3_2);
    matrix2x3_1.set(1,2,-9);
    EXPECT_FALSE(matrix2x3_1==matrix2x3_2);
    matrix2x3_2.set(1,2,9);
    EXPECT_FALSE(matrix2x3_1==matrix2x3_2);
    matrix2x3_2.set(1,2,-9);
    EXPECT_TRUE(matrix2x3_1==matrix2x3_2);
}

//Testu operatoru + (soucet matic)
TEST_F(Matrix_, Plus) {
    Matrix matrix1x1_1 = Matrix();
    Matrix matrix1x1_2 = Matrix();
    Matrix expect1x1 = Matrix();
    Matrix matrix2x3 = Matrix(2,3);
    Matrix matrix3x2 = Matrix(3,2);
    Matrix matrix4x4_1 = Matrix(4,4);
    Matrix matrix4x4_2 = Matrix(4,4);
    Matrix expect4x4 = Matrix();
    Matrix matrix5x2 = Matrix(5,2);
    Matrix matrix3x6 = Matrix(3,6);

    EXPECT_NO_THROW(matrix1x1_1+matrix1x1_2);
    EXPECT_NO_THROW(matrix5x2+matrix5x2);
    EXPECT_NO_THROW(matrix3x6+matrix3x6);

    EXPECT_ANY_THROW(matrix1x1_1+matrix2x3);
    EXPECT_ANY_THROW(matrix1x1_2+matrix4x4_1);
    EXPECT_ANY_THROW(matrix2x3+matrix3x2);
    EXPECT_ANY_THROW(matrix3x2+matrix2x3);
    EXPECT_ANY_THROW(matrix5x2+matrix3x6);
   
    std::vector<std::vector<double>> value{
        {22}
    };
    (matrix1x1_1.set(value));
    value = {
        {20}
    };
    (matrix1x1_2.set(value));
    std::vector<std::vector<double>> result{
        {42}
    };
    (expect1x1.set(result));
    Matrix result1 = matrix1x1_1+matrix1x1_2; 
    EXPECT_TRUE(result1 == expect1x1);

    value = {
        {5, -8, 41, 3.54},
        {6.2, 3, 99, 84},
        {11, 11.05, 4, 7},
        {6, 5, 7, 9}
    };
    (matrix4x4_1.set(value));
    value = {
        {-0.56, 8, 0, 9.4},
        {8, 58, 11, 6},
        {53, -11, 53, 6},
        {9, 54, 73, 0}
    };
    (matrix4x4_2.set(value));
    result = {
        {4.44, 0, 41, 12.94},
        {14.2, 61, 110, 90},
        {64, 0.05, 57, 13},
        {15, 59, 80, 9}
    };
    (expect4x4.set(result));
    Matrix result3 = matrix4x4_1+matrix4x4_2; 
    EXPECT_TRUE(result3 == expect4x4);
}

TEST_F(Matrix_, MultiplyMatrixes)   {
    Matrix matrix1x1 = Matrix();
    Matrix expect1x1 = Matrix();
    Matrix matrix2x3 = Matrix(2,3);
    Matrix matrix3x2 = Matrix(3,2);
    Matrix expect2x2 = Matrix(2,2);
    Matrix matrix4x4 = Matrix(4,4);
    Matrix expect4x4 = Matrix(4,4);
    Matrix matrix5x2 = Matrix(5,2);
    Matrix matrix3x6 = Matrix(3,6);

    EXPECT_NO_THROW(matrix1x1*matrix1x1);
    EXPECT_NO_THROW(matrix3x2*matrix2x3);
    EXPECT_NO_THROW(matrix2x3*matrix3x2);
    EXPECT_NO_THROW(matrix4x4*matrix4x4);

    EXPECT_ANY_THROW(matrix1x1*matrix2x3);
    EXPECT_ANY_THROW(matrix2x3*matrix2x3);
    EXPECT_ANY_THROW(matrix4x4*matrix1x1);
    EXPECT_ANY_THROW(matrix3x2*matrix3x2);
    EXPECT_ANY_THROW(matrix5x2*matrix3x6);
   
    std::vector<std::vector<double>> value{
        {22}
    };
    (matrix1x1.set(value));
    std::vector<std::vector<double>> result{
        {484}
    };
    (expect1x1.set(result));
    Matrix result1 = matrix1x1*matrix1x1; 
    EXPECT_TRUE(result1 == expect1x1);
 
    value = {
        {5.1, 1, 1, 0},
        {0, 9, 1, 1},
        {6, 1, -8, 0},
        {0, 0, 1, 12}
    };
    (matrix4x4.set(value));
    result = {
        {32.01, 15.1, -1.9, 1},
        {6, 82, 2, 21},
        {-17.4, 7, 71, 1},
        {6, 1, 4, 144}
    };
    (expect4x4.set(result));
    Matrix result2 = matrix4x4*matrix4x4; 
    EXPECT_TRUE(result2 == expect4x4);
    
    value = {
        {5, 1, 1},
        {0, 9, 1}
    };
    (matrix2x3.set(value));
    value = {
        {7, 11},
        {0, 3},
        {6, 1}
    };
    (matrix3x2.set(value));
    result = {
        {41, 59},
        {6, 28}
    };
    (expect2x2.set(result));
    Matrix result3 = matrix2x3*matrix3x2; 
    EXPECT_TRUE(result3 == expect2x2);
}

TEST_F(Matrix_, MultiplyScalar) {
    Matrix matrix1x1 = Matrix();
    Matrix expect1x1 = Matrix();
    Matrix matrix5x2 = Matrix(5,2);
    Matrix expect5x2 = Matrix(5,2);

    std::vector<std::vector<double>> value{
        {21}
    };
    (matrix1x1.set(value));
    std::vector<std::vector<double>> result{
        {42}
    };
    (expect1x1.set(result));
    Matrix result_ = matrix1x1*2; 
    EXPECT_TRUE(result_ == expect1x1);
    
    value = {
        {-3}
    };
    (matrix1x1.set(value));
    result = {
        {9}
    };
    (expect1x1.set(result));
    result_ = matrix1x1*(-3); 
    EXPECT_TRUE(result_ == expect1x1);

    value = {
        {3.25}
    };
    (matrix1x1.set(value));
    result = {
        {9.75}
    };
    (expect1x1.set(result));
    result_ = matrix1x1*3; 
    EXPECT_TRUE(result_ == expect1x1);
    
    value = {
        {-3, 2},
        {2.2, 5},
        {9, 3},
        {11, -7},
        {5.1, 22}
    };
    EXPECT_TRUE(matrix5x2.set(value));
    result = {
        {-9, 6},
        {6.6, 15},
        {27, 9},
        {33, -21},
        {15.3, 66}
    };
    EXPECT_TRUE(expect5x2.set(result));
    result_ = matrix5x2*3; 
    EXPECT_TRUE(result_ == expect5x2);
}

TEST_F(Matrix_, SolveEquation)  {
    Matrix matrix1x1 = Matrix();
    Matrix matrix2x2 = Matrix(2,2);
    Matrix matrix3x3 = Matrix(3,3);
    Matrix matrix2x3 = Matrix(2,3);
    Matrix matrix3x2 = Matrix(3,2);
    Matrix matrix4x4 = Matrix(4,4);

    std::vector<double> vector1{0};
    std::vector<double> vector2{0, 0};
    std::vector<double> vector3{0, 0, 0};
    std::vector<double> vector4{0, 0, 0, 0};

    EXPECT_ANY_THROW(matrix2x3.solveEquation(vector2));
    EXPECT_ANY_THROW(matrix2x3.solveEquation(vector3));
    EXPECT_ANY_THROW(matrix3x2.solveEquation(vector2));
    EXPECT_ANY_THROW(matrix3x2.solveEquation(vector3));
    
    EXPECT_ANY_THROW(matrix1x1.solveEquation(vector4));
    EXPECT_ANY_THROW(matrix2x2.solveEquation(vector3));
    EXPECT_ANY_THROW(matrix3x3.solveEquation(vector2));

    EXPECT_ANY_THROW(matrix2x2.solveEquation(vector2));
    EXPECT_ANY_THROW(matrix3x3.solveEquation(vector3));
 
    std::vector<std::vector<double>> value{
        {1}
    };
    (matrix1x1.set(value));
    std::vector<double> expect{0};
    std::vector<double> result{0};
    result = matrix1x1.solveEquation(vector1); 
    EXPECT_TRUE(result == expect);
    
    value = {
        {5, 2},
        {2, 1}
    };
    (matrix2x2.set(value));
    vector2 = {1, 7};
    expect = {-13, 33};
    result = matrix2x2.solveEquation(vector2); 
    EXPECT_TRUE(result == expect);
  
    value = {
        {0, 1, 2},
        {2, 1, 1},
        {1, 2, 1}
    };
    (matrix3x3.set(value));
    vector3 = {8, 7, 8};
    expect = {1, 2, 3};
    result = matrix3x3.solveEquation(vector3); 
    EXPECT_TRUE(result == expect);
    
    value = {
        {1, -2, 3, -4},
        {0, 1, -1, 1},
        {1, 3, 1, -3},
        {0, -7, 3, 1}
    };
    (matrix4x4.set(value));
    vector4 = {4, -3, 1, -3};
    expect = {-8, 0, 0, -3};
    result = matrix4x4.solveEquation(vector4); 
    EXPECT_TRUE(result == expect);
}


TEST_F(Matrix_, Transpose) {
    Matrix matrix1x1 = Matrix();
    Matrix expect1x1 = Matrix();
    Matrix matrix5x2 = Matrix(5,2);
    Matrix expect2x5 = Matrix(2,5);
    Matrix matrix3x6 = Matrix(3,6);
    Matrix expect6x3 = Matrix(6,3);
    
    std::vector<std::vector<double>> value{
        {22}
    };
    (matrix1x1.set(value));
    Matrix result1 = matrix1x1.transpose(); 
    EXPECT_TRUE(result1 == matrix1x1);
    
    value = {
        {-3, 7.4},
        {9, 7},
        {11, 5},
        {-1, 0},
        {4, 3}
    };
    (matrix5x2.set(value));
    std::vector<std::vector<double>> result{
        {-3, 9, 11, -1, 4},
        {7.4, 7, 5, 0, 3}
    };
    (expect2x5.set(result));
    Matrix result2 = matrix5x2.transpose(); 
    EXPECT_TRUE(result2 == expect2x5);
    
    value = {
        {-3, 2, 1, 0, 0, 0},
        {6, 3, 3, 11, 0.5, 0},
        {1, 4, 5, 0, 2, 0}
    };
    (matrix3x6.set(value));
    result = {
        {-3, 6, 1},
        {2, 3, 4},
        {1, 3, 5},
        {0, 11, 0},
        {0, 0.5, 2},
        {0, 0, 0}
    };
    (expect6x3.set(result));
    Matrix result3 = matrix3x6.transpose(); 
    EXPECT_TRUE(result3 == expect6x3);
}

TEST_F(Matrix_, Inverse) {
    Matrix matrix1x1 = Matrix();
    Matrix matrix2x3 = Matrix(2,3);
    Matrix matrix3x2 = Matrix(3,2);
    Matrix matrix2x2 = Matrix(2,2);
    Matrix expect2x2 = Matrix(2,2);
    Matrix matrix3x3 = Matrix(3,3);
    Matrix expect3x3 = Matrix(3,3);

    EXPECT_ANY_THROW(matrix1x1.inverse()); 
    EXPECT_ANY_THROW(matrix2x3.inverse());
    EXPECT_ANY_THROW(matrix3x2.inverse());

    EXPECT_ANY_THROW(matrix2x2.inverse());
    EXPECT_ANY_THROW(matrix3x3.inverse());
    
    std::vector<std::vector<double>> value{
        {3, 1},
        {4, 2}
    };
    (matrix2x2.set(value));
    std::vector<std::vector<double>> result{
        {1, -0.5},
        {-2, 1.5}
    };
    (expect2x2.set(result));
    Matrix result1 = matrix2x2.inverse(); 
    EXPECT_TRUE(result1 == expect2x2);

    value = {
        {7, 2, 1},
        {0, 3, -1},
        {-3, 4, -2}
    };
    (matrix3x3.set(value));
    result = {
        {-2, 8, -5},
        {3, -11, 7},
        {9, -34, 21}
    };
    (expect3x3.set(result));
    Matrix result2 = matrix3x3.inverse(); 
    EXPECT_TRUE(result2 == expect3x3);
}


/*** Konec souboru white_box_tests.cpp ***/
