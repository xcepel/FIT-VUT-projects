//======== Copyright (c) 2021, FIT VUT Brno, All rights reserved. ============//
//
// Purpose:     Test Driven Development - priority queue code
//
// $NoKeywords: $ivs_project_1 $tdd_code.cpp
// $Author:     Katerina Cepelkova <xcepel03@stud.fit.vutbr.cz>
// $Date:       $2021-03-07
//============================================================================//
/**
 * @file tdd_code.cpp
 * @author Katerina Cepelkova
 * 
 * @brief Implementace metod tridy prioritni fronty.
 */

#include <stdlib.h>
#include <stdio.h>

#include "tdd_code.h"

//============================================================================//
// ** ZDE DOPLNTE IMPLEMENTACI **
//
// Zde doplnte implementaci verejneho rozhrani prioritni fronty (Priority Queue)
// 1. Verejne rozhrani fronty specifikovane v: tdd_code.h (sekce "public:")
//    - Konstruktor (PriorityQueue()), Destruktor (~PriorityQueue())
//    - Metody Insert/Remove/Find a GetHead
//    - Pripadne vase metody definovane v tdd_code.h (sekce "protected:")
//
// Cilem je dosahnout plne funkcni implementace prioritni fronty implementovane
// pomoci tzv. "double-linked list", ktera bude splnovat dodane testy 
// (tdd_tests.cpp).
//============================================================================//

PriorityQueue::PriorityQueue()
{
    m_pHead = NULL;
}

PriorityQueue::~PriorityQueue()
{
    Element_t *elem = GetHead();
    Element_t *next;
    
    while(elem != NULL)
    {
        next = elem->pNext;
        delete elem;
        elem = next;
    }
    m_pHead = NULL;
}

void PriorityQueue::Insert(int value)
{
    Element_t *newElem = new Element_t;
    newElem->value = value;
    Element_t *prev = GetHead();

    if(prev == NULL)
    {
        newElem->pNext = NULL;
        m_pHead = newElem;
        return;
    }

    if(prev->value < value)
    {
        newElem->pNext = prev;
        m_pHead = newElem;
        return;
    }

    Element_t *actual = prev->pNext;

    while (actual != NULL && value < actual->value)
    {
        actual = actual->pNext;
        prev = prev->pNext;
    }
    
    newElem->pNext = actual;
    prev->pNext = newElem;
}

bool PriorityQueue::Remove(int value)
{
    Element_t *prev = GetHead();
    if (prev == NULL)
        return false;

    if (prev->value == value)
    {
        m_pHead = prev->pNext;
        delete prev;
        return true;
    }

    Element_t *actual = prev->pNext;

    while (actual != NULL && value <= actual->value)
    {
        if (actual->value == value)
        {
            prev->pNext = actual->pNext;
            delete actual;
            return true;
        }
        
        actual = actual->pNext;
        prev = prev->pNext;
    } 

    return false;
}

PriorityQueue::Element_t *PriorityQueue::Find(int value)
{
    Element_t *actual = GetHead();
    while (actual != NULL && value <= actual->value)
    {
        if(actual->value == value)
            return actual;
        
        actual = actual->pNext;
    }

    return NULL;
}

size_t PriorityQueue::Length()
{
    Element_t *actual = GetHead();
    if (actual == NULL)
	return 0;
    
    size_t len;
        
    for (len = 1; actual->pNext != NULL; len++)
        actual = actual->pNext;
    
    return len;
}

PriorityQueue::Element_t *PriorityQueue::GetHead()
{ 
    return m_pHead;
}

/*** Konec souboru tdd_code.cpp ***/
