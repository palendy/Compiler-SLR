//
//  slr.c
//  SLR
//
//  Created by 안준형 on 2017. 5. 5..
//  Copyright © 2017년 안준형. All rights reserved.
//

#include "slr.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define buf 50
#define MAX_RULE_SIZE 30
#define MAX_RULE_LENGTH 30
#define MAX_CLOSURE_SIZE 30
#define MAX_CLOSURE_LENGTH 20

#define MALLOC(p,s) \
    if (!((p) = malloc(s))) { \
    fprintf(stderr,"Insufficient memory"); \
    exit(EXIT_FAILURE); \
} 

#define SWAP(x,y,t) ((t) = (x), (x) = (y), (y) = (t))


char rule[MAX_RULE_SIZE][MAX_RULE_LENGTH];

char NonTerminal[MAX_RULE_SIZE];
char Terminal[MAX_RULE_SIZE];

char FIRST_TABLE[MAX_RULE_SIZE][MAX_RULE_LENGTH];
char FOLLOW_TABLE[MAX_RULE_SIZE][MAX_RULE_LENGTH];

char PARSING_TABLE_SYMBOLS[MAX_RULE_SIZE];
int PARSING_TABLE[MAX_RULE_SIZE][MAX_RULE_LENGTH];

index_node* i0_node; //pointing I0,I1....

char stack[MAX_RULE_LENGTH];        // stack for parsing construction
int top;            //


int main (int argc, char* argv[]) {
    //varibales
    char temp[MAX_RULE_LENGTH];

    int i=0;
    
    // start
    if(argc != 2) {
        puts("argument error");
        return 1;
    }
    
    FILE* fp = fopen(argv[1], "r");
    
    if(fp==NULL) {
        puts("file is not opened");
        return 1;
    }
    
    //1. add augmented production
    strcpy(temp, "S>E");
    printf("Input Rule \n%s\n",temp);
    strcpy(rule[i++],temp);
    
    //strcpy(temp, "E>E+T");
    //printf("length is %lu\n", strlen(temp));
    
    //get production rules
    while((fgets(temp, sizeof(temp), fp))!= NULL) {
        
        //filtering only rules from document
        if((strchr(temp,'>')) != NULL) {
            eliminate(temp, '\n');
            strcpy(rule[i],temp);
            printf("%s\n",rule[i]);
            i++;
        }
    }
    printf("\n");
    
 
    classify_symbol();

    make_first_table();
    
    make_follow_table();
    
    make_c_construction();
    
    printf("RULE should be started from E, and cannot have Non-Terminal symbol S ! \n");
    
    i=1;
    do {
        
        scanf("%s",temp);
        if(!strcmp(temp,"exit")) i=0;
        
        else if(!strcmp(temp,"FIRST")) {
            print_first_table();
        }
        
        else if(!strcmp(temp,"FOLLOW")) {
            print_follow_table();
        }
        
        else if(!strcmp(temp, "TABLE")) {
             print_c_table();
        }
        
        else if(!strcmp(temp, "ACTION")) {
            print_action();
        }
        
        else if(!strcmp(temp, "GOTO")) {
            print_goto();
        }
        else if(temp[0]=='I') {
            print_Ix(atoi(&temp[1]));
        }
        
        else {
            strcat(temp, "$");
            if(is_accepted(temp)) printf("ACCEPT \n");
            else printf("ERROR \n");
        }
        
    } while (i);
    
    
    fclose(fp);
    return 0;
    
}
int is_accepted(char* input) {
    int i=0;
    int j;
    int pi,pj;
    char symbol = input[0];
    char goto_symbol;
    int temp;
    int flag=-1;
    int rule_length;
    char* rule_p;
    
    //initialize stack
    top=-1;
    push(0);
    
 
    while(flag==-1) {
        pi = stack[top];
        pj = get_ptable_index(symbol);
        temp =PARSING_TABLE[pi][pj];
        
        if(temp ==0) {
            //printf("error Message : No parsing table elements  \n");
            flag =0;
        }
        
        //if PARSING TABLE element is shift
        if(temp>0) {
            push(symbol);
            push(temp);
            symbol=input[++i];
        }
        
        if(temp==-10000) flag=1; // -10000 is accept
        
        //if PARSING TABLE element is reduce
        else if(temp<0){
            
            temp = -temp;
            rule_p = strchr(rule[temp],'>')+1; //only rule part after >
            rule_length = (int) (strlen(rule_p));
            
            for(j=0; j<rule_length*2; j++) pop();
            
            goto_symbol = rule[temp][0];
            pi = stack[top];
            pj = get_ptable_index(goto_symbol);
            temp = PARSING_TABLE[pi][pj];
            push(goto_symbol);
            push(temp);
        }
        
        
    }
    
    return flag;
}




