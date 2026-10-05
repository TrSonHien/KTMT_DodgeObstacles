; LCG: state = (25173 * state + 13849) mod 65536.
; Shift/add computes the constant product without MUL (8086-compatible).
; Returns AX; clobbers BX, CX, DX.
Random:
    mov bx, [rngState]
    mov dx, 25173
    xor ax, ax
    mov cx, 16
.bit:
    shr dx, 1
    jnc .skip
    add ax, bx
.skip:
    shl bx, 1
    loop .bit
    add ax, 13849
    mov [rngState], ax
    ret

; SI = obstacle byte index (0, 2, 4), preserved.
RespawnObstacle:
    call Random
    xor dx, dx
    mov bx, 79
    div bx
    mov [obsCol+si], dx           ; Left edge 0..78; right edge <=79.
    mov word [obsRow+si], 1      ; Row 0 is reserved for the HUD.
    ret

; Swept AABB: horizontal inclusive intervals [obsCol,obsCol+1] and
; [playerCol,playerCol+2]; vertical interval [oldRow,newRow] crosses row 23.
; SI = obstacle index, AX = proposed row. Returns CF=1 for collision.
CheckCollision:
    cmp word [obsRow+si], PLAYER_ROW
    ja .miss
    cmp ax, PLAYER_ROW
    jb .miss
    mov bx, [obsCol+si]
    mov dx, [playerCol]
    add dx, 2
    cmp bx, dx
    ja .miss
    inc bx
    cmp bx, [playerCol]
    jb .miss
    stc
    ret
.miss:
    clc
    ret

UpdatePhysics:
    cmp byte [flashFrames], 0
    je .begin
    dec byte [flashFrames]
.begin:
    mov byte [hitThisFrame], 0
    xor si, si
.obstacle:
    mov ax, [obsRow+si]
    add ax, [stepRows]
    call CheckCollision
    jc .hit
    cmp ax, 25
    jae .dodged
    mov [obsRow+si], ax
    jmp .next
.hit:
    cmp byte [hitThisFrame], 0
    jne .respawn
    mov byte [hitThisFrame], 1
    mov byte [flashFrames], 2
    dec byte [lives]
    jnz .respawn
    mov byte [state], GAME_OVER
    jmp .respawn
.dodged:
    cmp word [score], 65535
    je .respawn                  ; Saturate instead of wrapping score.
    inc word [score]
.respawn:
    call RespawnObstacle
.next:
    add si, 2
    cmp si, 6
    jb .obstacle
    ; Level 1 at 0 points, level 2 at 50, etc. All actors in this frame
    ; use the same speed; new speed takes effect on the next frame.
    mov ax, [score]
    xor dx, dx
    mov bx, 50
    div bx
    inc ax
    mov [level], ax
    cmp ax, 23
    jbe .speed
    mov ax, 23                   ; Max useful stride through the playfield.
.speed:
    mov [stepRows], ax
    ret
