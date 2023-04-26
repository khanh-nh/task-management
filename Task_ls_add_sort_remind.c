#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define STD_STRING_SIZE 30
#define STD_TASKS_COUNT 100

/* ---------- */

/* Structure and Interface(s) to handle date */
struct date {
    int day;
    int month;
    int year;
};
typedef struct date Date;


Date* initialize_date(int day, int month, int year) {
    Date *container = (Date*)malloc(sizeof(Date));
    container->day = day;
    container->month = month;
    container->year = year;
    return container;
}


Date* get_current_local_date() {
    time_t periodic_time = time(NULL);
    struct tm* formatted_time = localtime(&periodic_time);

    Date *local_date = initialize_date(
        formatted_time->tm_mday,
        // the time structure stores months from the range 0-11
        1 + formatted_time->tm_mon,
        // the time structure stores years since 1900!
        1900 + formatted_time->tm_year 
    );
    printf("%d", local_date->day);
    return local_date;
}

int compare_dates(Date *d1, Date *d2) {
    if (d1->year != d2->year) {
        return d1->year - d2->year;
    } else if (d1->month != d2->month) {
        return d1->month - d2->month;
    } else {
        return d1->day - d2->day;
    }
}

/* ---------- */

/* Structure and Interface(s) to handle task(s) */

struct task {
    int priority;
    char text[STD_STRING_SIZE];   
    Date deadline;
};
typedef struct task Task;



// Declarations
char* get_printable_task(Task*);
Task* _read_single_task_from_filestream(FILE*);
Task** read_all_tasks_from_file(const char*, int*);





Task* initialize_task(const char *text, int priority, Date *d) {
    Task *new_task = (Task*)malloc(sizeof(Task));
    strcpy(new_task->text, text);
    new_task->priority = priority;
    new_task->deadline = *d; 
    return new_task;
}



Task** read_all_tasks_from_file(const char *filepath, int *num_tasks) {
    Task **tasks = (Task**)malloc(sizeof(Task*)*STD_TASKS_COUNT);
    *num_tasks = 0;
    // Read from file
    FILE *fp;
    char line[STD_TASKS_COUNT];
    char pr[10];
    char text[50];
    char deadline[20];
    int priority,day,month,year;
    // Open the file in read mode
    fp = fopen(filepath, "r");
    if (fp == NULL) {
        printf("Error: could not open file\n");
        return NULL;
    }

    // Read the file line by line
    while (fgets(line, sizeof(line), fp) != NULL) {
        // Extract the priority, task, and deadline information from the line
        sscanf(line, "[%[^]]] Priority %[^:]: \"%[^\"]\" - Deadline: %[^\n]", 
            pr, pr, text, deadline);
        sscanf(pr, "%d", &priority);
        sscanf(deadline, "%d/%d/%d", &day, &month, &year);

        tasks[*num_tasks] = initialize_task(text,priority,initialize_date(day,month,year));
        (*num_tasks)++;
    }
    
    fclose(fp);

    if(*num_tasks == 0) {
        free(tasks);
        return NULL;
    }
    tasks = (Task**)realloc(tasks, sizeof(Task*)*(*num_tasks));
    return tasks;
}



// print the info inside task including deadline
char* get_printable_task(Task *task) {
    // Calculate the required size for the task string
    int size = snprintf(NULL, 0, "[ ] Priority %d: \"%s\" - Deadline: %d/%d/%d\n",
                        task->priority, task->text, task->deadline.day,
                        task->deadline.month, task->deadline.year) + 1;

    // Allocate memory for the task string
    char *task_str = malloc(size);

    // Write the task string to the buffer
    snprintf(task_str, size, "[ ] Priority %d: \"%s\" - Deadline: %d/%d/%d\n",
             task->priority, task->text, task->deadline.day,
             task->deadline.month, task->deadline.year);

    return task_str;
}