void init_stack(void){
    top = -1;
}

void push(char t){
    
    if (top >= MAX_RULE_LENGTH - 1){
        printf("\n    Stack overflow.");
        return;
    }
    
    stack[++top] = t;
}

char pop(void){
    if (top < 0){
        printf("\n   Stack underflow.");
        return -1;
    }
    return stack[top--];
}




void classify_symbol(){
    int i=0;
    int j=0;
    char t1;
    int k=0;
    int l=0;
    
    //extract NonTerminals from rules
    for(i=0; i<MAX_RULE_SIZE; i++) {
        for(j=0; rule[i][j]!=0; j++) {
            t1=rule[i][j];
            if(isNonTerminal(t1)) {
                //if not exist in array
                if(!strchr(NonTerminal, t1)) {
                    NonTerminal[k]=t1;
                    //printf("%c \n",NonTerminal[k]);
                    k++;
                }
            }
            else {
                if(!strchr(Terminal,t1)) {
                    Terminal[l]=t1;
                    l++;
                }
            }
        }
    }
    Terminal[l] = '$';
    //NonTerminal[k]='\0'; //end of NonTerminals
    //Terminal[k]='\0';
   // printf("NT %s  Terminals %s",NonTerminal,Terminal);
    //printf("NT: %s length of NT is %d,  T: %s\n",NonTerminal,strlen(NonTerminal),Terminal );
}

void make_c_construction() {
    //printf("-------ok------ %s\n", temp);
    char symbol;
    char check[MAX_RULE_LENGTH];
    char temp [MAX_RULE_LENGTH];
    int ci=0;
    int t1;
    int t2;
    int t3;
    int i;
    
    
    index_node* i_n;
    node* j_n;
    
    make_i0_initialize();
    make_i0_closure();
    
    strcpy(PARSING_TABLE_SYMBOLS,Terminal);
    strcat(PARSING_TABLE_SYMBOLS, NonTerminal);
    //printf("%s \n",PARSING_TABLE[0]);
    
    //add shift element into parsing table
    for(i_n=i0_node; i_n!=NULL; i_n= i_n->next) {
        for(j_n=i_n->get; j_n!=NULL; j_n= j_n->next) {
            symbol = get_after_dot(j_n->p_rule);
            
            if(strchr(check,symbol)==NULL) {
                t1= i_n->i;
                t2= get_ptable_index(symbol);
                t3=_goto(i_n, symbol);
                PARSING_TABLE[t1][t2]=t3;
                check[ci++] = symbol;
                
            }
        }
        init_char_array(check,MAX_RULE_LENGTH);
        ci=0;
    }
    
    
    //add reduce element into parsing table
    for(i_n=i0_node; i_n!=NULL; i_n=i_n->next) {
        for(j_n=i_n->get; j_n!=NULL; j_n = j_n->next) {
            symbol = get_after_dot(j_n->p_rule);
            if(symbol =='\0') {
                //printf("%s \n", j_n->p_rule);
                t1= i_n->i;
                
                t3= find_prule(j_n->p_rule);
                strcpy(temp,follow(j_n->p_rule[0]));
                for(i=0; temp[i]!='\0'; i++) {
                    t2 = get_ptable_index(temp[i]);
                    PARSING_TABLE[t1][t2]= -(t3);
                    //printf("t1 %d t2 %d t3 %d \n",t1,t2,t3);
                }
               // printf("follow %s \n",temp);
                
            }
        }
    }
    
    
    
    

    /*
    // int i=0;
     int j=0;
     
    printf("PARSING TABLE \n");
    printf("%s\n",PARSING_TABLE_SYMBOLS);
    for(i=0; i<20; i++) {
        for(j=0; j<20; j++) printf("%d ",PARSING_TABLE[i][j]);
    }
    */
    
    
    
    //print_list();

    //dot(rule[0],2);
    //printf("dot %s \n", rule[0]);
    //printf("length %lu \n" ,strlen(rule[0]));  //bus error point
    //move_dot(rule[0]);
    //printf("dot %s \n", rule[0]);
    
}

