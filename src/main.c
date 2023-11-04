#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<stdbool.h>
#include<regex.h>

#define MAX_TOKENS 100000
struct Token {
  // token value
  char* value;

  // token type
  char* type;              // number, string, operator, keywords, delimiters
};

struct Node {
  struct Token* left;
  struct Token* right;

  // Expressions: BINARY_EXPR, NUMBER_EXPR, STRING_EXPR, IDENTIFIER_EXPR
  // Statements: VAR_DECL
  // Error: ERROR
  char* kind;             
};


char* keywords[] = {"variable", "integer", "string"};
const int keyword_count = sizeof(keywords) / sizeof(keywords[0]);

struct Token tokens[MAX_TOKENS];
int token_count = 0;

void print_token(struct Token token) {
  printf("[TOKEN] VALUE: %s, TYPE: %s\n", token.value, token.type);
  return;
}

long get_file_size(FILE *fptr) {
  fseek(fptr, 0, SEEK_END);
  long size = ftell(fptr);
  fseek(fptr, 0, SEEK_SET); // bring file pointer to the start of the file

  return size;
}

char* get_string(char* begin, char* end) {
  int size = end - begin + 1;
  char* string = (char*)malloc(size * sizeof(char));
  
  memcpy(string, begin, size);
  
  return string;
}

bool is_a_string(char* str) {
  // example -> "this is a string"
  int length = strlen(str);
  if (length < 2) {
    return false;
  }
  char* start = &str[0];
  char* end = &str[length-1];
  if(*start == *end && *start == '"') {
    return true;
  }
  return false;
}

// function that takes in string and try to figure out token type for it and create a token
struct Token get_token(char* value) {
  struct Token token;
  token.value = value;
  int length = strlen(value);

  if(strcmp(value, "variable") == 0 || strcmp(value, "integer") == 0 || strcmp(value, "string") == 0 || strcmp(value, "duh") == 0) {
    token.type = "keyword";
  }
  else if(isdigit(*value)) {
    token.type = "integer";
  }
  else if(is_a_string(value) == true) {
    token.value  = get_string(&value[1], &value[length-2]);
    token.type = "string";
  }
  else if(strcmp(value, ";") == 0 || strcmp(value, ",") == 0) {
    token.type = "symbol";
  }
  else if(strcmp(value, "=") == 0 || strcmp(value, "+") == 0 || strcmp(value, "-") == 0 || strcmp(value, "/") == 0 || strcmp(value, "*") == 0) {
    token.type = "operator";
  }
  else if(isalpha(*value)) {
    token.type = "identifier";
  }
  else {
    printf("Unable to classify vector: %s", value);
    exit(0);
  }
  print_token(token);
  return token;
}

// emulates the functionality of strtok function to tokenize a given string by delimiter
char* tokenize(char* source) {
  static char* str;
  char* delimiter = " \n;";
  
  if (source != NULL) {
    str = source;
  }

  char* result = (char*)malloc(strlen(str) * sizeof(char) + 1);

  int dptr = 0;     // delimiter at which we are at
  static int it;
  int start = it;   // since `it` is a static variable, we start from where we left off

  while (str[it] != '\0') {
    dptr = 0;

    while(delimiter[dptr] != '\0') {
      if (str[it] == delimiter[dptr]) {
        str[it] = '\0';
        ++it; // point to the next character

        // check if valid string is present at the time we encounter delimiter
        if (str[start] != '\0') {
          return (&str[start]);
        } 
        else {
          start = it;
          --it;
          break;
        }
      }
      ++dptr;
    }

    ++it;
  }

  str[it] = '\0';

  if (str[start] == '\0') {
    return NULL;
  }
  else {
    return &str[start];
  }
}

void scanner(char *source) {
  char* begin = source;

  // skipping whitespaces at first
  while(*begin == ' ') {
    begin++;
  }

  char *end = begin;

  struct Token token;

  char *ptr = tokenize(source); 

  while (ptr != NULL) {
    char *lexeme = get_string(ptr, ptr + strlen(ptr) - 1);
    token = get_token(lexeme);
    tokens[token_count++] = token;
    // print_token(token);
    ptr = tokenize(NULL);
  }
}


void parser(struct Token* tokens) {
  int iterator = 0;
  while(iterator < token_count) {
    struct Token curr_token = tokens[iterator];

    if (strcmp(curr_token.type, "keyword") == 0) {

    }
  }
}

char* read_program(char* file_path) {
  if(!file_path) {
    printf("Unable to find file path of the program");
    return NULL;
  }

  FILE *fptr = fopen(file_path, "r");
  long file_size = get_file_size(fptr);

  char *program = (char*)calloc(file_size, sizeof(char));
  fread(program, sizeof(char), file_size, fptr);

  fclose(fptr);

  if (program == NULL) {
    free(program);
  }
  
  program[file_size] = '\0';

  return program;
}

// Duh compiler takes the str program file path as argument here
int main(int argc, char** argv) {
  if (argc != 2) {
    printf("Duh compiler only takes program file path as argument");
    exit(0);
  }

  // Step 1: Read program
  char* file_path = argv[1];
  char* program = read_program(file_path);
  
  // Step 2: Scanner/Lexer for the program to categorise program into tokens
  scanner(program);
  
  // Step 3: Parser: Construct AST from the tokens
  parser(tokens);

} 
