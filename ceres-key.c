#include <stdio.h>

int rand_int(int max){
    return 0;
}

typedef struct{
    int count;
    char *** list;
} SentenceList;

void init_sentencelist(SentenceList * list){
    list->count = 0;
    list->list = (char ***)malloc(0 * sizeof(char **));
}

void malloc_sentences(SentenceList * list, char * sentence){
    list = (SentenceList *)malloc(sizeof(SentenceList));
    init_sentencelist(list);
    append_sentence(list, sentence);
}

void append_sentence(SentenceList * list, char * sentence){
    list->count++;
    list->list = (char ***)realloc(list->list, list->count * sizeof(char **));
    list->list[list->count-1] = (char **)malloc(sizeof(sentence));
    *(list->list[list->count-1]) = sentence;
}

typedef struct{
    int key;
    Key * next;
} Key;

typedef struct{
    int count;
    Key * first;
} KeyList;


void sort_keylist(KeyList * list){
    return;
}

typedef struct{
    int key;
    Node * sibling;
    Node * child;
    unsigned int hasSentences : 1;
    SentenceList * sentences
} Node;

void init_node(Node * node){
    node->key = 0;
    node->sibling = 0;
    node->child = 0;
    node->hasSentences = 0;
    node->sentences = 0;
}

Node * search_sentence_in_trie(KeyList * keys, Node * first_in_trie)
{
    Node * trie_runner = first_in_trie;
    Node * last_sentenced = 0;
    Key * key_runner = keys->first;
    while(key_runner != 0){
        if( key_runner->key == trie_runner->key ){
            if(trie_runner->child != 0){
                trie_runner = trie_runner->child;
            }else{
                key_runner = 0;
            }
        }else{
            if(trie_runner->sibling != 0){
                trie_runner = trie_runner->sibling;
            }else{
                //While parent ----->>>
                trie_runner = last_sentenced->sibling
            }
        }
        if(trie_runner->hasSentences = 1){
            last_sentenced = trie_runner;
        }
        key_runner = key_runner->next;
    }
    return last_sentenced;
}

void sentence_into_trie(Node * trie, KeyList * keys, char * sentence)
{
    Node * trie_runner = trie;
    Node * trie_runner_prev = trie_runner;
    Key * key_runner = keys->first;

    int need_child = 1;

    while( trie_runner !=0 && key_runner != 0){
        if( trie_runner->key != key_runner->key ){
            trie_runner = trie_runner->sibling;
            while( trie_runner != 0 && trie_runner->key < key_runner->key ){
                trie_runner = trie_runner->sibling
                trie_runner_prev = trie_runner_prev->sibling;
            }
            if( trie_runner->key == key_runner->key ){
                trie_runner_prev = trie_runner;
                trie_runner = trie_runner->child;
                key_runner = key_runner->next;
            }else{
                need_child = 0;
                trie_runner_prev->sibling = (Node *)malloc(sizeof(Node));
                init_node(trie_runner_prev->sibling);
                trie_runner_prev = trie_runner_prev->sibling;
                trie_runner_prev->sibling = trie_runner;
                break;
            }
        }else{
            trie_runner_prev = trie_runner;
            trie_runner = trie_runner->child;
            key_runner = key_runner->next;
        }
    }

    trie_runner = trie_runner_prev;

    while(key_runner !=0 ){
        trie_runner->child = (Node *)malloc(sizeof(Node));
        init_node(trie_runner->child);
        trie_runner = trie_runner->child;
    }

    if(trie_runner->hasSentences == 0){
        trie_runner->hasSentences = 1;
        malloc_sentences(trie_runner->sentences, sentence);
    }else{
        append_sentence(trie_runner->sentences, sentence);
    }
}

int main(){
    KeyList * keys_first_sentences;
    char * first_sentence;
    sort_keylist(keys_first_sentences);

    Node * Trie = keys->first;





    Key key_1, key_2, key_3;
    key_1 = {.key = 5, .next = 0};
    key_2 = {.key = 9, .next = 0};
    key_3 = {.key = 2, .next = 0};

    Keylist = keystack;

    keystack.first = *key_1;
    key_1.next = *key_2;
    key_2.next = *key_3;

    sort_keylist(*keystack);

    Node trie;

    Node * found = search_sentence_in_trie(&keystack, &trie);
    char * sentence = (found->sentences)->list[rand_int((found->sentences)->count)];
}