int find_prule(char* s) {
    int i;
    char temp[MAX_RULE_SIZE];
    
    for(i=0;s[i]!='.'; i++) {
        temp[i] = s[i];
    }
    temp[i] = '\0';
   // printf("%s\n",temp);
    for(i=0; strlen(rule[i])!=0; i++) {
        if(!strcmp(rule[i], temp)) {
            
            if(i==0) return 10000; //accept
            else return i;
        }
    }
    //error
    return 9999;
}





int get_ptable_index(char c) {
    int i;
    for(i=0; PARSING_TABLE_SYMBOLS[i]!='\0';i++) {
        //printf("ele %c", PARSING_TABLE[0][i]);
        if(PARSING_TABLE_SYMBOLS[i] == c) return i;
    }
    return -1;
}



int _goto(index_node* i, char c) {
    char temp[MAX_RULE_LENGTH];
    char symbol;
    char check[MAX_RULE_LENGTH];
    init_char_array(check,MAX_RULE_LENGTH);
    
    int ci=0;
    int ans =-1;
    
    node* np;
    node* mp;
    node* p = NULL;
    
    index_node* new;
    MALLOC(new, sizeof(*new));
    new->next=NULL;
    new->get=NULL;
    
    index_node* ip = i0_node;
    
    
    //move dot from the rule and make new I
    for(np= i->get; np!=NULL; np=np->next) {
        if(c == get_after_dot(np->p_rule)) {
            strcpy(temp,np->p_rule);
            move_dot(temp);
            if(new->get ==NULL) {
                add_first_list(new,temp);
                p = new->get;
            }
            else add_list(p, temp);
        }
    }
    
    
//i there made additional Nonterminal after dot, add that rule
    for(np= new->get; np!=NULL; np=np->next) {
        symbol = get_after_dot(np->p_rule);
        if(isNonTerminal(symbol) && strchr(check, symbol)==NULL) {
            for(mp = i0_node->get; mp!=NULL; mp= mp->next) {
                strcpy(temp,mp->p_rule);
                if(symbol == temp[0]) {
                    add_list(np, temp);
                    check[ci++] =symbol;
                }
            }
        }
    }

//if the I is overlap with other Ix then remove
    if(is_overlap(new,&ans)) {
        free_I(new);
        return ans;
        
    }
    
    else {
        while(ip->next !=NULL) {
            ip=ip->next;
        }
        ip->next = new;
        new->i=(ip->i) +1;
    
        return new->i;
    }
    
}




int is_overlap(index_node* i, int* ans) {
    int result=0;
    node* ni=NULL;
    node* nj;
    index_node* sp;
    
    for(sp =i0_node;sp!=NULL && result ==0 ; sp=sp->next) {
        result =1;
        for(ni=i->get; ni!=NULL && result ==1; ni=ni->next) {
            result = 0;
            for(nj=sp->get; nj!=NULL && result ==0 ; nj=nj->next) {
                if(!strcmp(ni->p_rule, nj->p_rule)) result =1;
            }
        }
        //which I is overlap with new one
        if(ni==NULL && result==1) {
            *ans= sp->i;
        }
    }
    return result;
}

void free_I(index_node* i) {
    node* ni;
    node* temp;
    for(ni= i->get; ni!=NULL; ni=temp){
        temp=ni->next;
        free(ni);
    }
    
    free(i);
}




void add_first_list(index_node* i_n,char* item ) {
    node* temp;
    MALLOC(temp,sizeof(*temp));
    temp->next = NULL;
    strcpy(temp->p_rule,item);
    
    i_n->get=temp;
}



