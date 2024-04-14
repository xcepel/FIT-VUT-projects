/*
 * Binárny vyhľadávací strom — iteratívna varianta
 *
 * S využitím dátových typov zo súboru btree.h, zásobníkov zo súborov stack.h a
 * stack.c a pripravených kostier funkcií implementujte binárny vyhľadávací
 * strom bez použitia rekurzie.
 */

#include "../btree.h"
#include "stack.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Inicializácia stromu.
 *
 * Užívateľ musí zaistiť, že incializácia sa nebude opakovane volať nad
 * inicializovaným stromom. V opačnom prípade môže dôjsť k úniku pamäte (memory
 * leak). Keďže neinicializovaný ukazovateľ má nedefinovanú hodnotu, nie je
 * možné toto detegovať vo funkcii.
 */
void bst_init(bst_node_t **tree) {
    *tree = NULL;
}

/*
 * Nájdenie uzlu v strome.
 *
 * V prípade úspechu vráti funkcia hodnotu true a do premennej value zapíše
 * hodnotu daného uzlu. V opačnom prípade funckia vráti hodnotu false a premenná
 * value ostáva nezmenená.
 *
 * Funkciu implementujte iteratívne bez použitia vlastných pomocných funkcií.
 */
bool bst_search(bst_node_t *tree, char key, int *value) {
    bool found = false;
    while (!found && tree != NULL) {
        if (tree->key == key) {
            *value = tree->value;
            found = true;
        } else if (tree->key > key) {
            tree = tree->left;
        } else
            tree = tree->right;
    }
    return found;
}

/*
 * Vloženie uzlu do stromu.
 *
 * Pokiaľ uzol so zadaným kľúčom v strome už existuje, nahraďte jeho hodnotu.
 * Inak vložte nový listový uzol.
 *
 * Výsledný strom musí spĺňať podmienku vyhľadávacieho stromu — ľavý podstrom
 * uzlu obsahuje iba menšie kľúče, pravý väčšie.
 *
 * Funkciu implementujte iteratívne bez použitia vlastných pomocných funkcií.
 */
void bst_insert(bst_node_t **tree, char key, int value) {
    bst_node_t *new_node = malloc(sizeof(*new_node));
    new_node->key = key;
    new_node->value = value;
    new_node->left = NULL;
    new_node->right = NULL;

    if (*tree == NULL) {
        *tree = new_node;
        return;
    }
    
    bst_node_t *iter = *tree; 
    while (true) {
        if (iter->key == key) {
            iter->value = value;
            free(new_node);
            break;
        } else if (iter->key > key) {
            if (iter->left == NULL) { // vytvoreni vlevo
                iter->left = new_node;
                break;
            } else {
                iter = iter->left;
            }
        } else {
            if (iter->right == NULL) { // vytvoreni vpravo
                iter->right = new_node;
                break;
            } else {
                iter = iter->right;
            }
        }

    }
}

/*
 * Pomocná funkcia ktorá nahradí uzol najpravejším potomkom.
 *
 * Kľúč a hodnota uzlu target budú nahradené kľúčom a hodnotou najpravejšieho
 * uzlu podstromu tree. Najpravejší potomok bude odstránený. Funkcia korektne
 * uvoľní všetky alokované zdroje odstráneného uzlu.
 *
 * Funkcia predpokladá že hodnota tree nie je NULL.
 *
 * Táto pomocná funkcia bude využitá pri implementácii funkcie bst_delete.
 *
 * Funkciu implementujte iteratívne bez použitia vlastných pomocných funkcií.
 */
void bst_replace_by_rightmost(bst_node_t *target, bst_node_t **tree) {
     if (tree != NULL && *tree != NULL) {
        bst_node_t *iter = *tree;
        while (iter->right != NULL) { // vyhledani nejpravejsiho a jeho otce
            if (iter->right->right == NULL) {
                target->key = iter->right->key;
                target->value = iter->right->value;
                // presunuti ukazatelu 
                bst_node_t *tmp = iter->right->left;
                free(iter->right);
                iter->right = tmp;
                break;
            } else {
                iter = iter->right;
            }
        }
    }
}

