#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<stdbool.h>
#include<regex.h>

#define MAX_TOKENS 100000

typedef struct Token {
  char* value;
  char* type;              // number, string, operator, keywords, delimiters
} Token;

typedef enum {
  INTEGER,
  STRING,
  UNDEFINED
} VariableType;

typedef struct VariableNode {
  char *name;
  VariableType type;
  char* scope;
  char* value; // typecast it according to the variable type 
} VariableNode;

typedef struct VariableDeclarationNode {
  VariableNode* variable; 
} VariableDeclarationNode;

typedef enum {
  ADDITION,
  SUBSTRACTION,
  DIVISION,
  MULTIPLICATION
} BinaryOperator;

typedef struct BinaryExpressionNode {
  struct BinaryExpressionNode* left;
  struct BinaryExpressionNode* right;
  BinaryOperator operator;
} BinaryExpressionNode;

typedef enum {
  PROGRAM_NODE,
  BINARY_NODE,
  VARIABLE_NODE,
  VARIABLE_DECLARATION_NODE
} NodeType;

typedef struct Node {
  NodeType type;
  union {
    VariableNode variable;
    VariableDeclarationNode variable_decl;
    BinaryExpressionNode binary_expr;
  } data;
} ASTNode;

typedef struct ProgramNode {
  ASTNode** children;
  int childCount;
} ProgramNode;


void printASTNode(ASTNode* node) {
  switch(node->type) {
    case PROGRAM_NODE:
      printf("PROGRAM_NODE\n");
      break;
    case BINARY_NODE:
      printf("BINARY NODE\n");
      break;
    case VARIABLE_NODE:
      printf("VARIABLE NODE: name: %s, value: %s", node->data.variable.name, node->data.variable.value);
      break;
    case VARIABLE_DECLARATION_NODE:
      printf("VARIABLE DECLARATION NODE");
      break;
    default:
      printf("Unknown mode\n");
      break;
  }
} 

BinaryExpressionNode* createBinaryExpression(BinaryOperator op, BinaryExpressionNode* left, BinaryExpressionNode* right) {
  BinaryExpressionNode* node = (BinaryExpressionNode*) malloc(sizeof(BinaryExpressionNode));

  if (node != NULL) {
    node->operator = op;
    node->left = left;
    node->right = right;
  }

  return node;
}

VariableDeclarationNode* createVariableDeclarationNode(VariableNode* variableNode) {
  VariableDeclarationNode* node = (VariableDeclarationNode*) malloc(sizeof(VariableDeclarationNode));

  if(node != NULL) {
    node->variable = variableNode;
  }

  return node;
}

VariableNode* createVariableNode(char* name, char* value, VariableType type) {
  VariableNode* node = (VariableNode*) malloc(sizeof(VariableNode));

  if (node != NULL) {
    node->name = name;
    node->type = type;
    node->value = value;
    node->scope = NULL; // change it later
  }

  return node;
} 

ProgramNode* createProgramNode() {
  ProgramNode* node = (ProgramNode*) malloc(sizeof(ProgramNode));
  
  if (node != NULL) {
    node->children = NULL;
    node->childCount = 0;
  }

  return node;
}

void addChildNode(ProgramNode* program_node, ASTNode* child) {
  program_node->childCount++;
  program_node->children = realloc(program_node->children, program_node->childCount * sizeof(ASTNode));
  program_node->children[program_node->childCount - 1] = child;
}

void parseTree(ProgramNode* root) {
  if(root == NULL) {
    return;
  }
  for(int child_count = 0; child_count < root->childCount; child_count++) {
    printf("\n");
    parseTree((ProgramNode*)root->children[child_count]);
  }
  for(int i = 0; i < root->childCount; i++) {
    printASTNode(root->children[i]);
  }
}

char* keywords[] = {"variable", "integer", "string"};
const int keyword_count = sizeof(keywords) / sizeof(keywords[0]);

Token tokens[MAX_TOKENS];
int token_count = 0;