char* get_printable_complete_task(Task *task) {
    // Calculate the required size for the complete task string
    int size = snprintf(NULL,0, "[X] %s ", task->text) + 1;

    // Allocate memory for the task string
    char *task_str = malloc(size);

    // Write the task string to the buffer
    sprintf(task_str, "[X] %s ", task->text);

    return task_str;
}




struct completed_task  {
    Task task;
    Date date_of_completion;
};
typedef struct completed_task CompletedTask;


void check_arguments(int argc, char* argv[])
{
    char print_text[STD_STRING_SIZE] = "";
    printf("\n\nArguments");
    for(int i=1; i<argc; i++)
    {
        strcat(print_text, argv[i]);
    }
    printf("\n\nargc: %d, argv[1:]:%s\n\n", argc, print_text);
}


/* ---------- */

// print the list of tasks
void task_ls() {
    int num_tasks = 0;

    // Use read_all_tasks_from_file() to read the list of tasks from the file
    Task **task_list = read_all_tasks_from_file("task.txt", &num_tasks);
    if (task_list == NULL) {
        printf("\n\nNo tasks to do.");
        return;
    }

    printf("\n\nList of incomplete tasks in order of tasks_index you have added:\n");
    
    // display all contents by iterating using get_printable_task() 
    for (int i = 0; i < num_tasks; i++) {
        printf("%d. ", i+1);
        printf("%s", get_printable_task(*(task_list + i)));
    }
    printf("The number of incomplete tasks: %d\n",num_tasks);
}

// print the list of tasks sorting in priority
int compare_tasks_priority(const void* t1, const void* t2) {
    Task* task1 = *(Task**)t1;
    Task* task2 = *(Task**)t2;
    return task1->priority - task2->priority;
}

void task_ls_priority() {
    int num_tasks = 0;

    // Use read_all_tasks_from_file() to read the list of tasks from the file
    Task **task_list = read_all_tasks_from_file("task.txt", &num_tasks);
    if (task_list == NULL) {
        printf("\n\nNo tasks to do.");
        return;
    }
    
    // sort the tasks in the list by priority using qsort()
    qsort(task_list, num_tasks, sizeof(Task*), compare_tasks_priority);

    printf("\n\nList of incomplete tasks in priority order:\n");
    
    // display all contents by iterating using get_printable_task() 
    for (int i = 0; i < num_tasks; i++) {
        printf("%d. ", i+1);
        printf("%s", get_printable_task(*(task_list + i)));
    }
    printf("The number of incomplete tasks: %d\n",num_tasks);

    // print into a file task_pr
    FILE *f = fopen("task_pr.txt", "w");
    for (int i = 0; i < num_tasks; i++) {
        fprintf(f, "%s", get_printable_task(*(task_list + i)));
    }
    fprintf(f,"The number of incomplete tasks: %d\n",num_tasks);
    fclose(f);
    
}


// print the list of tasks sorting in deadline
int compare_tasks_by_deadline(const void *task1, const void *task2) {
    Task *t1 = *(Task**)task1;
    Task *t2 = *(Task**)task2;

    if (t1->deadline.year != t2->deadline.year) {
        return t1->deadline.year - t2->deadline.year;
    } else if (t1->deadline.month != t2->deadline.month) {
        return t1->deadline.month - t2->deadline.month;
    } else {
        return t1->deadline.day - t2->deadline.day;
    }
}


void task_ls_deadline() {
    int num_tasks = 0;
    Task **task_list = read_all_tasks_from_file("task.txt", &num_tasks);
    if (task_list == NULL) {
        printf("\n\nNo tasks to do.");
        return;
    }

    // Sort tasks based on deadline using mergesort
    qsort(task_list, num_tasks, sizeof(Task*), compare_tasks_by_deadline);

    // Print sorted tasks
    printf("\n\nList of incomplete tasks in deadline order:\n");
    for (int i = 0; i < num_tasks; i++) {
        printf("%d. ", i+1);
        printf("%s", get_printable_task(*(task_list + i)));
    }
    printf("The number of incomplete tasks: %d\n",num_tasks);
    
    // print into a task_dl file
    FILE *f = fopen("task_dl.txt", "w");
    for (int i = 0; i < num_tasks; i++) {
        fprintf(f, "%s", get_printable_task(*(task_list + i)));   
    }
    fprintf(f,"The number of incomplete tasks: %d\n",num_tasks);
    fclose(f);
    
}

