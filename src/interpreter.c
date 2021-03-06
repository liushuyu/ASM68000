# include<stdint.h>
# include<stdlib.h>
# include <err.h>
# include"stdcpu.h"
# include"acces_ram.h"
# include"ccr.h"

/*void ccr(uint32_t mask, uint32_t* src, uint32_t* dst, uint32_t rslt)
{
 struct cpu* k = get_cpu();
 int i =0;
 uint32_t src2 = *src;
 uint32_t dst2 = *dst;
 if (!rslt)
 {
   //z a 1
 }
 else
 {
   //z a 0
 }
 while (mask !=0)
 {
   i++
   mask = mask >> 1; // exemple : cas du 16 bits i = 16
 }
 if (i < 32)
 {
    while (i >0)
    {
      src2 = src2 >> 1;
      dst2 = dst2 >> 1; //dst2, src2 et rslt = bit de poid fort
      rslt = rslt >> 1;
      i--
    }
    //n = rslt
    if (src2 == dst2 && rslt != dst2)
    {
      //v a 1
    }
    else
    {
      //v a 0
    }
    //cas de c
    rslt = rslt >>1;
    //c = rslt
    //x = c

 }
}*/

int getMoveSize(uint16_t src)
{
  switch (src)
  {
    case(1):
      return 0;
    case(2):
      return 2;
    case(3):
      return 1;
    default:
      return -1;
  }

}

void jsr(uint16_t opcode)
{
  struct cpu* k = get_cpu();
  char earegister = 0;
  char eamode = 0;
  eamode = (char)((opcode >> 3) & 0x7);
  earegister = (char)(opcode & 0x7);
  ram_write(0xFFFFFFFF, k->A[7] - 4, ram_read(0xFFFFFFFF, k->PC +2));
  k->A[7] = k->A[7]-4;
  k->PC = ram_read(0xFFFFFFFF, k->A[(int)earegister]);
}

void rts(uint16_t opcode)
{
  struct cpu* k = get_cpu();
  k->PC = ram_read(0xFFFFFFFF, k->A[7]);
  k->A[7] = k->A[7]+4;
}

void move(uint16_t opcode)
{
  int size;
  uint32_t dPC;
  uint16_t  temp;
  uint32_t mask;
  uint32_t mask2;
  uint32_t  src;
  uint32_t* dst;
  uint32_t  dst2;
  struct cpu* cpu;

  cpu = get_cpu();
  temp = opcode>>12;
  dPC = 2;
  size = getMoveSize(temp);
  switch (size)
  {
    case(0):
      mask = 0xFFFF0000;
      mask2 = 0x000000FF;
      break;
    case(1):
      mask = 0xFFFF0000;
      mask2 = 0x0000FFFF;
      break;
    case(2):
      mask = 0xFFFFFFFF;
      mask2 = mask;
      break;
    default:
      err(2,"invalid size");
  }
  temp = opcode & 0x0038;
  switch(temp>>3)
  {
    case(0):
      src = cpu->D[opcode & 0x0007] & mask2;
      break;
    case(1):
      src = cpu->A[opcode & 0x0007] & mask2;
      break;
    case(2):
      src = ram_read(mask, cpu->A[opcode & 0x0007]);
      break;
    case(3):
      src = ram_read(mask, cpu->A[opcode & 0x0007]);
      cpu->A[opcode & 0x0007] += 1;
      break;
    case(4):
      cpu->A[opcode & 0x007] -= 1;
      src = ram_read(mask, cpu->A[opcode & 0x0007]);
      break;
    case(7):
      src = ram_read(mask, (cpu->PC) + 2);
      if(size > 1)
        dPC = 6;
      else
        dPC = 4;
      break;
    default:
      err(3,"not supported");
  }
  temp = opcode & 0x0FFF;
  temp = temp>>9;

  switch((opcode & 0x01B0) >>6 )
  {
    case(0):
      dst = &(cpu->D[temp]);
      switch(size)
      {
        case(0):
          *dst = *dst & 0xFFFFFF00;
          break;
        case(1):
          *dst = *dst & 0xFFFF0000;
          break;
        case(2):
          *dst = *dst & 0x00000000;
          break;
      }

      *dst |= src;
      cpu->PC += dPC;
      return;

    case(2):
      dst2 = cpu->A[temp];
      break;
    case(3):
      dst2 = cpu->A[temp];
      cpu->A[temp] += 1;
      break;
    case(4):
      cpu->A[temp] -= 1;
      dst2 = cpu->A[temp];
      break;
    default:
      err(3,"not implemented");
  }
  if(size == 0){
    mask = 0xFF000000;
  }
  ram_write(mask, dst2, src);
  ccr(src, mask2);
  cpu->PC += dPC;
}

