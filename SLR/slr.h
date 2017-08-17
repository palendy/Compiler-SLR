//
//  slr.h
//  SLR
//
//  Created by 안준형 on 2017. 5. 5..
//  Copyright © 2017년 안준형. All rights reserved.
//

#ifndef slr_h
#define slr_h

#include <stdio.h>

typedef struct node  {
    char p_rule[20];
    struct node* next;
}node;

typedef struct index_node {
    int i;
    node* get;
    struct index_node* next;
}index_node;



void add_list(node* np, char* item);
void add_first_list(index_node* i_n,char* item );

void print_list();
void print_first_table();
void print_follow_table();
void print_c_table();
void print_action();
void print_goto();
void print_Ix(int i);

int is_accepted(char* input);

void init_stack();
void push(char item);
char pop();

char* first(char nt);
char* follow(char nt);

void eliminate(char* s, char c);
void dot(char*s, char c);
void move_dot(char*s);
char get_after_dot(char* s);

int _goto(index_node* i, char c);
int is_overlap(index_node* i, int* ans);
void  free_I(index_node* i);

void make_i0_initialize();
void make_i0_closure();


int isTerminal(char t);
int isNonTerminal(char t);
int isExist(char*s, char c);

int is_dervied_Epsilon(char s);

int get_ptable_index(char c);

void union_symbol(char* s, char c);
void union_string(char* s1, char* s2, int* f);
void union_string_without_epsilon(char* s1, char* s2);

void classify_symbol();

int find_prule(char* s);

void make_first_table();
void make_follow_table();
void make_c_construction();

void init_char_array(char* arr,int size);

int get_goto(int I,char c);
char* get_action(int I, char c);

#endif /* slr_h */
