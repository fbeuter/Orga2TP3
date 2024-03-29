; ** por compatibilidad se omiten tildes **
; ==============================================================================
; TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
; ==============================================================================
; definicion de rutinas de atencion de interrupciones

%include "imprimir.mac"

BITS 32

sched_tarea_offset:     dd 0x00
sched_tarea_selector:   dw 0x00

;; PIC
extern fin_intr_pic1

;; Sched
extern sched_tick
extern sched_tarea_actual
extern sched_proxima_a_ejecutar
extern sched_syscall
extern sched_pendiente

;; Screen
extern screen_actualizar_reloj_global
extern print

;; Game
extern game_syscall_pirata_mover
extern game_syscall_pirata_cavar
extern game_syscall_pirata_posicion
extern game_atender_teclado
extern game_guardar_estado

;;
;; Definición de MACROS
;; -------------------------------------------------------------------------- ;;

%macro ISR 1
global _isr%1

_isr%1:
	pushad
	call game_guardar_estado
	call fin_intr_pic1
	mov eax, 1
	push eax
	call sched_syscall
	add esp, 4

	popad
	jmp 0x70:0
	iret

%endmacro

;;
;; Datos
;; -------------------------------------------------------------------------- ;;
; Scheduler

;;
;; Rutina de atención de las EXCEPCIONES
;; -------------------------------------------------------------------------- ;;
ISR 0
ISR 1
ISR 2
ISR 3
ISR 4
ISR 5
ISR 6
ISR 7
ISR 8
ISR 9
ISR 10
ISR 11
ISR 12
ISR 13
ISR 14
ISR 15
ISR 16
ISR 17
ISR 18
ISR 19

;;
;; Rutina de atención del RELOJ
;; -------------------------------------------------------------------------- ;;

global _isr32
_isr32:
	; GUARDO REGISTROS ANTES DE LLAMAR A LA FUNCION DE C
	pushad

	call fin_intr_pic1
	call sched_tick
	str cx
	cmp ax, cx
	je .fin

	mov [sched_tarea_selector], ax
	jmp far [sched_tarea_offset]

	.fin:
	popad
	iret



;;
;; Rutina de atención del TECLADO
;; -------------------------------------------------------------------------- ;;

global _isr33
_isr33:
	pushad

	call fin_intr_pic1

	xor eax, eax
	in al, 0x60
	push eax
	
	call game_atender_teclado

	add esp, 4
	popad	
	iret

;;
;; Rutinas de atención de las SYSCALLS
;; -------------------------------------------------------------------------- ;;

global _isr70
_isr70:

	pushad
	mov ebx, eax ; Codigo del syscall
	mov esi, ecx ; Parametro secundario

	cmp ebx, 0x1
	je .moverse
	cmp ebx, 0x2
	je .cavar
	cmp ebx, 0x3
	je .posicion

	jmp .fin

	.moverse:
		call sched_tarea_actual
		push esi
		push eax
		call game_syscall_pirata_mover
		add esp, 8
		jmp .fin

	.cavar:
		call sched_tarea_actual
		push eax
		call game_syscall_pirata_cavar
		push eax
		call sched_syscall
		add esp, 8
		jmp .fin

	.posicion:
		call sched_tarea_actual
		push esi
		push eax
		call game_syscall_pirata_posicion
		add esp, 8
		jmp .fineax

	.fin:
	popad
	jmp 0x70:0
	iret

	.fineax: ; Fin con retorno de parametro por eax
	pop edi
	pop esi
	pop ebp
	add esp, 4
	pop ebx
	pop edx
	pop ecx
	add esp, 4
	push eax
	jmp 0x70:0
	pop eax
	iret
