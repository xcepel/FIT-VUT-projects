    //======== Copyright (c) 2017, FIT VUT Brno, All rights reserved. ============//
//
// Purpose:     Red-Black Tree - public interface tests
//
// $NoKeywords: $ivs_project_1 $black_box_tests.cpp
// $Author:     Kateřina Čepelková <xcepel03@stud.fit.vutbr.cz>
// $Date:       $2022-02-27
//============================================================================//
/**
 * @file black_box_tests.cpp
 * @author Kateřina Čepelková
 * 
 * @brief Implementace testu binarniho stromu.
 */

#include <vector>

#include "gtest/gtest.h"

#include "red_black_tree.h"

//============================================================================//
// ** ZDE DOPLNTE TESTY **
//
// Zde doplnte testy Red-Black Tree, testujte nasledujici:
// 1. Verejne rozhrani stromu
//    - InsertNode/DeleteNode a FindNode
//    - Chovani techto metod testuje pro prazdny i neprazdny strom.
// 2. Axiomy (tedy vzdy platne vlastnosti) Red-Black Tree:
//    - Vsechny listove uzly stromu jsou *VZDY* cerne.
//    - Kazdy cerveny uzel muze mit *POUZE* cerne potomky.
//    - Vsechny cesty od kazdeho listoveho uzlu ke koreni stromu obsahuji
//      *STEJNY* pocet cernych uzlu.
//============================================================================//

class EmptyTree : public ::testing::Test
{
protected:
    BinaryTree tree;
};


class NonEmptyTree : public ::testing::Test
{
protected:
    BinaryTree tree;

    virtual void SetUp() {
        int values[] = {1, 84, 96, 42, 37, 5, 19, 50, 87, 34, 0, 8 };
        for(int i = 0; i < 12; ++i)
            tree.InsertNode(values[i]);
    }
};

class TreeAxioms : public ::testing::Test
{
    protected:
    virtual void SetUp() {
        int values[] = {1, 84, 96, 42, 37, 5, 19, 50, 87, 34, 0, 8 };
        for(int i = 0; i < 12; ++i)
            tree.InsertNode(values[i]);
    }
        BinaryTree tree;
};

//Verejne rozhrani stromu - Testy nad prazdnym stromem

TEST_F(EmptyTree, InsertNode) {
    std::pair <bool,BinaryTree::Node_t *> pair1 = tree.InsertNode(111);
    EXPECT_TRUE(pair1.first);
    EXPECT_TRUE(pair1.second != NULL);
    EXPECT_EQ(pair1.second->key, 111);

    EXPECT_TRUE(pair1.second == tree.GetRoot());

    std::pair <bool,BinaryTree::Node_t *> pair2 = tree.InsertNode(111);
    EXPECT_FALSE(pair2.first);
    EXPECT_EQ(pair1.second, pair2.second);
}


TEST_F(EmptyTree, DeleteNode) {
    EXPECT_FALSE(tree.DeleteNode(1));
    tree.InsertNode(1);
    EXPECT_TRUE(tree.DeleteNode(1)); 
    EXPECT_FALSE(tree.DeleteNode(1));
}


TEST_F(EmptyTree, FindNode) { 
    EXPECT_TRUE(tree.FindNode(1) == NULL);
    tree.InsertNode(1);
    BinaryTree::Node_t *node = tree.FindNode(1);
    EXPECT_FALSE(node == NULL);
    EXPECT_EQ(node->key, 1); 
}

//Verejne rozhrani stromu - Testy nad NE-prazdnym stromem

TEST_F(NonEmptyTree, InsertNode)  {
    std::pair<bool,BinaryTree::Node_t *> pair1 = tree.InsertNode(111);
    EXPECT_TRUE(pair1.first);
    EXPECT_TRUE(pair1.second != NULL);
    EXPECT_EQ(pair1.second->key, 111);

    std::pair<bool,BinaryTree::Node_t *> pair2 = tree.InsertNode(111);
    EXPECT_FALSE(pair2.first);
    EXPECT_EQ(pair1.second, pair2.second);
}


TEST_F(NonEmptyTree, DeleteNode)  { 
    EXPECT_TRUE(tree.DeleteNode(1));
    EXPECT_FALSE(tree.DeleteNode(1));
}


TEST_F(NonEmptyTree, FindNode)    {  
    BinaryTree::Node_t *node = tree.FindNode(1);
    EXPECT_FALSE(node == NULL);
    EXPECT_EQ(node->key, 1); 

    tree.DeleteNode(1);
    EXPECT_TRUE(tree.FindNode(1) == NULL);
}

//Axiomy

TEST_F(TreeAxioms, Axiom1)    { 
    std::vector<BinaryTree::Node_t *> outLeafNodes;
    tree.GetLeafNodes(outLeafNodes); 

    for (int i = 0; i < outLeafNodes.size(); i++)
    {
        EXPECT_TRUE(outLeafNodes[i]->pLeft == NULL);
        EXPECT_TRUE(outLeafNodes[i]->pRight == NULL);
        EXPECT_EQ(outLeafNodes[i]->color, BLACK);
    } 
}

TEST_F(TreeAxioms, Axiom2)    { 
    std::vector<BinaryTree::Node_t *> outNonLeafNodes;
    tree.GetNonLeafNodes(outNonLeafNodes); 

    for (int i = 0; i < outNonLeafNodes.size(); i++)
    {
        if (outNonLeafNodes[i]->color == RED)
        {
            EXPECT_EQ(outNonLeafNodes[i]->pLeft->color, BLACK);
            EXPECT_EQ(outNonLeafNodes[i]->pRight->color, BLACK);
        }
    } 
}

TEST_F(TreeAxioms, Axiom3)    { 
    std::vector<BinaryTree::Node_t *> outLeafNodes;
    tree.GetLeafNodes(outLeafNodes);

    BinaryTree::Node_t *node = tree.FindNode(outLeafNodes[0]->key);
    
    int black_nodes = 0;
   
    //spocita black nodes v ceste
    BinaryTree::Node_t *rootNode = tree.GetRoot();
    while (node != rootNode)
    {
        if (node->color == BLACK)
            black_nodes++;
        node = node->pParent;
    }

    //kontroluje ekvivalenci poctu black nodes ve vsech cestach
    for (int i = 1; i < outLeafNodes.size(); i++)
    {
        int cnt = 0;
        node = tree.FindNode(outLeafNodes[i]->key);
        
        while (node != rootNode)
        {
            if (node->color == BLACK)
                cnt++;
            node = node->pParent;
        }
        EXPECT_EQ(black_nodes, cnt);
    } 
}

/*** Konec souboru black_box_tests.cpp ***/
