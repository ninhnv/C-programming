// Command Pattern: Encapsulates commands as objects with execute methods
// Strategy Pattern: Encapsulates algorithms or processing methods and makes them interchangeable
// Observer Pattern: Notifies observers of changes in the subject’s state

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For sleep function (POSIX)
#include <time.h>   // For time functions

#define MAX_LINE_LENGTH 100
#define INPUT_FILE "input.txt"

// Define a structure to handle commands
typedef struct {
    char command[MAX_LINE_LENGTH];
    void (*handler)(int);
} CommandHandler;

// Function prototypes for processing commands
void processIgnition(int state);
void processHorn(int signal);
void processHeadlights(int state);

// Function prototypes for command handlers
void processIgnitionHandler(int value);
void processHornHandler(int value);
void processHeadlightsHandler(int value);

// Function prototypes for registering, unregistering, and executing commands
void registerCommand(const char *command, void (*handler)(int));
void unregisterCommand(const char *command);
void executeCommand(const char *command);

// Function to write value to the file or update existing value
void writeOrUpdateValueToFile(const char *command, int value);

// Function to read and process commands from the file
void listenToEvents(const char *commandType);

// Function to clear the terminal screen
void clearScreen() {
    printf("\033[H\033[J"); // ANSI escape codes to clear the screen
}

// Function to write or update value to the file
void writeOrUpdateValueToFile(const char *command, int value) {
    FILE *file = fopen(INPUT_FILE, "r+");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char line[MAX_LINE_LENGTH];
    char fileContent[MAX_LINE_LENGTH * 1000] = "";
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = '\0';

        // Trim spaces
        char *start = line;
        while (*start == ' ') start++;
        char *end = start + strlen(start) - 1;
        while (end > start && *end == ' ') end--;
        *(end + 1) = '\0';

        // Check if the line contains the command to update
        if (strcmp(start, command) == 0) {
            sprintf(line, "%s\n%d", command, value);
            found = 1;
        }

        strcat(fileContent, line);
        strcat(fileContent, "\n");
    }

    // If command was not found, add it at the end
    if (!found) {
        sprintf(line, "%s\n%d", command, value);
        strcat(fileContent, line);
    }

    // Move file pointer to the beginning
    fseek(file, 0, SEEK_SET);
    // Truncate file to remove old content
    ftruncate(fileno(file), 0);
    // Write new content to the file
    fputs(fileContent, file);

    fclose(file);

    printf("Value %d updated for command %s\n", value, command);
}

