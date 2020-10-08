#include<stdio.h>
#include<bits/stdc++.h>
#include<dirent.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<sys/stat.h>
#include<termio.h>
#include<string>
#include <pwd.h>
#include <grp.h>
#define set_position printf("%c[%d;%dH", 27, x_cor, y_cor); 
using namespace std;


int listdirectory(char *);
void display(const char *);
void scrolling();
void navigate();
void upArrow();
void DownArrow();
void Enterkey();
void LeftArrow();
void RightArrow();
void BackspaceKey();
void HomeKey();
void commandmode();
void split();
void choose_command();
void copy_func();
void file_copy(string, string);
void dir_copy(string, string);
string create_absolute_path(string);
bool isDirectory(string);
void rename_func();
void delete_dir_func();
void delete_file_func();
void recursive_delete(string);
void move_func();
void create_file_func();
void create_dir_func();
void search_func();
void search_recursive(string , string);
void goto_func();

char root[1000];
char cur_dir[1000];
char **r_temp;
int x_cor;
int y_cor;
int term_row;
int term_col;
int cur_window = 0;
stack<string> back;
stack<string> front;
vector<string> list_paths;
vector<string> command_arg;
vector<char> command_char;
struct winsize terminal;
bool search_val = false;
bool goto_bool = false;

int main(int argc, char **argv)
{
    
    r_temp = argv;

    if(argc == 1)
    {
        getcwd(root, sizeof(root));
        strcpy(cur_dir, root);
    }
    else if(argc == 2)
    {
       //getcwd(root, sizeof(root));
       strcpy(root, r_temp[1]);
       strcpy(cur_dir, root);
    }
    
    back.push(root); // stores the previous visited directory.
    listdirectory(root);
    navigate();

}



int listdirectory(char * path)
{
   
   ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal);
    term_row = terminal.ws_row-2;
    term_col = terminal.ws_col;
   struct dirent *di;
   DIR *d;

   d = opendir(path);

   if(d == NULL)
   {
   perror("opendir");
   return -1;
   }

   list_paths.clear(); //clearing everything before a new operation

   while((di = readdir(d)))
   {
       list_paths.push_back(di->d_name);
   }

   //cout<< list_paths.size();

   sort(list_paths.begin(), list_paths.end());

   /*for(int i=0; i<list_paths.size(); i++)
   {
       cout<< list_paths[i]<< endl;
   }*/ 

   scrolling();
   x_cor=1;
   set_position;
  

   closedir(d);

   
  return 0;
}

void scrolling()
{
    if(!goto_bool)
    write(STDOUT_FILENO, "\x1b[2J", 4); //end clear screen
    y_cor = 1;
    if(goto_bool)
    x_cor = 2;
    set_position;

    int vector_size = list_paths.size();
    int start = cur_window;
    int end;
    if ((unsigned)vector_size <= term_row)
        end = vector_size - 1;
    else
        end = term_row + cur_window;
    

    int i=start;
    while(i<=end)
    {
        string t = list_paths[i];
        display(t.c_str());
        i++;
    }
}

void display(const char *paths){
   y_cor=0;
   string abs_path = "";
   abs_path = string(cur_dir) + "/" + paths;


   struct stat sb;

    stat(abs_path.c_str(), &sb);
    printf((S_ISDIR(sb.st_mode)) ? "d" : "-");
	printf((sb.st_mode & S_IRUSR) ? "r" : "-");
	printf((sb.st_mode & S_IWUSR) ? "w" : "-");
	printf((sb.st_mode & S_IXUSR) ? "x" : "-");
	printf((sb.st_mode & S_IRGRP) ? "r" : "-");
	printf((sb.st_mode & S_IWGRP) ? "w" : "-");
	printf((sb.st_mode & S_IXGRP) ? "x" : "-");
	printf((sb.st_mode & S_IROTH) ? "r" : "-");
	printf((sb.st_mode & S_IWOTH) ? "w" : "-");
	printf((sb.st_mode & S_IXOTH) ? "x" : "-");
    y_cor+=10;

    struct passwd* user_name;
    user_name = getpwuid(sb.st_uid);
    string u_name = user_name->pw_name;
    y_cor += printf(" %10s ", u_name.c_str());

    struct group* group_name;
    group_name = getgrgid(sb.st_gid);
    string g_name = group_name->gr_name;
    y_cor += printf(" %10s ", g_name.c_str());

    y_cor+=printf("%8.2fK", ((double)sb.st_size) / 1024);

    char *time = ctime(&sb.st_mtime);
    time[strlen(time) - 1] = '\0';
    y_cor+=printf("%25s" , time);


    printf(" %s\n", paths);
    y_cor++;

}

