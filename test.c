#include <stdio.h>
#include <stddef.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <elf.h>
#include <stdbool.h>

#define MAX_SYMBOL_LEN 32
#define MAX_NUM_SYMBOLS 128
char symbols[MAX_NUM_SYMBOLS*MAX_SYMBOL_LEN] = {0};
char* symbols_off = (char*)&symbols;

#define MAX_SYMTABS 16
char symtabs[sizeof(Elf64_Shdr)*2*MAX_SYMTABS] = {0};
char* symtabs_off = (char*)&symtabs;

void get_symbols(char* path) {
    FILE* fptr = fopen(path, "r");
	Elf64_Ehdr header = {0};
	fread(&header, sizeof(Elf64_Ehdr), 1, fptr);
	if (strncmp((char*)&header, ELFMAG, 3) != 0) {
        printf("Attempted read of invalid ELF file.");
    }
    fseek(fptr, header.e_shoff, SEEK_SET);
	Elf64_Shdr syment[2] = {0};
    for (int i = 0; i < header.e_shnum-1; i++) {
		fread(&syment, sizeof(Elf64_Shdr), 2, fptr); // NOTE: questionable
		if (syment[0].sh_type == SHT_SYMTAB && syment[1].sh_type == SHT_STRTAB) {
            memcpy(symtabs_off, &syment[0], sizeof(Elf64_Shdr));
            memcpy(symtabs_off+sizeof(Elf64_Shdr), &syment[1], sizeof(Elf64_Shdr));
            symtabs_off = symtabs_off+(sizeof(Elf64_Shdr)*2);
		}
		fseek(fptr, -sizeof(Elf64_Shdr), SEEK_CUR); // HACK 
	}
	for (int i = 0; i < (symtabs_off-symtabs)/(sizeof(Elf64_Shdr)*2); i++) {
        Elf64_Shdr symtab0 = ((Elf64_Shdr*)symtabs)[i*2];
        Elf64_Shdr symtab1 = ((Elf64_Shdr*)symtabs)[i*2+1];
        
        fseek(fptr, symtab1.sh_offset, SEEK_SET);
		char* strtbl = malloc(symtab1.sh_size);
		fread(strtbl, symtab1.sh_size, 1, fptr);
		fseek(fptr, symtab0.sh_offset, SEEK_SET);
		Elf64_Sym symbol = {0};
		for (size_t i = 0; i < symtab0.sh_size; i+=sizeof(Elf64_Sym)) {
			fread(&symbol, sizeof(Elf64_Sym), 1, fptr);
			if (symbol.st_name != 0) {
				char* mangled = (char*)(&strtbl[symbol.st_name]);
                if (strncmp(mangled, "test_", 5) == 0) {
                    strcpy(symbols_off, mangled);
                    symbols_off += MAX_SYMBOL_LEN;
                }				
			}
		}
	}    
	fclose(fptr);
}



int main(int argc, char** argv) {
    get_symbols(argv[1]);
    char path[64] = {0};
    sprintf(path, "./%s", argv[1]);
    void* lib = dlopen(path, RTLD_LAZY);
    for (char* sym = symbols; sym < symbols_off; sym+=MAX_SYMBOL_LEN) {
        printf("%s ", sym+5);
        fflush(STDIN_FILENO);
        bool(*testf)(void) = dlsym(lib, sym);        
        pid_t pid = fork();
        if (pid == 0) {
            testf();
            exit(0);
        } else {
            int status;
            waitpid(pid, &status, 0);
            printf(" [%s]\n", status == 0 ? "GOOD" : "BAD");
        }
    }
    dlclose(lib);
}
