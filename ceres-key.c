#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <wchar.h>

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

struct TrieNode
{
    KeyType key;
    struct TrieNode * parent;
    struct TrieNode * sibling;
    struct TrieNode * child;
    unsigned int hasSentences : 1;
    struct SentenceList * sentenceList;
    int sentenceCount;
};

struct Trie
{
    struct TrieNode * root;
    int sentenceCount;
;};

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

void init_keylist(struct KeyList * list)
{
    list->count = 0;
    list->first = 0;
}

void init_trienode(struct TrieNode * node)
{
    node->key = 0;
    if( !node->parent )
        node->parent = NULL;
    node->child = NULL;
    node->sibling = NULL;
    node->hasSentences = 0;
    node->sentenceList = NULL;
    node->sentenceCount = 1;
}

void init_trie(struct Trie * trie)
{
    trie->root = (struct TrieNode *)malloc(sizeof(struct TrieNode));
    init_trienode(trie->root);
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
    struct SentenceList * list = (struct SentenceList *)malloc(sizeof(struct SentenceList));
    init_sentencelist(list);
    append_sentence(list, sentence);
    wprintf(L"malloc: %s\n", list->list[0]);
    return list;
}

void sort_keylist(struct KeyList * list)
{
    return;
}

struct TrieNode * search_node_in_trie(struct Trie * trie, struct KeyList * keylist)
{
    struct TrieNode * trie_iterator = trie->root->child;
    struct KeyListItem * keylist_iterator = keylist->first;
    struct TrieNode * last_sentenced = trie->root;

    while(keylist_iterator != 0 && trie_iterator != 0){
        if(trie_iterator->key == keylist_iterator->key){
            if(trie_iterator != 0 && trie_iterator->hasSentences != 0)
                last_sentenced = trie_iterator;
            trie_iterator = trie_iterator->child;
            keylist_iterator = keylist_iterator->next;
        }else{
            trie_iterator = trie_iterator->sibling;
        }
    }
    return last_sentenced;
}

STRING search_sentence_in_trie(struct Trie * trie, struct KeyList * keylist)
{
    struct TrieNode * found_node = search_node_in_trie(trie, keylist);
    int sentence_id = rand_int(found_node->sentenceList->count);
    return found_node->sentenceList->list[sentence_id];
}

void sentence_into_trie(struct Trie * trie, struct KeyList * keylist, STRING sentence )
{
    struct TrieNode * trie_iterator = trie->root->child;
    struct TrieNode * trie_iterator_prec = trie->root;
    struct KeyListItem * keylist_iterator = keylist->first;

    //Solange nicht an einem Trie Ende oder Listen Ende
    //hier soll trie_iterator_prec = trie_iterator-1 erfüllt sein!!!!
    while( trie_iterator && keylist_iterator ){
        //Suche in den Siblings
        //trie_iterator muss den _prec noch abhängen
        //if(trie_iterator == trie_iterator_prec)
        //    trie_iterator = trie_iterator->sibling;

        //Suche ersten größeren Sibling
        while( trie_iterator && trie_iterator->key < keylist_iterator->key ){
            trie_iterator_prec = trie_iterator;
            trie_iterator = trie_iterator->sibling;
        }

        //Gleich großen Sibling-Node gefunden, auf zum Kind
        if( trie_iterator && trie_iterator->key == keylist_iterator->key ){
            trie_iterator_prec = trie_iterator;
            trie_iterator->sentenceCount++;
            trie_iterator = trie_iterator->child;
            keylist_iterator = keylist_iterator->next;
            continue;
        }

        //Neuer Node wird Child
        if( trie_iterator && trie_iterator->key > keylist_iterator->key ){
            trie_iterator_prec->child = (struct TrieNode *)malloc(sizeof(struct TrieNode));
            trie_iterator_prec->child->parent = trie_iterator_prec;
            trie_iterator_prec = trie_iterator_prec->child;
        }else{
            trie_iterator_prec->sibling = (struct TrieNode *)malloc(sizeof(struct TrieNode));
            trie_iterator_prec->sibling->parent = trie_iterator_prec;
            trie_iterator_prec = trie_iterator_prec->sibling;
        }

        init_trienode(trie_iterator_prec);
        trie_iterator_prec->sibling = trie_iterator; // == 0 wenn trie_iterator = 0!
        trie_iterator_prec->key = keylist_iterator->key;
        keylist_iterator = keylist_iterator->next;
        trie_iterator = trie_iterator_prec->child; // == 0
    }

    if( !trie_iterator )
        trie_iterator = trie_iterator_prec;

    //Hängt noch solange Kinder an wie der Stack elemente hat
    for( ; keylist_iterator; keylist_iterator = keylist_iterator->next ){
        trie_iterator->child = (struct TrieNode *)malloc(sizeof(struct TrieNode));
        trie_iterator->child->parent = trie_iterator;
        trie_iterator = trie_iterator->child;
        init_trienode(trie_iterator);
        trie_iterator->key = keylist_iterator->key;
    }

    if( !trie_iterator->hasSentences ){
        trie_iterator->hasSentences = 1;
        trie_iterator->sentenceList = malloc_sentences(sentence);
        wprintf(L"into_trie: %s\n", trie_iterator->sentenceList->list[0]);
    }else{
        append_sentence(trie_iterator->sentenceList, sentence);
    }
}