/*
 * Odstránenie uzlu v strome.
 *
 * Pokiaľ uzol so zadaným kľúčom neexistuje, funkcia nič nerobí.
 * Pokiaľ má odstránený uzol jeden podstrom, zdedí ho otec odstráneného uzla.
 * Pokiaľ má odstránený uzol oba podstromy, je nahradený najpravejším uzlom
 * ľavého podstromu. Najpravejší uzol nemusí byť listom!
 * Funkcia korektne uvoľní všetky alokované zdroje odstráneného uzlu.
 *
 * Funkciu implementujte iteratívne pomocou bst_replace_by_rightmost a bez
 * použitia vlastných pomocných funkcií.
 */
void bst_delete(bst_node_t **tree, char key) {
     if (tree != NULL) {
        bst_node_t *iter = *tree;
        bst_node_t *prev = NULL;
        while (iter != NULL) {
            if (iter->key == key) {
                if (iter->right != NULL && iter->left != NULL) { // 2 sons
                    bst_replace_by_rightmost(iter, &iter->left);
                    break;
                } else if (iter->right != NULL) { // only right son
                    if (prev != NULL && prev->left == iter) {
                        prev->left = iter->right;
                    } else if (prev != NULL && prev->right == iter) {
                        prev->right = iter->right;
                    } else { //root
                        *tree = iter->right;
                    }
                    free(iter);
                    break;
                } else if (iter->left != NULL) { // only left son 
                    if (prev != NULL && prev->left == iter) {
                        prev->left = iter->left;
                    } else if (prev != NULL && prev->right == iter) {
                        prev->right = iter->left;
                    } else { //root
                        *tree = iter->left;
                    }
                    free(iter);
                    break;
                } else {
                    if (prev != NULL && prev->left == iter) {
                        prev->left = NULL;
                    } else if (prev != NULL && prev->right == iter) {
                        prev->right = NULL;
                    } else { //root
                        *tree = NULL;
                    }
                    free(iter);
                    break;
                }
            } else if (iter->key > key) {
                prev = iter;
                iter = iter->left;
            } else {
                prev = iter;
                iter = iter->right;
            }
        }
    }
}

/*
 * Zrušenie celého stromu.
 *
 * Po zrušení sa celý strom bude nachádzať v rovnakom stave ako po
 * inicializácii. Funkcia korektne uvoľní všetky alokované zdroje rušených
 * uzlov.
 *
 * Funkciu implementujte iteratívne pomocou zásobníku uzlov a bez použitia
 * vlastných pomocných funkcií.
 */
void bst_dispose(bst_node_t **tree) {
    if (tree != NULL) {
        stack_bst_t s1;
        stack_bst_init(&s1);
        do {
            if (*tree == NULL) {
                if (!stack_bst_empty(&s1)) {
                    (*tree) = stack_bst_top(&s1);
                    stack_bst_pop(&s1);
                }
            } else {
                if ((*tree)->right != NULL) {
                    stack_bst_push(&s1, (*tree)->right);
                }
                bst_node_t *tmp = *tree;
                *tree = (*tree)->left;
                free(tmp);
            }
        } while (*tree != NULL || !stack_bst_empty(&s1));
    }
}

/*
 * Pomocná funkcia pre iteratívny preorder.
 *
 * Prechádza po ľavej vetve k najľavejšiemu uzlu podstromu.
 * Nad spracovanými uzlami zavola bst_print_node a uloží ich do zásobníku uzlov.
 *
 * Funkciu implementujte iteratívne pomocou zásobníku uzlov a bez použitia
 * vlastných pomocných funkcií.
 */
void bst_leftmost_preorder(bst_node_t *tree, stack_bst_t *to_visit) {
    while (tree != NULL) {
        stack_bst_push(to_visit, tree);
        bst_print_node(tree);
        tree = tree->left;
    }
}

