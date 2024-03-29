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

int run_test_safe(bool(*testf)(void)) {
    pid_t pid = fork();
    if (pid == 0) {
        testf();
        exit(0);
    } else {
        int status;
        waitpid(pid, &status, 0);
        return status;
    }        
}

#define ANSI_RED "\x1b[31m"
#define ANSI_GREEN "\x1b[32m"
#define ANSI_BOLD "\x1b[1m"
#define ANSI_RESET "\x1b[0m"

int main(int argc, char** argv) {
    bool imode = false;
    if (argc > 2 && strcmp(argv[2], "-i") == 0) imode = true;        
    get_symbols(argv[1]);
    char path[64] = {0};
    sprintf(path, "./%s", argv[1]);
    void* lib = dlopen(path, RTLD_LAZY);
    printf(ANSI_BOLD "Running tests...\n" ANSI_RESET);
    size_t passed = 0;
    size_t total = (symbols_off-symbols)/MAX_SYMBOL_LEN;
    for (char* sym = symbols; sym < symbols_off; sym+=MAX_SYMBOL_LEN) {
        printf("  %s ", sym+5);
        fflush(STDIN_FILENO);
        bool(*testf)(void) = dlsym(lib, sym);        
        int status = run_test_safe(testf);
        
        if (status == 0) {
            passed += 1;
            puts(" " ANSI_GREEN "[GOOD]" ANSI_RESET);
        } else {
            printf(" " ANSI_RED "[BAD]" ANSI_RESET);
            if (status != 1) printf(ANSI_RED " (%d)" ANSI_RESET, status);
            printf("\n");
        }
        if (status != 0 && status != 1 && imode) testf();
    }
    printf("\n" ANSI_GREEN "%ld tests" ANSI_RESET " passed, " ANSI_RED "%ld tests" ANSI_RESET " failed.\n" ANSI_RESET,
           passed, total-passed);
    dlclose(lib);
}
