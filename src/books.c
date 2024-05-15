#include "../header/book.h"
#include "../header/borrower.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_DATE_LENGTH 20


// Add genre to the BST
void addGenre(struct BSTNode **root, const char *genreName) {
    struct BSTNode *newNode = (struct BSTNode *)malloc(sizeof(struct BSTNode));
    if (newNode == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    strcpy(newNode->genre.name, genreName);
    newNode->genre.numBooks = 0;
    newNode->left = newNode->right = NULL;

    *root = newNode;
}

// Add book to the BST
void addBook(struct BSTNode **root, const char *genreName, struct LibraryBook *book) {
    if (*root == NULL) {
        addGenre(root, genreName);
    }

    struct BSTNode *current = *root;
    struct BSTNode *parent = NULL;

    while (current != NULL) {
        parent = current;
        if (strcmp(genreName, current->genre.name) < 0) {
            current = current->left;
        } else if (strcmp(genreName, current->genre.name) > 0) {
            current = current->right;
        } else {
            // Add book to the genre
            current->genre.books[current->genre.numBooks++] = *book;
            return;
        }
    }

    struct BSTNode *newNode = (struct BSTNode *)malloc(sizeof(struct BSTNode));
    if (newNode == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    strcpy(newNode->genre.name, genreName);
    newNode->genre.books[newNode->genre.numBooks++] = *book;
    newNode->left = newNode->right = NULL;

    if (strcmp(genreName, parent->genre.name) < 0) {
        parent->left = newNode;
    } else {
        parent->right = newNode;
    }
}

// Display all books in the BST
void displayAllBooks(struct BSTNode *root) {
    if (root != NULL) {
        displayAllBooks(root->left);

        printf("Genre: %s\n", root->genre.name);
        for (int i = 0; i < root->genre.numBooks; i++) {
            struct LibraryBook *book = &root->genre.books[i];
            printf("Title: %s\n", book->title);
            printf("Author: %s\n", book->author);
            printf("ISBN: %s\n", book->ISBN);
            printf("Year Published: %d\n", book->yearPublished);
            printf("Issue Date: %s", asctime(localtime(&book->issueDate)));
            printf("Return Date: %s", asctime(localtime(&book->returnDate)));
            printf("\n");
        }

        displayAllBooks(root->right);
    }
}

// Remove all copies of the Book from the BST
void removeBook(struct BSTNode **root, const char *genreName, const char *ISBN) {
    if (*root == NULL) {
        return;
    }

    struct BSTNode *current = *root;
    struct BSTNode *parent = NULL;

    while (current != NULL) {
        if (strcmp(genreName, current->genre.name) < 0) {
            parent = current;
            current = current->left;
        } else if (strcmp(genreName, current->genre.name) > 0) {
            parent = current;
            current = current->right;
        } else {
            // Search for the book in the genre
            for (int i = 0; i < current->genre.numBooks; i++) {
                if (strcmp(ISBN, current->genre.books[i].ISBN) == 0) {
                    // Remove the book by shifting all books to the left
                    for (int j = i; j < current->genre.numBooks - 1; j++) {
                        current->genre.books[j] = current->genre.books[j + 1];
                    }
                    current->genre.numBooks--;
                    return;
                }
            }
            return;
        }
    }
}


// Search for a book in the BST
struct LibraryBook *searchBook(struct BSTNode *root, const char *genreName, const char *ISBN) {
    if (root == NULL) {
        return NULL;
    }

    struct BSTNode *current = root;

    while (current != NULL) {
        if (strcmp(genreName, current->genre.name) < 0) {
            current = current->left;
        } else if (strcmp(genreName, current->genre.name) > 0) {
            current = current->right;
        } else {
            // Search for the book in the genre
            for (int i = 0; i < current->genre.numBooks; i++) {
                if (strcmp(ISBN, current->genre.books[i].ISBN) == 0) {
                    return &current->genre.books[i];
                }
            }
            return NULL;
        }
    }

    return NULL;
}

// check if the book is available
int isBookAvailable(struct BSTNode *root, const char *genreName, const char *ISBN) {
    struct LibraryBook *book = searchBook(root, genreName, ISBN);
    if (book == NULL) {
        return 0;
    }

    return book->isAvailable;
}


void borrowBook(struct BSTNode *root, const char *genreName, const char *ISBN, struct Borrower *borrower) {
    struct LibraryBook *book = searchBook(root, genreName, ISBN);
    if (book == NULL) {
        printf("Book not found\n");
        return;
    }

    if (book->isAvailable) {
        if (book->numCopies > 0) {
            book->numCopies--; // Decrement number of copies
            if (book->numCopies == 0) {
                book->isAvailable = 0; // Set availability to false if no copies left
            }
            if (borrower->numBorrowedBooks < 3) {
                book->borrower = *borrower;
                book->issueDate = time(NULL);
                borrower->borrowedBooks[borrower->numBorrowedBooks++] = book;
                printf("Book borrowed successfully\n");
            } else {
                printf("You have already borrowed 3 books\n");
            }
        } else {
            printf("No copies available for borrowing\n");
        }
    } else {
        printf("Book is not available\n");
    }
}




// Display all books borrowed by a borrower
void displayBorrowedBooks(struct BSTNode *root, struct Borrower *borrower) {
    for (int i = 0; i < borrower->numBorrowedBooks; i++) {
        struct LibraryBook *book = borrower->borrowedBooks[i];
        printf("Title: %s\n", book->title);
        printf("Author: %s\n", book->author);
        printf("ISBN: %s\n", book->ISBN);
        printf("Year Published: %d\n", book->yearPublished);
        printf("Issue Date: %s", asctime(localtime(&book->issueDate)));
        printf("Return Date: %s", asctime(localtime(&book->returnDate)));
        printf("\n");
    }
}

// Calculate fine for a borrower
int calculateFine(struct BSTNode *root, struct Borrower *borrower) {
    int fine = 0;
    time_t currentTime = time(NULL);

    for (int i = 0; i < borrower->numBorrowedBooks; i++) {
        struct LibraryBook *book = borrower->borrowedBooks[i];
        if (currentTime > book->returnDate) {
            fine += (currentTime - book->returnDate) / (24 * 60 * 60) * 10; // 10 rupees per day
        }
    }

    return fine;
}

// Return a book
void returnBook(struct BSTNode *root, const char *genreName, const char *ISBN, struct Borrower *borrower) {
    struct LibraryBook *book = searchBook(root, genreName, ISBN);
    if (book == NULL) {
        printf("Book not found\n");
        return;
    }

    if (!book->isAvailable) {
        for (int i = 0; i < borrower->numBorrowedBooks; i++) {
            if (borrower->borrowedBooks[i] == book) {
                time_t currentTime = time(NULL);
                int fine = calculateFine(root, borrower); // Calculate fine
                if (fine > 0) {
                    printf("You have a fine of %d rupees. Please pay before returning the book.\n", fine);
                    return;
                }
                book->isAvailable = 1;
                book->issueDate = 0; // Reset issue date
                book->returnDate = 0; // Reset return date
                book->borrower = (struct Borrower){0};
                for (int j = i; j < borrower->numBorrowedBooks - 1; j++) {
                    borrower->borrowedBooks[j] = borrower->borrowedBooks[j + 1];
                }
                borrower->numBorrowedBooks--;
                printf("Book returned successfully\n");
                return;
            }
        }
        printf("You have not borrowed this book\n");
    } else {
        printf("Book is already available\n");
    }
}


// Check if a borrower is late in returning books
int isBorrowerLate(struct BSTNode *root, struct Borrower *borrower) {
    time_t currentTime = time(NULL);

    for (int i = 0; i < borrower->numBorrowedBooks; i++) {
        struct LibraryBook *book = borrower->borrowedBooks[i];
        if (currentTime > book->returnDate) {
            return 1;
        }
    }

    return 0;
}

// Display all genres in the BST
void displayAllGenres(struct BSTNode *root) {
    if (root != NULL) {
        displayAllGenres(root->left);
        printf("Genre: %s\n", root->genre.name);
        displayAllGenres(root->right);
    }
}


// Helper function to read a string enclosed in double quotes
void readString(FILE *file, char *buffer, size_t bufferSize) {
    int c;
    size_t i = 0;
   
    while ((c = fgetc(file)) != EOF && (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '"')) {
        if (c == '"') {
            break; // Break if the opening double quote is encountered
        }
    }

    while ((c = fgetc(file)) != EOF && c != '"') {
        if (i < bufferSize - 1) {
            buffer[i++] = (char)c;
        }
    }
    buffer[i] = '\0'; 
}


void ReadDatabase(struct BSTNode **root, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char genreName[MAX_GENRE_LENGTH];
    struct LibraryBook book;
    char titleBuffer[MAX_TITLE_LENGTH];
    char authorBuffer[MAX_AUTHOR_LENGTH];
    char issueDateBuffer[MAX_DATE_LENGTH];
    char returnDateBuffer[MAX_DATE_LENGTH];
    int borrowerID;

    int n = 0;
    while (fscanf(file, "%s", genreName) == 1 && n < 7) {

        readString(file, titleBuffer, sizeof(titleBuffer));
        readString(file, authorBuffer, sizeof(authorBuffer));
        fscanf(file, "%s", book.ISBN);
        fscanf(file, "%d", &book.numCopies);
        fscanf(file, "%d", &book.isAvailable);
        fscanf(file, "%d", &book.yearPublished);
        fscanf(file, "%s", issueDateBuffer);
        fscanf(file, "%s", returnDateBuffer);
        fscanf(file, "%d", &borrowerID); 
        // Convert issueDate and returnDate to time_t
        struct tm tm = {0};
        strptime(issueDateBuffer, "%Y-%m-%d", &tm);
        book.issueDate = mktime(&tm);
        strptime(returnDateBuffer, "%Y-%m-%d", &tm);
        book.returnDate = mktime(&tm);

    
        // Add book to the BST
        addBook(root, genreName, &book);

        printf("\n");
    }

    fclose(file);
}



// Helper function to perform in-order traversal and write to file
void writeBSTToFileHelper(struct BSTNode *root, FILE *file) {
    if (root == NULL)
        return;

    writeBSTToFileHelper(root->left, file);

    // Write the current node to the file
    for (int i = 0; i < root->genre.numBooks; i++) {
        struct LibraryBook *book = &(root->genre.books[i]);
        fprintf(file, "%s \"%s\" \"%s\" %s %d %d %d %ld %ld\n", 
                root->genre.name, book->title, book->author, 
                book->ISBN, book->numCopies, book->isAvailable, 
                book->yearPublished, book->issueDate, book->returnDate);
    }

    writeBSTToFileHelper(root->right, file);
}

// Function to write BST to file
void writeBSTToFile(struct BSTNode *root, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }

    writeBSTToFileHelper(root, file);

    fclose(file);
}



int main(){
    struct BSTNode *root = NULL;
    ReadDatabase(&root, "../database/Books/books.txt");
    displayAllBooks(root);
    writeBSTToFile(root, "../database/Books/books.txt");
    return 0;
}