/* ---------- */
/*Add task to task.txt*/
void task_add(int task_priority, char* task_text, Date *d, const char *filepath) {
    int num_tasks;
    Task **task_list = read_all_tasks_from_file(filepath, &num_tasks);

    // Check if the new task already exists in the file
    for (int i = 0; i < num_tasks; i++) {
        if (strcmp(task_list[i]->text, task_text) == 0 && compare_dates(&task_list[i]->deadline, d) == 0) {
            printf("\n\nTask already exists:\n");
            printf("%s", get_printable_task(task_list[i]));
            FILE *f = fopen("task_add_hist.txt", "w");
                fprintf(f,"Task already exists:\n");
                fprintf(f,"%s", get_printable_task(task_list[i]));
            fclose(f);
            return; // Exit function if task already exists
        }
    }

    Task *new_task = initialize_task(task_text, task_priority, d);
    FILE *f = fopen(filepath, "a");
    fprintf(f, "%s", get_printable_task(new_task));
    fclose(f);
    printf("Added task:\n");
    
    // display the task using - get_printable_task()
    char *task_str = get_printable_task(new_task);
    printf("%s", task_str);


    // print in task_add_hist.txt
    FILE *f1 = fopen("task_add_hist.txt", "w");
        fprintf(f1,"Added task:\n");
        fprintf(f1,"%s", task_str);
    fclose(f1);

}


/* ---------- */

/*Delete task from task.txt*/
void task_del(int task_index, const char *filepath) {
    int num_tasks = 0;
    Task **task_list = read_all_tasks_from_file(filepath, &num_tasks);
    if (task_list == NULL || num_tasks == 0) {
        printf("Error: task list is empty or file not found\n");
        return;
    }
    if (task_index < 0 || task_index >= num_tasks) {
        printf("Index %d does not exist\n", task_index);
        return;
    }
    Task *task_to_delete = task_list[task_index];
    free(task_to_delete);
    for (int i = task_index; i < num_tasks - 1; i++) {
        task_list[i] = task_list[i + 1];
    }
    task_list[num_tasks - 1] = NULL;
    num_tasks--;
    printf("Deleted task at index %d:\n%s\n", task_index, get_printable_task(task_to_delete));

    // print after deleting task
    printf("List of incomplete task after deleting: \n");
    for (int i = 0; i < num_tasks; i++) {
        printf("%d. ", i+1);
        printf("%s", get_printable_task(*(task_list + i)));
    }

    // fix txt file
    FILE *f = fopen(filepath, "w");
    for (int i = 0; i < num_tasks; i++) {
        fprintf(f, "%s", get_printable_task(task_list[i]));
    }
    fclose(f);
    
    free(task_to_delete);
    free(task_list);
}

