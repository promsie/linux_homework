#include <iostream>
#include<iomanip>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include<pwd.h>
#include<grp.h>
#include <time.h>

class ls_l
{	public:
        ls_l(){
            path = ".";
	        pdir = opendir(path);
            if(pdir == NULL)
		       perror("opendir");
	    
		};
		~ls_l(){
			if (pdir != NULL)
		              closedir(pdir);
		}
        void read_dir(){
		    while(preaddir = readdir(pdir)){
			    if(0 == lstat(preaddir->d_name, &st)){
			        if(preaddir->d_name[0] != '.'){
				        char file_display[10];
                        file_message(file_display);
				        if (file_display[0] != 'l')
					        if( (st.st_size % 4096) == 0)
						        i += (st.st_size/4096 ) * 4;
					        else
						        i += (st.st_size/4096 + 1 ) * 4;
				        display_all(file_display);
				    }
		        }
	        }
			    std::cout << "总计: "<< i << std::endl;
	            return ;
        }
	    void file_message( char * display_message){
		    if (S_ISREG (st.st_mode)) display_message[0] =  '-';
		    else if (S_ISDIR (st.st_mode)) display_message[0] =  'd';
		    else if (S_ISCHR (st.st_mode)) display_message[0] =  'c';
            else if (S_ISBLK (st.st_mode)) display_message[0] =  'b';
            else if (S_ISFIFO (st.st_mode)) display_message[0] =  'p';
	        else if (S_ISLNK (st.st_mode)) display_message[0] =  'l';
            else  display_message[0] =  's';
		    display_message[1] = (S_IRUSR & st.st_mode )? 'r' : '-';
		    display_message[2] = (S_IWUSR & st.st_mode )? 'w' : '-';
		    display_message[3] = (S_IXUSR & (st.st_mode) )? 'x' : '-';
		    display_message[4] = (S_IRGRP & (st.st_mode) )? 'r' : '-';
		    display_message[5] = (S_IWGRP & (st.st_mode) )? 'w' : '-';
		    display_message[6] = (S_IXGRP & (st.st_mode) ) ? 'x' : '-';
            display_message[7] = (S_IROTH & (st.st_mode) )? 'r' : '-';
	        display_message[8] = (S_IWOTH & (st.st_mode) )? 'w' : '-';
		    display_message[9] = (S_IXOTH & (st.st_mode) )? 'x' : '-';
		    return ;
		}
	    void display_all(char * display_message){
		    std::cout << display_message <<" ";
		    std::cout << std::setw(2) << std::right<<  st.st_nlink << " ";
		    std::cout << getpwuid(st.st_uid)->pw_name << " ";
		    std::cout << getgrgid(st.st_gid)->gr_name << " ";
		    struct tm* pt = localtime(&(st.st_mtime));
		    std::cout << std::setw(6) << std::right << st.st_size << " ";
		    std::cout << pt->tm_mon + 1<<"月" << " ";       
		    std::cout << std::setw(2) << std::right << pt->tm_mday << " ";
		    std::cout << std::setw(2) << std::right << pt->tm_hour<< ":";
		    std::cout << std::setw(2) << std::right << pt->tm_min;
		    std::cout<< " " << (preaddir->d_name);
		    
		    
		    if ( display_message[0] == 'l' ){
		        char target_path[256];
			    ssize_t len = readlink(preaddir->d_name, target_path, sizeof(target_path) - 1);
			    target_path[len] = '\0';
			    std::cout << "->" <<  target_path;
		    }
		    std::cout << std::endl;
		}
    private:
        const char* path ; 
	    int i = 0;
	    struct stat st;
	    DIR* pdir = NULL;
	    struct dirent* preaddir = NULL;
        
};

int main()
{
	
    ls_l a;
	a.read_dir();
	return 0;
}
