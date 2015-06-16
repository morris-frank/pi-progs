#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <wchar.h>

#define MALLOC_NEW(x) (x *)malloc(sizeof(x))

typedef int KeyType;

typedef char CHAR;
typedef CHAR * STRING;

int rand_int(int max)
{
    return 0;
}

void print_times(STRING string, int x)
{
    for(x; x > 0; x--)
        wprintf(L"%s", string);
}

struct SentenceList
{
    int count;
    STRING * list;
};

struct KeyListItem
{
    KeyType key;
    struct KeyListItem * next;
};

struct KeyList
{
    int count;
    struct KeyListItem * first;
};

struct Node
{
    KeyType key;
    struct Node * parent;
    struct Node * sibling;
    struct Node * junior;
    struct Node * child;
    unsigned int hasSentences : 1;
    struct SentenceList * sentenceList;
    int sentenceCount;
};

struct Trie
{
    struct Node * root;
    int sentenceCount;
};

void init_sentencelist(struct SentenceList * list)
{
    list->count = 0;
    list->list = (STRING *)malloc(0 * sizeof(STRING));
}

void init_keylistitem(struct KeyListItem * key)
{
    key->key = 0;
    key->next = 0;
}

inline void next(struct KeyListItem * key){
    key = key->next;
}

void init_keylist(struct KeyList * list)
{
    list->count = 0;
    list->first = 0;
}

void init_node
(
        struct Node * node, 
        struct Node * parent, 
        struct Node * child, 
        struct Node * junior, 
        struct Node * sibling
)
{
    node->key = 0;
    node->parent = parent;
    node->child = child;
    node->sibling = sibling;
    node->junior = junior;
    node->hasSentences = 0;
    node->sentenceList = NULL;
    node->sentenceCount = 1;
}

inline void parent(struct Node * node){
    node = node->parent;
}

inline void sibling(struct Node * node){
    node = node->sibling;
}

inline void junior(struct Node * node){
    node = node->junior;
}

inline void child(struct Node * node){
    node = node->child;
}

void init_trie(struct Trie * trie)
{
    trie->root = MALLOC_NEW(struct Node);
    init_node(trie->root, NULL, NULL, NULL, NULL);
    trie->sentenceCount = 0;
}

void append_sentence(struct SentenceList * list, STRING sentence)
{
    int old_count = list->count;
    list->list = (STRING *)realloc(list->list, (old_count+1) * sizeof(STRING) );
    list->list[old_count] = (STRING)malloc(40 * sizeof(CHAR) );
    //wcsncpy(list->list[old_count], sentence, sizeof(list->list[old_count]) );
    strncpy(list->list[old_count], sentence, sizeof(list->list[old_count]) );
    wprintf(L"%d -> %p -> %s \n", old_count, list->list[old_count], sentence);
    wprintf(L"%c_%lc\n", list->list[old_count][6], list->list[old_count][6]);
    list->count++;
}

struct SentenceList * malloc_sentences(STRING sentence)
{
    struct SentenceList * list = MALLOC_NEW(struct SentenceList);
    init_sentencelist(list);
    append_sentence(list, sentence);
    wprintf(L"malloc: %s\n", list->list[0]);
    return list;
}

void sort_keylist(struct KeyList * list)
{
    return;
}

struct Node * search_node_in_trie(struct Trie * trie, struct KeyList * keylist)
{
    struct Node * nIt = trie->root->child;
    struct KeyListItem * lIt = keylist->first;
    struct Node * last_sentenced = trie->root;

    while(lIt && nIt){
        if(nIt->key == lIt->key){
            if(nIt && nIt->hasSentences)
                last_sentenced = nIt;
            nIt = nIt->child;
            next(lIt);
        }else{
            nIt = nIt->sibling;
        }
    }
    return last_sentenced;
}