void navigate()
{
  struct termios initial, newinit;
  tcgetattr(STDIN_FILENO, &initial);
  newinit = initial;
  
  newinit.c_lflag &= ~(ICANON | ECHO | IEXTEN | ISIG);
  newinit.c_iflag &= ~(BRKINT);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &newinit);

  
  
  char ch[3] = { 0 };
        while (true) {
            int t = terminal.ws_row;
            printf("%c[%d;%dH", 27, t, 1);
            printf("********Normal Mode**********");
            set_position;
            fflush(0);
            if (read(STDIN_FILENO, ch, 3) == 0)
                continue;
            else if (ch[0] == 27 && ch[1] == '[' && ch[2] == 'B')
                DownArrow();
            else if (ch[0] == 27 && ch[1] == '[' && ch[2] == 'A')
                upArrow();
            else if (ch[0] == 27 && ch[1] == '[' && ch[2] == 'D')
                LeftArrow();
            else if (ch[0] == 27 && ch[1] == '[' && ch[2] == 'C')
                RightArrow();
            else if (ch[0] == 10)
                Enterkey();
            else if (ch[0] == 127)
                BackspaceKey();
            else if (ch[0] == 58)
            {   
               commandmode();
               listdirectory(cur_dir);
            }
            else if (ch[0] == 'H' || ch[0] == 'h')
                HomeKey();
            fflush(0);
            //memset(ch, 0, 3 * sizeof(ch[0]));
            
        }

    

}

void upArrow()
{
    //cout << "hi";
    if (x_cor > 1) {
        x_cor--;
        set_position;
        //cout<< "hi";
    }
    else if (x_cor == 1 && x_cor + cur_window > 1) {
        cur_window--;
        scrolling();
        set_position;
    }
}

void DownArrow()
{

    if (x_cor <= term_row && list_paths.size() > x_cor) {
        x_cor++;
        set_position;
    }
    else if (x_cor > term_row &&  list_paths.size() > x_cor + cur_window) {
        cur_window++;
        scrolling();
        set_position;
    }
}


void RightArrow()
{
     x_cor = 1;
     y_cor = 0;
     set_position;
    if(!front.empty())
    {
        string st = front.top();
        front.pop();
        back.push(st);
        strcpy(cur_dir, st.c_str());
        listdirectory(cur_dir);       
    }
}

void LeftArrow()
{
   x_cor=1;
   y_cor=0;
   set_position;

   
   if(back.size() == 1)
   {
       string t = back.top();
       strcpy(cur_dir, t.c_str());
       listdirectory(cur_dir);
   }
   else if(back.size() > 1)
   {
       string st = back.top();
       back.pop();
       front.push(st);
       st = back.top();
       strcpy(cur_dir, st.c_str());
       listdirectory(cur_dir);
   }
    
}

void BackspaceKey()
{
    x_cor=1;
    y_cor=0; 
    set_position;
    if(strcmp(cur_dir, root) != 0)
    { 
         string st = string(cur_dir);
         back.push(st);
         int found;
         found = st.find_last_of("/\\");
         string temp = st.substr(0, found);
         strcpy(cur_dir, temp.c_str());
         while(front.size()!=0)
         front.pop();
         x_cor=1;
         y_cor=0;
         set_position;
         listdirectory(cur_dir);
          
    }
}

void HomeKey()
{


    x_cor =1;
    y_cor=0;
    set_position;

    strcpy(cur_dir, root);
    while(!front.empty())
    front.pop();
    listdirectory(cur_dir);

}