void make_i0_initialize() {
    char temp[MAX_RULE_LENGTH];
    strcpy(temp,rule[0]);
    dot(temp,'>');
    
    node* closure_node;

    MALLOC(closure_node,sizeof(*closure_node));
    MALLOC(i0_node,sizeof(*i0_node));
    i0_node->next=NULL;
    i0_node->get= closure_node;
    i0_node->i  = 0;
    
    closure_node->next=NULL;
    strcpy(closure_node->p_rule,temp);
}

void make_i0_closure() {
    
    int i;
    int j=0;
    char temp[MAX_CLOSURE_LENGTH];
    char check[MAX_RULE_SIZE];
    
    node* i_n ;
    char symbol;
    
    for(i_n=i0_node->get; i_n!=NULL; i_n=i_n->next) {
        symbol = get_after_dot(i_n->p_rule);
        
        if(isNonTerminal(symbol)&& (strchr(check, symbol)==NULL)) {
            for(i=0; strlen(rule[i])!=0; i++) {
                if(symbol == rule[i][0]){
                    strcpy(temp,rule[i]);
                    dot(temp,'>');
                    add_list(i_n, temp);
                    check[j++] = symbol;
                }
            }
        }
    }
}

void print_Ix(int i) {
    index_node* i_n;
    node* np;
    int count=0;
    
    for(i_n=i0_node; count!=i; i_n=i_n->next,count++) {
        if(i_n->next==NULL) {
            printf("I%d is out of bound\n",i);
            return;
        }
    }
    
    for(np=i_n->get; np!=NULL; np=np->next) printf("%s \n",np->p_rule);
    
}

void print_goto() {
    int i,j;
    int nt_len = (int) (strlen(NonTerminal));
    int t_len = (int) (strlen(Terminal));
    int count=0;
    char symbol;
    int element;
    
    index_node* i_n;
    
    for(i_n = i0_node; i_n!=NULL ; i_n=i_n->next) count++;
    
    for(j=0; j<count; j++) {
        printf("%d : ",j);
        for (i=t_len; i<nt_len+t_len; i++) {
            symbol = PARSING_TABLE_SYMBOLS[i];
            element = PARSING_TABLE[j][i];
            
                if(element>0) printf("%c[%d] ",symbol,element);
                else if(element<0) printf("%c[%d] ",symbol,-element);
        }
        printf("\n");
    }
    
}

void print_action () {
    int i;
    int j;
    int t_len = (int) (strlen(Terminal));
    char symbol;
    int element;
    int count=0;
    
    index_node* i_n;
    for(i_n = i0_node; i_n!=NULL ; i_n=i_n->next) count++;
    
    for(j=0; j<count; j++) {
        printf("%d : ",j);
        for (i=0; i<t_len; i++) {
            symbol = PARSING_TABLE_SYMBOLS[i];
            element = PARSING_TABLE[j][i];
            
            if(element == -10000) printf("%c[ACCEPT]",symbol);
            else if(element>0) printf("%c[s%d] ",symbol,element);
            else if(element<0 ) printf("%c[r%d] ",symbol,-element);
            
        }
        printf("\n");
    }
}




void print_c_table() {
    int i,j;
    int nt_len = (int) (strlen(NonTerminal));
    int t_len = (int) (strlen(Terminal));
    int s_len = nt_len + t_len;
    int count=0;
    char symbol;
    int element;
    
    index_node* i_n;
    
    for(i_n = i0_node; i_n!=NULL ; i_n=i_n->next) count++;
    
    for(j=0; j<count; j++) {
        printf("%d : ",j);
        for (i=0; i<s_len; i++) {
            symbol = PARSING_TABLE_SYMBOLS[i];
            element = PARSING_TABLE[j][i];
        
            if(i<t_len) {
                if(element == -10000) printf("%c[ACCEPT]",symbol);
                else if(element>0) printf("%c[s%d] ",symbol,element);
                else if(element<0 ) printf("%c[r%d] ",symbol,-element);
                
                }
            else {
                if(element>0) printf("%c[%d] ",symbol,element);
                else if(element<0) printf("%c[%d] ",symbol,-element);
            }
        }
        printf("\n");
    }
}


void add_list(node* np, char* item) {
    node* temp;
    MALLOC(temp,sizeof(*temp));
    
    temp->next = NULL;
    strcpy(temp->p_rule,item);
    
    while(np->next !=NULL) {
        np= np->next;
    }
    np->next=temp;
}

