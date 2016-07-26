
/*You can make and maintenance a book of entries using this program

program arguments                           argument params
filename:                          
c: creates a file                       give maxdata per param and maxrows for the list
s: sets data in a new entry             give id , name, email
g: displays an entry if it's set        give id
d: deletes an entry                     give id
l: displays the list                    
f: displays matched entries             give as many keywords as you want (id, name, email)

*/

/* check the file book at home directory*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>


struct Address {
    char *name;
    char *email;
    int id;
    int set;
};

struct Database {
    struct Address *rows;
    int MAX_DATA;
    int MAX_ROWS;
};

struct Connection {
    FILE *file;
    struct Database *db;
};

void Database_close(struct Connection *conn)
{
    if(conn) 
    {
        for(int i = 0; i < conn->db->MAX_ROWS; i ++)
        {
            if(conn->db->rows[i].name) free(conn->db->rows[i].name);
            if(conn->db->rows[i].email) free(conn->db->rows[i].email);
        }
        if(conn->db->rows) free(conn->db->rows);
        if(conn->db) free(conn->db);
        if(conn->file) fclose(conn->file);
        free(conn);
    }
}

void die(struct Connection *end, const char *message)
{
    if(errno) 
    {
        perror(message);
    } else 
    {
        printf("ERROR: %s\n", message);
    }

        Database_close(end);
        exit(1);
}

void Database_load(struct Connection *connld)
{ 
    if(fread(connld->db, sizeof(struct Database), 1, connld->file) != 1)
        die(connld, "Failed to load database.");
    int MAX_ROWS = connld->db->MAX_ROWS;
    int MAX_DATA = connld->db->MAX_DATA;
    
    connld->db->rows = (struct Address*) malloc(sizeof(struct Address) * MAX_ROWS);
    
    for(int i = 0; i < MAX_ROWS; i++)   // Here must try without struct Address *row help
    {
        struct Address *row = &connld->db->rows[i];
        
        if(fread(&row->id, sizeof(int), 1, connld->file) != 1)
            die(connld, "Failed to load id.");
        if(fread(&row->set, sizeof(int), 1, connld->file) != 1)
            die(connld, "Failed to load set.");
            
        row->name = (char*) malloc(sizeof(char) * MAX_DATA);
        row->email = (char*) malloc(sizeof(char) * MAX_DATA);
        
        if(fread(row->name, MAX_DATA, 1, connld->file) != 1)
            die(connld, "Failed to load name.");
        if(fread(row->email, MAX_DATA, 1, connld->file) != 1)
            die(connld, "Failed to load email.");
    }                         
}

struct Connection *Database_open(const char *filename, char mode, int MAX_DATA, int MAX_ROWS)
{
    struct Connection *conntmp = malloc(sizeof(struct Connection)); 
    if(!conntmp) die(conntmp, "Memory error");

    conntmp->db = malloc(sizeof(struct Database)); 
    if(!conntmp->db) die(conntmp, "Memory error");

    if(mode == 'c') 
    {
        //check if the file already exists
        conntmp->file = fopen(filename, "r+");
        if(conntmp->file) 
        {
            Database_load(conntmp);   
            die(conntmp, "The file already exists, delete it first using rm <filename> in CLI  ");
        }
        
        //and if not, create it
        conntmp->file = fopen(filename, "w");
        if(!conntmp->file) die(conntmp, "Failed to create the file");
        conntmp->db->MAX_DATA = MAX_DATA;
        conntmp->db->MAX_ROWS = MAX_ROWS;
    }
    else 
    {
        conntmp->file = fopen(filename, "r+");
        if(!conntmp->file) die(conntmp, "Failed to open the file");

        Database_load(conntmp);
    }  
    
    return conntmp;
}

void Database_create(struct Connection *conncr)
{
    int i = 0;
    int MAX_ROWS = conncr->db->MAX_ROWS;
    int MAX_DATA = conncr->db->MAX_DATA;
    
    conncr->db->rows = (struct Address*)malloc(sizeof(struct Address)*MAX_ROWS);
    
    for(i = 0; i < MAX_ROWS; i++) 
    {
        char *name = (char*) malloc(MAX_DATA);
        memset(name, 0, MAX_DATA);
        char *email = (char*) malloc(MAX_DATA);
        memset(email, 0, MAX_DATA);
        struct Address tmp = {.id = i, .set = 0, .name = name, .email = email};
        
        conncr->db->rows[i] = tmp;
    }
}

void Database_write(struct Connection *connwr)
{
    int maxd = connwr->db->MAX_DATA;
    int maxr = connwr->db->MAX_ROWS;
    
    rewind(connwr->file);
    
    if(fwrite(connwr->db, sizeof(struct Database), 1, connwr->file) != 1) 
            die(connwr, "Failed to write database."); 
    for(int i = 0; i < maxr; i++)
    {
        if(fwrite(&((connwr->db->rows[i]).id), sizeof(int), 1, connwr->file) != 1)
                        die(connwr, "Failed to write id.");
        if(fwrite(&((connwr->db->rows[i]).set), sizeof(int), 1, connwr->file) != 1)
                        die(connwr, "Failed to write set.");
        if(fwrite((connwr->db->rows[i]).name, maxd, 1, connwr->file) != 1)
                        die(connwr, "Failed to write name.");
        if(fwrite((connwr->db->rows[i]).email, maxd, 1, connwr->file) != 1)
                        die(connwr, "Failed to write email.");
    }                         
                                                                            
    if(fflush(connwr->file) == -1)                              
            die(connwr, "Cannot flush database.");
}

void Address_print(struct Address *addr)
{
    printf("\t\t\t %4d	%s	%s\n",
            addr->id, addr->name, addr->email);
}

void Database_get(struct Connection *conngt, int id)
{
    struct Address *tmp = &conngt->db->rows[id];

    if(tmp->set) {
        Address_print(tmp);
    } else {
        die(conngt, "ID is not set");
    }
}

void Database_set(struct Connection *connset, int id, const char *name, const char *email)
{
    struct Address *tmp = &connset->db->rows[id];
    if(tmp->set) die(connset, "Already set, delete it first");

    int MAX_DATA = connset->db->MAX_DATA;
    tmp->set = 1;
    
    // WARNING: bug, read the "How To Break It" and fix this
    char *res = strncpy(tmp->name, name, MAX_DATA);
    tmp->name[MAX_DATA - 1] = '\0';
    // demonstrate the strncpy bug
    if(!res) die(connset, "Name copy failed");

    res = strncpy(tmp->email, email, MAX_DATA);
    tmp->email[MAX_DATA - 1] = '\0';
    if(!res) die(connset, "Email copy failed");
}

void Database_delete(struct Connection *conndlt, int id)
{
    struct Address *addr = &conndlt->db->rows[id];
    addr->id = id; addr->set = 0; 
    memset(addr->name, 0, conndlt->db->MAX_DATA);
    memset(addr->name, 0, conndlt->db->MAX_DATA);
    
}

void Database_list(struct Connection *connlst)
{
    int i = 0;
    int MAX_ROWS = connlst->db->MAX_ROWS;

    for(i = 0; i < MAX_ROWS; i++) 
    {
        if(connlst->db->rows[i].set) 
        {
            Address_print(&(connlst->db->rows[i]));
        }
    }
}

int Compare_strings (const char  s1[], const char  s2[])
{
    int  i = 0;
    int comparison = 0;
            
    while ( s1[i] == s2[i] && s1[i] != '\0'  &&  s2[i] != '\0' )
           i++;
                                
    if ( s1[i] == s2[i])
           comparison = 0;
    else if (s1[i] < s2[i])
           comparison = -1;
    else
           comparison = 1;
                                                                    
    return comparison;
}
int Check_data(char *key, struct Address *row) 
{
    int number = -1;
    char *word;
    
    // check what type of data it's looking for (int: id, phone number, address number, postal code - 
    //char * : name, email, address, favorite dish etc.
    int i = 0;
    int j = strlen(key);
    for(i = 0; i < j; i++)
    {   
        //keyword is number so it searches for ints
        if(isdigit(key[i])) 
        {
            if(i == j - 1) number = atoi(key); 
        }
        //keyword is not a number, break and asigned to char* word
        if(!isdigit(key[i]))                   
        break;
    }
    word = key; 
    
    //search for resalts                    
    if(number == row->id/* we can add here || == phone || == postalcode || == addrNo*/)
    {
        return 1;
    } 
    if(number == -1)
    {
        if(Compare_strings(word, row->name) == 0 || Compare_strings(word, row->email) == 0) //Compare_strings() returns 0 in a match case
                                                                                            //strstr() / strcasestr() could be used as well                          
        return 1;                     
    }
    return 0;
}

