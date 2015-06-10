#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int rand_int(int max){
    return 0;
}

void print_times(char * string, int x){
    for(x; x > 0; x--)
        printf("%s", string);
}

typedef int KeyType;

struct SentenceList{
    int count;
    char *** list;
};

struct KeyListItem{
    KeyType key;
    struct KeyListItem * next;
};

struct KeyList{
    int count;
    struct KeyListItem * first;
};

struct TrieNode{
    KeyType key;
    struct TrieNode * sibling;
    struct TrieNode * child;
    unsigned int hasSentences : 1;
    struct SentenceList * sentences;
};

struct Trie{
    struct TrieNode * root;
};

void init_sentencelist(struct SentenceList * list){
    list->count = 0;
    list->list = (char ***)malloc(0 * sizeof(char **));
}

void init_keylistitem(struct KeyListItem * key){
    key->key = 0;
    key->next = 0;
}

void init_keylist(struct KeyList * list){
    list->count = 0;
    list->first = 0;
}

void init_trienode(struct TrieNode * node){
    node->key = 0;
    node->child = 0;
    node->sibling = 0;
    node->hasSentences = 0;
    node->sentences = 0;
}

void init_trie(struct Trie * trie){
    trie->root = 0;
}

void append_sentence(struct SentenceList * list, char * sentence){
    list->count++;
    list->list = (char ***)realloc(list->list, list->count * sizeof(char **));
    list->list[list->count-1] = (char **)malloc(sizeof(sentence));
    *(list->list[list->count-1]) = sentence;
}

void malloc_sentences(struct SentenceList * list, char * sentence){
    list = (struct SentenceList *)malloc(sizeof(struct SentenceList));
    init_sentencelist(list);
    append_sentence(list, sentence);
}

void sort_keylist(struct KeyList * list){
    return;
}

struct TrieNode * search_sentence_in_trie(struct Trie * trie, struct KeyList * keylist)
{
    struct TrieNode * trie_iterator = trie->root;
    struct TrieNode * last_sentenced = 0;
    struct KeyListItem * keylist_iterator = keylist->first;

    while(keylist_iterator != 0){

        if( keylist_iterator->key == trie_iterator->key )
            if(trie_iterator->child != 0)
                trie_iterator = trie_iterator->child;
            else
                keylist_iterator = 0;

        else
            if(trie_iterator->sibling != 0)
                trie_iterator = trie_iterator->sibling;
            else
                trie_iterator = last_sentenced->sibling;

        if(trie_iterator->hasSentences = 1)
            last_sentenced = trie_iterator;

        keylist_iterator = keylist_iterator->next;
    }

    return last_sentenced;
}

void sentence_into_trie(struct Trie * trie, struct KeyList * keylist, char * sentence)
{
    struct TrieNode * trie_iterator = trie->root;
    struct TrieNode * trie_iterator_prec = trie_iterator;
    struct KeyListItem * keylist_iterator = keylist->first;

    //Trie ist leer:
    if(trie_iterator == 0)
    {
        trie_iterator = (struct TrieNode *)malloc(sizeof(struct TrieNode));
        init_trienode(trie_iterator);
        trie_iterator->key = keylist_iterator->key;
        keylist_iterator = keylist_iterator->next;
        trie->root = trie_iterator;
        trie_iterator_prec = trie_iterator;

    //Trie ist nicht leer
    }else{
        //Solange nicht an einem Trie Ende oder Listen Ende
        while( trie_iterator !=0 && keylist_iterator != 0){

            //hier ist trie_iterator == trie_iterator_prec erfüllt sein
            //Node passt, auf zum Kind
            if(trie_iterator->key == keylist_iterator->key){

                trie_iterator = trie_iterator->sibling;

                while( trie_iterator != 0 && trie_iterator->key < keylist_iterator->key ){
                    trie_iterator = trie_iterator->sibling;
                    trie_iterator_prec = trie_iterator_prec->sibling;
                }

                //Neuer Sibling am Ende nötig
                if(trie_iterator == 0){
                    trie_iterator_prec->sibling = (struct TrieNode *)malloc(sizeof(struct TrieNode));
                    trie_iterator = trie_iterator_prec->sibling;
                    trie_iterator_prec = trie_iterator;
                    init_trienode(trie_iterator);
                    trie_iterator->key = keylist_iterator->key;
                    keylist_iterator = keylist_iterator->next;
                    break;

                //Passender Sibling gefunden
                }else if(trie_iterator->key == keylist_iterator->key){
                    trie_iterator = trie_iterator->child;
                    trie_iterator_prec = trie_iterator;
                    keylist_iterator = keylist_iterator->next;

                //Neuer Sibling in Mitte
                }else if(trie_iterator->key < keylist_iterator->key){
                    trie_iterator_prec->sibling = (struct TrieNode *)malloc(sizeof(struct TrieNode));
                    trie_iterator_prec->sibling->sibling = trie_iterator;
                    trie_iterator = trie_iterator_prec->sibling;
                    trie_iterator_prec = trie_iterator;
                    init_trienode(trie_iterator);
                    trie_iterator->key = keylist_iterator->key;
                    keylist_iterator = keylist_iterator->next;
                }
            //Node passt nicht, auf zum Sibling
            }else{
                trie_iterator = trie_iterator->child;
                trie_iterator_ prec = trie_iterator;
                keylist_iterator = keylist_iterator->next;
            }
        }
    }

    while(keylist_iterator !=0 ){
        trie_iterator->child = (struct TrieNode *)malloc(sizeof(struct TrieNode));
        trie_iterator = trie_iterator->child;
        init_trienode(trie_iterator);
        trie_iterator->key = keylist_iterator->key;
        keylist_iterator = keylist_iterator->next;
    }

    if(trie_iterator->hasSentences == 0){
        trie_iterator->hasSentences = 1;
        malloc_sentences(trie_iterator->sentences, sentence);
    }else{
        append_sentence(trie_iterator->sentences, sentence);
    }
}

