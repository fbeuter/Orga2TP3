/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
  definicion de funciones del manejador de memoria
*/

#include "game.h"
#include "screen.h"
#include "mmu.h"
#include "i386.h"
/* Atributos paginas */
/* -------------------------------------------------------------------------- */
int* mem_manager;

/* Direcciones fisicas de codigos */
/* -------------------------------------------------------------------------- */
/* En estas direcciones estan los códigos de todas las tareas. De aqui se
 * copiaran al destino indicado por TASK_<i>_CODE_ADDR.
 */


/* Direcciones fisicas de directorios y tablas de paginas del KERNEL */
/* -------------------------------------------------------------------------- */


// Inicializo em manager de memoria en la posicion MEM_MANAGER, salto a la siguiente pagina
void inicializar_mmu() {
	mem_manager = (int*)AREA_LIBRE + PAGE_SIZE;
}

// Pedir una pagina al kernel, se mueve a la siguiente pagina libre, y devuelve la pagina 
int* mmu_get_pagina() {
	int* pagina = mem_manager;
	mem_manager += PAGE_SIZE;
	return pagina;
}

// Inicializar el descriptor de tablas y tablas del kernel para identity mapping de 0x0 a 0x3FFFFF
void mmu_inicializar_dir_kernel() {
	int* dir = (int*)DIR_PAGINAS_KERNEL;
	
	(*dir) = 0x28003;
	(*(dir + 1)) = 0x29003;
	(*(dir + 2)) = 0x30003;
	(*(dir + 3)) = 0x31003;

	int* table = (int*)(0x28000);
	int i;
	for(i = 0; i < 1024; i++) {
		(*table) = (i << 12) | 0x003;
		table += 1;
	}
}

// Inicializar un directorio de tablas y tabla de una tarea
int* mmu_inicializar_dir_pirata(unsigned int pos_mapa, unsigned int cr3, int* codigo) {
	int i;
	int* dir = mmu_get_pagina();
	int* table = mmu_get_pagina();

	(*dir) = (((int)table) & 0xFFFFF000) | 0x003;

	for(i = 0; i < 1024; i++) {
		(*table) = (i << 12) | 0x003;
		table += 1;
	}

	unsigned int virtual = 0x0400000 + PAGE_SIZE;
	mmu_mapear_pagina(virtual, (unsigned int)cr3, pos_mapa, READWRITE);
	mmu_mapear_pagina(0x0400000, (unsigned int)cr3, pos_mapa, READWRITE);
	mmu_mapear_pagina(0x0400000, (unsigned int)dir, pos_mapa, READWRITE);
	mmu_copiar_pagina(codigo, (int*)virtual);
	mmu_unmapear_pagina(virtual, (unsigned int)cr3);
	mmu_unmapear_pagina(0x0400000, (unsigned int)cr3);

	if(pos_mapa == INICIO_PIRATAA) {
		print(" ", 0, 1, C_BG_GREEN | C_FG_BLACK);
		print(" ", 1, 1, C_BG_GREEN | C_FG_BLACK);
		print(" ", 2, 1, C_BG_GREEN | C_FG_BLACK);
		print(" ", 0, 2, C_BG_GREEN | C_FG_BLACK);
		print("x", 1, 2, C_BG_GREEN | C_FG_BLACK);
		print(" ", 2, 2, C_BG_GREEN | C_FG_BLACK);
		print(" ", 0, 3, C_BG_GREEN | C_FG_BLACK);
		print(" ", 1, 3, C_BG_GREEN | C_FG_BLACK);
		print(" ", 2, 3, C_BG_GREEN | C_FG_BLACK);
	} else {
		print(" ", 77, 42, C_BG_BLUE | C_FG_BLACK);
		print(" ", 78, 42, C_BG_BLUE | C_FG_BLACK);
		print(" ", 79, 42, C_BG_BLUE | C_FG_BLACK);
		print(" ", 77, 43, C_BG_BLUE | C_FG_BLACK);
		print("x", 78, 43, C_BG_BLUE | C_FG_BLACK);
		print(" ", 79, 43, C_BG_BLUE | C_FG_BLACK);
		print(" ", 77, 44, C_BG_BLUE | C_FG_BLACK);
		print(" ", 78, 44, C_BG_BLUE | C_FG_BLACK);
		print(" ", 79, 44, C_BG_BLUE | C_FG_BLACK);
	}

	mmu_mapear_pagina(pos_mapa + 0x300000, (unsigned int)dir, pos_mapa, READONLY);
	mmu_mapear_pagina(pos_mapa + 0x300000 + PAGE_SIZE, (unsigned int)dir, pos_mapa + PAGE_SIZE, READONLY);
	mmu_mapear_pagina(pos_mapa + 0x300000 - PAGE_SIZE, (unsigned int)dir, pos_mapa - PAGE_SIZE, READONLY);
	mmu_mapear_pagina(pos_mapa + 0x300000 - PAGE_SIZE * 80, (unsigned int)dir, pos_mapa - PAGE_SIZE * 80, READONLY);
	mmu_mapear_pagina(pos_mapa + 0x300000 - PAGE_SIZE * 80 - PAGE_SIZE, (unsigned int)dir, pos_mapa - PAGE_SIZE * 80 - PAGE_SIZE, READONLY);
	mmu_mapear_pagina(pos_mapa + 0x300000 - PAGE_SIZE * 80 + PAGE_SIZE, (unsigned int)dir, pos_mapa - PAGE_SIZE * 80 + PAGE_SIZE, READONLY);
	mmu_mapear_pagina(pos_mapa + 0x300000 + PAGE_SIZE * 80, (unsigned int)dir, pos_mapa + PAGE_SIZE * 80, READONLY);
	mmu_mapear_pagina(pos_mapa + 0x300000 + PAGE_SIZE * 80 - PAGE_SIZE, (unsigned int)dir, pos_mapa + PAGE_SIZE * 80 - PAGE_SIZE, READONLY);
	mmu_mapear_pagina(pos_mapa + 0x300000 + PAGE_SIZE * 80 + PAGE_SIZE, (unsigned int)dir, pos_mapa + PAGE_SIZE * 80 + PAGE_SIZE, READONLY);

	return dir;
}

