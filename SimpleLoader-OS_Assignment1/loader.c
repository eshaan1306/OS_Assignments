#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

/*
 * release memory and other cleanups
 */
void loader_cleanup() {
  if (fd>=0){
    close(fd);
    fd=-1;
  }
  if (ehdr){
    free(ehdr);
    ehdr=NULL;
  }
  if (phdr){
    free(phdr);
    phdr=NULL;
  }
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char** argv) {
  fd = open(argv[1], O_RDONLY);
  if (fd<0){
    perror("Error, Couldn't open file");
    exit(1);
  }
  // 1. Load entire binary content into the memory from the ELF file.
  ehdr=malloc(sizeof(Elf32_Ehdr));
  if (ehdr==NULL){
    perror("Error");
	exit(1);
  }
  if (read(fd,ehdr,sizeof(Elf32_Ehdr))!=sizeof(Elf32_Ehdr)){
        perror("Error");
        exit(1);
   }
  const unsigned int entry=ehdr->e_entry;
  const unsigned short phsize=ehdr->e_phentsize;
  const unsigned int phoff=ehdr->e_phoff;
  const unsigned short phnum=ehdr->e_phnum;
  if (ehdr->e_phnum == 0){
    fprintf(stderr,"Error: ELF file has no program headers\n");
    exit(1);
  }
  if (lseek(fd,phoff,SEEK_SET)<0){
    perror("Error");
    exit(1);
  }
  phdr=malloc(sizeof(Elf32_Phdr));
  if (phdr==NULL){
    perror("Error");
    exit(1);
    }
  // 2. Iterate through the PHDR table and find the section of PT_LOAD 
  //    type tht contains the address of the entrypoint method in fib.c
  void *virtual_mem;
  int found=0;
  for (int i=0;i<phnum;i++){
    if (read(fd,phdr,sizeof(Elf32_Phdr))!=sizeof(Elf32_Phdr)){
      perror("Error");
      exit(1);
     }
    if (phdr->p_memsz<phdr->p_filesz){
    fprintf(stderr, "Error: Segment memory size < file size\n");
    exit(1);
    }
    if(phdr->p_type!=PT_LOAD){continue;}
    if (entry>=phdr->p_vaddr && entry<phdr->p_vaddr + phdr->p_memsz){
      found=1;
      virtual_mem = mmap(NULL,
                             phdr->p_memsz,
                             PROT_READ | PROT_WRITE | PROT_EXEC,
                             MAP_PRIVATE | MAP_ANONYMOUS,
                             -1, 0);
   }else {continue;}
    if (virtual_mem == MAP_FAILED) {
      perror("Error");
      exit(1);
    }
    if (lseek(fd,phdr->p_offset,SEEK_SET)==-1){
      perror("lseek failed");
      exit(1);
    }
    if (read(fd,virtual_mem,phdr->p_memsz) != phdr->p_memsz){
      perror("Error");
      exit(1);
    }
  unsigned int off=entry-phdr->p_vaddr;
  int (*_start)() = (int (*)())((char *)virtual_mem + off);
  // 3. Allocate memory of the size "p_memsz" using mmap function 
  //    and then copy the segment content
  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  // 6. Call the "_start" method and print the value returned from the "_start"
  int result = _start();
  printf("User _start return value = %d\n",result);
}
if (!found) {
    fprintf(stderr, "Error: Entry point outside loadable segment\n");
    exit(1);
}

}
int main(int argc,char** argv){
  if(argc != 2){
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  if (argv[1]==NULL){
    perror("Error");
    exit(1);
  }
  if (strlen(argv[1])==0){
    perror("Error");
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file - done
  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(argv);
  // 3. invoke the cleanup routine inside the loader  
  loader_cleanup();
  return 0;
}
