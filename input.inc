; Poll at most 16 buffered keys per frame so input cannot starve physics.
; BIOS scan codes handle arrows; ASCII handles A/D/R/ESC.
ScanKeyboard:
    mov bp, 16
.poll:
    mov ah, 01h
    int 16h
    jz .done
    xor ah, ah
    int 16h
    cmp al, 27
    je .quit
    cmp byte [state], TITLE
    je .title
    cmp byte [state], GAME_OVER
    je .over
    cmp ah, 4Bh
    je .left
    cmp ah, 4Dh
    je .right
    or al, 20h
    cmp al, 'a'
    je .left
    cmp al, 'd'
    je .right
    jmp .next
.left:
    cmp word [playerCol], 0
    je .next
    dec word [playerCol]
    jmp .next
.right:
    cmp word [playerCol], 77
    jae .next
    inc word [playerCol]
    jmp .next
.title:
    cmp al, ' '
    je .start
    cmp al, 13
    je .start
    jmp .next
.over:
    or al, 20h
    cmp al, 'r'
    jne .next
.start:
    call NewGame
    ret
.quit:
    mov byte [quit], 1
    ret
.next:
    dec bp
    jnz .poll
.done:
    ret

ShowTitle:
    call ClearScreen
    mov di, 10*160+27*2
    mov si, titleText
    mov ah, 0Ah
    call PrintString
    mov di, 12*160+26*2
    mov si, startText
    mov ah, 0Fh
    call PrintString
    mov di, 15*160+18*2
    mov si, controlText
    mov ah, 07h
    call PrintString
    mov di, 17*160+13*2
    mov si, rulesText
    call PrintString
    mov di, 19*160+32*2
    mov si, exitText
    call PrintString
    ret

ShowGameOver:
    call ClearScreen
    mov di, 10*160+35*2
    mov si, overText
    mov ah, 0Ch
    call PrintString
    mov di, 12*160+31*2
    mov si, finalText
    mov ah, 0Fh
    call PrintString
    mov ax, [score]
    call PrintNumber
    mov di, 15*160+28*2
    mov si, restartText
    mov ah, 0Fh
    call PrintString
    ret