void Enterkey()
{
    if(list_paths[cur_window + x_cor-1] == ".")
    {
         
    }
    else if(list_paths[cur_window + x_cor-1] == "..")
    {
         string st = string(cur_dir);
         back.push(st);
         int found;
         found = st.find_last_of("/\\");
         string temp = st.substr(0, found);
         strcpy(cur_dir, temp.c_str());
         while(front.size()!=0)
         front.pop();
         x_cor=1;
         y_cor=0;
         set_position;
         listdirectory(cur_dir);
    }
    else
    {
        //cout<< root;
        char *p;
        char *abspath;
        string path = "/" + list_paths[cur_window+x_cor-1];
        p = cur_dir;
        abspath = new char[strlen(cur_dir)+path.length()+1];
        strcpy(abspath, p);
        strcat(abspath, path.c_str());
        strcpy(cur_dir, abspath);
        write(STDOUT_FILENO, "\x1b[2J", 4);
        x_cor=1;
        //y_cor=0;
        //cur_window = 0;
        //set_position;
        //cout<< p;
        back.push(cur_dir);
        while(front.size()!=0)
        front.pop();


        struct stat sb;
        stat(abspath, &sb);


        if((sb.st_mode & S_IFMT) == S_IFDIR)
        listdirectory(p);
        else if((sb.st_mode & S_IFMT)== S_IFREG)
        {
            back.pop();
            string temp = back.top();
            strcpy(cur_dir, temp.c_str());
            pid_t pid = fork();
            if(pid == 0)// child process
            {
                 execlp("xdg-open", "xdg-open", abspath, NULL);
                 exit(0);
            }
        }
        else
        {
            perror("file error");
        }
        
    }
    
}


void commandmode()
{
    write(STDOUT_FILENO, "\x1b[2J", 4); //clear screen
    x_cor = terminal.ws_row;
    y_cor = 1;
    set_position;
    printf("COMMANDS WHICH CAN BE USED : copy  rename  delete_file   delete_dir   create_file  create_dir  move  search  goto");
    x_cor = 1;
    y_cor = 1;
    set_position;
    printf("\x1b[0K");
    //write(STDOUT_FILENO, "\x1b[2J", 4);
    y_cor+=printf("Command mode:");
    fflush(0);

    //y_cor++;
    char ch[3] = { 0 };
    while(true)
    {
        if(read(STDIN_FILENO, ch, 3) == 0)
        continue;

        else if (ch[0] == 27 && ch[1] == '[' && ch[2] == 'B')
                continue;
        else if (ch[0] == 27 && ch[1] == '[' && ch[2] == 'A')
                continue;
        else if (ch[0] == 27 && ch[1] == '[' && ch[2] == 'D')
                continue;
        else if (ch[0] == 27 && ch[1] == '[' && ch[2] == 'C')
                continue;

        else if(ch[0] == 10)
        {
            
            command_char.push_back('\n'); //refering end of command.
            //for(int i : command_char)
            //cout<< i;
            
            split();
            choose_command();
            
            x_cor = 1;
            y_cor = 14;
            set_position;
            //if(!goto_bool)
            printf("\x1b[0K");
            command_arg.clear();
            command_char.clear();
            goto_bool = false;
        }

        else if(ch[0] == 27)
        {
            
            x_cor = 1;
            y_cor = 1;
            set_position;
            break;
        }
        else if(ch[0]==127){
            if(y_cor>14)
            {
             y_cor--;
             set_position;
             printf("\x1b[0K");
             command_char.pop_back();
            }
        }
        
        else
        {
            cout << ch[0];
            y_cor++;
            set_position;
            command_char.push_back(ch[0]);
        }


        
        
        fflush(0);
        //memset(ch, 0, 3 * sizeof(ch[0]));
    }
}

void split()
{

    string x = {};
    
    for(int i=0; i< command_char.size(); i++)
    {
        //cout<< x<< endl;
        if(command_char[i] == ' ' || command_char[i] == '\n')
        {
            
            if(x.size()!=0)
            {
            command_arg.push_back(x);
            //cout << x << " ";
            }
            
            x = {};
            continue;
        }

        x+=command_char[i];
        
    }
}


void choose_command()
{
   string s = command_arg[0];

   y_cor = 0;
   for(int i=2; i<terminal.ws_row-1; i++)
    {
       x_cor = i;
       set_position;
       printf("\x1b[0K");
    }
    x_cor = 2;
    y_cor = 1;
    set_position;

   if(s == "copy")
   copy_func();

   else if(s == "rename")
   rename_func();

   else if(s == "delete_file")
   delete_file_func();

   else if(s == "delete_dir")
   delete_dir_func();

   else if(s == "move")
   move_func();

   else if(s == "create_file")
   create_file_func();

   else if(s == "create_dir")
   create_dir_func();

   else if(s == "search")
   {
   search_func();
   }
   else if(s == "goto")
   goto_func();

   else
   {
       cout<< endl << "Wrong command entered";
   }
   

}