/* ---------*/
// Remind of tasks
void task_ls_remind() {

    time_t t = time(NULL);
    struct tm *local_time = localtime(&t);
    int uncompleted_today_count = 0;
    int uncompleted_future_count = 0;

    printf("\n\nCurrent local date: %02d/%02d/%04d\n", local_time->tm_mday, local_time->tm_mon + 1, local_time->tm_year + 1900);

    int num_tasks = 0;

    // Use read_all_tasks_from_file() to read the list of tasks from the file
    Task **task_list = read_all_tasks_from_file("task.txt", &num_tasks);
    if (task_list == NULL) {
        printf("\n\nNo tasks to do.");
        return;
    }
    
    // sort the tasks in the list by priority using qsort()
    qsort(task_list, num_tasks, sizeof(Task*), compare_tasks_priority);

    printf("\nRemind incomplete tasks today in priority order:\n");
    
    // display all contents whose deadline is the same as the date today by iterating using get_printable_task() 
    for (int i = 0; i < num_tasks; i++) {
        if ((*(task_list + i))->deadline.day == local_time->tm_mday && (*(task_list + i))->deadline.month == (local_time->tm_mon + 1) && (*(task_list + i))->deadline.year == (local_time->tm_year + 1900)) {
            printf("%d. ", i+1);
            printf("%s", get_printable_task(*(task_list + i)));
            uncompleted_today_count ++;
        }
        
    }
    if (uncompleted_today_count == 0) {
        printf("\nNo tasks have deadline today.\n");
    }
    printf("The number of incomplete tasks today: %d\n",uncompleted_today_count);

    // print into task_remind.txt file
    FILE *f = fopen("task_remind.txt", "w");
    for (int i = 0; i < num_tasks; i++) {
        if ((*(task_list + i))->deadline.day == local_time->tm_mday && (*(task_list + i))->deadline.month == (local_time->tm_mon + 1) && (*(task_list + i))->deadline.year == (local_time->tm_year + 1900)) {
            fprintf(f,"%d. ", i+1);
            fprintf(f, "%s", get_printable_task(*(task_list + i)));
        }
        
    }
    if (uncompleted_today_count == 0) {
        fprintf(f, "\nNo tasks have deadline today.\n");
    }
    fprintf(f,"The number of incomplete tasks today: %d\n",uncompleted_today_count);   

    fclose(f);
}


int main() {
    char input[STD_STRING_SIZE];
    int option;
    int sort_option;
    int delete_index;
    char task_text[STD_STRING_SIZE];
    int task_priority, task_day, task_month, task_year;
    
    printf("------TASK MANAGEMENT----- \n");

    do {
        printf("------MENU------- \n");
        printf("Enter an option:\n");
        printf("1. Add a task\n");
        printf("2. Display all tasks\n");
        printf("3. Delete a task\n");
        printf("4.Show today tasks \n");
        printf("0. Exit\n");
        printf("Option: ");
        // fgets(input, sizeof(input), stdin);
        // option = atoi(input);
        scanf("%d", &option);

        switch (option) {
            case 1:
                printf("Enter task text: ");
                fgets(task_text, sizeof(task_text), stdin);
                printf("Enter task priority: ");
                fgets(input, sizeof(input), stdin);
                task_priority = atoi(input);
                printf("Enter task deadline (dd/mm/yyyy): ");
                scanf("%d/%d/%d", &task_day, &task_month, &task_year);
                getchar(); // consume newline character from scanf
                task_add(task_priority, task_text, initialize_date(task_day, task_month, task_year), "task.txt");
                free(option);
                break;

            case 2:
                printf("Choose an option to display the task list: \n");
                printf("1. Sorting by the index \n");
                printf("2. Sorting by priority \n");
                printf("3. Sorting by deadline \n");
                printf("Option: ");
                scanf("%d", &sort_option);
                
                // switch (sort_option)
                // {
                //     case 1:
                //         task_ls();
                        
                //     case 2:
                //         task_ls_priority();
                        
                    
                //     case 3:
                //         task_ls_deadline();
                        
                
                //     default:
                //         printf("Invalid option. Please enter a valid option.\n");
                // };
                

               if (sort_option == 1) {
                    task_ls();
                    free(sort_option);
               } else if (sort_option == 2) {
                    task_ls_priority();
                    free(sort_option);
               } else if (sort_option == 3) {
                    task_ls_priority();
                    free(sort_option);
               }
                free(option);
                break;

            case 3:
                printf("Type in the index of the tasks you want to delete: ");
                scanf("%d", &delete_index);
                task_del(delete_index, "task.txt");
                free(option);

            case 4: 
                task_ls_remind();
                free(option);

            case 0:
                printf("Exiting program...\n");
                break;
            default:
                printf("Invalid option. Please enter a valid option.\n");
        }

    } while (option != 0);

    return 0;
}
