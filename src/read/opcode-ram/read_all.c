#include "read_all.h"
#include "../../acces_ram.h"

void read_all() {
  struct cpu *cpu = get_cpu();
  uint32_t i = cpu->PC;
  int j = 0;
  printf("\nSTART\n");
  ram_write(0xffff0000,0x504,0x64);
  while(ram_read(0xFFFF0000, i) != 0x4afc /*&& ram_read(0xFFFFFFFF, i) != 0*/){
   // if(ram_read(0xFFFFFFFF, i) != 0) {
      //exec()
      j = cpu->RAM[i];
      j >>= 4;
      j &= 15;
      if(j <= 3 && j >= 1) {
	//i += move();
	printf("move\n");
	printf("%04x\n", ram_read(0xFFFF0000, i));
      } else {
        printf("%04x\n", ram_read(0xFFFF0000, i));
      }
   // }
    i += 2;
  }
  printf("%04x\n", ram_read(0xFFFF0000, i));
  printf("END\n");
  cpu->PC = i;
}