void print_list() {
    index_node* i ;
    node* j;
    for(i= i0_node; i!=NULL; i=i->next) {
        printf("I%d : ", i->i);
        for(j=i->get; j!=NULL; j=j->next) {
            printf("%s ",j->p_rule);
        }
        printf("\n");
    }
}

char get_after_dot(char* s) {
    char* pos = strchr(s, '.');
    return pos[1];
}


void dot(char* s, char c) {
    char temp[MAX_RULE_LENGTH];
    int i;
    size_t pos=0;
    size_t end = strlen(s);
    for(i=0; s[i]!=c;i++) pos++;
    
    strncpy(temp, s, ++pos);
    temp[pos++] = '.';
    
    for(i=(int)pos-1 ; i<end; i++) {
        temp[pos++] = s[i];
    }
    temp[pos] = '\0';
    strcpy(s,temp);
    //printf("-------ok------ %s\n", s);
}

void move_dot(char* s) {
    char * pos = strchr(s, '.');
    char temp;
    if((pos[1])!= '\0') {
        SWAP(pos[0],pos[1],temp);
    }
}

int is_dervied_Epsilon(char s) {
    if(isTerminal(s)) return 0;
    
    if(isNonTerminal(s)) {
        int i;
        char* p;
        char check[MAX_RULE_LENGTH];
        int j=0;
        init_char_array(check, MAX_RULE_LENGTH);
        check[j] =s;
        
        while(check[j++]!='\0') {
        for(i=0; strlen(rule[i])!=0; i++) {
            p= strchr(rule[i], '>')+1;
            if(p[0]=='#') return 1;
            
            else if(strlen(p)==1 && isNonTerminal(p[0])) {
                check[j++] = p[0];
            }
        }
        }
        }
    return 0;
}






void make_follow_table() {
    int i=0;
    int j=0;
    int change_flag = 0;
    char symbol, next_symbol;
    
    //phase 0 initalize follow table
    for(i=0; NonTerminal[i] != '\0'; i++) {
        FOLLOW_TABLE[i][0] = NonTerminal[i];
        FOLLOW_TABLE[i][1] = ':';
        FOLLOW_TABLE[i][2] = '$';
    }
    
    //phase 1
    for(i=0; strlen(rule[i])!=0; i++) {
        //printf("length is %lu\n", strlen(rule[i]));
        for(j=2; j<strlen(rule[i]) ;j++) {
            symbol = rule[i][j];
            next_symbol = rule[i][j+1];
            //printf("j%d : symbol is %c , next is %c \n",j,symbol,next_symbol);
            //if(isNonTerminal(symbol) && (next_symbol!='\0')) {
            if(isNonTerminal(symbol) && (next_symbol!='\0') && (next_symbol!='#')) {
                if(isTerminal(next_symbol)) union_symbol(follow(symbol), next_symbol);
                if(isNonTerminal(next_symbol))
                    //union_string(follow(symbol), first(next_symbol), &change_flag);
                    union_string_without_epsilon(follow(symbol),first(next_symbol));
            }
        }
    }
    
    
    //phase 2
    do {
        change_flag=0;
        for(i=0; strlen(rule[i])!=0; i++) {
            size_t len= strlen(rule[i]);
            symbol = rule[i][len-1];
            next_symbol = rule[i][len-2];
            if(isNonTerminal(symbol)) union_string(follow(symbol), follow(rule[i][0]), &change_flag);
            
            if(isNonTerminal(next_symbol) && is_dervied_Epsilon(symbol)) union_string(follow(next_symbol), follow(rule[i][0]), &change_flag);
        }

    } while (change_flag);
    
    /*
    //print follow
    printf("FOLLOW TABLE \n");
    for(i=0; strlen(FOLLOW_TABLE[i])!=0;i++) printf("%s \n" , FOLLOW_TABLE[i]);
    */
}




