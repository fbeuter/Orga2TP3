; ** por compatibilidad se omiten tildes **
; ==============================================================================
; TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
; ==============================================================================

%include "imprimir.mac"

global start

extern screen_inicializar
extern screen_pintar_puntajes
extern idt_inicializar
extern print_group

extern mmu_inicializar
extern mmu_inicializar_dir_pirata
extern mmu_inicializar_dir_kernel

extern GDT_DESC
extern IDT_DESC

;; Saltear seccion de datos
jmp start

;;
;; Seccion de datos.
;; -------------------------------------------------------------------------- ;;
iniciando_mr_msg db     'Iniciando kernel (Modo Real)...'
iniciando_mr_len equ    $ - iniciando_mr_msg

iniciando_mp_msg db     'Iniciando kernel (Modo Protegido)...'
iniciando_mp_len equ    $ - iniciando_mp_msg

;;
;; Seccion de código.
;; -------------------------------------------------------------------------- ;;

;; Punto de entrada del kernel.
BITS 16
start:
    ; Deshabilitar interrupciones
    cli

    ; Cambiar modo de video a 80 X 50
    mov ax, 0003h
    int 10h ; set mode 03h
    xor bx, bx
    mov ax, 1112h
    int 10h ; load 8x8 font

    ; Imprimir mensaje de bienvenida
    imprimir_texto_mr iniciando_mr_msg, iniciando_mr_len, 0x07, 0, 0

    ; Ejercicio 1
    ; Habilitar A20
    CALL habilitar_A20
    ; Cargar la GDT
    LGDT [GDT_DESC]
    ; Setear el bit PE del registro CR0
    MOV eax, cr0
    or eax, 1
    mov cr0, eax
    ; Saltar a modo protegido
    jmp 0x40:modoprotegido
    BITS 32
    modoprotegido:
    ; Establecer selectores de segmentos
    MOV AX, 0x48    
    MOV DS, AX
    MOV AX, 0x48
    MOV SS, AX
    ; Establecer la base de la pila
    MOV EBP, 0x27000
    MOV ESP, EBP
    ; Imprimir mensaje de bienvenida
    imprimir_texto_mp iniciando_mp_msg, iniciando_mp_len, 0x07, 2, 0
    ; Limpiar la pantallita wiiii :P
    ;CALL screen_inicializar

    ; Ejercicio 2
    ; Inicializar el juego

    ; Inicializar pantalla
    CALL screen_inicializar
    CALL screen_pintar_puntajes

    ; Inicializar el manejador de memoria


    ; Inicializar el directorio de paginas
    CALL mmu_inicializar_dir_kernel

    ; Cargar directorio de paginas

    ; Habilitar paginacion
    MOV EAX, 0x27000
    mov CR3, EAX

    MOV EAX, CR0
    OR  EAX, 0x80000000
    MOV CR0, EAX

    ; Imprimir el nombre del grupo
    CALL print_group

    CALL mmu_inicializar
    PUSH 0x200000   ;posMapa
    PUSH 0x100000   ;codigo
    MOV  EAX, CR3
    PUSH EAX        ;cr3
    xchg bx, bx
    CALL mmu_inicializar_dir_pirata

    ; Inicializar tss

    ; Inicializar tss de la tarea Idle

    ; Inicializar el scheduler
    
    ; Inicializar la IDT
    call idt_inicializar

    ; Cargar IDT
    lidt [IDT_DESC]

    ; Configurar controlador de interrupciones

    ; Cargar tarea inicial

    ; Habilitar interrupciones

    ; Saltar a la primera tarea: Idle

    ; Ciclar infinitamente (por si algo sale mal...)
    mov eax, 0xFFFF
    mov ebx, 0xFFFF
    mov ecx, 0xFFFF
    mov edx, 0xFFFF
    jmp $
    jmp $

;; -------------------------------------------------------------------------- ;;

%include "a20.asm"