; FAT32 FDD
    ORG 0x7c00
pbr:
        JMP     entry
        NOP
        DB      "HELLOIPL"
        DW      512
        DB      1
        DW      1
        DB      2
        DW      224
        DW      2880
        DB      0xf0
        DW      9
        DW      18
        DW      2
        DD      0
        DD      2880
        DB      0,0,0x29
        DD      0xffffffff
        DB      "HELLO-OS   "
        DB      "FAT12   "
        TIMES   18  DB  0

; MAIN
; レジスタ初期化処理
entry:
        MOV     AX,0
        MOV     SS,AX
        MOV     SP,0x7c00
        MOV     DS,AX

; ディスクの読み込み

        MOV     AX,0x0820   ; OS 読み込み先番地を設定
        MOV     ES,AX
        MOV     BX,0        ; アドレスはEX * 16 + BX
        MOV     CH,0        ; シリンダ  0
        MOV     DH,0        ; ヘッド    0
        MOV     CL,2        ; セクタ    2

readloop:
        MOV     SI,0        ; エラーカウンタ(レジスタ)
retry:
        MOV     AH,0x02     ; ディスク読み出し
        MOV     AL,1        ; 1セクタ
        MOV     DL,0x00     ; Aドライブ
        INT     0x13        ; ディスクBIOSコール
        JNC     fin         ; エラーがなければFINへ
        ADD     SI,1
        CMP     SI,5
        JAE     error       ; エラーカウント >= 5 でエラー出力
        MOV     AH,0x00     ; ドライブのリセット
        MOV     DL,0x00     ; A ドライブ
        INT     0x13        ; ディスクBIOSコール
        JMP     retry

; 無限ループ
fin:
        HLT
        JMP     fin

;  error message
error:
        MOV     SI,msg
putloop:
        MOV     AL,[SI]
        ADD     SI,1
        CMP     AL,0
        JE      fin
        MOV     AH,0x0e
        MOV     BX,15
        INT     0x10
        JMP     putloop

msg:
        DB      0x0a, 0x0a
        DB      "LOAD ERROR"
        DB      0x0a
        DB      0

; ブートフラグ設定
        ;TIMES   0x7dfe-($-$$)  DB  0
        TIMES   0x1fe-($-$$)  DB  0
        DB      0x55, 0xaa

