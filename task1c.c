#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INT sizeof(int)
#define SHORT sizeof(short)
#define BYTE sizeof(char)
//structs

typedef struct {
  char debug_mode;
  char file_name[128];
  int unit_size;
  unsigned char mem_buf[10000];
  size_t mem_count;
  /*
   .
   .
   Any additional fields you deem necessary
  */
} state;

struct fun_desc{
	char *name;
	void (*func)(state* s);
};


void set_unit_size(state* s){
	char opt[4];
	fgets(opt, sizeof(opt), stdin);
	int chosen= atoi(opt);
	if(s->debug_mode)
		fprintf(stderr, "%s %s\n", "Debug: set size to ", opt);
	if(chosen==1||chosen==2||chosen==4){
		s->unit_size=chosen;
		return;
	}
	else
		printf("%s\n", "Wrong size value");
}

void toggle_debug(state* s){
	s->debug_mode=(s->debug_mode+1)%2;
	if(s->debug_mode)
		fprintf(stderr, "%s\n", "Debug flag now on");
	else
		printf("%s\n", "Debug flag now off");
}

void set_file(state* s){
	printf("%s\n", "Please enter a file name");
	fgets(s->file_name, sizeof(s->file_name), stdin);
	s->file_name[strlen(s->file_name)-1]='\0';
	if(s->debug_mode){
		fprintf(stderr, "%s %s \n", "Debug: file name set to ", s->file_name);
	}

}

void init_state(state *s){
	s->debug_mode=0;
	s->unit_size=1;
}

void load_into_memory(state* s){
	FILE* file;
	int location, length;
	char input[256];
	if(strlen(s->file_name)==0){
		printf("%s\n", "No filename");
		return;
	}
	if(!(file=fopen(s->file_name, "r"))){
		printf("%s\n", "Filename is not valid");
		return;
	}
	if(s->debug_mode)
		fprintf(stderr, "%s %s %s\n", "File", s->file_name, "is now opened");
	printf("%s\n", "Please enter <location> <length>");
	fgets(input, 256, stdin);
	if(sscanf(input, "%x%d", &location, &length)!=2){
		printf("%s\n", "Load Failed");
		return;
	}	
	int n= fread(s->mem_buf + location, s->unit_size, length, file);
	printf("Loaded %d units into memory\n", n);
	fclose(file);
}

char* unit_to_format(int unit) {
    static char* formats[] = {"%#hhd\t\t%#hhx\n", "%#hx\t\t%#hd\n", "No such unit", "%#x\t\t%#d\n"};
    return formats[unit-1];
    /* If the above is too confusing, this can also work:
    switch (unit_size) {
        case 1:
            return "%#hhx\n";
        case 2:
            return "%#hx\n";
        case 4:
            return "%#hhx\n";
        default:
            return "Unknown unit";
    }
    */
} 
void print_units(state *s, int addr, int units) {
	char* buffer;
	if(addr==0)
		buffer = s->mem_buf;
	else
		buffer = addr;
    char* end = buffer + s->unit_size*units;
    while (buffer < end) {
        //print ints
        int var = *((int*)(buffer));
        fprintf(stdout, unit_to_format(s->unit_size), var, var);
        buffer += s->unit_size;
    }
}

void memory_display(state *s){
	char input[256];
	int units, addr;
	printf("%s\n", "Please enter <units> <addr>");
	fgets(input, 256, stdin);
	if(sscanf(input, "%d%x", &units, &addr)!=2){
		printf("%s\n", "Arguments Reading Failed");
		return;
	}	
	if(s->debug_mode)
		fprintf(stderr, "Displaying %d units, each sized %d bytes from memory\n", units, s->unit_size);
	printf("Decimal\t\tHexadecimal \n=====================\n");
	print_units(s, addr, units);
}

void save_to_file(state *s){
	FILE* file;
	void *ptr;
	char input[256];
	int source_addr, target_loc, length, file_size;
	if(strlen(s->file_name)==0){
		printf("%s\n", "No filename");
		return;
	}
	if(!(file=fopen(s->file_name, "r+"))){
		printf("%s\n", "Filename is not valid");
		return;
	}
	printf("%s\n", "Please enter <source address> <target-location> <length>");
	fgets(input, 256, stdin);
	if(sscanf(input, "%x%x%d", &source_addr, &target_loc, &length)!=3){
		printf("%s\n", "Arguments Reading Failed");
		return;
	}	
	fseek(file, 0L, SEEK_END);
	file_size=ftell(file);
	if(file_size<target_loc){
		printf("%s\n", "Target location is not valid");
		return;
	}
	fseek(file, target_loc, SEEK_SET);

	if(source_addr==0)
		ptr=s->mem_buf;
	else
		ptr=source_addr;
	fwrite(ptr, s->unit_size, length, file);
	fclose(file);
}

void file_modify(state *s){
	FILE* file;
	int *ptr;
	char input[256];
	int location, val, file_size;
	if(strlen(s->file_name)==0){
		printf("%s\n", "No filename");
		return;
	}
	if(!(file=fopen(s->file_name, "r+"))){
		printf("%s\n", "Filename is not valid");
		return;
	}
	printf("%s\n", "Please enter <location> <val>");
	fgets(input, 256, stdin);
	if(sscanf(input, "%x%x", &location, &val)!=2){
		printf("%s\n", "Arguments Reading Failed");
		return;
	}	
	fseek(file, 0L, SEEK_END);
	file_size=ftell(file);
	if(file_size<location){
		printf("%s\n", "Location is not valid");
		return;
	}
	ptr=malloc(s->unit_size);
	*ptr=val;
	fseek(file, location, SEEK_SET);
	fwrite(ptr, s->unit_size, 1, file);
	if(s->debug_mode)
		fprintf(stderr, "%d bytes in File: %s at location %x was modified to %x",s->unit_size, s->file_name, location, val);
	free(ptr);
	fclose(file);
}

void quit(state* s){
	free(s);
	exit(0);
}	

//end of methods

int main(int argc, char **argv)
{	
	char opt[4];
	int chosen, desc_size;
	state *curr_state=malloc(sizeof(char)+sizeof(int)+sizeof(size_t)+10128*sizeof(char));
	init_state(curr_state);
	struct fun_desc func_array[] ={
	{"Toggle Debug Mode", toggle_debug},
	{"Set File Name", set_file},
	{"Set Unit Size", set_unit_size},
	{"Load Into Memory", load_into_memory}, 
	{"Memory Display", memory_display},
	{"Save Into File", save_to_file},
	{"File Modify", file_modify},
	{"Quit", quit}};
	desc_size=sizeof(func_array)/sizeof(*func_array);

	while(1){
		int i;
		printf("%s\n", "Choose action:");
		for(i=0; i<desc_size; i++){
			printf("%d", i);
			printf("- %s\n", func_array[i].name);
		}
		fgets(opt, sizeof(opt), stdin);
		chosen= atoi(opt);
		printf("%s", "Option: ");
		printf("%s\n", opt);
		if((chosen>=0) && (chosen <desc_size))
			func_array[chosen].func(curr_state);
		else{
			printf("%s\n", "Not within bounds");
            exit(0);
        }
	}
	return 0;
}