// Function to read and process commands from the file
void listenToEvents(const char *commandType) {
    FILE *file = fopen(INPUT_FILE, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Register commands with their corresponding handler functions
    registerCommand("IGNITION", processIgnitionHandler);
    registerCommand("HORN", processHornHandler);
    registerCommand("HEADLIGHTS", processHeadlightsHandler);

    time_t startTime = time(NULL);
    time_t lastExecutionTime = time(NULL); // Last execution time

    while (difftime(time(NULL), startTime) < 20.0) { // Run for 20 seconds
        clearScreen();

        if (difftime(time(NULL), lastExecutionTime) >= 2.0) {
            fseek(file, 0, SEEK_SET); // Reset file pointer to the beginning

            char line[MAX_LINE_LENGTH];
            char currentCommand[MAX_LINE_LENGTH] = "";
            int currentValue = -1;
            int isCurrentCommand = 0;

            while (fgets(line, sizeof(line), file)) {
                // Remove newline character
                line[strcspn(line, "\n")] = '\0';

                // Trim spaces
                char *start = line;
                while (*start == ' ') start++;
                char *end = start + strlen(start) - 1;
                while (end > start && *end == ' ') end--;
                *(end + 1) = '\0';

                // Skip empty lines
                if (strlen(start) == 0) {
                    continue;
                }

                // Check if the line contains the command type
                if (strcmp(start, commandType) == 0) {
                    strcpy(currentCommand, start);
                    isCurrentCommand = 1;
                } else if (isCurrentCommand) {
                    // Read the value line for the current command
                    if (sscanf(start, "%d", &currentValue) == 1) {
                        // Successfully read the integer value
                        executeCommand(currentCommand);
                        isCurrentCommand = 0; // Reset for the next command
                    }
                }
            }

            lastExecutionTime = time(NULL); // Update last execution time
        }

        sleep(1); // Sleep for 1 second to reduce CPU usage
    }

    fclose(file);
}

// Register command functions
CommandHandler commandHandlers[50];
int commandCount = 0;

void registerCommand(const char *command, void (*handler)(int)) {
    strcpy(commandHandlers[commandCount].command, command);
    commandHandlers[commandCount].handler = handler;
    commandCount++;
}

void unregisterCommand(const char *command) {
    for (int i = 0; i < commandCount; i++) {
        if (strcmp(commandHandlers[i].command, command) == 0) {
            for (int j = i; j < commandCount - 1; j++) {
                commandHandlers[j] = commandHandlers[j + 1];
            }
            commandCount--;
            printf("Command '%s' unregistered successfully.\n", command);
            return;
        }
    }
    printf("Command '%s' not found.\n", command);
}

void executeCommand(const char *command) {
    for (int i = 0; i < commandCount; i++) {
        if (strcmp(commandHandlers[i].command, command) == 0) {
            int value;
            FILE *file = fopen(INPUT_FILE, "r");
            if (file == NULL) {
                perror("Error opening file");
                return;
            }
            // Find the command and get the value
            char line[MAX_LINE_LENGTH];
            while (fgets(line, sizeof(line), file)) {
                // Remove newline character
                line[strcspn(line, "\n")] = '\0';

                // Trim spaces
                char *start = line;
                while (*start == ' ') start++;
                char *end = start + strlen(start) - 1;
                while (end > start && *end == ' ') end--;
                *(end + 1) = '\0';

                // Skip empty lines
                if (strlen(start) == 0) {
                    continue;
                }

                // Check if the line contains the command type
                if (strcmp(start, command) == 0) {
                    if (fgets(line, sizeof(line), file)) {
                        if (sscanf(line, "%d", &value) == 1) {
                            // Successfully read the integer value
                            commandHandlers[i].handler(value);
                        }
                    }
                    fclose(file);
                    return;
                }
            }
            fclose(file);
            return;
        }
    }
    printf("Unknown Command: '%s'\n", command);
}

void processIgnitionHandler(int value) {
    processIgnition(value);
}

void processHornHandler(int value) {
    processHorn(value);
}

void processHeadlightsHandler(int value) {
    processHeadlights(value);
}

void processIgnition(int state) {
    printf("Ignition State: %d\n", state);
}

void processHorn(int signal) {
    printf("Horn Activated: %d\n", signal);
}

void processHeadlights(int state) {
    printf("Headlight State: %d\n", state);
}

// Menu functions
void ignitionMenu() {
    int option, value;
    printf("IGNITION Menu:\n");
    printf("1. Write value to file\n");
    printf("2. Listen to event\n");
    printf("Enter your choice: ");
    scanf("%d", &option);

    switch (option) {
        case 1:
            printf("Enter ignition state (0=Off, 1=On): ");
            scanf("%d", &value);
            writeOrUpdateValueToFile("IGNITION", value);
            break;
        case 2:
            listenToEvents("IGNITION");
            break;
        default:
            printf("Invalid choice. Returning to menu.\n");
    }
}

void hornMenu() {
    int option, value;
    printf("HORN Menu:\n");
    printf("1. Write value to file\n");
    printf("2. Listen to event\n");
    printf("Enter your choice: ");
    scanf("%d", &option);

    switch (option) {
        case 1:
            printf("Enter horn activation signal (0=Off, 1=On): ");
            scanf("%d", &value);
            writeOrUpdateValueToFile("HORN", value);
            break;
        case 2:
            listenToEvents("HORN");
            break;
        default:
            printf("Invalid choice. Returning to menu.\n");
    }
}

void headlightsMenu() {
    int option, value;
    printf("HEADLIGHTS Menu:\n");
    printf("1. Write value to file\n");
    printf("2. Listen to event\n");
    printf("Enter your choice: ");
    scanf("%d", &option);

    switch (option) {
        case 1:
            printf("Enter headlights state (0=Off, 1=Low, 2=High): ");
            scanf("%d", &value);
            writeOrUpdateValueToFile("HEADLIGHTS", value);
            break;
        case 2:
            listenToEvents("HEADLIGHTS");
            break;
        default:
            printf("Invalid choice. Returning to menu.\n");
    }
}

// Main menu function
int main() {
    int choice;
    while (1) {
        printf("\nMain Menu:\n");
        printf("1. Ignition\n");
        printf("2. Horn\n");
        printf("3. Headlights\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                ignitionMenu();
                break;
            case 2:
                hornMenu();
                break;
            case 3:
                headlightsMenu();
                break;
            case 4:
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}
