; NASM: nasm -f bin main.asm -o dodge.com
[org 0x100]
[bits 16]
cpu 8086

jmp Start

TITLE equ 0
PLAYING equ 1
GAME_OVER equ 2
PLAYER_ROW equ 23

state db TITLE
quit db 0
playerCol dw 38
oldPlayerCol dw 38
obsCol dw 8, 36, 65
obsRow dw 1, 8, 15
oldObsCol dw 8, 36, 65
oldObsRow dw 1, 8, 15
obsColor db 0Eh, 0Dh, 0Bh
lives db 3
flashFrames db 0
hitThisFrame db 0
score dw 0
level dw 1
stepRows dw 1
rngState dw 1
lastTickLo dw 0
lastTickHi dw 0
rowOff dw 0,160,320,480,640,800,960,1120,1280,1440
       dw 1600,1760,1920,2080,2240,2400,2560,2720,2880
       dw 3040,3200,3360,3520,3680,3840
hudScore db 'Score: ',0
hudLevel db 'Level: ',0
hudLives db 'Lives: ',0
titleText db 'DODGE THE FALLING OBSTACLES',0
startText db 'Press SPACE or ENTER to start',0
controlText db 'A / Left: move left    D / Right: move right',0
rulesText db 'Dodge blocks. Every 50 points increases falling speed.',0
exitText db 'ESC: exit to DOS',0
overText db 'GAME OVER',0
restartText db 'R: new game    ESC: exit',0
finalText db 'Final score: ',0

Start:
    ; Own stack in the COM segment. No custom interrupt vectors are installed;
    ; BIOS INT 10h/16h/1Ah and DOS INT 21h remain responsible for services.
    cli
    mov ax, cs
    mov ss, ax
    mov sp, stackTop
    mov ds, ax
    sti
    cld
    mov ax, 0003h
    int 10h
    mov ah, 01h
    mov cx, 2000h                 ; Hide hardware text cursor.
    int 10h
    mov ax, 0B800h
    mov es, ax
    xor ah, ah
    int 1Ah
    mov [rngState], dx
    call ResetClock
    call ShowTitle

MainLoop:
    call WaitFrame
    cmp byte [state], PLAYING
    jne .screenInput
    call SaveActors
    call ScanKeyboard
    cmp byte [quit], 0
    jne ExitGame
    call EraseActors
    call UpdatePhysics
    call DrawAll
    cmp byte [state], GAME_OVER
    jne MainLoop
    call ShowGameOver
    jmp MainLoop
.screenInput:
    call ScanKeyboard
    cmp byte [quit], 0
    jne ExitGame
    jmp MainLoop

ExitGame:
    mov ax, 0003h                ; Restores normal text screen and cursor.
    int 10h
    mov ax, 4C00h
    int 21h

ResetClock:
    xor ah, ah
    int 1Ah
    mov [lastTickLo], dx
    mov [lastTickHi], cx
    ret

WaitFrame:
    ; Comparing the full tick count also handles low-word and midnight wrap.
    xor ah, ah
    int 1Ah
    cmp dx, [lastTickLo]
    jne .ready
    cmp cx, [lastTickHi]
    jne .ready
    hlt                           ; Interrupts enabled; timer/keyboard wake us.
    jmp WaitFrame
.ready:
    mov [lastTickLo], dx
    mov [lastTickHi], cx
    ret                           ; No catch-up burst after slow host frames.

NewGame:
    mov byte [state], PLAYING
    mov byte [lives], 3
    mov byte [flashFrames], 0
    mov byte [hitThisFrame], 0
    mov word [playerCol], 38
    mov word [score], 0
    mov word [level], 1
    mov word [stepRows], 1
    xor si, si
.spawn:
    call RespawnObstacle
    add si, 2
    cmp si, 6
    jb .spawn
    mov word [obsRow+2], 8
    mov word [obsRow+4], 15
    call SaveActors
    call ClearScreen
    call DrawAll
    call ResetClock
    ret

%include "render.inc"
%include "physics.inc"
%include "input.inc"

align 2
stackSpace: times 512 db 0
stackTop:
