# LL(1) Predictive Parser in C++

A C++ implementation of a **table-driven LL(1) predictive parser** for arithmetic expressions.

This project demonstrates how a predictive parser can recognize an input string using:

* **FIRST sets**
* **FOLLOW sets**
* **LL(1) parsing table**
* **Stack-based predictive parsing**
* **One-symbol lookahead**
* **Step-by-step parsing trace**

The implementation is designed to make the internal working of an LL(1) parser visible rather than treating the parser as a black box.

---

## 📌 Overview

An LL(1) parser is a top-down parser that constructs a **leftmost derivation** of an input string while using only one symbol of lookahead to determine which production rule should be applied.

The parser implemented here follows the standard table-driven architecture:

```text
                ┌────────────────────┐
                │   Input String     │
                │ id + id * id $      │
                └─────────┬──────────┘
                          │
                          ▼
                 ┌──────────────────┐
                 │ Predictive Parser│
                 └───────┬──────────┘
                         │
              ┌──────────┴──────────┐
              ▼                     ▼
       ┌──────────────┐      ┌───────────────┐
       │    Stack     │      │ Parsing Table │
       └──────────────┘      └───────────────┘
                                      ▲
                                      │
                              FIRST / FOLLOW
```

For every step, the parser examines:

1. The symbol on top of the stack.
2. The current input token.
3. The corresponding entry in the LL(1) parsing table.

Based on these two symbols, it either:

* Matches a terminal.
* Expands a non-terminal using a production.
* Reports an error.
* Accepts the input.

---

## 🧠 Grammar

The parser currently works with the following standard expression grammar:

```text
E  → T E'
E' → + T E' | ε
T  → F T'
T' → * F T' | ε
F  → ( E ) | id
```

This grammar is suitable for LL(1) parsing because it does not contain left recursion and the productions can be distinguished using a single lookahead symbol.

### Why this grammar?

A naive expression grammar such as:

```text
E → E + T | T
```

contains **left recursion**, which is unsuitable for a standard predictive LL(1) parser.

The grammar used here eliminates left recursion:

```text
E  → T E'
E' → + T E' | ε
```

The same transformation is applied to multiplication:

```text
T  → F T'
T' → * F T' | ε
```

This makes the grammar suitable for constructing an LL(1) parsing table.

---

# 🔍 Features

## 1. FIRST Set Computation

The program automatically computes FIRST sets for all non-terminals.

For example:

```text
FIRST(E)  = { (, id }
FIRST(E') = { +, ε }
FIRST(T)  = { (, id }
FIRST(T') = { *, ε }
FIRST(F)  = { (, id }
```

The implementation repeatedly applies the FIRST-set rules until no additional symbols can be added.

---

## 2. FOLLOW Set Computation

The program also calculates FOLLOW sets using the standard LL(1) rules.

For the given grammar, the expected sets are:

```text
FOLLOW(E)  = { ), $ }
FOLLOW(E') = { ), $ }
FOLLOW(T)  = { +, ), $ }
FOLLOW(T') = { +, ), $ }
FOLLOW(F)  = { *, +, ), $ }
```

The end-of-input marker `$` is initially placed in the FOLLOW set of the start symbol:

```text
FOLLOW(E) = { $ }
```

The remaining symbols are then propagated according to the grammar.

---

## 3. LL(1) Parsing Table Construction

Once FIRST and FOLLOW sets have been calculated, the parser constructs the LL(1) parsing table.

The table is represented internally as:

```cpp
map<string, map<string, vector<string>>> parsingTable;
```

Conceptually, the table has:

```text
Rows    → Non-terminals
Columns → Terminals
Cells   → Production rules
```

For example:

```text
        id          +          *          (          )          $
--------------------------------------------------------------------
E       E→TE'                              E→TE'
E'                  E'→+TE'                          E'→ε       E'→ε
T       T→FT'                              T→FT'
T'                  T'→ε       T'→*FT'                T'→ε       T'→ε
F       F→id                               F→(E)
```

Empty cells are treated as parsing errors.

---

# ⚙️ Parsing Algorithm

The parser uses a stack initialized as:

```text
$ E
```

The input is terminated using:

```text
$
```

For an input such as:

```text
id + id * id
```

the parser internally processes:

```text
id + id * id $
```

At every step:

### Case 1 — Top of stack is a terminal

If:

```text
STACK_TOP == INPUT_SYMBOL
```

the terminal is matched and removed from the stack.

Example:

```text
Stack      Input
id ...     id ...

→ Match id
```

---

### Case 2 — Top of stack is a non-terminal

The parser looks up:

```text
M[STACK_TOP][INPUT_SYMBOL]
```

For example:

```text
M[E][id] = E → T E'
```

Therefore:

```text
E
```

is replaced by:

```text
T E'
```

Since the stack follows LIFO ordering, the production's symbols are pushed in reverse order.

---

### Case 3 — No valid table entry

If:

```text
M[A][a]
```

contains no production, the parser reports:

```text
ERROR
```

and terminates the parsing process.

---

### Case 4 — Both stack and input contain `$`

When:

```text
Stack top = $
Input symbol = $
```

the parser reports:

```text
Accept
```

meaning that the complete input string belongs to the language generated by the grammar.

---

# 🏗️ Implementation Structure

The implementation is organized around several core components.

### Grammar Representation

```cpp
map<string, vector<vector<string>>> grammar;
```

Each non-terminal maps to a collection of possible productions.

---

### Symbol Sets

```cpp
set<string> nonTerminals;
set<string> terminals;
```

These identify the grammar symbols used during FIRST, FOLLOW, and parsing-table construction.

---

### FIRST Sets

```cpp
map<string, set<string>> FIRST;
```

The function:

```cpp
computeFIRST()
```

calculates FIRST sets iteratively until a fixed point is reached.

---

### FOLLOW Sets

```cpp
map<string, set<string>> FOLLOW;
```

The function:

```cpp
computeFOLLOW()
```

propagates FOLLOW information through the grammar until no further changes occur.

---

### FIRST of a Sequence

```cpp
firstOfSequence(...)
```

This helper function is important when processing productions such as:

```text
T E'
```

Instead of calculating FIRST only for an individual symbol, it determines:

```text
FIRST(T E')
```

while correctly handling epsilon propagation.

---

### Parsing Table

```cpp
map<string, map<string, vector<string>>> parsingTable;
```

The function:

```cpp
constructParsingTable()
```

uses FIRST and FOLLOW sets to populate the LL(1) table.

---

### Parser

The actual predictive parsing is performed by:

```cpp
parseInput()
```

It maintains:

```text
Stack
Input pointer
Matched symbols
Current input token
Parsing action
```

and prints a complete parsing trace.

---

# 📊 Parsing Trace

For:

```text
id + id * id
```

the parser produces a trace containing columns such as:

```text
Matched        Stack                    Input                    Action
--------------------------------------------------------------------------------
               $ E                     id + id * id $           Output E -> T E'
               $ E' T                  id + id * id $           Output T -> F T'
               $ E' T' F               id + id * id $           Output F -> id
               $ E' T' id              id + id * id $           Match id
id             $ E' T'                 + id * id $              Output T' -> ε
id             $ E'                    + id * id $              Output E' -> + T E'
id             $ E' T +                + id * id $              Match +
...
```

The trace makes the parser's decisions observable at every stage.

---

# 🛠️ Technologies Used

* **Language:** C++
* **Standard Library:** STL
* **Data Structures:**

  * `map`
  * `set`
  * `vector`
  * `string`
* **Concepts:**

  * Compiler Design
  * Context-Free Grammars
  * FIRST/FOLLOW
  * LL(1) Parsing
  * Predictive Parsing
  * Stack-based Parsing

---

# 🚀 Getting Started

## Prerequisites

You need a C++ compiler supporting standard C++ features.

Recommended:

* GCC / MinGW
* Clang
* MSVC

Check your compiler:

```bash
g++ --version
```

---

## Clone the Repository

```bash
git clone https://github.com/mayanksinharay/ll1-predictive-parser.git
cd <repository-name>
```

---

## Compile

Using GCC:

```bash
g++ main.cpp -o parser
```

On Windows:

```bash
g++ main.cpp -o parser.exe
```

---

## Run

Linux/macOS:

```bash
./parser
```

Windows:

```bash
parser.exe
```

The program will first display:

1. FIRST sets
2. FOLLOW sets
3. LL(1) parsing table

It will then request an input string:

```text
Enter input string:
```

For example:

```text
id + id * id
```

---

# 🧪 Example Input

```text
id + id * id
```

The `$` end marker is automatically appended by the program.

The parser should eventually reach:

```text
Accept
```

indicating that the input is successfully recognized by the grammar.

---

# 📁 Suggested Repository Structure

```text
LL1-Predictive-Parser/
│
├── main.cpp
├── README.md
└── .gitignore
```

For a larger version of the project, the implementation can later be divided into:

```text
LL1-Predictive-Parser/
│
├── src/
│   ├── first_follow.cpp
│   ├── parsing_table.cpp
│   └── parser.cpp
│
├── include/
│   └── parser.h
│
├── examples/
│   └── sample_input.txt
│
├── README.md
└── .gitignore
```

---

# 🔬 How the Components Work Together

The complete pipeline is:

```text
             Grammar
                │
                ▼
       ┌─────────────────┐
       │ Identify Symbols│
       └────────┬────────┘
                │
                ▼
       ┌─────────────────┐
       │   FIRST Sets    │
       └────────┬────────┘
                │
                ▼
       ┌─────────────────┐
       │   FOLLOW Sets   │
       └────────┬────────┘
                │
                ▼
       ┌─────────────────┐
       │ Parsing Table   │
       │    M[A, a]      │
       └────────┬────────┘
                │
                ▼
       ┌─────────────────┐
       │ Predictive      │
       │ Parser + Stack  │
       └────────┬────────┘
                │
                ▼
          Accept / Error
```

This separation is useful because each stage depends on the result of the previous stage.

---

# 📐 LL(1) Decision Process

The central decision made by the parser can be summarized as:

```text
             ┌──────────────────────┐
             │ Top of Stack = X     │
             │ Lookahead = a        │
             └──────────┬───────────┘
                        │
             ┌──────────▼───────────┐
             │ Is X a terminal?     │
             └───────┬───────┬─────┘
                     │Yes    │No
                     ▼       ▼
                ┌────────┐  ┌──────────────┐
                │ X == a?│  │ Lookup M[X,a]│
                └───┬────┘  └──────┬───────┘
                    │              │
              Yes ──┘              │
                    │         ┌────▼────┐
                    ▼         │Production│
                  Match       │ available│
                              └────┬─────┘
                                   │
                                   ▼
                              Expand stack
```

---

# ⚠️ Current Scope and Limitations

This implementation intentionally focuses on understanding the mechanics of an LL(1) parser rather than building a complete parser-generator framework.

Current limitations include:

* The grammar is defined directly in the C++ source code.
* The start symbol is currently fixed as `E`.
* The parser is designed around the provided expression grammar.
* Tokenization is lightweight and recognizes identifiers using the `id` token representation.
* Parsing-table conflicts are not explicitly reported.
* The implementation assumes that the supplied grammar is suitable for LL(1) parsing.
* Error recovery is not implemented; parsing stops when an error is encountered.

These limitations also provide natural directions for future development.

---

# 🔮 Future Improvements

Possible extensions include:

### 1. Generic Grammar Input

Allow users to provide grammar productions dynamically instead of defining them directly in the source.

### 2. Automatic Terminal / Non-terminal Detection

Instead of manually maintaining:

```cpp
nonTerminals
terminals
```

the parser could infer them from the grammar.

### 3. LL(1) Conflict Detection

The parser could detect cases where multiple productions are assigned to the same:

```text
M[A, a]
```

cell and report that the grammar is not LL(1).

### 4. Better Lexer

Replace the current lightweight tokenizer with a proper lexical analyzer supporting:

```text
identifiers
integers
floating-point numbers
operators
keywords
parentheses
```

### 5. Parse Tree Generation

The parser could construct an explicit parse tree rather than only displaying the derivation steps.

### 6. Syntax Error Recovery

Implement error recovery techniques such as:

* Panic-mode recovery
* Synchronizing tokens
* FOLLOW-set based recovery

### 7. Grammar Visualization

Generate graphical representations of:

* Parse trees
* FIRST/FOLLOW relationships
* Parsing tables

---

# 📚 Concepts Demonstrated

This project provides practical implementation of several compiler-design concepts:

```text
Context-Free Grammar
        ↓
Grammar Transformation
        ↓
FIRST Sets
        ↓
FOLLOW Sets
        ↓
LL(1) Parsing Table
        ↓
Predictive Parsing
        ↓
Leftmost Derivation
        ↓
Accepted / Rejected Input
```

It therefore serves as a compact implementation of the core ideas behind **top-down syntax analysis**.

---

# 🎯 Example Expressions

The grammar supports expressions involving:

### Identifiers

```text
id
```

### Addition

```text
id + id
```

### Multiplication

```text
id * id
```

### Operator precedence

```text
id + id * id
```

which is interpreted according to the grammar as:

```text
id + (id * id)
```

### Parenthesized expressions

```text
(id + id) * id
```

---

# 💡 Learning Objective

The main purpose of this implementation is to understand what happens **inside a predictive parser**.

Rather than simply using a parser generator, this project explicitly implements:

```text
FIRST
   ↓
FOLLOW
   ↓
Parsing Table
   ↓
Stack Operations
   ↓
Production Selection
   ↓
Input Matching
```

This makes it possible to observe the complete decision-making process of an LL(1) parser.

---

# 📖 References

The implementation is based on standard compiler-design concepts related to:

* Top-down parsing
* Predictive parsing
* LL(1) grammars
* FIRST and FOLLOW sets
* Table-driven parsing
* Context-free grammars

A useful reference for these concepts is:

> Alfred V. Aho, Monica S. Lam, Ravi Sethi, Jeffrey D. Ullman — *Compilers: Principles, Techniques, and Tools*

---

# 👨‍💻 Author

**Mayank Sinharay**

B.Tech — Computer Science & Engineering

IIIT Kalyani

---

## ⭐ Project Status

**Status:** Completed / Functional

The current implementation successfully demonstrates the complete LL(1) predictive parsing pipeline for the supported expression grammar, including FIRST/FOLLOW computation, parsing-table generation, and stack-based input parsing.
