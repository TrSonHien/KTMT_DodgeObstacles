; ES always addresses color text VRAM. Offsets never use MUL.
; CellOffset: AX=row (0..24), DX=column (0..79); returns DI.
; Clobbers BX, DI; preserves AX and DX.
CellOffset:
    mov bx, ax
    shl bx, 1
    mov di, [rowOff+bx]
    mov bx, dx
    shl bx, 1
    add di, bx
    ret

ClearScreen:
    mov ax, 0720h
    xor di, di
    mov cx, 2000
    rep stosw
    ret

; DS:SI = zero-terminated text, ES:DI = destination, AH = attribute.
PrintString:
    lodsb
    test al, al
    jz .done
    stosw
    jmp PrintString
.done:
    ret

; AX = unsigned number, ES:DI = destination. Five right-aligned digits.
PrintNumber:
    mov bx, 10
    mov cx, 5
.convert:
    xor dx, dx
    div bx
    add dl, '0'
    push dx
    loop .convert
    mov cx, 5
    mov bx, 1
.print:
    pop ax
    mov ah, 0Fh
    cmp al, '0'
    jne .significant
    cmp cx, 1
    je .significant
    test bx, bx
    jz .emit
    mov al, ' '
    jmp .emit
.significant:
    xor bx, bx
.emit:
    stosw
    loop .print
    ret

SaveActors:
    mov ax, [playerCol]
    mov [oldPlayerCol], ax
    xor si, si
.copy:
    mov ax, [obsCol+si]
    mov [oldObsCol+si], ax
    mov ax, [obsRow+si]
    mov [oldObsRow+si], ax
    add si, 2
    cmp si, 6
    jb .copy
    ret

EraseActors:
    mov ax, PLAYER_ROW
    mov dx, [oldPlayerCol]
    call CellOffset
    mov ax, 0720h
    stosw
    stosw
    stosw
    xor si, si
.erase:
    mov ax, [oldObsRow+si]
    mov dx, [oldObsCol+si]
    call CellOffset
    mov ax, 0720h
    stosw
    stosw
    add si, 2
    cmp si, 6
    jb .erase
    ret

DrawAll:
    xor si, si
.obstacles:
    mov ax, [obsRow+si]
    mov dx, [obsCol+si]
    call CellOffset
    mov bx, si
    shr bx, 1
    mov ah, [obsColor+bx]
    mov al, 219
    stosw
    stosw
    add si, 2
    cmp si, 6
    jb .obstacles
    mov ax, PLAYER_ROW
    mov dx, [playerCol]
    call CellOffset
    mov ah, 0Ah
    cmp byte [flashFrames], 0
    je .player
    mov ah, 0Ch
.player:
    mov al, 220
    stosw
    mov al, 219
    stosw
    mov al, 220
    stosw
    call DrawHUD
    ret

DrawHUD:
    xor di, di
    mov ah, 0Fh
    mov si, hudScore
    call PrintString
    mov ax, [score]
    call PrintNumber
    mov di, 50
    mov ah, 0Fh
    mov si, hudLevel
    call PrintString
    mov ax, [level]
    call PrintNumber
    mov di, 120
    mov ah, 0Fh
    mov si, hudLives
    call PrintString
    xor bx, bx
.hearts:
    mov ax, 0720h
    cmp bl, [lives]
    jae .heartCell
    mov ax, 0C03h
.heartCell:
    stosw
    inc bx
    cmp bx, 3
    jb .hearts
    ret
