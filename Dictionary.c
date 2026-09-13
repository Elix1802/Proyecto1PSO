#include "Dictionary.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DIC_SIZE 256

struct KeyValue {
    char key;
    char * value;
};

struct Dictionary {
    KeyValue ** values;
};

//Aux
int hashFunction(char c) {
    unsigned char caracter = (unsigned char) c;
    return caracter % DIC_SIZE;
}

Dictionary * createDictionary() {
    Dictionary * dictionary = (Dictionary*) malloc(sizeof(Dictionary));
    if(dictionary == NULL) return NULL;
    dictionary->values = (KeyValue**) malloc(DIC_SIZE*sizeof(KeyValue*));

    if(dictionary->values == NULL) return NULL;

    for (int i = 0; i < DIC_SIZE; i++) {
        dictionary->values[i] = NULL; 
    }

    return dictionary;
}

void destroy(Dictionary * self) {
    if(self != NULL) {
        for (int i = 0; i < DIC_SIZE; i++) {
            if(self->values[i] != NULL) {
                free(self->values[i]);
            } 
        }
        free(self);
    }
}

void addDictionaryElement(Dictionary * self, char c, char * value){
    if(self != NULL) {
        KeyValue * keyValue = (KeyValue*)malloc(sizeof(KeyValue));
        keyValue->key = c;
        keyValue->value = (char*) malloc(strlen(value)+1);
        if(keyValue->value == NULL) {
            free(keyValue);
            return;
        }
        strcpy(keyValue->value, value);

        int index = hashFunction(c);
        self->values[index] = keyValue;
    } 
}


char * getDictionaryValue(Dictionary * self, char c) {
    if(self != NULL) {
        int index = hashFunction(c);
        return self->values[index]->value;
    }

    return NULL;
}

void printDictionaryValues(Dictionary * self) {
    if(self != NULL) {
        for(int i = 0; i < DIC_SIZE; i++) {
            if(self->values[i] != NULL) {
                printf("%c : %s\n", self->values[i]->key, self->values[i]->value);
            }
        }
        printf("\n");
    }
}
