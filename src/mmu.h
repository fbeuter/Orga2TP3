/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
  definicion de funciones del manejador de memoria
*/

#ifndef __MMU_H__
#define __MMU_H__

#include "defines.h"
#include "game.h"

#define CODIGO_BASE         0x400000

#define MAPA_BASE_FISICA    0x500000
#define MAPA_BASE_VIRTUAL   0x800000

void mmu_inicializar();
void mmu_inicializar_dir_kernel();
int* mmu_inicializar_dir_pirata();

int* mmu_get_pagina();
void mmu_copiar_pagina(int* original, int* copia);
void mmu_unmapear_pagina(unsigned int virtual, unsigned int cr3);
void mmu_mapear_pagina(unsigned int virtual, unsigned int cr3, unsigned int fisica);

#endif	/* !__MMU_H__ */