int get_variable_type(char* type) {
  if(strcmp(type, "integer") == 0) {
    return INTEGER;
  }
  if(strcmp(type, "string") == 0) {
    return STRING;
  }
  return UNDEFINED;
}

void print_token(Token token) {
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
Token get_token(char* value) {
  Token token;
  token.value = value;
  int length = strlen(value);

  if(strcmp(value, "variable") == 0 || strcmp(value, "integer") == 0 || strcmp(value, "string") == 0 || strcmp(value, "duh") == 0) {
    token.type = "keyword";
  }
  else if(isdigit(*value)) {
    token.type = "int";
  }
  else if(is_a_string(value) == true) {
    token.value  = get_string(&value[1], &value[length-2]);
    token.type = "str";
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

  Token token;

  char *ptr = tokenize(source); 

  while (ptr != NULL) {
    char *lexeme = get_string(ptr, ptr + strlen(ptr) - 1);
    token = get_token(lexeme);
    tokens[token_count++] = token;
    ptr = tokenize(NULL);
  }
}


// eg: variable integer x = 10
bool is_variable_declaration(int* curr_index, Token* tokens) {
  const int var_decl_len = 5;

  if (*curr_index < 0 || *curr_index > token_count || tokens == NULL) {
    return false;
  }

  // variable declaration should be of 5 tokens
  int tokens_left = token_count - *curr_index;
  if (tokens_left < var_decl_len) {
    return false;
  }

  // check for keyword variable
  if(strcmp(tokens[*curr_index].type, "keyword") == 0 && strcmp(tokens[*curr_index].value, "variable") == 0) {
    *curr_index = *curr_index + 1;
  } else {
    return false;
  }

  // check for keyword and should be a data type
  if(strcmp(tokens[*curr_index].type, "keyword") == 0 && (strcmp(tokens[*curr_index].value, "integer") == 0 || strcmp(tokens[*curr_index].value, "string") == 0)) {
    *curr_index = *curr_index + 1;
  } else {
    return false;
  }
  
  // this should be an identifier
  if (strcmp(tokens[*curr_index].type, "identifier") == 0) {
    *curr_index = *curr_index + 1;
  } else {

    return false;
  }

  // should be an equal operator
  if (strcmp(tokens[*curr_index].type, "operator") == 0 && strcmp(tokens[*curr_index].value, "=") == 0) {
    *curr_index = *curr_index + 1;
  } else {
    return false;
  }

  // should be the value and should be matching the data type
  if(strcmp(tokens[*curr_index].type, "int") == 0 && strcmp(tokens[*curr_index - 3].type, "integer")) {
    *curr_index = *curr_index + 1;
    return true;
  }

  if(strcmp(tokens[*curr_index].type, "str") == 0 && strcmp(tokens[*curr_index - 3].type, "string")) {
    *curr_index = *curr_index + 1;
    return true;
  }
  
  return false;
}


void parser(Token* tokens) {
  // create root node which is program node
  ProgramNode* root = createProgramNode();
  int iterator = 0;
  Token curr_token;
  while(iterator < token_count) {
    curr_token = tokens[iterator];
    // parse tokens into different expressions/declarations
    if (strcmp(curr_token.type, "keyword") == 0) {
      if(strcmp(curr_token.value, "variable") == 0) {
        int start_at = iterator;
        // should be variable declaration
        if (is_variable_declaration(&iterator, tokens)) {
          int var_type = get_variable_type(tokens[start_at+1].value);
          char* var_name = tokens[start_at+2].value;
          char* var_value = tokens[start_at+4].value;
          ASTNode* var_node = (ASTNode*)createVariableNode(var_name, var_value, var_type);
          var_node->type = VARIABLE_NODE;
          addChildNode(root, var_node);
        } else {
          break;
        }
      }
    } else {
      break;
    }
  }
  if (iterator < token_count) {
    printf("Syntax error while parsing %s", curr_token.value);
    exit(0);
  }
  parseTree(root);
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