void copy_func()
{
   
   string dest = command_arg[command_arg.size()-1];
   dest = create_absolute_path(dest);
   //cout<< dest;
   //cout<< command_arg.size()<< endl;
   //for(int i=0; i<command_arg.size(); i++)
   //cout << command_arg[i]<< " ";
   //cout<< command_arg[0];
   for(int i=1; i<command_arg.size()-1; i++)
   {
       //cout << command_arg[i];
       string from = create_absolute_path(command_arg[i]);
       //cout << from<< endl;
       long found = from.find_last_of("/\\");
       string to = dest + "/" + from.substr(found + 1, from.length() - found);
       //cout << to<< endl;
      

      if(isDirectory(from))
      {
       if (mkdir(to.c_str(), 0755) != 0) 
       {
                    perror("");
                    return;
       }
          dir_copy(from, to);
      }
       else
       file_copy(from, to);
   }

  cout<< "successful";
   //file_copy()
}



void file_copy(string source, string dest)
{
    FILE *from, *to;

   if((from = fopen(source.c_str(), "r")) == NULL) {
        perror("fileopen");
        return;
   }
    if ((to = fopen(dest.c_str(), "w")) == NULL) {
        perror("fileopen");
        return;
    }

    char c;
    while (!feof(from)) {
        c = getc(from);
        putc(c, to);
    }
    
    struct stat src_stat;
    stat(source.c_str(), &src_stat);
    chown(dest.c_str(), src_stat.st_uid, src_stat.st_gid);
    chmod(dest.c_str(), src_stat.st_mode);
    fclose(from);
    fclose(to);

    //cout << "done 1" << endl;

}

void dir_copy(string source, string dest)
{
    struct dirent *di;

    DIR *d;

    d = opendir(source.c_str());

    if(d == NULL)
   {
   perror("opendir");
   return;
   }

   while((di = readdir(d)))
   {
       //cout << "inside";
       if(string(di->d_name) == "." || string(di->d_name) == "..")
       continue;
       string st1 = source + "/" + string(di->d_name);
       string st2 = dest + "/" + string(di->d_name);

       if(isDirectory(st1))
       {
           if (mkdir(st2.c_str(), 0755) == -1)
           {
                perror("error creating directory");
                return;
           }
           else
           dir_copy(st1, st2);
       }
       else
       file_copy(st1, st2);
   }
   closedir(d);

}


string create_absolute_path(string relative_path)
{
    string abs_path = "";
    if (relative_path[0] == '~') {
        relative_path = relative_path.substr(1, relative_path.length());
        abs_path = string(root) + relative_path;
        //abs_path = relative_path;
    }
    else if (relative_path[0] == '.' && relative_path[1] == '/') {
        abs_path = string(cur_dir) + relative_path.substr(1, relative_path.length());
    }
    else if (relative_path[0] == '/') {
        abs_path = string(root) + relative_path;
    }
    
    else {
        abs_path = string(cur_dir) + "/" + relative_path;
        //cout << abs_path;
    }
    return abs_path;
}


bool isDirectory(string st)
{
    struct stat sb;
    stat(st.c_str(), &sb);

    if(S_ISDIR(sb.st_mode))
    return true;
    else
    {
        return false;
    }
    
}


void rename_func()
{
  if(command_arg.size() < 3)
  {
      cout<< "insufficent arguments";
      return;
  }

  string st1 = command_arg[1];
  string st2 = command_arg[2];

  st1 = create_absolute_path(st1);
  st2 = create_absolute_path(st2);
   

  if(rename(st1.c_str(), st2.c_str()) == -1 )
  {
      perror("error renaming");
      return;
  }
cout<< "successful";
}

void delete_file_func()
{
    if(command_arg.size() < 2)
    {
        cout<< "Insufficient arguments";
        return;
    }

    for(int i=1; i<command_arg.size(); i++)
    {
    string abs_path = create_absolute_path(command_arg[i]);

    if(remove(abs_path.c_str()) == -1)
    {
        perror("error deleting file");
        return;
    }
    }
    //cout<< "successful";
}