STRING search_sentence_in_trie(struct Trie * trie, struct KeyList * keylist)
{
    struct Node * found_node = search_node_in_trie(trie, keylist);
    int sentence_id = rand_int(found_node->sentenceList->count);
    return found_node->sentenceList->list[sentence_id];
}

void sentence_into_trie(struct Trie * trie, struct KeyList * keylist, STRING sentence )
{
    struct Node * nIt = trie->root->child;
    struct KeyListItem * lIt = keylist->first;
    struct Node * sibl;

    for(; nIt && lIt; next(lIt)){
        for(; nIt->sibling && nIt->key < lIt->key; sibling(nIt));

        if(nIt->key == lIt->key){
            nIt->sentenceCount++;
            child(nIt);
            continue;
        }

        if(nIt->key > lIt->key)
            if(!nIt->junior){
                sibl = nIt;
                parent(nIt);
                break;
            }else
                junior(nIt);

        sibl = nIt->sibling;

        nIt->sibling = MALLOC_NEW(struct Node);
        init_node(nIt->sibling, nIt->parent, NULL, nIt, sibl);
        sibling(nIt);
        nIt->key = lIt->key;
        sibl->junior = nIt;
        sibl = NULL;
    }

    //Hängt noch solange Kinder an wie der Stack elemente hat
    for(; lIt; next(lIt), child(nIt)){
        nIt->child = MALLOC_NEW(struct Node);
        init_node(nIt->child, nIt, NULL, NULL, sibl);
        if(sibl)
            sibl->junior = nIt->child;
        sibl = NULL;
        nIt->key = lIt->key;
    }

    if( !nIt->hasSentences ){
        nIt->hasSentences = 1;
        nIt->sentenceList = malloc_sentences(sentence);
        wprintf(L"into_trie: %s\n", nIt->sentenceList->list[0]);
    }else{
        append_sentence(nIt->sentenceList, sentence);
    }
}

void print_trie_prefixed(struct Node * root, int prefix)
{
    struct Node * nIt;
    print_times("    ", prefix);
    wprintf(L" | \n");
    print_times("    ", prefix);

    for(nIt = root; nIt; child(nIt))
        if( nIt->hasSentences == 0 )
            wprintf(L" %d -", nIt->key);
        else
            wprintf(L"[%d]-", nIt->key);

    wprintf(L"\n");

    for(nIt = root; nIt; child(nIt), prefix++)
        if(nIt->sibling != 0)
            print_trie_prefixed(nIt->sibling, prefix);
}

void print_trie(struct Trie * trie)
{
    print_trie_prefixed(trie->root->child, 0);
}

struct Trie * create_trie()
{
    setlocale(LC_ALL, "");
    struct Trie * trie = MALLOC_NEW(struct Trie);
    init_trie(trie);
    return trie;
}

struct KeyList * create_keylist()
{
    struct KeyList * list = MALLOC_NEW(struct KeyList);
    init_keylist(list);
    return list;
}

struct KeyListItem * create_keylistitem(KeyType key)
{
    struct KeyListItem * item = MALLOC_NEW(struct KeyListItem);
    init_keylistitem(item);
    item->key = key;
    return item;
}

void place_into_keylist(struct KeyList * list, struct KeyListItem * item)
{
    struct KeyListItem * lIt = list->first;

    if( !lIt ){
        list->first = item;
        list->count++;
        return;
    }

    while(lIt->next && lIt->next->key < item->key)
        next(lIt);

    if(!lIt->next || lIt->next->key < item->key){
        item->next = lIt->next;
        lIt->next = item;
        list->count++;
    }
}

void create_and_place_into_keylist(struct KeyList * list, KeyType key)
{
    place_into_keylist(list, create_keylistitem(key));
}

struct KeyList * keylist_from_array(int size, KeyType arr[size])
{
    struct KeyList * list = create_keylist();
    int i = 0;
    for(; i < size; i++)
        create_and_place_into_keylist(list, arr[i]);
    return list;
}