// Copia la pagina original a copia
void mmu_copiar_pagina(int* original, int* copia) {
	int i;
	
	for(i = 0; i < 1024; i++) {
		(*copia) = (*original);

		original = (int*)(original + 1);
		copia = (int*)(copia + 1);
	}
}

// Mapea una pagina virtual, cr3, fisica
void mmu_mapear_pagina(unsigned int virtual, unsigned int cr3, unsigned int fisica, unsigned char attr) {
	unsigned int directory_offset = (virtual & 0xFFC00000) >> 22;
	unsigned int table_offset = (virtual & 0x003FF000) >> 12;

	int* dir = (int*)(cr3 & 0xFFFFF000);
	dir += directory_offset;

	int* table;
	if(((*dir) & 0x1) == 1) {
		table = (int*)((*dir) & 0xFFFFF000);
		table += table_offset;

		(*table) = (fisica & 0xFFFFF000) | attr;
	} else {
		table = mmu_get_pagina();
		(*dir) = (((int)table) & 0xFFFFF000) | attr;

		table += table_offset;
		(*table) = (fisica & 0xFFFFF000) | attr;
	}
}

// Unpeaear una pagina virtual del cr3
void mmu_unmapear_pagina(unsigned int virtual, unsigned int cr3) {
	unsigned int directory_offset = (virtual & 0xFFC00000) >> 22;
	unsigned int table_offset = (virtual & 0x003FF000) >> 12;

	int* dir = (int*)(cr3 & 0xFFFFF000);
	dir += directory_offset;

	int* table = (int*)((*dir) + table_offset);
	(*table) = (*table) & 0xFFFFF000;
}