void print_trie_prefixed(struct TrieNode * root, int prefix)
{
    struct TrieNode * trie_iterator;
    print_times("    ", prefix);
    wprintf(L" | \n");
    print_times("    ",prefix);

    for(trie_iterator = root; trie_iterator; trie_iterator = trie_iterator->child)
        if( trie_iterator->hasSentences == 0 )
            wprintf(L" %d -", trie_iterator->key);
        else
            wprintf(L"[%d]-", trie_iterator->key);

    wprintf(L"\n");

    for(trie_iterator = root; trie_iterator; trie_iterator = trie_iterator->child, prefix++)
        if(trie_iterator->sibling != 0)
            print_trie_prefixed(trie_iterator->sibling, prefix);
}

void print_trie(struct Trie * trie)
{
    print_trie_prefixed(trie->root->child, 0);
}

struct Trie * create_trie()
{
    setlocale(LC_ALL, "");
    struct Trie * trie = (struct Trie *)malloc(sizeof(struct Trie));
    init_trie(trie);
    return trie;
}

struct KeyList * create_keylist()
{
    struct KeyList * keylist = (struct KeyList *)malloc(sizeof(struct KeyList));
    init_keylist(keylist);
    return keylist;
}

struct KeyListItem * create_keylistitem(KeyType key)
{
    struct KeyListItem * keylistitem = (struct KeyListItem *)malloc(sizeof(struct KeyListItem));
    init_keylistitem(keylistitem);
    keylistitem->key = key;
    return keylistitem;
}

void place_into_keylist(struct KeyList * keylist, struct KeyListItem * keylistitem)
{
    struct KeyListItem * keylist_iterator = keylist->first;

    if( !keylist_iterator ){
        keylist->first = keylistitem;
        keylist->count++;
        return;
    }

    while( keylist_iterator->next && keylist_iterator->next->key < keylistitem->key )
        keylist_iterator = keylist_iterator->next;

    if( !keylist_iterator->next || keylist_iterator->next->key < keylistitem->key ){
        keylistitem->next = keylist_iterator->next;
        keylist_iterator->next = keylistitem;
        keylist->count++;
    }
}

void create_and_place_into_keylist(struct KeyList * keylist, KeyType key)
{
    place_into_keylist(keylist, create_keylistitem(key));
}

struct KeyList * keylist_from_array(int size, KeyType list[size])
{
    struct KeyList * keylist = create_keylist();
    int i;
    for( i = 0; i < size; i++ )
        create_and_place_into_keylist(keylist, list[i]);
    return keylist;
}