void add(uint16_t opcode)
{
  //recuperer les bits avec des bitwise, en deduire les modes d'addressage et les valeurs, faire le calcul
  char dn = 0;
  char opmode = 0;
  char eamode = 0;
  char earegister = 0;
  dn = (char)((opcode >> 9) & 0x7);
  opmode = (char)((opcode >> 6) & 0x7);
  eamode = (char)((opcode >> 3) & 0x7);
  earegister = (char)(opcode & 0x7);
  struct cpu* k = get_cpu();
  uint32_t mask = 0;
  uint32_t mask_ram = 0;
  uint32_t tmp;
  if ((opmode == 0) || (opmode == 4))
  {
    mask = 0xFF;
    mask_ram = 0xFFFF0000;
  }
  else if ((opmode == 1) || (opmode == 5))
  {
    mask = 0xFFFF;
    mask_ram = 0xFFFF0000;
  }
  else
  {
    mask = 0xFFFFFFFF;
    mask_ram = 0xFFFFFFFF;
  }
  if (eamode == 0) //cas Dn
  {
    if((opmode == 0) || (opmode == 1) || (opmode == 2))
    {
      k->D[(int)dn] = (k->D[(int)dn] & ~mask) | ((k->D[(int)earegister] & mask) + (k->D[(int)dn] & mask));
      tmp = k->D[(int)dn];
    }
    else
    {
      k->D[(int)earegister] = (k->D[(int)earegister] & ~mask) | ((k->D[(int)dn] & mask) + (k->D[(int)earegister] & mask));
      tmp = k->D[(int)earegister];
    }

  }
  if (eamode == 1) //cas An
  {
    if((opmode == 0) || (opmode == 1) || (opmode == 2))
    {
      k->D[(int)dn] = (k->D[(int)dn] & ~mask) | ((k->A[(int)earegister] & mask) + (k->D[(int)dn] & mask));
      tmp = k->D[(int)dn];
    }
    else
    {
      k->A[(int)earegister] = (k->A[(int)earegister] & ~mask) | ((k->D[(int)dn] & mask) + (k->A[(int)earegister] & mask));
      tmp = k->A[(int)earegister];
    }
  }
  if (eamode == 2) //cas (An), demande bidoui!llage ram car en char, ferais demain
  {
    if((opmode == 0) || (opmode == 1) || (opmode == 2))
    {
      k->D[(int)dn] = (k->D[(int)dn] & ~mask) | (ram_read(mask_ram, k->A[(int)earegister] & mask) + (k->D[(int)dn] & mask));
      tmp = k->D[(int)dn];
    }
    else
    {
      tmp = ((k->D[(int)dn] & mask) + (ram_read(mask_ram, k->A[(int)earegister])));
      if (mask == 0xFF) {////////////////
        mask_ram = 0xFF000000;
      }

      ram_write(mask_ram, k->A[(int)earegister] & mask, tmp);
    }
  }
  if (eamode == 3) //cas (An)+
  {
    if((opmode == 0) || (opmode == 1) || (opmode == 2))
    {
      k->D[(int)dn] = (k->D[(int)dn] & ~mask) | (ram_read(mask_ram, k->A[(int)earegister] & mask) + (k->D[(int)dn] & mask));
      tmp = k->D[(int)dn];
    }
    else
    {
      tmp = ((k->D[(int)dn] & mask) + (ram_read(mask_ram, k->A[(int)earegister])));
      if (mask == 0xFF) {////////////////
        mask_ram = 0xFF000000;
      }
      ram_write(mask_ram, k->A[(int)earegister] & mask, tmp);
    }
    if (mask == 0xFF)
    {
      k->A[(int)earegister] = k->A[(int)earegister] + 1;
    }
    else if (mask == 0xFFFF)
    {
      k->A[(int)earegister] = k->A[(int)earegister] + 2;
    }
    else
    {
      k->A[(int)earegister] = k->A[(int)earegister] + 4;
    }
  }
  if (eamode == 4) //cas -(An)
  {
    if (mask == 0xFF)
    {
      k->A[(int)earegister] = k->A[(int)earegister] - 1;
    }
    else if (mask == 0xFFFF)
    {
      k->A[(int)earegister] = k->A[(int)earegister] - 2;
    }
    else
    {
      k->A[(int)earegister] = k->A[(int)earegister] - 4;
    }
    if((opmode == 0) || (opmode == 1) || (opmode == 2))
    {
      k->D[(int)dn] = (k->D[(int)dn] & ~mask) | (ram_read(mask_ram, k->A[(int)earegister] & mask) + (k->D[(int)dn] & mask));
      tmp = k->D[(int)dn];
    }
    else
    {
      tmp = ((k->D[(int)dn] & mask) + (ram_read(mask_ram, k->A[(int)earegister])));
      if (mask == 0xFF) {////////////////
        mask_ram = 0xFF000000;
      }
      ram_write(mask_ram, k->A[(int)earegister] & mask, tmp);
    }
  }
  if (eamode == 7) //cas #data
  {
    k->D[(int)dn] = (k->D[(int)dn] & ~mask) | ((k->D[(int)dn] & mask) + (ram_read(mask_ram, k->PC+2)));
    tmp = k->D[(int)dn];
    if (mask == 0xFFFF)
    {
      k->PC = k->PC + 2;
    }
    else if (mask == 0xFFFFFFFF)
    {
      k->PC = k->PC + 4;
    }
  }
  ccr(mask, tmp);
  k->PC = k->PC + 2;
}