void make_first_table() {
    int i=0;
    int j;
    int epsilon_flag;
    
    //phase 0 initalize first table
    while(NonTerminal[i]!='\0') {
        FIRST_TABLE[i][0] = NonTerminal[i];
        FIRST_TABLE[i][1] = ':';
        i++;
    }
    
    //phase 1 terminal union
    for(i=0; strlen(rule[i])!=0; i++) {
        //printf("i : %d ",i);
        char* p = strchr(rule[i], '>');
        char symbol = p[1];
        //printf("%c\n", symbol);
        char nt = rule[i][0];
        //printf("%c\n", nt);
        if(isTerminal(symbol)) {
            union_symbol(first(nt), symbol);
            //printf("first : %c : %s\n" , nt, first(nt));
        }
    }
    //phase 2 ring sum First();
    int change_flag;
    char* p ;
    do {
        change_flag=0;
        for(i=0; strlen(rule[i]) != 0 ; i++) {
            p = strchr(rule[i], '>');
            p = &p[1]; //starting point of production rule
            
            epsilon_flag=1;
            for(j=0; j< strlen(p) && epsilon_flag==1; j++ ) {
                epsilon_flag=0;
                
                if(isNonTerminal(p[j])) union_string(first(rule[i][0]), first(p[j]), &change_flag);
                
                if(strchr(p, '#')!=NULL) epsilon_flag=1;
            }
        }
        
    } while (change_flag);
    /*
    printf("FIRST TABLE \n");
    for(i=0;i<strlen(FIRST_TABLE[i])!=0;i++) printf("%s \n" , FIRST_TABLE[i]);
    */
}





void print_first_table() {
    int i;
    for(i=1; strlen(FIRST_TABLE[i])!=0; i++ ) {
        printf("%s \n",FIRST_TABLE[i]);
    }
}

void print_follow_table() {
    int i;
    for(i=1; strlen(FOLLOW_TABLE[i])!=0; i++) {
        printf("%s \n",FOLLOW_TABLE[i]);
    }
}


void union_symbol(char* s, char c) {
    size_t len =strlen(s);
    
    if(!isExist(s,c)) {
        s[len] = c;
        s[len+1] = '\0';
    }
}

void union_string(char* s1, char* s2, int* flag) {
    int i;
    
    for(i=0; i< strlen(s2); i++) {
        if(!isExist(s1, s2[i])) {
            size_t len = strlen(s1);
            s1[len] = s2[i];
            s1[len+1] = '\0';
            *flag = 1;
        }
    }
}
void union_string_without_epsilon(char*s1, char*s2) {
    int i;
    
    for(i=0; i< strlen(s2); i++) {
        if((!isExist(s1, s2[i])) && s2[i]!='#') {
            size_t len = strlen(s1);
            s1[len] = s2[i];
            s1[len+1] = '\0';
        }
    }
}

char* follow(char nt) {
    int i;
    char* p ;
    for(i=0; strlen(FOLLOW_TABLE[i])!= 0 ; i++) {
        if(FOLLOW_TABLE[i][0] == nt) {
            p = strchr(FOLLOW_TABLE[i], ':');
            p= &p[1];
            //printf("p is %s \n",p);
            return p;
        }
    }
    p="error";
    return p;
}

char* first(char nt) {
    int i;
    char* p ;
    for(i=0; strlen(FIRST_TABLE[i])!= 0 ; i++) {
        if(FIRST_TABLE[i][0] == nt) {
            p = strchr(FIRST_TABLE[i], ':');
            p= &p[1];
            //printf("p is %s \n",p);
            return p;
        }
    }
    p="error";
    return p;
}

void eliminate(char *str, char ch)
{
    for (; *str != '\0'; str++)
    {
        if (*str == ch)
        {
            strcpy(str, str + 1);
            str--;
        }
    }
}


int isExist(char* s, char c) {
    int f=0;
    int i;
    for(i=0; i<strlen(s); i++) {
        if(s[i] == c) f=1;
     //   printf(" -- %c",s[i]);
    }
    //printf("\n");
    return f;
}


int isNonTerminal(char t) {
    if(t>='A' && t <='Z') return 1;
    else return 0;
}

int isTerminal(char t) {
    if(t>='A' && t <='Z') return 0;
    else return 1;
}

void init_char_array(char* arr,int size) {
    int i;
    for(i=0; i<size; i++) arr[i] ='\0';
}






