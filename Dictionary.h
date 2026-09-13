#ifndef DICTIONARY_H
#define DICTIONARY_H

typedef struct KeyValue KeyValue;
typedef struct Dictionary Dictionary;

Dictionary * createDictiory();
void destroyDictionary(Dictionary * self);

//setters;
void addDictionaryElement(Dictionary * self, char c, char * value);
char * getDictionaryValue(Dictionary * self, char c);

#endif