/*
 * Preorder prechod stromom.
 *
 * Pre aktuálne spracovávaný uzol nad ním zavolajte funkciu bst_print_node.
 *
 * Funkciu implementujte iteratívne pomocou funkcie bst_leftmost_preorder a
 * zásobníku uzlov bez použitia vlastných pomocných funkcií.
 */
void bst_preorder(bst_node_t *tree) {
    stack_bst_t s1;
    stack_bst_init(&s1);
    bst_leftmost_preorder(tree, &s1);
    while (!stack_bst_empty(&s1)) {
        tree = stack_bst_top(&s1);
        stack_bst_pop(&s1);
        bst_leftmost_preorder(tree->right, &s1);
    }
}

/*
 * Pomocná funkcia pre iteratívny inorder.
 *
 * P<F12>rechádza po ľavej vetve k najľavejšiemu uzlu podstromu a ukladá uzly do
 * zásobníku uzlov.
 *
 * Funkciu implementujte iteratívne pomocou zásobníku uzlov a bez použitia
 * vlastných pomocných funkcií.
 */
void bst_leftmost_inorder(bst_node_t *tree, stack_bst_t *to_visit) {//TODO
    while (tree != NULL) {
        stack_bst_push(to_visit, tree);
        tree = tree->left;
    }
}

/*
 * Inorder prechod stromom.
 *
 * Pre aktuálne spracovávaný uzol nad ním zavolajte funkciu bst_print_node.
 *
 * Funkciu implementujte iteratívne pomocou funkcie bst_leftmost_inorder a
 * zásobníku uzlov bez použitia vlastných pomocných funkcií.
 */
void bst_inorder(bst_node_t *tree) {//TODO
    stack_bst_t s1;
    stack_bst_init(&s1);
    bst_leftmost_inorder(tree, &s1);
    while (!stack_bst_empty(&s1)) {
        tree = stack_bst_top(&s1);
        stack_bst_pop(&s1);
        bst_print_node(tree);
        bst_leftmost_inorder(tree->right, &s1);
    }
}

/*
 * Pomocná funkcia pre iteratívny postorder.
 *
 * Prechádza po ľavej vetve k najľavejšiemu uzlu podstromu a ukladá uzly do
 * zásobníku uzlov. Do zásobníku bool hodnôt ukladá informáciu že uzol
 * bol navštívený prvý krát.
 *
 * Funkciu implementujte iteratívne pomocou zásobníkov uzlov a bool hodnôt a bez použitia
 * vlastných pomocných funkcií.
 */
void bst_leftmost_postorder(bst_node_t *tree, stack_bst_t *to_visit,
                            stack_bool_t *first_visit) {//TODO
    while (tree != NULL) {
        stack_bst_push(to_visit, tree);
        stack_bool_push(first_visit, true);
        tree = tree->left;
    }
}

/*
 * Postorder prechod stromom.
 *
 * Pre aktuálne spracovávaný uzol nad ním zavolajte funkciu bst_print_node.
 *
 * Funkciu implementujte iteratívne pomocou funkcie bst_leftmost_postorder a
 * zásobníkov uzlov a bool hodnôt bez použitia vlastných pomocných funkcií.
 */
void bst_postorder(bst_node_t *tree) {//TODO
    bool fromLeft;
    stack_bst_t s1;
    stack_bst_init(&s1);
    stack_bool_t sb1;
    stack_bool_init(&sb1);

    bst_leftmost_postorder(tree, &s1, &sb1);

    while (!stack_bst_empty(&s1)) {
        tree = stack_bst_top(&s1);
        fromLeft = stack_bool_top(&sb1);
        stack_bool_pop(&sb1);
        if (fromLeft) {
            stack_bool_push(&sb1, false);
            bst_leftmost_postorder(tree->right, &s1, &sb1);
        } else {
            stack_bst_pop(&s1);
            bst_print_node(tree);
        }
    }
}