void print_trie_recursive(struct TrieNode * root, int prefix)
{
    struct TrieNode * trie_iterator = root;
    print_times(" ", prefix);
    printf(" | \n", prefix);
    while(trie_iterator != 0)
    {
        if(trie_iterator->hasSentences == 0)
            printf(" %d -", trie_iterator->key);
        else
            printf("[%d]-", trie_iterator->key);
        trie_iterator = trie_iterator->child;
    }
    printf("\n");
    trie_iterator = root;
    while(trie_iterator != 0)
    {
        if(trie_iterator->sibling != 0){
            printf("\nsib: %d\n", (trie_iterator->sibling)->key);
            print_trie_recursive(trie_iterator->sibling, prefix);
        }
        prefix++;
        trie_iterator = trie_iterator->child;
    }
}

void print_trie(struct Trie * trie)
{
    print_trie_recursive(trie->root, 0);
}

struct TrieNode * parse_file_to_trie(char * filename)
{
}

int main(){
    char drivedb_file[] = "sentenceDB.txt";

    /*
    MASTER -> 1
    MAIL -> 3
    ASKER -> 5
    */

    char sentence_1[] = "mhhh $MASTER, there is $MAIL for you.";
    struct KeyList keylist_1;
    init_keylist(&keylist_1);
    struct KeyListItem key_1_1, key_1_2;
    init_keylistitem(&key_1_1); init_keylistitem(&key_1_2);
    key_1_1.key = 1; key_1_2.key = 3;
    keylist_1.first = &key_1_1; key_1_1.next = &key_1_2;
    keylist_1.count = 2;

    char sentence_2[] = "beep, beep.. $MAIL from $ASKER to $MASTER";
    struct KeyList keylist_2;
    init_keylist(&keylist_2);
    struct KeyListItem key_2_1, key_2_2, key_2_3;
    init_keylistitem(&key_2_1); init_keylistitem(&key_2_2); init_keylistitem(&key_2_3);
    key_2_1.key = 1; key_2_2.key = 3; key_2_1.key = 4;
    keylist_2.first = &key_2_1; key_2_1.next = &key_2_2; key_2_2.next = &key_2_3;
    keylist_2.count = 3;

    char sentence_3[] = "$ASKER send you some $MAIL";
    struct KeyList keylist_3;
    init_keylist(&keylist_3);
    struct KeyListItem key_3_1, key_3_2;
    init_keylistitem(&key_3_1); init_keylistitem(&key_3_2);
    key_3_1.key = 3; key_3_2.key = 5;
    keylist_3.first = &key_3_1; key_3_1.next = &key_3_2;
    keylist_3.count = 2;

    struct Trie * trie = (struct Trie *)malloc(sizeof(struct Trie));
    init_trie(trie);

    sentence_into_trie(trie, &keylist_1, sentence_1);
    sentence_into_trie(trie, &keylist_2, sentence_2);
    //sentence_into_trie(trie, &keylist_3, sentence_3);
    print_trie(trie);
}