void mmu_mapear_pirata_V(uint id, char jugador, int posX, int posY) {
	int i;
	jugador_t* j;
	int cr3s;
	unsigned short color;

	if(jugador == 'A') {
		cr3s = CR3_JUGADORA;
		j = &jugadorA;
		color = C_BG_GREEN | C_FG_BLACK;
	} else {
		cr3s = CR3_JUGADORB;
		j = &jugadorB;
		color = C_BG_BLUE | C_FG_BLACK;
	}

	//int cr3 = *(cr3s);

	for(i = 0; i < MAX_CANT_PIRATAS_VIVOS; i++) {
		int cr3 = *((int*)(cr3s + 4 * i));

		if((*j).piratas[i].enJuego == TRUE) {
			if(posY >= 0) {
				mmu_mapear_pagina(MAPA_VIRTUAL + (posY * MAPA_ANCHO + posX) * PAGE_SIZE, cr3, MAPA + (posY * MAPA_ANCHO + posX) * PAGE_SIZE, READONLY);
				if ((*j).posicionesMapeadas[posY * MAPA_ANCHO + posX] != 1) {
			    	print(" ", posX, posY + 1, color);
			    	(*j).posicionesMapeadas[posY * MAPA_ANCHO + posX] = 1;
		    	}
		    }

			mmu_mapear_pagina(MAPA_VIRTUAL + ((posY + 1) * MAPA_ANCHO + posX) * PAGE_SIZE, cr3, MAPA + ((posY + 1) * MAPA_ANCHO + posX) * PAGE_SIZE, READONLY);
		    if ((*j).posicionesMapeadas[(posY + 1) * MAPA_ANCHO + posX] != 1) {
			    print(" ", posX, posY + 2, color);
			    //print("M", posX, posY + 1, C_BG_RED | C_FG_BLACK);
			    (*j).posicionesMapeadas[(posY + 1) * MAPA_ANCHO + posX] = 1;
			}

		    if(posY + 2 <= 43) {
				mmu_mapear_pagina(MAPA_VIRTUAL + ((posY + 2) * MAPA_ANCHO + posX) * PAGE_SIZE, cr3, MAPA + ((posY + 2) * MAPA_ANCHO + posX) * PAGE_SIZE, READONLY);
		    	if ((*j).posicionesMapeadas[(posY + 2) * MAPA_ANCHO + posX] != 1) {
			    	print(" ", posX, posY + 3, color);
			    	(*j).posicionesMapeadas[(posY + 2) * MAPA_ANCHO + posX] = 1;
			    }
		    }
		}
	}
}

void mmu_mapear_pirata_H(uint id, char jugador, int posX, int posY) {
	int i;
	jugador_t* j;
	int cr3s;
	unsigned short color;

	if(jugador == 'A') {
		cr3s = CR3_JUGADORA;
		j = &jugadorA;
		color = C_BG_GREEN | C_FG_BLACK;
	} else {
		cr3s = CR3_JUGADORB;
		j = &jugadorB;
		color = C_BG_BLUE | C_FG_BLACK;
	}

	//int cr3 = *(cr3s);

	for(i = 0; i < MAX_CANT_PIRATAS_VIVOS; i++) {
	int cr3 = *((int*)(cr3s + 4 * i));

		if((*j).piratas[i].enJuego == TRUE) {
			if(posX >= 0) {
				mmu_mapear_pagina(MAPA_VIRTUAL + (posY * MAPA_ANCHO + posX) * PAGE_SIZE, cr3, MAPA + (posY * MAPA_ANCHO + posX) * PAGE_SIZE, READONLY);
				if((*j).posicionesMapeadas[posY * MAPA_ANCHO + posX] != 1) {
				   	print(" ", posX, posY + 1, color);
				    (*j).posicionesMapeadas[posY * MAPA_ANCHO + posX] = 1;
				}
			}
		
			mmu_mapear_pagina(MAPA_VIRTUAL + (posY * MAPA_ANCHO + (posX + 1)) * PAGE_SIZE, cr3, MAPA + (posY * MAPA_ANCHO + (posX + 1)) * PAGE_SIZE, READONLY);
			if((*j).posicionesMapeadas[posY * MAPA_ANCHO + (posX + 1)] != 1) {
				print(" ", posX + 1, posY + 1, color);
				//print("M", posX + 1, posY + 1, C_BG_RED | C_FG_BLACK);
				(*j).posicionesMapeadas[posY * MAPA_ANCHO + (posX + 1)] = 1;
			}
			if(posX + 2 <= 79) {
				mmu_mapear_pagina(MAPA_VIRTUAL + (posY * MAPA_ANCHO + (posX + 2)) * PAGE_SIZE, cr3, MAPA + (posY * MAPA_ANCHO + (posX + 2)) * PAGE_SIZE, READONLY);
				if((*j).posicionesMapeadas[posY * MAPA_ANCHO + (posX + 2)] != 1) {
					print(" ", posX + 2, posY + 1, color);
					(*j).posicionesMapeadas[posY * MAPA_ANCHO + (posX + 2)] = 1;
				}
			}
		}
	}
}