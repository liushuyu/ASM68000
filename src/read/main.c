# include "head.h"
# include "test.h"


struct label *push_label(struct label *l, char *subroutine, uint32_t adress) {
  struct label *res=malloc(sizeof(struct label));
  res->subroutine = subroutine;
  res->adress = adress;
  res->next=NULL;
  if(l){
    struct label *cur = l;
    for(; cur->next; cur=cur->next){}
    cur->next = res;
  }
  else
    l = res;
  return l;
}


struct cpu *cp = NULL;

int main(int argc, char *argv[]){
  if (argc > 1) {
    int r;
    FILE *file = fopen(argv[1], "r");
    cp = get_cpu();
    struct cpu cpu = *cp;
    uint32_t pc = 0;
    struct label *l = NULL;
    do { // 1st parsing label
      char **arg = arg_init();
      r = read_line(file,arg);
      if((arg[1]||arg[3]||arg[4]) && r) {
        pc += 2;
	if(strcmp(arg[1],"org")==0){
	  pc = conv_hex(arg[3]);
	}
      }
      if(arg[0]){
	l=push_label(l,arg[0],pc);
      }
    } while(r && (r!=-1));
    read_label(l);
    fclose(file);
    FILE *file2 = fopen(argv[1], "r");
    printf("\nParsing : \n");
    do { // 2nd parsing
      char **arg = arg_init();
      r = read_line(file2,arg);
      if((arg[0]||arg[1]||arg[3]||arg[4]) && r) {
        test_read(arg);
      }
      arg_free(arg);
    } while(r && (r!=-1));
    fclose(file2);
    free(l);
    printf("\n");
    //cpu.RAM[1] = 160; // TEST
    get_op(argv[1],"out.bin");
    optoram("out.bin",&cpu);
//    DUMPMEM(cpu.RAM, 1000*sizeof(uint16_t));
    cp = &cpu;
    read_all();
  }
  else
    err(1,"no arg");
  return 0;
}