void Keywords_find(struct Connection *connsrch, char **keywords, int counter)
{
    int rescntr = 0;                // counts findings
    struct Address *cur = connsrch->db->rows;
    int kwrd_index = counter + 2;   // gives the keyword index( 3 are the initial arguments of which we are totaly un-inderested so 2 their max index)
    int i;
        
    for( i = 0; i < counter; i++, kwrd_index--)
    {  
        printf("_Search term: \"%s\" \n",keywords[kwrd_index]);    	                  
        
        for(int j = 0; j < connsrch->db->MAX_ROWS; j++)
        {   
            if(cur[j].set)
            {
                if(Check_data(keywords[kwrd_index], &cur[j]))
                {
                    Address_print(&cur[j]);
                    rescntr++; 
                }
            }                              
        }
    }
    printf("\n	Search proccess returns %d result(s).\n\n", rescntr);
    printf("----------------------------------------------------------------------------------------------------------------------------------------\n");          
}
/*--------------------------------------------------------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	printf("\n");
	printf("----------------------------------------------------------------------------------------------------------------------------------------\n");
    if(argc < 3) die(NULL, "USAGE: addressbook <dbfile> <action> [action params:min 3]");
    
    char *filename = argv[1];
    char action = argv[2][0];
    int id = 0;
    int maxd = 1;
    int maxr = 1;
    int kwrds_counter = (argc - 3);                 // How many terms we're serching for
    
    if(argc > 6 && action != 'f') die(NULL, "USAGE: addressbook <dbfile> <action: !=f> [action params: max 6 ]");
    
    // check if params are consistent with program CL arguments specs and asign them accordingly or die  
    if(argc == 3 && action != 'l') die(NULL, "USAGE: addressbook <dbfile> <action> [action params]");
    if(argc > 3 && action != 'f') 
    {
        int i = 0;
        int j = strlen(argv[3]);
        for(i = 0; i < j; i++)
        {   
            if(!isdigit(argv[3][i])) 
            die(NULL, "USAGE: addressbook <dbfile> <action> [action params: id(number), name(string), email(string)]");
        }
        (action == 'c') ? (maxd = atoi(argv[3])) : (id = atoi(argv[3]));
    }                                                                   
    if(argc > 4 && action == 'c')
    {
        int i = 0;
        int j = strlen(argv[4]);
        for(i = 0; i < j; i++)
        {   
            if(!isdigit(argv[4][i])) 
            die(NULL, "USAGE: addressbook <dbfile> <action> [action params: MAX_DATA, MAX_ROWS]");
        }
        maxr = atoi(argv[4]);
    }
           
    struct Connection *conn = Database_open(filename, action, maxd, maxr);
    maxd = conn->db->MAX_DATA;
    maxr = conn->db->MAX_ROWS;
    
    for (int i = 0; i < argc; i++)
    {
        if(strlen(argv[i]) >= maxd)
        { 
            printf("maximum argument/param length is %d\n", maxd);
            die (conn,"maximum argument/param length exceeded\n");
        }
    }
        
    if(id >= maxr) die(conn, "There's not that many records.");
                                                                         
    switch(action)
    {
        case 'c':
            Database_create(conn);
            Database_write(conn);
            break;
                                        
        case 'g':
            if(argc != 4) die(conn, "Need an id to get");
            Database_get(conn, id);
            break;
            
        case 'f':
            Keywords_find(conn, argv, kwrds_counter);
            break;        
                                                                
        case 's':
            if(argc != 6) die(conn, "Need id, name, email to set");   
            Database_set(conn, id, argv[4], argv[5]);           
            Database_write(conn);                               
            break;

        case 'd':
            if(argc != 4) die(conn, "Need id to delete");
            Database_delete(conn, id);
            Database_write(conn);
            break;

        case 'l':
            Database_list(conn);
            break;
            
        default:
            die(conn, "Invalid action, only: c=create, g=get, s=set, d=del, l=list");
    }
                                                
    Database_close(conn);
    
    printf("\n");

    return 0;
}
