#include "loader.h"
#include <signal.h>
Elf32_Ehdr *ehdr;
Elf32_Phdr *phdrs;
int fd;
unsigned short phnum;
const unsigned long page_size=4096;
int internal_fragmentation=0;
int num_pagefault=0;
int num_pagealloc=0;
void* mapped_pages[10000];
int mapped_page_count=0;

unsigned long max(unsigned long a, unsigned long b) {
    if (a>b){
        return a;
    } else{
        return b;
    }
}

unsigned long min(unsigned long a, unsigned long b) {
    if (a<b){
        return a;
    } else{
        return b;
    }
}

void my_handler(int sig, siginfo_t *si, void *ucontext){
    if (sig==SIGSEGV){
        void *fault_addr=si->si_addr;
        int flag=0;
        for(int i=0;i<phnum;i++){
            if (phdrs[i].p_type==PT_LOAD && fault_addr>=(void *)phdrs[i].p_vaddr &&
                fault_addr<(void *)(phdrs[i].p_vaddr+phdrs[i].p_memsz)){
                  flag=1;
                  unsigned long fault_addr_long=(unsigned long)fault_addr;
                  unsigned long q=fault_addr_long/page_size;
                  void *page_start=(void *)(q*page_size);
                  void *mem=mmap(page_start,page_size,PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,-1, 0);
                  if (mem==MAP_FAILED){
                      perror("mmap failed in handler");
                      exit(1);
                  }
                  if (mapped_page_count<10000){
                      mapped_pages[mapped_page_count++]=page_start;
                  } else{
                      fprintf(stderr, "exceeds maximum tracked pages\n");
                      exit(1);
                  }
                  num_pagefault++;
                  unsigned long seg_v_start=(unsigned long)phdrs[i].p_vaddr;
                  unsigned long seg_file_offset=phdrs[i].p_offset;
                  unsigned long seg_file_end=seg_v_start+phdrs[i].p_filesz;
                  unsigned long seg_mem_end=seg_v_start+phdrs[i].p_memsz;
                  unsigned long read_start_vaddr=max((unsigned long)page_start, seg_v_start);
                  unsigned long read_end_vaddr=min((unsigned long)page_start + page_size, seg_file_end);

                  if (read_start_vaddr<read_end_vaddr){ 
                    size_t bytes_to_read=read_end_vaddr-read_start_vaddr;
                    off_t file_offset=seg_file_offset+(read_start_vaddr-seg_v_start);
                    void *write_to_addr=(void *)read_start_vaddr;

                    if (lseek(fd, file_offset, SEEK_SET)==-1){
                        perror("lseek failed in handler");
                        exit(1);
                    }
                    if (read(fd, write_to_addr, bytes_to_read)!=bytes_to_read){
                        perror("read failed in handler");
                        exit(1);
                    }
                  }
                    if (seg_mem_end>(unsigned long)page_start && seg_mem_end<(unsigned long)page_start+page_size){
                    internal_fragmentation+=((unsigned long)page_start+page_size)-seg_mem_end;
                  }
                  num_pagealloc++;
                  break;
            }
        }
        if (flag==0){
           fprintf(stderr, "Genuine Segmentation Fault at 0x%p\n", fault_addr);
           exit(1);
        }
    }
}

void loader_cleanup(){
    for (int i=0;i<mapped_page_count;i++){
        if (munmap(mapped_pages[i], page_size)==-1){
            perror("munmap fail");
        }
    }
    mapped_page_count = 0;
    if (fd>=0){
        close(fd);
        fd=-1;
    }
    if (ehdr){
        free(ehdr);
        ehdr=NULL;
    }
    if (phdrs){
        free(phdrs);
        phdrs=NULL;
    }
}

void load_and_run_elf(char** argv){
    fd=open(argv[1],O_RDONLY);
    if (fd<0){
        perror("error, couldn't open file");
        exit(1);
    }
    ehdr=malloc(sizeof(Elf32_Ehdr));
    if (ehdr==NULL){
        perror("error");
        exit(1);
    }
    if (read(fd,ehdr,sizeof(Elf32_Ehdr))!=sizeof(Elf32_Ehdr)){
        perror("error");
        exit(1);
    }
    const unsigned int entry=ehdr->e_entry;
    const unsigned short phsize=ehdr->e_phentsize;
    const unsigned int phoff=ehdr->e_phoff;
    phnum=ehdr->e_phnum;
    if (ehdr->e_phnum==0){
        fprintf(stderr,"error, ELF file has no program headers\n");
        exit(1);
    }
    if (lseek(fd,phoff,SEEK_SET)<0){
        perror("error");
        exit(1);
    }
    phdrs=malloc(phnum*sizeof(Elf32_Phdr));
    if (phdrs==NULL){
        perror("error");
        exit(1);
    }
    if (read(fd,phdrs,phnum * sizeof(Elf32_Phdr))!=(phnum * sizeof(Elf32_Phdr))) {
        perror("error reading program headers");
        exit(1);
    }
    int (*_start)()=(int (*)())ehdr->e_entry;
    int result=_start();
    printf("%d\n\n",result);
}

int main(int argc,char** argv){
    if (argc!=2){
        printf("Usage: %s <ELF Executable> \n", argv[0]);
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
    struct sigaction sig;
    memset(&sig,0,sizeof(struct sigaction));
    sig.sa_sigaction=my_handler;
    sig.sa_flags=SA_SIGINFO;
    if (sigaction(SIGSEGV,&sig,NULL)==-1){
        perror("sigaction SEGV failed");
        exit(1);
    }
    load_and_run_elf(argv);
    printf("--- SimpleSmartLoader Statistics ---\n");
    printf("Total Page Faults: %d\n", num_pagefault);
    printf("Total Page Allocations: %d\n", num_pagealloc);
    printf("Total Internal Fragmentation: %f kilobytes\n", (double)internal_fragmentation/1024.0);
    loader_cleanup();
    return 0;
}