void bcc(uint16_t opcode)
{
  char displacement = (char)opcode;
  char condition = (char)((opcode >> 8) & 0xF);
  struct cpu* k = get_cpu();
  if (condition == 0x7) //beq, branche si z = 1
  {
    if (((k->SR >> 2) & 1) == 1)
    {
      if (displacement == 0x00)
      {
        k->PC = k->PC + ram_read(0xFFFF0000,k->PC + 2);
      }
      else if ((unsigned char)displacement == 0xFF)
      {
        k->PC = k->PC + ram_read(0xFFFFFFFF, k->PC + 2);
      }
      else
      {
        k->PC = k->PC + (uint32_t)displacement;
      }
    }
    else
    {
      if (displacement == 0x00)
      {
        k->PC = k->PC + 2;
      }
      else if ((unsigned char)displacement == 0xFF)
      {
        k->PC = k->PC + 4;
      }
      k->PC = k->PC + 2;
    }
  }
  else if (condition == 0x6) //bne, branche si z = 0
  {
    if (((k->SR >> 2) & 1) == 0)
    {
      if (displacement == 0x00)
      {
        k->PC = k->PC + ram_read(0xFFFF0000,k->PC + 2);
      }
      else if ((unsigned char)displacement == 0xFF)
      {
        k->PC = k->PC + ram_read(0xFFFFFFFF, k->PC + 2);
      }
      else
      {
        k->PC = k->PC + (uint32_t)displacement;
      }
    }
    else
    {
      if (displacement == 0x00)
      {
        k->PC = k->PC + 2;
      }
      else if ((unsigned char)displacement == 0xFF)
      {
        k->PC = k->PC + 4;
      }
      k->PC = k->PC + 2;
    }
  }
}

/*void sub(uint32_t mask, uint32_t* src, uint32_t* dst)
{
  uint32_t src2 = src & mask;
  uint32_t dst2 = dst & mask;
  dst2 = dst2-src2;
  mask = !mask;
  *dst = mask & *dst;
  *dst = *dst | dst2;
}

void jsr(uint32_t addr)
{
  struct cpu* k = get_cpu();
  //k->RAM[k->A[7]] = k->PC +2;
  int i =0;
  uint32_t pq = k->PC
  while (i <4)
  {
    k->A[7] = k ->A[7] -1;
    k->RAM[k->A[7]] = (char)pq;
    pq = pq >> 8;
    i++;
  }
  k->PC = k->PC +2;
  k->PC = addr;
}

void rts()
{
  struct cpu* k = get_cpu();
  int i =0;
  uint32_t tmp = 0;
  while (i < 4)
  {
    tmp = tmp || k->RAM[k->A[7]];
    tmp = tmp << 8;
    k->A[7] = k->A[7]+ 1;
    i++;
  }
  k->PC = tmp;
}

void beq(uint32_t addr)
{
  struct cpu* k = get_cpu();
  uint32_t z = (k->SR >> 2) & 1;
  if(z)
  {
    k->PC = addr;
  }
}

void bne(uint32_t addr)
{
  uint32_t z = (k->SR >>2) & 1;
  if (!z)
  {
    k->PC = addr;
  }
}*/