void delete_dir_func()
{
   if(command_arg.size() < 2)
    {
        cout<< "Insufficient arguments";
        return;
    }


    for(int i=1; i<command_arg.size(); i++)
    {

    string st1 = create_absolute_path(command_arg[i]);

    if(isDirectory(st1))
    { 
        //cout << "here";
         recursive_delete(st1);
    }
    else
    {
        cout<< "Not a directory";
    }
    }
    cout<< "successful";
}

void recursive_delete(string st)
{
    struct dirent *di;

    DIR *d;

    d = opendir(st.c_str());

    if (d == NULL) {
        perror("opendir");
        return;
    }

    while((di = readdir(d)))
    {
        //cout << "here";
        if(string(di->d_name) == "." || string(di->d_name) == "..")
        {
            continue;
        }

        string abs_path = string(di->d_name);
        abs_path = st + "/" + string(di->d_name);
        if(isDirectory(abs_path))
        {
            recursive_delete(abs_path);
        }
        else
        {
            if(remove(abs_path.c_str()) == -1)
            {
                   perror("error deleting file");
                   return;
            }
        }
        
    }
    rmdir(st.c_str());
    closedir(d);   
}

void move_func()
{

    if(command_arg.size() < 3)
    {
        cout<< "Insufficient arguments";
        return;
    }
    copy_func();

    for(int i=1; i<command_arg.size()-1; i++)
    {
        string st = create_absolute_path(command_arg[i]);

        if(isDirectory(st))
        {
            recursive_delete(st);
        }
        else
        {
            if(remove(st.c_str()) == -1)
            {
                   perror("error deleting file");
                   return;
            }
        }
        
    }
   cout<< "successful";
}

void create_file_func()
{
    

    if(command_arg.size() < 3)
    {
        cout<< "insufficient arguments";
        return;
    }

    string dest = command_arg[command_arg.size()-1];
    dest = create_absolute_path(dest);
    
    //cout<< endl << dest;

    for(int i =1; i<command_arg.size()-1; i++)
    {
        string src;
        src = dest + "/" + command_arg[i];
        FILE *f;
        f = fopen(src.c_str(), "w+");

        if(f == NULL)
        {
            perror("error creating file");
            return;
        }

        if(fclose(f) != 0)
        {
            perror("");
            return;
        }

    }
    cout<< "successful";
}

void create_dir_func()
{

    string dest = command_arg[command_arg.size()-1];
    dest = create_absolute_path(dest);

    for(int i =1; i<command_arg.size()-1; i++)
    {
        string src;
        src = dest + "/" + command_arg[i];
        
        DIR *d;
        if(mkdir(src.c_str(), 0755) == -1)
        {
            perror("error creating directory");
            return;
        }

    }
    cout<< "successful";
}

void search_func()
{
    if(command_arg.size() < 2)
    {
        cout<< "Insufficient arguments";
        return;
    }
    string to_be_searched = string(root) + "/" + command_arg[1];
    //cout<< "hi";
    search_recursive((string)root, command_arg[1]);
    if(search_val)
    {
    cout << endl << "true";
    search_val =false;
    }
    else
    {
        cout<< endl <<"false";
    }
    
    //cout << search_val;
}

void search_recursive(string direc, string dest)
{
    struct dirent *di;
    DIR *d;
    
    d = opendir(direc.c_str());

    if (d == NULL) {
        perror("opendir");
        return;
    }
    dest= direc + "/" + command_arg[1];
    while((di = readdir(d)))
    {
        //cout<< "here"<< " ";
        if(string(di->d_name) == "." || string(di->d_name) == "..")
        continue;

        string st = direc + "/" + string(di->d_name);
        
       
        //cout << st << endl;
        //cout << "DEST:" << " " << dest <<endl;
        if(dest == st)
        search_val = true;

        if(isDirectory(st))
        {
           
           search_recursive(st, dest);
        }
    
    }

    closedir(d);
    return;
    
}

void goto_func()
{
    goto_bool = true;
    if(command_arg.size() < 2)
    {
        cout<< "Insufficient arguments";
        return;
    }
    
   //back.push(string(cur_dir));
   string st = command_arg[1];
   st = st.substr(1, command_arg[1].length());
   st = string(root)+st;
   strcpy(cur_dir, st.c_str());
   //cout<< endl;
   listdirectory(cur_dir); 

}