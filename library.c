#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "library.h"

// Global variables to manage library state
Book* library = NULL;
int total_books = 0;
ReturnedBooksStack returned_books_stack;
BorrowRequestQueue borrow_request_queue;

// Function to add a new book to the library
void addBook() {
    // Reallocate memory to add a new book
    library = realloc(library, (total_books + 1) * sizeof(Book));
    
    Book new_book;
    
    printf("Enter Book ID: ");
    scanf("%d", &new_book.book_id);
    
    // Clear input buffer
    getchar();
    
    printf("Enter Book Title: ");
    fgets(new_book.title, MAX_TITLE_LENGTH, stdin);
    // Remove newline character
    new_book.title[strcspn(new_book.title, "\n")] = 0;
    
    printf("Enter Author Name: ");
    fgets(new_book.author, MAX_AUTHOR_LENGTH, stdin);
    // Remove newline character
    new_book.author[strcspn(new_book.author, "\n")] = 0;
    
    // New book is available by default
    new_book.is_available = true;
    
    // Add book to library
    library[total_books] = new_book;
    total_books++;
    
    printf("Book added successfully!\n");
}

// Function to borrow a book
void borrowBook() {
    int book_id;
    printf("Enter Book ID to borrow: ");
    scanf("%d", &book_id);
    
    // Search for the book
    for (int i = 0; i < total_books; i++) {
        if (library[i].book_id == book_id) {
            if (library[i].is_available) {
                // Book is available
                library[i].is_available = false;
                printf("Book \"%s\" borrowed successfully!\n", library[i].title);
                return;
            } else {
                // Book is not available, add to queue
                User new_user;
                printf("Book is currently unavailable. Adding to borrow request queue.\n");
                
                printf("Enter User ID: ");
                scanf("%d", &new_user.user_id);
                
                // Clear input buffer
                getchar();
                
                printf("Enter User Name: ");
                fgets(new_user.name, MAX_NAME_LENGTH, stdin);
                // Remove newline character
                new_user.name[strcspn(new_user.name, "\n")] = 0;
                
                new_user.requested_book_id = book_id;
                
                // Add user to borrow request queue
                enqueue(&borrow_request_queue, new_user);
                printf("You've been added to the borrow request queue.\n");
                return;
            }
        }
    }
    
    printf("Book not found in the library.\n");
}

// Function to return a book
void returnBook() {
    int book_id;
    printf("Enter Book ID to return: ");
    scanf("%d", &book_id);
    
    // Search for the book
    for (int i = 0; i < total_books; i++) {
        if (library[i].book_id == book_id) {
            if (!library[i].is_available) {
                // Mark book as available
                library[i].is_available = true;
                
                // Push to recently returned books stack
                pushToStack(&returned_books_stack, library[i]);
                
                printf("Book \"%s\" returned successfully!\n", library[i].title);
                
                // Process any pending borrow requests
                processRequests();
                return;
            } else {
                printf("This book was already in the library.\n");
                return;
            }
        }
    }
    
    printf("Book not found in the library.\n");
}



typedef struct {
    User* data;
    int size;
} queue;

// Function to peek at the next user in the queue
User  peekQueue(queue* q) {
    if (q->size > 0) {
        return q->data[0];
    } else {
        // Handle the case where the queue is empty
        User empty_user;
        empty_user.user_id = -1;
        strcpy(empty_user.name, "Unknown");
        empty_user.requested_book_id = -1;
        return empty_user;
    }
}

// Function to process borrow requests
void processRequests() {
    while (!isQueueEmpty(&borrow_request_queue)) {
        // Check if any requested book is now available
        User next_user = peekQueue(&borrow_request_queue);
        
        for (int i = 0; i < total_books; i++) {
            if (library[i].book_id == next_user.requested_book_id) {
                if (library[i].is_available) {
                    // Remove user from queue
                    dequeue(&borrow_request_queue);
                    
                    // Mark book as unavailable
                    library[i].is_available = false;
                    
                    printf("Book \"%s\" borrowed by %s (User ID: %d)\n",
                           library[i].title, next_user.name, next_user.user_id);
                    break;
                }
            }
        }
        break;
    }
}

// Function to search for a book
void searchBook() {
    int book_id;
    printf("Enter Book ID to search: ");
    scanf("%d", &book_id);
    
    // Search for the book
    for (int i = 0; i < total_books; i++) {
        if (library[i].book_id == book_id) {
            printf("Book Found:\n");
            printf("Title: %s\n", library[i].title);
            printf("Author: %s\n", library[i].author);
            printf("Available: %s\n", library[i].is_available ? "Yes" : "No");
            return;
        }
    }
    
    printf("Book not found in the library.\n");
}

// Function to display recently returned books
void displayReturnedBooks() {
    if (isStackEmpty(&returned_books_stack)) {
        printf("No recently returned books.\n");
        return;
    }
    
    printf("Recently Returned Books:\n");
    for (int i = returned_books_stack.top; i >= 0; i--) {
        Book book = returned_books_stack.items[i];
        printf("Book ID: %d, Title: %s, Author: %s\n",
               book.book_id, book.title, book.author);
    }
}

// Function to display borrow request queue
void displayBorrowRequests() {
    if (isQueueEmpty(&borrow_request_queue)) {
        printf("No pending borrow requests.\n");
        return;
    }
    
    printf("Borrow Request Queue:\n");
    int count = borrow_request_queue.size;
    int index = borrow_request_queue.front;
    
    for (int i = 0; i < count; i++) {
        User user = borrow_request_queue.items[index];
        printf("User ID: %d, Name: %s, Requested Book ID: %d\n",
               user.user_id, user.name, user.requested_book_id);
        index = (index + 1) % MAX_QUEUE_SIZE;
    }
}

// Function to display menu
void displayMenu() {
    printf("\n--- Library Management System ---\n");
    printf("1. Add New Book\n");
    printf("2. Borrow Book\n");
    printf("3. Return Book\n");
    printf("4. Search Book\n");
    printf("5. Display Recently Returned Books\n");
    printf("6. Display Borrow Requests\n");
    printf("7. Exit\n");
    printf("Enter your choice: ");
}

// Main function
int main() {
    // Initialize stack and queue
    initStack(&returned_books_stack);
    initQueue(&borrow_request_queue);
    
    int choice;
    do {
        // Display menu and get user choice
        displayMenu();
        scanf("%d", &choice);
        
        // Process user choice
        switch(choice) {
            case 1:
                addBook();
                break;
            case 2:
                borrowBook();
                break;
            case 3:
                returnBook();
                break;
            case 4:
                searchBook();
                break;
            case 5:
                displayReturnedBooks();
                break;
            case 6:
                displayBorrowRequests();
                break;
            case 7:
                printf("Exiting the program. Goodbye!\n");
                // Free dynamically allocated memory
                free(library);
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 7);
    
    return 0;
}
